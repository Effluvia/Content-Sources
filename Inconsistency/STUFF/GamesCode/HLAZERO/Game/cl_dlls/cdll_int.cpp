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
//  cdll_int.c
//
// this implementation handles the linking of the engine to the DLL
//

// ??
#include <string.h>


#include "hud.h"
#include "cl_util.h"
#include "netadr.h"
#include "vgui_schememanager.h"

extern "C"
{
#include "pm_shared.h"
}

#include "hud_servers.h"
#include "vgui_int.h"
#include "interface.h"

//MODD - show me what ya got.
#include "utils/vgui/include/VGUI_Panel.h"

#include "hl/hl_weapons.h"

#include "cvar_custom_info.h"
#include "cvar_custom_list.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"


EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(drawHUD);



// NOTICE - the as-is descriptions of the init methods are kinda out of touch with how they really work it seems.

// Initialize - very first event, called on booting the game up only (so much as the menu shown).  Never twice, I think.
// HUD_Init - called shortly after Initialize.  Also likely never twice.
// HUD_VidInit - Unsure if it's actually called on changing display modes (game just seems to crash but remember settings
// between runs anyway),   but this method reall runs anytime the player goes ingame.  New game, loaded game, or created/
// joined server.




extern BOOL g_cl_HUD_Frame_ran;
extern BOOL g_cl_HUD_UpdateClientData_ran;
extern BOOL g_HUD_Redraw_ran;
extern int g_currentanim;
extern int g_cl_frameCount;
extern BOOL resetNormalRefDefVars;
extern float sp_ClientPreviousTime;

extern double ary_g_prevTime[1024];
extern float ary_g_prevFrame[1024];
extern double ary_g_LastEventCheck[1024];
extern double ary_g_LastEventCheckEXACT[1024];

extern BOOL g_cl_egonEffectCreatedYet;
extern int g_framesSinceRestore;
extern BOOL g_cl_queueSharedPrecache;
extern BOOL g_cl_firstSendoffSinceMapLoad;

extern CGameStudioModelRenderer g_StudioRenderer;
extern float g_cl_mapStartTime;




cl_enginefunc_t gEngfuncs;
CHud gHUD;

// Someday...
//vgui::Panel* gViewPort = NULL;
TeamFortressViewport* gViewPort = NULL;



BOOL hasAutoMus = FALSE;
float globalPSEUDO_drawHUDMem = -1;

// Var from the mp5, used in the 'ancient SDK'.
// (renamed from m_flNextAnimTime; no longer in mp5.cpp)
float g_cl_mp5_NextAnimTime;



void InitInput (void);
void EV_HookEvents( void );
void IN_Commands( void );

/*
========================== 
    Initialize

Called when the DLL is first loaded.
==========================
*/
extern "C"
{
int	DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion );
int	DLLEXPORT HUD_VidInit( void );
void DLLEXPORT HUD_Init( void );
int	DLLEXPORT HUD_Redraw( float flTime, int intermission );
int	DLLEXPORT HUD_UpdateClientData( client_data_t *cdata, float flTime );
void DLLEXPORT HUD_Reset ( void );
void DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server );
void DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove );
char DLLEXPORT HUD_PlayerMoveTexture( char *name );
int	DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
int	DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs );
void DLLEXPORT HUD_Frame( double time );
void DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking);
void DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf );
}

/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	switch ( hullnumber )
	{
	case 0:				// Normal player
		mins = Vector(-16, -16, -36);
		maxs = Vector(16, 16, 36);
		iret = 1;
		break;
	case 1:				// Crouched player
		mins = Vector(-16, -16, -18 );
		maxs = Vector(16, 16, 18 );
		iret = 1;
		break;
	case 2:				// Point based hull
		mins = Vector( 0, 0, 0 );
		maxs = Vector( 0, 0, 0 );
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
HUD_ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

void DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove )
{
	PM_Init( ppmove );
}

char DLLEXPORT HUD_PlayerMoveTexture( char *name )
{
	return PM_FindTextureType( name );
}

void DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server )
{
	PM_Move( ppmove, server );
}

int DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	gEngfuncs = *pEnginefuncs;

	if (iVersion != CLDLL_INTERFACE_VERSION)
		return 0;

	memcpy(&gEngfuncs, pEnginefuncs, sizeof(cl_enginefunc_t));

	EV_HookEvents();

	return 1;
}


//#include "GameStudioModelRenderer.h"
//#include "r_studioint.h"
//extern engine_studio_api_t IEngineStudio;

/*
==========================
	HUD_VidInit

Called when the game initializes
and whenever the vid_mode is changed
so the HUD can reinitialize itself.
==========================
*/
int DLLEXPORT HUD_VidInit( void )
{
	// safety
	g_cl_HUD_Frame_ran = FALSE;
	g_cl_HUD_UpdateClientData_ran = FALSE;
	g_HUD_Redraw_ran = FALSE;

	// clientside only variable.
	reloadBlocker = FALSE;

	// TEST.
	blockUntilModelChange = FALSE;
	oldModel = -1;
	queuedBlockedModelAnim = -1;

	forgetBlockUntilModelChangeTime = -1;
	resistTime = -1;

	seqPlayDelay = -1;
	seqPlay = 0;
	queuecall_lastinv = FALSE;
	g_currentanim = -1;

	sp_ClientPreviousTime = -1;
	g_cl_frameCount = 0;

	// safety?
	g_cl_egonEffectCreatedYet = FALSE;

	// NOTE - can't really differentiate between a whole new map-load, saved game restore,
	// or taking a transition but it isn't too important.
	g_framesSinceRestore = 0;

	// is this safe?
	// No.
	//g_cl_mapStartTime = g_StudioRenderer.m_clTime;
	// Wait uh.  What's the difference between g_engfuncs.pfnTime() and gEngfuncs.GetClientTime() ?
	// Most stuff clientside looks to use the latter.

	// OOooookay, both of these don't get the updated time since loading a game, fantastic

	// Nowhere uses pfnTime().  It's NULL clientside and never even given a reference to replace that
	// in hl_weapons.cpp (HUD_InitClientWeapons) like a few other engine methods for g_engfuncs.
	//float test1 = g_engfuncs.pfnTime();
	// And, GetClientTime() always reports 0 at this point, even if the time of the current game doesn't
	// begin at 0 (loaded game or coming from a transition to a previously run map with a remembered time).
	// Seems the best idea is to set a queued var for HUD_InitClientWeapons to see and set the map's first
	// time to GetClientTime() then.  For now, force the time var to -1 so that rendering that sees it extra
	// early at least knows 'oh, it's been really soon since loading the map' which is the point.
	// In fact, going to let the queue-first-frame piggyback off g_cl_queueSharedPrecache, it is the exact
	// same time/place it would've been checked anyway
	//float test2 = gEngfuncs.GetClientTime();

	/*
	// Nope, no difference
	int dummy1;
	double test3;
	double dummy2;
	IEngineStudio.GetTimes(&dummy1, &test3, &dummy2);
	*/

	g_cl_mapStartTime = -1;	
	g_StudioRenderer.m_nCachedFrameCount = -1;

	g_cl_mp5_NextAnimTime = -1;



	//extern CGameStudioModelRenderer g_StudioRenderer;
	//MODDD - from studioModelRenderer.cpp, initialize to the current game global time.
	// ...nope, always still 0.    ugh.
	for (int i = 0; i < MAX_ENTITY_CONSTANT_THAT_PLEASES_THE_DARK_GODS; i++) {
		// NOTE - gpGlobals->time is unavailable, has to be dummied and routinely set to gEngfuncs.GetClientTime().
		// So get it from the source then.
		ary_g_prevTime[i] = 0; //g_StudioRenderer.m_clTime; //gEngfuncs.GetClientTime();
		ary_g_prevFrame[i] = 0;
		ary_g_LastEventCheck[i] = 0; //g_StudioRenderer.m_clTime; //gEngfuncs.GetClientTime();
		ary_g_LastEventCheckEXACT[i] = 0;
	}


	//MODDD
	resetNormalRefDefVars = TRUE;  //does this work

	// should be fine then.
	// NNNNNNNNNNnnnnnnnnnnnoooooooooooppppppppppppppppeeeeeeeeeee-PUH.
	// It does appear that precaches done this early will always find a model of '0', not do anything.
	// Let this be queued to run very soon when HUD_InitClientWeapons in hl_weapons.cpp (yes it gets called again)
	// sees this.
	//PrecacheShared();
	g_cl_queueSharedPrecache = TRUE;

	g_cl_firstSendoffSinceMapLoad = TRUE;


	gHUD.VidInit();

	VGui_Startup();

	return 1;
}

/*
==========================
	HUD_Init

Called whenever the client connects
to a server.  Reinitializes all 
the hud variables.
==========================
*/
EASY_CVAR_EXTERN_MASS
EASY_CVAR_DECLARE_HASH_ARRAY

void DLLEXPORT HUD_Init( void )
{
	// Is this a good idea?
	// No, gpGlobals->time has not even been establiched.  Look in hl_weapons.cpp.
	//if (sp_ClientPreviousTime == -1){sp_ClientPreviousTime = gpGlobals->time;}
	
	
	EASY_CVAR_HASH_MASS


	InitShared();
	
	

	determineHiddenMemPath();
	
	hasAutoMus = checkSubFileExistence("media/AutoMus.mp3");

	loadHiddenCVars();

	// nope, this call will happen before starting any game (game startup to the menu), which isn't the place to call lateCVarInit.
	//lateCVarInit();

	//MODDD - see if we need to make any special printouts in the future.
	//testForHelpFile();

	InitInput();
	gHUD.Init();
	Scheme_Init();
}


/*
==========================
	HUD_Redraw

called every screen frame to
redraw the HUD.
===========================
*/


int DLLEXPORT HUD_Redraw( float time, int intermission )
{
	g_HUD_Redraw_ran = TRUE;

	if(globalPSEUDO_drawHUDMem != EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(drawHUD)){
		gHUD.m_Ammo.updateCrosshair();
		globalPSEUDO_drawHUDMem = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(drawHUD);
	}


	//MODDD - the ultamite blocker!
	if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(drawHUD) != 1 && EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(drawHUD) != 2){
		return 1;
	}


	//MODDD - for things that can not access time ordinarily.
	gHUD.recentTime = time;

	gHUD.Redraw( time, intermission );

	return 1;
}


/*
==========================
	HUD_UpdateClientData

called every time shared client
dll/engine data gets changed,
and gives the cdll a chance
to modify the data.

returns 1 if anything has been changed, 0 otherwise.
==========================
*/

int DLLEXPORT HUD_UpdateClientData(client_data_t *pcldata, float flTime )
{
	IN_Commands();
	g_cl_HUD_UpdateClientData_ran = TRUE;

	return gHUD.UpdateClientData(pcldata, flTime );
}

/*
==========================
	HUD_Reset

Called at start and end of demos to restore to "non"HUD state.
==========================
*/

void DLLEXPORT HUD_Reset( void )
{
	gHUD.VidInit();
}


/*
==========================
HUD_Frame

Called by engine every frame that client .dll is loaded
==========================
*/

void DLLEXPORT HUD_Frame( double time )
{
	ServersThink( time );
	g_cl_HUD_Frame_ran = TRUE;

	GetClientVoiceMgr()->Frame(time);
}


/*
==========================
HUD_VoiceStatus

Called when a player starts or stops talking.
==========================
*/

void DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	GetClientVoiceMgr()->UpdateSpeakerStatus(entindex, bTalking);
}

/*
==========================
HUD_DirectorEvent

Called when a director event message was received
==========================
*/

void DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf )
{
	 gHUD.m_Spectator.DirectorMessage( iSize, pbuf );
}


