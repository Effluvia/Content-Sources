

#ifndef CDLL_INT_GOLDSRC_H
#define CDLL_INT_GOLDSRC_H

#include "vector.h"


// goldsrc often referred to vectors as vec3_t, same idea as here in source2003,
// I think
#ifndef __cplusplus
typedef vec_t vec3_t[3];	// x,y,z
#else
#define vec3_t Vector
#endif


// this was also missing
typedef struct rect_s
{
	int			left, right, top, bottom;
} wrect_t;



// ???
#define CLDLL_INTERFACE_VERSION 7




//MODDD - moved from cl_dll.h and hud_iface.h.
// What?  Why was this even part of the game files like this if it's required by the engine?
typedef int (*pfnUserMsgHook)(const char* pszName, int iSize, void* pbuf);


// this file is included by both the engine and the client-dll,
// so make sure engine declarations aren't done twice

typedef int GOLDSRC_SpriteHandle_t;	// handle to a graphic

#define SCRINFO_SCREENFLASH 1
#define SCRINFO_STRETCHED	2

typedef struct SCREENINFO_s
{
	int	iSize;
	int	iWidth;
	int	iHeight;
	int	iFlags;
	int	iCharHeight;
	short	charWidths[256];
} SCREENINFO;


//MODDD - NOTE.
// Not to be confused with... common/entity_state.h's clientdata_s/clientdata_t.
// WHO COMES UP WITH THESE NAMES GOD
typedef struct GOLDSRC_client_data_s
{
	// fields that cannot be modified  (ie. have no effect if changed)
	vec3_t origin;

	// fields that can be changed by the cldll
	vec3_t viewangles;
	int	iWeaponBits;
	float fov;	// field of view
} GOLDSRC_client_data_t;

typedef struct GOLDSRC_client_sprite_s
{
	char szName[64];
	char szSprite[64];
	int hspr;
	int iRes;
	wrect_t rc;
} GOLDSRC_client_sprite_t;

typedef struct GOLDSRC_client_textmessage_s
{
	int	effect;
	byte r1, g1, b1, a1;		// 2 colors for effects
	byte r2, g2, b2, a2;
	float x;
	float y;
	float fadein;
	float fadeout;
	float holdtime;
	float fxtime;
	const char *pName;
	const char *pMessage;
} GOLDSRC_client_textmessage_t;

typedef struct GOLDSRC_hud_player_info_s
{
	char *name;
	short ping;
	byte thisplayer;  // TRUE if this is the calling player

  // stuff that's unused at the moment,  but should be done
	byte spectator;
	byte packetloss;

	char *model;
	short topcolor;
	short bottomcolor;

} GOLDSRC_hud_player_info_t;







typedef struct cl_enginefuncs_s
{
	// sprite handlers
	GOLDSRC_SpriteHandle_t				( *pfnSPR_Load )			( const char *szPicName );
	int						( *pfnSPR_Frames )			( GOLDSRC_SpriteHandle_t hPic );
	int						( *pfnSPR_Height )			( GOLDSRC_SpriteHandle_t hPic, int frame );
	int						( *pfnSPR_Width )			( GOLDSRC_SpriteHandle_t hPic, int frame );
	void					( *pfnSPR_Set )				( GOLDSRC_SpriteHandle_t hPic, int r, int g, int b );
	void					( *pfnSPR_Draw )			( int frame, int x, int y, const wrect_t *prc );
	void					( *pfnSPR_DrawHoles )		( int frame, int x, int y, const wrect_t *prc );
	void					( *pfnSPR_DrawAdditive )	( int frame, int x, int y, const wrect_t *prc );
	void					( *pfnSPR_EnableScissor )	( int x, int y, int width, int height );
	void					( *pfnSPR_DisableScissor )	( void );
	GOLDSRC_client_sprite_t				*( *pfnSPR_GetList )			( char *psz, int *piCount );

	// screen handlers
	void					( *pfnFillRGBA )			( int x, int y, int width, int height, int r, int g, int b, int a );
	int						( *pfnGetScreenInfo ) 		( SCREENINFO *pscrinfo );
	void					( *pfnSetCrosshair )		( GOLDSRC_SpriteHandle_t hspr, wrect_t rc, int r, int g, int b );

	// cvar handlers
	struct cvar_s				*( *pfnRegisterVariable )	( char *szName, char *szValue, int flags );
	float					( *pfnGetCvarFloat )		( char *szName );
	char*						( *pfnGetCvarString )		( char *szName );

	// command handlers
	int						( *pfnAddCommand )			( char *cmd_name, void (*function)(void) );
	int						( *pfnHookUserMsg )			( char *szMsgName, pfnUserMsgHook pfn );
	int						( *pfnServerCmd )			( char *szCmdString );
	int						( *pfnClientCmd )			( char *szCmdString );

	void					( *pfnGetPlayerInfo )		( int ent_num, GOLDSRC_hud_player_info_t *pinfo );

	// sound handlers
	void					( *pfnPlaySoundByName )		( char *szSound, float volume );
	void					( *pfnPlaySoundByIndex )	( int iSound, float volume );

	// vector helpers
	void					( *pfnAngleVectors )		( const float * vecAngles, float * forward, float * right, float * up );

	// text message system
	GOLDSRC_client_textmessage_t		*( *pfnTextMessageGet )		( const char *pName );
	int						( *pfnDrawCharacter )		( int x, int y, int number, int r, int g, int b );

	//MODDD - mentions of "string" changed to "par_string"!  Can't really hurt anything,
	// as the connection to DLL's only cares about parameter order and type.
	// Paranoia about the C++ compiler mixing up this 'string' name with the built-in 'string' type.
	int						( *pfnDrawConsoleString )	( int x, int y, char *par_string );
	void					( *pfnDrawSetTextColor )	( float r, float g, float b );
	void					( *pfnDrawConsoleStringLen )(  const char *par_string, int *length, int *height );

	void					( *pfnConsolePrint )		( const char *par_string );
	void					( *pfnCenterPrint )			( const char *par_string );


	// Added for user input processing
	int						( *GetWindowCenterX )		( void );
	int						( *GetWindowCenterY )		( void );
	void					( *GetViewAngles )			( float * );
	void					( *SetViewAngles )			( float * );
	int						( *GetMaxClients )			( void );
	void					( *Cvar_SetValue )			( char *cvar, float value );

	int       					(*Cmd_Argc)					(void);	
	char						*( *Cmd_Argv )				( int arg );
	void					( *Con_Printf )				( char *fmt, ... );
	void					( *Con_DPrintf )			( char *fmt, ... );
	void					( *Con_NPrintf )			( int pos, char *fmt, ... );
	void					( *Con_NXPrintf )			( struct con_nprint_s *info, char *fmt, ... );

	const char					*( *PhysInfo_ValueForKey )	( const char *key );
	const char					*( *ServerInfo_ValueForKey )( const char *key );
	float					( *GetClientMaxspeed )		( void );
	int						( *CheckParm )				( char *parm, char **ppnext );
	void					( *Key_Event )				( int key, int down );
	void					( *GetMousePosition )		( int *mx, int *my );
	int						( *IsNoClipping )			( void );

	struct cl_entity_s			*( *GetLocalPlayer )		( void );
	struct cl_entity_s			*( *GetViewModel )			( void );
	struct cl_entity_s			*( *GetEntityByIndex )		( int idx );

	float					( *GetClientTime )			( void );
	void					( *V_CalcShake )			( void );
	void					( *V_ApplyShake )			( float *origin, float *angles, float factor );

	int						( *PM_PointContents )		( float *point, int *truecontents );
	int						( *PM_WaterEntity )			( float *p );
	struct pmtrace_s			*( *PM_TraceLine )			( float *start, float *end, int flags, int usehull, int ignore_pe );

	struct model_s				*( *CL_LoadModel )			( const char *modelname, int *index );
	int						( *CL_CreateVisibleEntity )	( int type, struct cl_entity_s *ent );

	const struct model_s *		( *GetSpritePointer )		( GOLDSRC_SpriteHandle_t hSprite );
	void					( *pfnPlaySoundByNameAtLocation )	( char *szSound, float volume, float *origin );

	unsigned short				( *pfnPrecacheEvent )		( int type, const char* psz );
	void					( *pfnPlaybackEvent )		( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
	void					( *pfnWeaponAnim )			( int iAnim, int body );
	float					( *pfnRandomFloat )			( float flLow, float flHigh );
	long						( *pfnRandomLong )			( long lLow, long lHigh );
	void					( *pfnHookEvent )			( char *name, void ( *pfnEvent )( struct event_args_s *args ) );
	int						(*Con_IsVisible)			();
	const char					*( *pfnGetGameDirectory )	( void );
	struct cvar_s				*( *pfnGetCvarPointer )		( const char *szName );
	const char					*( *Key_LookupBinding )		( const char *pBinding );
	const char					*( *pfnGetLevelName )		( void );
	void					( *pfnGetScreenFade )		( struct screenfade_s *fade );
	void					( *pfnSetScreenFade )		( struct screenfade_s *fade );
	void                        *( *VGui_GetPanel )         ( );
	void                         ( *VGui_ViewportPaintBackground ) (int extents[4]);

	byte*						(*COM_LoadFile)				( char *path, int usehunk, int *pLength );
	char*						(*COM_ParseFile)			( char *data, char *token );
	void					(*COM_FreeFile)				( void *buffer );

	struct triangleapi_s		*pTriAPI;
	struct efx_api_s			*pEfxAPI;
	struct event_api_s			*pEventAPI;
	struct demo_api_s			*pDemoAPI;
	struct net_api_s			*pNetAPI;
	struct IVoiceTweak_s		*pVoiceTweak;

	// returns 1 if the client is a spectator only (connected to a proxy), 0 otherwise or 2 if in dev_overview mode
	int						( *IsSpectateOnly ) ( void );
	struct model_s				*( *LoadMapSprite )			( const char *filename );

	// file search functions
	void					( *COM_AddAppDirectoryToSearchPath ) ( const char *pszBaseDir, const char *appName );
	int						( *COM_ExpandFilename)				 ( const char *fileName, char *nameOutBuffer, int nameOutBufferSize );

	// User info
	// playerNum is in the range (1, MaxClients)
	// returns NULL if player doesn't exit
	// returns "" if no value is set
	const char					*( *PlayerInfo_ValueForKey )( int playerNum, const char *key );
	void					( *PlayerInfo_SetValueForKey )( const char *key, const char *value );

	// Gets a unique ID for the specified player. This is the same even if you see the player on a different server.
	// iPlayer is an entity index, so client 0 would use iPlayer=1.
	// Returns false if there is no player on the server in the specified slot.
	qboolean					(*GetPlayerUniqueID)(int iPlayer, char playerID[16]);

	// TrackerID access
	int						(*GetTrackerIDForPlayer)(int playerSlot);
	int						(*GetPlayerForTrackerID)(int trackerID);

	// Same as pfnServerCmd, but the message goes in the unreliable stream so it can't clog the net stream
	// (but it might not get there).
	int						( *pfnServerCmdUnreliable )( char *szCmdString );

	void					( *pfnGetMousePos )( struct tagPOINT *ppt );
	void					( *pfnSetMousePos )( int x, int y );
	void					( *pfnSetMouseEnable )( qboolean fEnable );
} cl_enginefunc_t;



#endif //CDLL_INT_GOLDSRC_H
