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
// hud.cpp
//
// implementation of CHud class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "hud_servers.h"
#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#include "demo.h"
#include "demo_api.h"
#include "vgui_scorepanel.h"
#include "util_version.h"
#include "player.h"

#include "hl/hl_weapons.h"


#include "cvar_custom_info.h"
#include "cvar_custom_list.h"

//MODDD - externs
EASY_CVAR_EXTERN(hud_version)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(preE3UsesFailColors)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(E3UsesFailColors)
EASY_CVAR_EXTERN(hud_moveselect_sound)
EASY_CVAR_EXTERN(hud_drawsidebarmode)
EASY_CVAR_EXTERN(cl_interp_entity)
EASY_CVAR_EXTERN(hud_brokentrans)


//MODDD - because I am diabolical
extern CBasePlayer localPlayer;


//NEWSDK: these cvars are Absent from the new SDK.  We'll keep them though.
/*
cvar_t *cl_viewrollangle;
cvar_t *cl_viewrollspeed;
*/


EASY_CVAR_EXTERN_MASS





class CHLVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 255;

		if( entindex >= 0 && entindex < sizeof(g_PlayerExtraInfo)/sizeof(g_PlayerExtraInfo[0]) )
		{
			int iTeam = g_PlayerExtraInfo[entindex].teamnumber;

			if ( iTeam < 0 )
			{
				iTeam = 0;
			}

			iTeam = iTeam % iNumberOfTeamColors;

			color[0] = iTeamColors[iTeam][0];
			color[1] = iTeamColors[iTeam][1];
			color[2] = iTeamColors[iTeam][2];
		}
	}

	virtual void UpdateCursorState()
	{
		gViewPort->UpdateCursorState();
	}

	virtual int GetAckIconHeight()
	{
		return ScreenHeight - gHUD.m_iFontHeight*3 - 6;
	}

	virtual bool			CanShowSpeakerLabels()
	{
		if( gViewPort && gViewPort->m_pScoreBoard )
			return !gViewPort->m_pScoreBoard->isVisible();
		else
			return false;
	}
};
static CHLVoiceStatusHelper g_VoiceStatusHelper;


//MDODD - constructor added.
CHud::CHud() : m_iSpriteCount(0), m_pHudList(NULL), m_fPlayerDead(FALSE), recentTime(-1), m_prc_brokentransparency(NULL), frozenMem(FALSE), recentDamageBitmask(-1), PESUDO_cameraModeMem(-1)  {
	//start off assuming the player is NOT dead.

	alphaCrossHairIndex = -1;
	
	crosshairMem = -1;
	allowAlphaCrosshairWithoutGunsMem = -1;

	numMirrors = 0;  //MODDDMIRROR.  Here too, why not.
}



//MODDD - NOTE.   ... What even are these?? double-underscores in front?  what?  why?
// Oh, I see why now.  These are stand-ins for the DECLARE_MESSAGE calls.
// Because these don't belong to any particular gHUD instance, like m_Ammo.
// Likely wasn't deemed worth the effort to even make some instances like m_Logo, so
// the deves skipped needing an instance like this (m_Logo does not exist like that
// call wants).

// Note that other __MsgFunc and __CmdFunc calls throughout this file cannot be 
// replaced by the same DECLARE_MESSAGE_HUD macro.
// Could use a new macro that generate calls to "gViewPort->MsgFunc_##X" instead.

// Anyway, below has been replaced by these macro calls.
// macro calls moved to custom_message.cpp.

/*
//DECLARE_MESSAGE(m_Logo, Logo)
int __MsgFunc_Logo(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Logo(pszName, iSize, pbuf );
}
*/
//...  (rest removed)

//MODDD - TF-specific messages removed.
// Several others can likely be safely removed but playing it safe for now

void __CmdFunc_ToggleServerBrowser( void )
{
	if ( gViewPort )
	{
		gViewPort->ToggleServerBrowser();
	}
}


int __MsgFunc_TeamNames(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamNames( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_VGUIMenu(const char* pszName, int iSize, void* pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_VGUIMenu(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_MOTD( pszName, iSize, pbuf );
	return 0;
}

 
int __MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ServerName( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ScoreInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamScore( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamInfo( pszName, iSize, pbuf );
	return 0;
}


//MODDD - don't even know if it's possible for these to be called, but keeping them to be safe.
int __MsgFunc_Spectator(const char* pszName, int iSize, void* pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Spectator(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_AllowSpec(const char* pszName, int iSize, void* pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_AllowSpec(pszName, iSize, pbuf);
	return 0;
}




/*
//MODDD - temp method
void tempGetHudMainRef2(CHudBase* tempRef){
	//tempRef->topHealth = m_Health.m_iHealth;
}
*/

void CHud::drawPartialFromBottom(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b){
	drawPartialFromBottom(arg_sprite, arg_rect, arg_portion, x, y, r, g, b, 0);
}
void CHud::drawPartialFromBottom(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b, const int& canDrawBrokenTrans){
	
	wrect_t rc = *arg_rect;
	int heightDiff = arg_rect->bottom - arg_rect->top;

	rc.top  += heightDiff * (-arg_portion + 1); //((float)(100-(min(100,m_iBat))) * 0.01);	// battery can go from 0 to 100 so * 0.01 goes from 0 to 1
	
	if (rc.bottom > rc.top)
	{

		//drawAdditiveFilter( 0, x, y - iOffset + (rc.top - m_prc2->top), &rc);
		drawAdditiveFilter( arg_sprite, r, g, b, 0, x, y + (rc.top - arg_rect->top), &rc, canDrawBrokenTrans);
		
	}

}

void CHud::drawPartialFromLeft(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b){
	drawPartialFromLeft(arg_sprite, arg_rect, arg_portion, x, y, r, g, b, 0);
}

void CHud::drawPartialFromLeft(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b, const int& canDrawBrokenTrans){
	
	wrect_t rc = *arg_rect;
	int widthDiff = arg_rect->right - arg_rect->left;

	//rc.left  += widthDiff * (-arg_portion + 1); //((float)(100-(min(100,m_iBat))) * 0.01);	// battery can go from 0 to 100 so * 0.01 goes from 0 to 1
	rc.right += (int)ceil(widthDiff * (arg_portion + -1));

	if (rc.right > rc.left){
		//drawAdditiveFilter( 0, x, y - iOffset + (rc.top - m_prc2->top), &rc);
		drawAdditiveFilter( arg_sprite, r, g, b, 0, x, y, &rc, canDrawBrokenTrans);
	}
}


void CHud::drawPartialFromRight(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b){
	drawPartialFromRight(arg_sprite, arg_rect, arg_portion, x, y, r, g, b, 0);
}
void CHud::drawPartialFromRight(const SpriteHandle_t & arg_sprite, const wrect_t* arg_rect, const float arg_portion, const int & x, const int & y,  int & r, int & g, int & b, const int& canDrawBrokenTrans){
	wrect_t rc = *arg_rect;
	int draw_x = x;
	int widthDiff = arg_rect->right - arg_rect->left;

	//rc.right += widthDiff * (arg_portion + -1);

	//do this instead?
	rc.left += widthDiff * (-arg_portion + 1);
	draw_x += widthDiff * (-arg_portion + 1);

	if (rc.right > rc.left){
		drawAdditiveFilter( arg_sprite, r, g, b, 0, draw_x, y, &rc, canDrawBrokenTrans);
	}
	
	//alternate?
	/*
	float m_flBat = arg_portion;
	int m_iWidth = widthDiff;
	
    int iOffset = (int) floor(m_iWidth * (1.0 - m_flBat));
    if (iOffset < m_iWidth)
    {
        rc.left += iOffset;

        SPR_Set(arg_sprite, r, g, b );
        SPR_DrawAdditive( 0, x + iOffset, y, &rc);
    }
	*/
}


int CHud::canDrawSidebar(void){
	
	//NOTICE - override this if the screenheight is below some number of pixels, hud_drawsidebarmode is non-zero, and hud_version is 0, 1, or 3.
	// Or forget the screen height check, whichever.
	float vers = EASY_CVAR_GET(hud_version);
	
	if(
	    //EASY_CVAR_GET(hud_drawsidebarmode) != 0 &&
		vers == 0 || vers == 1 || (vers == 3 && ScreenHeight < 600)
		//EASY_CVAR_GET(hud_weaponSelectHidesLower) == 2 &&
	){
		// act as though hud_drawsidebarmode were 0 then.
		return !canDrawBottomStats;
	}else{
		// normal checks
		if (
			 (EASY_CVAR_GET(hud_drawsidebarmode) == 2) ||
			   (
				(!canDrawBottomStats && EASY_CVAR_GET(hud_drawsidebarmode) == 0) ||
				 (canDrawBottomStats && EASY_CVAR_GET(hud_drawsidebarmode) == 1)
			   )
		){
			return TRUE;
		}else{
			return FALSE;
		}
	}
}


//inline
//maybe don't inline methods expected to call other methods.  Or if they're implemented in a file/place
//external to the class's {   } body.  I don't remember what about this caused the crash exactly.
void CHud::drawAdditiveFilter(int sprite, const int& r, const int& g, const int& b, int huh, int x, int y, wrect_t* rect){
	drawAdditiveFilter(sprite, r, g, b, huh, x, y, rect, 0);
}

//MODDD - new method.  Does the same thing as "drawAdditive", except this factors in "hud_brokentrans".
//If it is true (1), draw some faded gray on top of the image's rect.
void CHud::drawAdditiveFilter(int sprite, const int& r, const int& g, const int& b, int huh, int x, int y, wrect_t* rect, const int& canDrawBrokenTrans){
	//SPR_Draw
	//SPR_DrawAdditive
	//SPR_DrawHoles

	/*
	if(hud_version->value < 3 && EASY_CVAR_GET(hud_brokentrans) == 1 && canDrawBrokenTrans == 1){
		//FillRGBA(0, 0, 35, 35, 88, 8, 8, 185);
		int times = 25;
		while(times > 0){
			SPR_Set(NULL, 0, 0, 0 );
			FillRGBA(x, y, rect->right - rect->left, rect->bottom - rect->top, 1, 1, 1, 255);
			times -= 1;
		}
	}
	*/
	
	if(canDrawBrokenTrans == 1){
		//does CVar checks.
		attemptDrawBrokenTrans(x, y, rect);
	}else if(canDrawBrokenTrans == 2){
		//
		attemptDrawBrokenTransLightAndWhite(x, y, rect);

		//attemptDrawBrokenTransLight(x, y, rect);
		//attemptDrawBrokenTransWhite(x, y, rect);
	}
	//SPR_Set(GetSprite(myFontIDZero + k), r, g, b );
	SPR_Set(sprite, r, g, b );
	SPR_DrawAdditive(huh, x, y, rect);
	
}


//inline
void CHud::attemptDrawBrokenTrans(int arg_startx, int arg_starty, wrect_t* rect){
	if(rect!=NULL)attemptDrawBrokenTrans(arg_startx, arg_starty, rect->right-rect->left, rect->bottom - rect->top);
}
//inline
void CHud::attemptDrawBrokenTransLight(int arg_startx, int arg_starty, wrect_t* rect){
	if(rect!=NULL)attemptDrawBrokenTransLight(arg_startx, arg_starty, rect->right-rect->left, rect->bottom - rect->top);
}
//inline
void CHud::attemptDrawBrokenTransWhite(int arg_startx, int arg_starty, wrect_t* rect){
	if(rect!=NULL)attemptDrawBrokenTransWhite(arg_startx, arg_starty, rect->right-rect->left, rect->bottom - rect->top);
}
//inline
void CHud::attemptDrawBrokenTransLightAndWhite(int arg_startx, int arg_starty, wrect_t* rect){
	if(rect!=NULL)attemptDrawBrokenTransLightAndWhite(arg_startx, arg_starty, rect->right-rect->left, rect->bottom - rect->top);
}


void CHud::attemptDrawBrokenTrans(int arg_startx, int arg_starty, int arg_width, int arg_height){
	
	if(EASY_CVAR_GET(hud_version) < 3 && EASY_CVAR_GET(hud_brokentrans) > 0 && m_HUD_brokentransparency >= 0 ){
		
		//CUT - adjustable isn't working.
		float opaqueness = 1.0;

		wrect_t tempwrectThis = wrect_t();
		tempwrectThis.left = 0;
		tempwrectThis.top = 0;

		for(int y = 0; y < arg_height; y+=brokenTransHeight){
			for(int x = 0; x < arg_width; x+=brokenTransWidth){

				if(x + brokenTransWidth < arg_width){
					tempwrectThis.right = brokenTransWidth;
				}else{
					tempwrectThis.right = arg_width - x;
				}
				if(y + brokenTransHeight < arg_height){
					tempwrectThis.bottom = brokenTransHeight;
				}else{
					tempwrectThis.bottom = arg_height - y;
				}

				//draw the whole thing here.
				SPR_Set( GetSprite(this->m_HUD_brokentransparency), 255 * opaqueness, 255 * opaqueness, 255 * opaqueness );
				SPR_DrawHoles(0, arg_startx + x, arg_starty + y, &tempwrectThis);
				
			}
		}

	}

}


void CHud::attemptDrawBrokenTransLight(int arg_startx, int arg_starty, int arg_width, int arg_height){
	
	if(EASY_CVAR_GET(hud_version) < 3 && EASY_CVAR_GET(hud_brokentrans) > 0 && m_HUD_brokentransparency0 >= 0){
		float opaqueness = 1.0;
		
		wrect_t tempwrectThis = wrect_t();
		tempwrectThis.left = 0;
		tempwrectThis.top = 0;
				
		for(int y = 0; y < arg_height; y+=brokenTransHeight){
			for(int x = 0; x < arg_width; x+=brokenTransWidth){

				if(x + brokenTransWidth < arg_width){
					tempwrectThis.right = brokenTransWidth;
				}else{
					tempwrectThis.right = arg_width - x;
				}
				if(y + brokenTransHeight < arg_height){
					tempwrectThis.bottom = brokenTransHeight;
				}else{
					tempwrectThis.bottom = arg_height - y;
				}

				//draw the whole thing here.
				SPR_Set( GetSprite(this->m_HUD_brokentransparency0), 255 * opaqueness, 255 * opaqueness, 255 * opaqueness );
				SPR_DrawHoles(0, arg_startx + x, arg_starty + y, &tempwrectThis);
				
			}
		}
	}
}


void CHud::attemptDrawBrokenTransWhite(int arg_startx, int arg_starty, int arg_width, int arg_height){
	
	if(EASY_CVAR_GET(hud_version) < 3 && EASY_CVAR_GET(hud_brokentrans) > 0 && m_HUD_brokentransparencyw >= 0){
		float opaqueness = 0.04;
		

		wrect_t tempwrectThis = wrect_t();
		tempwrectThis.left = 0;
		tempwrectThis.top = 0;

				
		for(int y = 0; y < arg_height; y+=brokenTransHeight){
			for(int x = 0; x < arg_width; x+=brokenTransWidth){

				if(x + brokenTransWidth < arg_width){
					tempwrectThis.right = brokenTransWidth;
				}else{
					tempwrectThis.right = arg_width - x;
				}
				if(y + brokenTransHeight < arg_height){
					tempwrectThis.bottom = brokenTransHeight;
				}else{
					tempwrectThis.bottom = arg_height - y;
				}

				//draw the whole thing here.
				SPR_Set( GetSprite(this->m_HUD_brokentransparencyw), 255 * opaqueness, 255 * opaqueness, 255 * opaqueness );
				SPR_DrawHoles(0, arg_startx + x, arg_starty + y, &tempwrectThis);
				
			}
		}
	}

}


void CHud::attemptDrawBrokenTransLightAndWhite(int arg_startx, int arg_starty, int arg_width, int arg_height){
	
	if(EASY_CVAR_GET(hud_version) < 3 && EASY_CVAR_GET(hud_brokentrans) > 0 && m_HUD_brokentransparencyw >= 0){
		float opaqueness = 0.04;
		

		wrect_t tempwrectThis = wrect_t();
		tempwrectThis.left = 0;
		tempwrectThis.top = 0;

				
		for(int y = 0; y < arg_height; y+=brokenTransHeight){
			for(int x = 0; x < arg_width; x+=brokenTransWidth){

				if(x + brokenTransWidth < arg_width){
					tempwrectThis.right = brokenTransWidth;
				}else{
					tempwrectThis.right = arg_width - x;
				}
				if(y + brokenTransHeight < arg_height){
					tempwrectThis.bottom = brokenTransHeight;
				}else{
					tempwrectThis.bottom = arg_height - y;
				}
				opaqueness = 1.0;
				SPR_Set( GetSprite(this->m_HUD_brokentransparency0), 255 * opaqueness, 255 * opaqueness, 255 * opaqueness );
				SPR_DrawHoles(0, arg_startx + x, arg_starty + y, &tempwrectThis);
				
				// NOTICE!  Very faint but this is transparent pure white.
				opaqueness = 0.04;
				SPR_Set( GetSprite(this->m_HUD_brokentransparencyw), 255 * opaqueness, 255 * opaqueness, 255 * opaqueness );
				SPR_DrawAdditive(0, arg_startx + x, arg_starty + y, &tempwrectThis);
				
			}
		}

	}

}




//MODDD - new.
void CHud::playWeaponSelectMoveSound(){

	if(EASY_CVAR_GET(hud_moveselect_sound) == 1){
		PlaySound("common/wpn_moveselect.wav", 1);
	}else if(EASY_CVAR_GET(hud_moveselect_sound) == 2){
		long rand = gEngfuncs.pfnRandomLong(0,2);
		switch(rand){
		case 0:
			PlaySound("weapons/reload1.wav", 1);
		break;
		case 1:
			PlaySound("weapons/reload2.wav", 1);
		break;
		case 2:
			PlaySound("weapons/reload3.wav", 1);
		break;
		}
	}
}







void command_removeFireFly(){
	gEngfuncs.pfnClientCmd("removeFireFly2");
}
void command_showBounds(){
	gEngfuncs.pfnClientCmd("showBounds2");
}
void command_showBoundsAll(){
	gEngfuncs.pfnClientCmd("showBoundsAll2");
}



// OLD VERSION, doesn't seem to work.
/*
void command_sneaky(){
	//gEngfuncs.GetLocalPlayer()
	easyPrintLine("CALLED!");

	BOOL madeSneaky = (! (player.pev->flags & FL_NOTARGET) );


	if(madeSneaky){
		player.pev->flags |= FL_NOTARGET;
		easyPrintLine("Sneaky on!");
	}else{
		player.pev->flags &= ~FL_NOTARGET;
	}

	player.m_fNoPlayerSound = madeSneaky;

	//clientdata_s
	//gEngfuncs.GetLocalPlayer()->curstate

}
*/


void command_sneaky(){
	//gEngfuncs.GetLocalPlayer()

	//gEngfuncs.GetLocalPlayer()->curstate.fl

	gEngfuncs.pfnClientCmd("impulse 105");
	gEngfuncs.pfnClientCmd("notarget");

	
	//gEngfuncs.pfnClientCmd("mp3 play media/Half-Life11.mp3");
	//HEY LOOK THIS WORKS.   Huh.  Noooo idea.  ah well....?  why?

	//CLIENT_COMMAND(tempEd, "mp3 play media/Half-Life11.mp3");


	//  mp3 play media/Half-Life11.mp3
	//~try this?  or
	//  mp3 play sound/media/Half-Life11.mp3
	//~sometime?

}//END OF command_sneaky


void method_mod_version_client(){
	char aryChr[128];
	char aryChrD[128];
	writeVersionInfo(aryChr, 128);
	writeDateInfo(aryChrD, 128);

	easyForcePrintLine("AZ client.dll  Version: %s  Date: %s", aryChr, aryChrD);
}//END OF method_mod_version_client


void method_mod_version_server() {

	//cl_entity_s* tempRef = gEngfuncs.GetLocalPlayer();
	//easyForcePrintLine("null? %d", (tempRef == NULL));
	// Trying to get "GetLocalPlayer" while the server is not running
	// returns a pointer with address "0x00000bb8" consistently, verify
	// this though.
	// "0x16750bd8" is a more typical result if it is working correctly.
	// So being below 0x00100000 is a good way to see if this is invalid
	// memory I assume.

	// ...or just use gEngfuncs.GetMaxClients().  It's 0 when not in a game.  OK!
	  
	/*
		int hay1 = gEngfuncs.GetMaxClients();
		easyForcePrintLine("maxCli? %d", hay1);

	*/

	//if (tempRef != NULL) {
	//if( (int)tempRef >= 0x00100000){
	if(gEngfuncs.GetMaxClients() > 0){
		gEngfuncs.pfnClientCmd("_mod_version_server");
	}else {
		// Not in a game?  Whoops, let the client know why this call wouldn't work
		easyForcePrintLine("***Must be ingame/server for this call to work!");
	}
	
}//END OF method_mod_version_client



cvar_t* global_test_cvar_ref = NULL; 


void command_test_cvar_client_get_direct();
void command_test_cvar_client_get_struct();
void command_test_cvar_reset();



void command_test_cvar_all() {
	command_test_cvar_client_get_direct();
	command_test_cvar_client_get_struct();

	gEngfuncs.pfnClientCmd("tcs_get_direct");
	gEngfuncs.pfnClientCmd("tcs_get_struct");
}


void command_test_cvar_init_link() {
	global_test_cvar_ref = CVAR_GET_POINTER("test_cvar");
	easyForcePrintLine("***client cvar ref established, probably.  Found? %d", (global_test_cvar_ref!=NULL) );
	
	gEngfuncs.pfnClientCmd("tcs_init_link");
}
void command_test_cvar_reset() {

	global_test_cvar_ref = CVAR_GET_POINTER("test_cvar");
	if (global_test_cvar_ref != NULL) {
		global_test_cvar_ref->value = 6;
	}
	CVAR_SET_FLOAT("test_cvar", 6);
	easyForcePrintLine("***client cvar reset.");

	gEngfuncs.pfnClientCmd("tcs_reset");
}
void command_test_cvar_client_set_direct(){
	CVAR_SET_FLOAT("test_cvar", 13.0f);
	easyForcePrintLine("***Set to 13 success, probably.");
}
void command_test_cvar_client_set_struct(){
	//cvar_t* tempRef = CVAR_GET_POINTER("test_cvar");
	cvar_t* tempRef = global_test_cvar_ref;
	if(tempRef != NULL){
		tempRef->value = 13.0f;
		easyForcePrintLine("***Set to 13 success, probably.");
	}else{
		easyForcePrintLine("***ERROR: test_cvar struct call did not work.");
	}
}
void command_test_cvar_client_get_direct(){
	float tempVal = CVAR_GET_FLOAT("test_cvar");
	easyForcePrintLine("***Value: %.2g", tempVal);
}
void command_test_cvar_client_get_struct(){
	//cvar_t* tempRef = CVAR_GET_POINTER("test_cvar");
	cvar_t* tempRef = global_test_cvar_ref;
	if(tempRef != NULL){
		char binaryBuffer[33];
		convertIntToBinary(binaryBuffer, tempRef->flags, 32);
		easyForcePrintLine("***Value: %.2g, flags: %s", tempRef->value, binaryBuffer);
	}else{
		easyForcePrintLine("***ERROR: test_cvar struct call did not work.");
	}
}




void command_updateCameraPerspectiveF(void){
	gEngfuncs.pfnClientCmd("cameraper_f");
}
void command_updateCameraPerspectiveT(void){
	gEngfuncs.pfnClientCmd("cameraper_t");
}



// ohhh yeah, this lets a generally serverside command be seen in autocomplete in console.
// That is, it's a way of getting around being unable to use "pfnAddCommand" serverside.
// You do it clientside, then tell the server the same thing and the server reads it in
// dlls/client.cpp.
// These in particular are alternate "god" and "noclip" commands, since we can't
// make the engine allow them in multiplayer and/or even after turning sv_cheats on
// (without restarting the map that is, but these don't care about map restarts).
void command_god2(void){
	gEngfuncs.pfnClientCmd("god2");
}
void command_noclip2(void){
	gEngfuncs.pfnClientCmd("noclip2");
}
// And to make the fvox toggle show up in auto-complete too, but the server needs to know
// this was called for that player, apparently.
void command_fvoxtoggle(void){

	if (EASY_CVAR_GET(cl_fvox) == 0) {
		EASY_CVAR_SET(cl_fvox, 1);
	}
	else {
		EASY_CVAR_SET(cl_fvox, 0);
	}

	//gEngfuncs.pfnClientCmd("fvoxtoggle");
}

// because autocomplete is nice
void command_mapname(void) {
	gEngfuncs.pfnClientCmd("_mapname");
}


//MODDD - TEST.  I'll suck up references to "lastinv" and tell the server "_lastinv" instead, clear separation.
// Not doing the idea of clientside weapon switching though, even in hud/ammo.cpp and weapons_resource.cpp,
// they don't try to look at the clientside copy of playey inventory, they send a command to the server and
// do nothing else that frame:
//     ServerCmd(gWR.gpActiveSel->szName);
// ...In short, having a weapon-select order go straight to serverside, which then relays what it does to the client,
// is nothing unusual.  Don't try to mimck other logic like the timer for unholstering weapons, which may be set by
// some calls to switch weapons (like a number key + click), but others like 'lastinv' don't.
// !!!
// Not quite right, changes to slot from input numbers/click do make it over to weapon-related script in hl_weapons.cpp,
// mainly ItemPostFrame and ItemPostFrameThink because they're called from checking for clicks clientside.
// "lastinv" however, is still called only from serverside. 
void command_lastinv(void) {
	
	CBasePlayerItem* thingy = localPlayer.m_pLastItem;
	

	// Close the weapon select menu if it is open.
	// This does not check whether there is an item to be swapped to though (nothing would happen).
	gHUD.m_Ammo.gWR.CancelSelection();



	localPlayer.SelectLastItem();

	/*
	//if (!IsMultiplayer()) {
	//	localPlayer.SelectLastItem();
	//}
	//else
	{
		// just... <disregard> trying to get that to look right.  Holy <frick>.
		// That works or it doesn't.  I'm out.
		// Oh holy <crap> that's way better!  <severely disregard> this <blasted> <nematode-consuming> engine!!!    <oh dear me>
		// even these settings may not make a difference though.
		localPlayer.m_bHolstering = TRUE;
		localPlayer.m_fCustomHolsterWaitTime = gpGlobals->time + 10;
	}
	*/

	// can call for localPlayer.SelectLastItem() during the next input frame too but eh.
	// Still strangely inferior to the above approach.  Just.  Weird.
	// Still not amazing, shows its flaws on multiplayer.  The approach to replicate more clientside.
	// Has more issues in multiplayer.
	// Go figure!
	//queuecall_lastinv = TRUE;

	gEngfuncs.pfnClientCmd("_lastinv");
}



void command_test3(void) {
	float theRes = EASY_CVAR_GET(pregame_server_cvar);
	easyForcePrintLine("pregame_server_cvar is %.2f", theRes);
}
void command_test4(void) {
	EASY_CVAR_SET(pregame_server_cvar, 16);
	easyForcePrintLine("pregame_server_cvar set?");
}



// This is called every time the DLL is loaded
// MODDD - NOTE. 
// See cl_dlls/cdll_int.cpp for the HUD_Init call that leads here, although there isn't anything
// special there.
void CHud::Init( void )
{
	//MODDD - HOOK_MESSAGE calls for hud_msg.cpp methods moved
	// to custom_message.cpp, along with the methods themselves.
	
	HOOK_COMMAND( "togglebrowser", ToggleServerBrowser );

	HOOK_MESSAGE( TeamNames );

	HOOK_MESSAGE( VGUIMenu );
	HOOK_MESSAGE( MOTD );
	HOOK_MESSAGE( ServerName );
	HOOK_MESSAGE( ScoreInfo );
	HOOK_MESSAGE( TeamScore );
	HOOK_MESSAGE( TeamInfo );

	HOOK_MESSAGE(Spectator);
	HOOK_MESSAGE(AllowSpec);


	//MODDDDMIRROR - this block.   ...wait, really?  Don't you mean just the 'numMirrors' bit/
	viewEntityIndex = 0; // trigger_viewset stuff
	viewFlags = 0;
	m_iLogo = 0;
	m_iPlayerFOV = 0;
	numMirrors = 0;
	//m_iHUDColor = 0x00FFA000; //255,160,0 -- LRC


	gEngfuncs.pfnAddCommand( "noclipalt", command_noclip2 );
	gEngfuncs.pfnAddCommand( "godalt", command_god2 );
	gEngfuncs.pfnAddCommand( "_noclip", command_noclip2 );
	gEngfuncs.pfnAddCommand( "_god", command_god2 );


	gEngfuncs.pfnAddCommand("fvoxtoggle", command_fvoxtoggle);
	// sadly the "mapname" command is not an option, it competes with autocomplete for "map", as in mapname fills itself in
	// on typing "map" and then a space, which is irritating if you wanted to use the 'map' command.  UGH.
	//gEngfuncs.pfnAddCommand("mapname", command_mapname);
	gEngfuncs.pfnAddCommand("getmap", command_mapname);
	gEngfuncs.pfnAddCommand("currentmap", command_mapname);
	gEngfuncs.pfnAddCommand("thismap", command_mapname);
	gEngfuncs.pfnAddCommand("curmap", command_mapname);

	gEngfuncs.pfnAddCommand("lastinv", command_lastinv);

	


	//CVar_cameraMode = CVAR_CREATE("IGNOREcameraMode", "0", FCVAR_CLIENTDLL);
	//CVar_cameraModeMem = 0;

	//Apply the method for referring to sounds by sentences.txt instead of precaching them to save precache space.
	
	

	//cvarHUD_letswatchamovie = CVAR_CREATE( "letswatchamovie", "0", FCVAR_CLIENTDLL );

	gEngfuncs.pfnAddCommand( "sneaky", command_sneaky );
	//Toggles both "impulse 105" (no player noise) and "notarget".

	gEngfuncs.pfnAddCommand("mod_version_client", method_mod_version_client);
	gEngfuncs.pfnAddCommand("mod_version_server", method_mod_version_server);
	

	

	gEngfuncs.pfnAddCommand("test3", command_test3);
	gEngfuncs.pfnAddCommand("test4", command_test4);





	//TEST!!!
	CVAR_CREATE("pregame_server_cvar", "0", FCVAR_ARCHIVE | FCVAR_SERVER);






	//MODDD - CVAR TEST
	//CVAR_CREATE("test_cvar", "6", 0);
	// FCVAR_CLIENTDLL | FCVAR_ARCHIVE


	// TEST CVARS, remove later.
	CVAR_CREATE("ctt1", "1", 0);
	CVAR_CREATE("ctt2", "-1", 0);
	CVAR_CREATE("ctt3", "1", 0);  //used to be 2, works better with 0 here!
	CVAR_CREATE("ctt4", "0", 0);


	
	gEngfuncs.pfnAddCommand("tc_init", command_test_cvar_init_link);

	gEngfuncs.pfnAddCommand("tc_reset", command_test_cvar_reset);

	//global_test_cvar_ref = CVAR_GET_POINTER("test_cvar");
	

	gEngfuncs.pfnAddCommand( "tcc_set_direct", command_test_cvar_client_set_direct );
	gEngfuncs.pfnAddCommand( "tcc_set_struct", command_test_cvar_client_set_struct );
	gEngfuncs.pfnAddCommand( "tcc_get_direct", command_test_cvar_client_get_direct );
	gEngfuncs.pfnAddCommand( "tcc_get_struct", command_test_cvar_client_get_struct );
	///////////////////////////////////////////////
	gEngfuncs.pfnAddCommand("tc_all", command_test_cvar_all);



	/*
	char aryChr[128];
	char aryChrD[128];
	writeVersionInfo(aryChr, 128);
	writeDateInfo(aryChrD, 128);

	//NOTE - SPECIAL.  Not using the EasyCVAR system, these are for getting info about the game.
	//...getting replaced by commands, makes more sense as read-only values anyway.
	CVAR_CREATE("cl_mod_version", aryChr, FCVAR_CLIENTDLL);
	CVAR_CREATE("cl_mod_date", aryChrD, FCVAR_CLIENTDLL);
	*/

	//to be written to by the server dll (default name: hl.dll) at game start.
	//NO, register on the server now (dlls/game.cpp)!
	//CVAR_CREATE("sv_mod_version", "Start the game!", 0);
	//CVAR_CREATE("sv_mod_date", "Start the game!", 0);
	
	
	EASY_CVAR_CREATE_CLIENT_MASS


	//CVAR_CREATE("barnacleEatsAnything", "0", FCVAR_ARCHIVE | FCVAR_PROTECTED | FCVAR_UNLOGGED | FCVAR_PRINTABLEONLY);
	//CVAR_CREATE("barnacleEatsBarnacles", "0", FCVAR_ARCHIVE);
	
	
	
	// These... are a confusing case as I don't remember anything about them, other than the note further above
	// about thees CVars coming from a different version of the SDK.  Best not to touch.
	cl_viewrollangle = CVAR_CREATE("cl_viewrollangle", "0.65", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
	cl_viewrollspeed = CVAR_CREATE("cl_viewrollspeed", "300", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);

	// Same for these...?  No idea what business Team Fortress CVars have in HL but okay.
	//MODDD - and bye.
	// I don't know if hud_takesshots can even work in HL multiplayer.  If not definitely keep removed.
	//CVAR_CREATE( "hud_classautokill", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );		// controls whether or not to suicide immediately on TF class switch
	//CVAR_CREATE( "hud_takesshots", "0", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	


	
	/*
	CVAR_CREATE("_sv_aim", "0", FCVAR_ARCHIVE);

	if (CVAR_GET_FLOAT("_sv_aim") == 0) {
		//enforce a new default for sv_aim
		CVAR_SET_FLOAT("sv_aim", 0);
		CVAR_SET_FLOAT("_sv_aim", 1);
	}else {

	}
	*/

	m_iLogo = 0;
	m_iPlayerFOV = 0;


	CVAR_CREATE( "zoom_sensitivity_ratio", "1.2", 0 );
	//MODDD - changed how this works.  Remembers since last time, and the default is different.
	//default_fov = CVAR_CREATE( "default_fov", "90", 0 );
	//In the base game, the default_fov always starts out at 90.
	// Also leaving this outside the EASY_CVAR system!
	default_fov = CVAR_CREATE( "default_fov", "90", FCVAR_ARCHIVE );
	m_pCvarStealMouse = CVAR_CREATE( "hud_capturemouse", "1", FCVAR_ARCHIVE );
	m_pCvarDraw = CVAR_CREATE( "hud_draw", "1", FCVAR_ARCHIVE );
	cl_lw = gEngfuncs.pfnGetCvarPointer( "cl_lw" );
	
	
	
	m_pSpriteList = NULL;

	// Clear any old HUD list
	if ( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	// In case we get messages before the first update -- time will be valid
	m_flTime = 1.0;

	m_Ammo.Init();
	m_Health.Init();
	m_SayText.Init();
	m_Spectator.Init();
	m_Geiger.Init();
	m_Train.Init();
	m_Battery.Init();
	m_Flash.Init();
	m_Message.Init();
	m_StatusBar.Init();
	m_DeathNotice.Init();
	m_AmmoSecondary.Init();
	m_TextMessage.Init();
	m_StatusIcons.Init();
	m_Pain.Init();
	GetClientVoiceMgr()->Init(&g_VoiceStatusHelper, (vgui::Panel**)&gViewPort);

	m_Menu.Init();
	
	ServersInit();

	MsgFunc_ResetHUD(0, 0, NULL );


}

// CHud destructor
// cleans up memory allocated for m_rg* arrays
CHud::~CHud()
{
	delete [] m_rgSpriteHandle_ts;
	delete [] m_rgrcRects;
	delete [] m_rgszSpriteNames;

	if ( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	ServersShutdown();
}




void CHud::getGenericGUIColor(int &r, int &g, int &b){
	
	if(EASY_CVAR_GET(hud_version) < 3){
		// pre E3
		getGenericGreenColor(r, g, b);
	}else{
		getGenericOrangeColor(r, g, b);
	}

}


void CHud::getGenericEmptyColor(int &r, int &g, int &b){

	
	if(EASY_CVAR_GET(hud_version) < 3){
		// pre E3 (green-based; prototype)
		if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(preE3UsesFailColors) == 1){
			getGenericRedColor(r,g,b);
		}else{
			getGenericGreenColor(r, g, b);
		}

	}else{
		// E3 (yellow-based; closer to retail)
		if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(E3UsesFailColors) == 1){
			getGenericRedColor(r,g,b);
		}else{
			getGenericOrangeColor(r, g, b);
		}
	}

}




void CHud::getGenericRedColor(int &r, int &g, int &b){
	r = 245;
	g = 8;
	b = 8;
}

void CHud::getGenericOrangeColor(int &r, int &g, int &b){
	r = 255;
	g = 152;
	b = 0;
}


void CHud::getGenericGreenColor(int &r, int &g, int &b){
	r = 5;
	g = COLOR_PRE_E3_BRIGHTNESS;
	b = 5;
}




// GetSpriteIndex()
// searches through the sprite list loaded from hud.txt for a name matching SpriteName
// returns an index into the gHUD.m_rgSpriteHandle_ts[] array
// returns 0 if sprite not found
int CHud::GetSpriteIndex( const char *SpriteName )
{

	//MODDD - printouts.
	//easyPrint( "SPRITE COUNT IS %d\n", m_iSpriteCount );
	//ClientPrintAll( HUD_PRINTNOTIFY, numstr);

	
	// look through the loaded sprite name list for SpriteName
	for ( int i = 0; i < m_iSpriteCount; i++ )
	{
		if ( strncmp( SpriteName, m_rgszSpriteNames + (i * MAX_SPRITE_NAME_LENGTH), MAX_SPRITE_NAME_LENGTH ) == 0 )
			return i;
	}

	return -1; // invalid sprite
}

void CHud::VidInit( void )
{

	//?
	//easyPrintLine("HUD:::VIDINIT!");

	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);


	//gEngfuncs.pfnGetCvarPointer( "cl_lw" );???


	updateAutoFOV();


	//MODDDDMIRROR - this block.  Also here too (?)
	viewEntityIndex = 0; // trigger_viewset stuff
	viewFlags = 0;
	m_iLogo = 0;
	m_iPlayerFOV = 0;
	numMirrors = 0;
	//m_iHUDColor = 0x00FFA000; //255,160,0 -- LRC



	//m_hsprGNFOS = 0;


	if(playingMov == TRUE){
		//Why does loading it here avoid crashes sometimes? Who knows.
		m_hsprGNFOS = SPR_Load("sprites/ymg.spr");
	}


	// ----------
	// Load Sprites
	// ---------
//	m_hsprFont = LoadSprite("sprites/%d_font.spr");
	
	m_hsprLogo = 0;	
	m_hsprCursor = 0;

	if (ScreenWidth < 640)
		m_iRes = 320;
	else
		m_iRes = 640;

	// Only load this once
	if ( !m_pSpriteList )
	{

		//MODDD - printouts.  And test edit.
		//m_iSpriteCountAllRes = 124;
		//easyPrint("THE SIZE IS %d\n", m_iSpriteCountAllRes);

		// we need to load the hud.txt, and all sprites within
		m_pSpriteList = SPR_GetList("sprites/hud.txt", &m_iSpriteCountAllRes);

		//MODDD - printouts.
		//easyPrint("THE SIZE IS 222 %d\n", m_iSpriteCountAllRes);


		if (m_pSpriteList)
		{
			// count the number of sprites of the appropriate res
			m_iSpriteCount = 0;
			client_sprite_t *p = m_pSpriteList;
			int j;
			for ( j = 0; j < m_iSpriteCountAllRes; j++ )
			{

				if ( p->iRes == m_iRes )
					m_iSpriteCount++;
				p++;
			}

			//MODDD - printouts.
			//easyPrint("THE SIZE IS 333 %d\n", m_iSpriteCount);



			// allocated memory for sprite handle arrays
 			m_rgSpriteHandle_ts = new SpriteHandle_t[m_iSpriteCount];
			m_rgrcRects = new wrect_t[m_iSpriteCount];
			m_rgszSpriteNames = new char[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH];

			p = m_pSpriteList;
			int index = 0;
			for ( j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if ( p->iRes == m_iRes )
				{
					char sz[256];
					sprintf(sz, "sprites/%s.spr", p->szSprite);
					m_rgSpriteHandle_ts[index] = SPR_Load(sz);
					m_rgrcRects[index] = p->rc;
					strncpy( &m_rgszSpriteNames[index * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH );

					/*
					char tempString[MAX_SPRITE_NAME_LENGTH + 1];
					int tempindex = 0;
					for(tempindex = 0; tempindex < MAX_SPRITE_NAME_LENGTH; tempindex++){
						char tempChar = m_rgszSpriteNames[(j * MAX_SPRITE_NAME_LENGTH) + tempindex];
						if(tempChar == '\0'){
							break;
						}
						tempString[tempindex] = tempChar;
					}
					tempString[tempindex] = '\0';
					easyPrint("I LOADED SPRITE #%d: %s\n", j, tempString ) ;
					*/
					

					index++;
				}

				//MODDD - printouts.
				//easyPrint("I LOADED SPRITE #%d: %s\n", j, p->szName ) ;
				
				p++;
				//easyPrint("IS P NULL YET? %d\n", (p == NULL) );

			}


			//MODDD - printouts.
			/*
			easyPrint("EEEEEEEEEEEEEEEEEEEEEEEEEEEE\n" ) ;



			p = m_pSpriteList;
			int times = 0;
			int maxTimes = 125;
			while(p != nullptr){
				easyPrint("I SEE SPRITE #%d: %s : %s      %d\n", times, p->szName, p->szSprite, p->rc.right ) ;
				p++;
				//easyPrint("IS P NULL YET? %d   times: %d\n", (p == NULL), times );
				times++;

				if(times >= maxTimes){
					break;
				}

			}
			*/

		}
	}
	else
	{
		// we have already have loaded the sprite reference from hud.txt, but
		// we need to make sure all the sprites have been loaded (we've gone through a transition, or loaded a save game)
		client_sprite_t *p = m_pSpriteList;
		int index = 0;
		for ( int j = 0; j < m_iSpriteCountAllRes; j++ )
		{
			if ( p->iRes == m_iRes )
			{
				char sz[256];
				sprintf( sz, "sprites/%s.spr", p->szSprite );
				m_rgSpriteHandle_ts[index] = SPR_Load(sz);
				index++;
			}

			p++;
		}
	}

	// assumption: number_1, number_2, etc, are all listed and loaded sequentially
	m_HUD_number_0 = GetSpriteIndex( "number_0" );

	//MODDD - added
	m_HUD_number_0_health = GetSpriteIndex( "number_0health" );
	m_HUD_number_1_tiny = GetSpriteIndex( "number_1tiny" );

	// for hud_version 0
	m_HUD_e_number_0 = GetSpriteIndex("e_number_0");
	m_HUD_e_number_0_health = GetSpriteIndex("e_number_0health");

	


	//MODDD - altgui
	m_HUD_number_0_E3R = GetSpriteIndex("number_0_E3R");
	
	m_HUD_battery_empty_E3 = GetSpriteIndex("battery_empty_E3");
	m_HUD_battery_full_E3 = GetSpriteIndex("battery_full_E3");


	alphaCrossHairIndex = GetSpriteIndex( "alphacrosshair" );
	if(alphaCrossHairIndex == -1){
		easyPrint("ALPHACROSSHAIR NOT LOADED A\n");
	}else{
		//easyPrint("ALPHACROSSHAIR LOADED\n");
	}
	
	m_HUD_brokentransparency = GetSpriteIndex("brokentrans");
	/*
	m_HUD_brokentransparency1 = GetSpriteIndex("brokentrans1");
	m_HUD_brokentransparency2 = GetSpriteIndex("brokentrans2");
	m_HUD_brokentransparency3 = GetSpriteIndex("brokentrans3");
	m_HUD_brokentransparency4 = GetSpriteIndex("brokentrans4");
	*/
	
	m_HUD_brokentransparency0 = GetSpriteIndex("brokentransalt");
	m_HUD_brokentransparencyw = GetSpriteIndex("brokentranswww");


	//try to get the new glock silencer and old RPG (if it wasn't there) in!
	m_glockSilencerWpnIcoActive = GetSpriteIndex("glocksilactive");
	m_glockSilencerWpnIcoInactive = GetSpriteIndex("glocksilinact");
	

	if(m_HUD_brokentransparency >= 0){
		m_prc_brokentransparency = &GetSpriteRect( m_HUD_brokentransparency );
		brokenTransWidth = m_prc_brokentransparency->right - m_prc_brokentransparency->left;
		brokenTransHeight = m_prc_brokentransparency->bottom - m_prc_brokentransparency->top;
	}
	
	// ammo number font sizes
	m_iFontWidth = m_rgrcRects[m_HUD_number_0].right - m_rgrcRects[m_HUD_number_0].left;
	m_iFontHeight = m_rgrcRects[m_HUD_number_0].bottom - m_rgrcRects[m_HUD_number_0].top;
	// health number font sizes
	m_iFontWidthAlt = m_rgrcRects[m_HUD_number_0_health].right - m_rgrcRects[m_HUD_number_0_health].left;
	m_iFontHeightAlt = m_rgrcRects[m_HUD_number_0_health].bottom - m_rgrcRects[m_HUD_number_0_health].top;
		
	// (the above are same for the early HUD too (hud_version 0).




	//MODDD - just do it
	Init_CustomMessage();
	
	m_Ammo.VidInit();
	m_Health.VidInit();
	m_Spectator.VidInit();
	m_Geiger.VidInit();
	m_Train.VidInit();
	m_Battery.VidInit();
	m_Flash.VidInit();
	m_Message.VidInit();
	m_StatusBar.VidInit();
	m_DeathNotice.VidInit();
	m_SayText.VidInit();
	m_Menu.VidInit();
	m_AmmoSecondary.VidInit();
	m_TextMessage.VidInit();
	m_StatusIcons.VidInit();
	m_Pain.VidInit();
	GetClientVoiceMgr()->VidInit();
}

//MODDD - MsgFunc_Logo moved to hud_msg.cpp for consistency.


/*
============
COM_FileBase
============
*/
// Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
void COM_FileBase ( const char *in, char *out)
{
	int len, start, end;

	len = strlen( in );
	
	// scan backward for '.'
	end = len - 1;
	while ( end && in[end] != '.' && in[end] != '/' && in[end] != '\\' )
		end--;
	
	if ( in[end] != '.' )		// no '.', copy to end
		end = len-1;
	else 
		end--;					// Found ',', copy to left of '.'


	// Scan backward for '/'
	start = len-1;
	while ( start >= 0 && in[start] != '/' && in[start] != '\\' )
		start--;

	if ( in[start] != '/' && in[start] != '\\' )
		start = 0;
	else 
		start++;

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy( out, &in[start], len );
	// Terminate it
	out[len] = 0;
}

/*
=================
HUD_IsGame

=================
*/
int HUD_IsGame( const char *game )
{
	const char *gamedir;
	char gd[ 1024 ];

	gamedir = gEngfuncs.pfnGetGameDirectory();
	if ( gamedir && gamedir[0] )
	{
		COM_FileBase( gamedir, gd );
		if ( !stricmp( gd, game ) )
			return 1;
	}
	return 0;
}

/*
=====================
HUD_GetFOV

Returns last FOV
=====================
*/
float HUD_GetFOV( void )
{
	if ( gEngfuncs.pDemoAPI->IsRecording() )
	{
		// Write it
		int i = 0;
		unsigned char buf[ 100 ];

		// Active
		*( float * )&buf[ i ] = g_lastFOV;
		i += sizeof( float );

		Demo_WriteBuffer( TYPE_ZOOM, i, buf );
	}

	if ( gEngfuncs.pDemoAPI->IsPlayingback() )
	{
		g_lastFOV = g_demozoom;
	}
	return g_lastFOV;
}


//MODDD - MsgFunc_SetFOV moved to hud_msg.cpp for consistency.


void CHud::AddHudElem(CHudBase *phudelem)
{
	HUDLIST *pdl, *ptemp;

//phudelem->Think();

	if (!phudelem)
		return;

	pdl = (HUDLIST *)malloc(sizeof(HUDLIST));
	if (!pdl)
		return;

	memset(pdl, 0, sizeof(HUDLIST));
	pdl->p = phudelem;

	if (!m_pHudList)
	{
		m_pHudList = pdl;
		return;
	}

	ptemp = m_pHudList;

	while (ptemp->pNext)
		ptemp = ptemp->pNext;

	ptemp->pNext = pdl;
}

float CHud::GetSensitivity( void )
{
	return m_flMouseSensitivity;
}


