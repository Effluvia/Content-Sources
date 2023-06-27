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
// hud_redraw.cpp
//
#include <math.h>

#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "build_settings.h"


//MODDD - nice
//EASY_CVAR_EXTERN_CLIENT_MASS
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(thatWasntGrass)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(imAllFuckedUp)
EASY_CVAR_EXTERN(hud_logo)



#define MAX_LOGO_FRAMES 56


extern int g_iVisibleMouse;
extern cvar_t *sensitivity;
extern float global2PSEUDO_IGNOREcameraMode;


// Variables needed by the PainFlash method moved from health.cpp
extern float cumulativeFadeDrown;
extern float cumulativeFade;
extern float fAttackFrontMem;
extern float fAttackRearMem;
extern float fAttackRightMem;
extern float fAttackLeftMem;


extern void command_updateCameraPerspectiveF(void);
extern void command_updateCameraPerspectiveT(void);


float HUD_GetFOV( void );



int grgLogoFrame[MAX_LOGO_FRAMES] = 
{
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 12, 11, 10, 9, 8, 14, 15,
	16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 
	29, 29, 29, 29, 29, 28, 27, 26, 25, 24, 30, 31 
};



//MODDD
extern "C" 
{
	int CL_IsThirdPerson( void );
}



//MODDD - see where this is called in "hud_update.cpp", in "UpdateClientData".
// Think
void CHud::Think(void)
{
	/*
	// Debug feature for testing different printout methods.
	// Still must be ingame, and calls to "CHud::Think" are blocked when the console or menu (Esc) is open.
	static float nextPrintTime = 0;

	if (nextPrintTime == 0 || gHUD.recentTime >= nextPrintTime) {
		nextPrintTime = gHUD.recentTime + 2;
		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 0) {
			gEngfuncs.pfnConsolePrint("Atest print\n");
		}
		else if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 1) {
			gEngfuncs.Con_Printf("Btest print %i %d %f %s end\n", 4, 4, 4.2f, "X");
		}
		else if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 2) {
			gEngfuncs.Con_DPrintf("Ctest print %i %d %f %s end\n", 4, 4, 4.2f, "X");
		}
	}
	*/

	int newfov;
	HUDLIST *pList = m_pHudList;

	while (pList)
	{
		if (pList->p->m_iFlags & HUD_ACTIVE)
			pList->p->Think();
		pList = pList->pNext;
	}

	//easyForcePrintLine("IMA lovely person FOV STUFF:::%f %i %f", HUD_GetFOV(), m_iPlayerFOV, gHUD.getPlayerBaseFOV());

	// MODDD - NOTE.  I have no clue why this one place uses "HUD_GetFOV" instead of the standard
	// "m_iPlayerFOV" (used to be named m_iFOV).
	// Looks like HUD_GetFOV is just a buffer method for letting demo recording/playback affect 
	// what's returned if necessary.  And its own "g_lastFOV" comes straight from what the client
	// must've recently been using from other edits anyway.
	// Just switching out references to "default_fov" or "default_fov->value"...
	// Think I kindof get it now.  That "g_lastFOV" stays 0 when the FOV is forced to be set to 0,
	// so that it's a call to pick the default_fov value itself (like 90).  But that set (to 90)
	// doesn't affect the original "0" signal received, for future frames.
	newfov = HUD_GetFOV();
	if ( newfov == 0 )
	{
		//MODDD
		//m_iPlayerFOV = default_fov->value;
		m_iPlayerFOV = gHUD.getPlayerBaseFOV();
	}
	else
	{
		m_iPlayerFOV = newfov;
	}

	// the clients fov is set in the client data update section of the hud

	// Set a new sensitivity
	//MODDD
	//if (m_iPlayerFOV == default_fov->value )
	if (m_iPlayerFOV == gHUD.getPlayerBaseFOV())
	{  
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{  
		// set a new sensitivity that is proportional to the change from the FOV default
		//MODDD
		//m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)default_fov->value) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)gHUD.getPlayerBaseFOV()) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	// think about default fov
	if (m_iPlayerFOV == 0 )
	{  // only let players adjust up in fov,  and only if they are not overriden by something else
		//MODDD... wait, this is a little confusing.  So ignoring default_fov choices below 90 was the intention?
		// don't really know what side to change here, mmm...
		//m_iPlayerFOV = max( default_fov->value, 90 );
		//m_iPlayerFOV = max(default_fov->value, gHUD.getPlayerBaseFOV());
		m_iPlayerFOV = max(gHUD.getPlayerBaseFOV(), 90);
	}
	

	if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(thatWasntGrass) == 1){
		//gHUD.m_iPlayerFOV = getTimePeriodAndBackSmooth(recentTime, 0.7f, 87, 110);
		gHUD.m_iPlayerFOV = getTimePeriodAndBackSmooth(recentTime, 0.23f, 0.23f, 94, 110);
	}
	
	if(
		CVAR_GET_FLOAT("crosshair") != gHUD.crosshairMem ||
		EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns) != gHUD.allowAlphaCrosshairWithoutGunsMem
	){
		// includes updates to the MEM vars.
		gHUD.m_Ammo.updateCrosshair();
	}
	
	
	//MODDD
	int isThirdPerson = CL_IsThirdPerson();
	if(gHUD.PESUDO_cameraModeMem != isThirdPerson){
		//return if it is not equal to 0, OR just itself?
		gHUD.PESUDO_cameraModeMem = (isThirdPerson != 0);
		//let the server know...
		global2PSEUDO_IGNOREcameraMode = gHUD.PESUDO_cameraModeMem;
		
		if(global2PSEUDO_IGNOREcameraMode == 0){
			command_updateCameraPerspectiveF();
		}else{
			command_updateCameraPerspectiveT();
		}

	}//END OF camera perspective (first or third person) check.

}//END OF Think













extern int playingMov;
extern float movieStartTime;


// Redraw
// step through the local data,  placing the appropriate graphics & text as appropriate
// returns 1 if they've changed, 0 otherwise
int CHud::Redraw(float flTime, int intermission)
{
	//easyForcePrintLine("CLIENT GUI: Redraw: %.2f %d", flTime, intermission);

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(imAllFuckedUp) == 1) {
		drawCrazyShit(flTime);
	}


	m_fOldTime = m_flTime;	// save time of previous redraw
	m_flTime = flTime;
	m_flTimeDelta = (double)m_flTime - m_fOldTime;
	static int m_flShotTime = 0;

	// Clock was reset, reset delta
	if (m_flTimeDelta < 0)
		m_flTimeDelta = 0;

	// Bring up the scoreboard during intermission
	if (gViewPort)
	{
		if (m_iIntermission && !intermission)
		{
			// Have to do this here so the scoreboard goes away
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideScoreBoard();
			gViewPort->UpdateSpectatorPanel();
		}
		else if (!m_iIntermission && intermission)
		{
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideVGUIMenu();
			gViewPort->ShowScoreBoard();
			gViewPort->UpdateSpectatorPanel();

			// Take a screenshot if the client's got the cvar set
			if (CVAR_GET_FLOAT("hud_takesshots") != 0)
				m_flShotTime = flTime + 1.0;	// Take a screenshot in a second
		}
	}

	if (m_flShotTime && m_flShotTime < flTime)
	{
		gEngfuncs.pfnClientCmd("snapshot\n");
		m_flShotTime = 0;
	}

	m_iIntermission = intermission;

	// if no redrawing is necessary
	// return 0;


	// hud_draw
	if (m_pCvarDraw->value)
	{
		HUDLIST* pList = m_pHudList;

		while (pList)
		{
			if (!intermission)
			{
				if ((pList->p->m_iFlags & HUD_ACTIVE) && !(m_iHideHUDDisplay & HIDEHUD_ALL)) {
					pList->p->Draw(flTime);
				}
			}
			else
			{  // it's an intermission,  so only draw hud elements that are set to draw during intermissions
				if (pList->p->m_iFlags & HUD_INTERMISSION) {
					pList->p->Draw(flTime);
				}
			}

			pList = pList->pNext;
		}
	}//END OF hud_draw check
	

	//MODDD - must also make the gHUD.m_Health.DrawPain call, since m_Health isn't doing it on its own to give
	// control up to here.
	gHUD.m_Pain.DrawPain(flTime);





	//MODDD - this was already here.  Left in the game... very interesting.
	// are we in demo mode? do we need to draw the logo in the top corner?

	//MODDD - new if-then to use the "hud_logo" cvar instead:
	//if (m_iLogo)
	if (EASY_CVAR_GET(hud_logo) == 1)
	{
		int x, y, i;

		if (m_hsprLogo == 0)
			m_hsprLogo = LoadSprite("sprites/%d_logo.spr");

		SPR_Set(m_hsprLogo, 250, 250, 250);

		x = SPR_Width(m_hsprLogo, 0);
		x = ScreenWidth - x;
		y = SPR_Height(m_hsprLogo, 0) / 2;

		// Draw the logo at 20 fps
		int iFrame = (int)(flTime * 20) % MAX_LOGO_FRAMES;
		i = grgLogoFrame[iFrame] - 1;

		SPR_DrawAdditive(i, x, y, NULL);
	}



	if (EASY_CVAR_GET(cl_earlyaccess)) {
		int buildInfoY;
		if (CVAR_GET_FLOAT("developer") >= 1) {
			// shift things down a little more to not overlap with the recent console line
			// printed along the top-left of the screen.
			buildInfoY = 28;
		}
		else {
			buildInfoY = 14;
		}

		
		// using this condition instead of "gHUD.canDrawSidebar()" because that requires the suit.  This one doesn't and works fine.
		// Point is not to draw the version notice at the same time as the weapon-select menu when open, too cluttered.
		if (!gHUD.m_Ammo.gWR.gpActiveSel) {
			// this gives the default color.
			gEngfuncs.pfnDrawSetTextColor(1.00, 0.63, 0);
			DrawConsoleString(16, buildInfoY, BUILD_INFO_TITLE);
			gEngfuncs.pfnDrawSetTextColor(1.00, 0.63, 0);
			DrawConsoleString(16, buildInfoY + 20, globalbuffer_cl_mod_display);
			if (globalbuffer_sv_mod_display[0] != '\0') {
				// go ahead
				gEngfuncs.pfnDrawSetTextColor(1.00, 0.63, 0);
				DrawConsoleString(16, buildInfoY + 40, globalbuffer_sv_mod_display);
			}
			else {
				// nothing from the server yet? Say so
				gEngfuncs.pfnDrawSetTextColor(1.00, 0.63, 0);
				DrawConsoleString(16, buildInfoY + 40, "SV: wait...");
			}
		}

		//gHUD.DrawHudString(16, 100, 500, "test text", 255, 0, 0);
	}//END OF cl_earlyaccess check

	
	
	
			//200x126
#define CUSTOMVID_WIDTH 200.0f
#define CUSTOMVID_HEIGHT 126.0f
//work it!
#define GNFOS_FRAMES 31
#define GNFOS_FRAMES_PER_SEC 4.2518173090111095871622548347277

	if(playingMov == TRUE){
		
		if(movieStartTime != -1){
			
			int x, y, i;
			int widthFits = (int) ceil((float)ScreenWidth / CUSTOMVID_WIDTH);
			int heightFits = (int) ceil((float)ScreenHeight / CUSTOMVID_HEIGHT);
			int v_i_w = 0;
			int v_i_h = 0;


			if (m_hsprGNFOS == 0)
				m_hsprGNFOS = SPR_Load("sprites/ymg.spr");

			SPR_Set(m_hsprGNFOS, 255, 255, 255 );

			// Draw the movie at 8 fps
			i = (int)( (flTime-movieStartTime) * GNFOS_FRAMES_PER_SEC );

			if(i >= GNFOS_FRAMES){
				//can't draw.
				movieStartTime = -1;
				playingMov = FALSE;
				
			}else{

				for(v_i_h = 0; v_i_h < heightFits; v_i_h++){
					for(v_i_w = 0; v_i_w < widthFits; v_i_w++){
					

						x = v_i_w * CUSTOMVID_WIDTH;
						y = v_i_h * CUSTOMVID_HEIGHT;

						//while(TRUE){
							//easyForcePrintLine("FLAG_1");
							//easyForcePrintLine("FLAG_2");

							///x = SPR_Width(m_hsprGNFOS, 0);
							//x = ScreenWidth - x;
							//y = SPR_Height(m_hsprLogo, 0)/2;
							//y = 0;
				
							//easyForcePrintLine("FLAG_3");
							//easyPrintLine("FRAME # %d", i);
				
							//easyForcePrintLine("FLAG_4");
							//easyForcePrintLine("FLAG_5");

							SPR_Draw(i, x, y, NULL);

							//break;
						//}//END OF while(true)

					}
				}
			}
		}
	}//END OF playingMov check
	


	/*
	if ( g_iVisibleMouse )
	{
		void IN_GetMousePos( int *mx, int *my );
		int mx, my;

		IN_GetMousePos( &mx, &my );
		
		if (m_hsprCursor == 0)
		{
			char sz[256];
			sprintf( sz, "sprites/cursor.spr" );
			m_hsprCursor = SPR_Load( sz );
		}

		SPR_Set(m_hsprCursor, 250, 250, 250 );
		
		// Draw the logo at 20 fps
		SPR_DrawAdditive( 0, mx, my, NULL );
	}
	*/


	//draw the plus char?
	/*
	char crossChar = '+';

	int xPos = (ScreenWidth - gHUD.m_scrinfo.charWidths[crossChar]) / 2;
	int yPos = (ScreenHeight - gHUD.m_scrinfo.iCharHeight) / 2;
	TextMessageDrawChar( xPos, yPos, crossChar, 255, 255, 255 );
	*/


	return 1;
}

//NOTICE: This method probably looks strange, not being one of "CHud".
// It is just a top-level method sitting on its own.
// It is the implementation of a top-level prototype in "cl_util.h".
// This method may always be referred to in any other context, just don't try to re-declare or re-define it.
void ScaleColors( int &r, int &g, int &b, int a )
{
	float x = (float)a / 255.0f;
	r = (int)(r * x);
	g = (int)(g * x);
	b = (int)(b * x);
}

int CHud::DrawHudString(int xpos, int ypos, int iMaxX, char *szIt, int r, int g, int b )
{
	// draw the string until we hit the null character or a newline character
	for ( ; *szIt != 0 && *szIt != '\n'; szIt++ )
	{
		int next = xpos + gHUD.m_scrinfo.charWidths[ *szIt ]; // variable-width fonts look cool
		if ( next > iMaxX )
			return xpos;

		TextMessageDrawChar( xpos, ypos, *szIt, r, g, b );
		xpos = next;		
	}

	return xpos;
}

int CHud::DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b )
{
	char szString[32];
	sprintf( szString, "%d", iNumber );
	return DrawHudStringReverse( xpos, ypos, iMinX, szString, r, g, b );

}

// draws a string from right to left (right-aligned)
int CHud::DrawHudStringReverse( int xpos, int ypos, int iMinX, char *szString, int r, int g, int b )
{
	// find the end of the string
	char *szIt;
	for ( szIt = szString; *szIt != 0; szIt++ )
	{ // we should count the length?		
	}

	// iterate throug the string in reverse
	for ( szIt--;  szIt != (szString-1);  szIt-- )	
	{
		int next = xpos - gHUD.m_scrinfo.charWidths[ *szIt ]; // variable-width fonts look cool
		if ( next < iMinX )
			return xpos;
		xpos = next;

		TextMessageDrawChar( xpos, ypos, *szIt, r, g, b );
	}

	return xpos;
}



//How long would this number be if drawn?
int CHud::DrawHUDNumber_widthOnly(int iFlags, int iNumber, int fontID){
	
	int iWidth = 0;

	int myFontIDZero = 0;

	if(fontID == 0){
		if(EASY_CVAR_GET(hud_version) == 0){
			myFontIDZero = m_HUD_e_number_0;
		}else{
			myFontIDZero = m_HUD_number_0;
		}
	}else if(fontID == 1){
		if(EASY_CVAR_GET(hud_version) == 0){
			myFontIDZero = m_HUD_e_number_0_health;
		}else{
			myFontIDZero = m_HUD_number_0_health;
		}
	}else if(fontID == 2){
		myFontIDZero = m_HUD_number_1_tiny;
	}
	//MODDD - altgui
	else if(fontID == 3){
		myFontIDZero = m_HUD_number_0_E3R;
	}else if(fontID == 4){
		//SAME.
		myFontIDZero = m_HUD_number_0_E3R;
	}

	
	iWidth = GetSpriteRect(myFontIDZero).right - GetSpriteRect(myFontIDZero).left;
	
	if(iNumber == 0){

	}

	if(iFlags & DHN_3DIGITS || iNumber >= 100){
		return iWidth*3;
	}else if(iFlags & DHN_2DIGITS || iNumber >= 10){
		return iWidth*2;
	}else{

		if(iNumber > 0){
			return iWidth;
		}else{
			if(iFlags & DHN_DRAWZERO){
				return iWidth;
			}
		}
		return 0;
		
	}

	//what?
	return 0;
}//END OF DrawHUDNumber_widthOnly



//MODDD - implementation of "DrawHudNumber" without the new last arg.  Imply that it is 0 (plain text).
int CHud::DrawHudNumber( int x, int y, int iFlags, int iNumber, const int& r, const int& g, const int& b){
	return DrawHudNumber(x, y, iFlags, iNumber, r, g, b, 0, 0);
}

int CHud::DrawHudNumber( int x, int y, int iFlags, int iNumber, const int& r, const int& g, const int& b, int fontID){
	return DrawHudNumber(x, y, iFlags, iNumber, r, g, b, fontID, 0);
}

//MODDD - first DrawHudNumber has a new argument: "fontID".
//For fontID, 0 = normal (ammo usually), boxed (health), and tiny (antidote HUD)
int CHud::DrawHudNumber( int x, int y, int iFlags, int iNumber, const int& r, const int& g, const int& b, int fontID, const int& canDrawBrokenTrans)
{
	int drawAltTransValue = 0;
	if(canDrawBrokenTrans){
		drawAltTransValue = 1;
	}

	int iWidth = 0;

	int myFontIDZero = 0;

	if (fontID == 0) {
		if (EASY_CVAR_GET(hud_version) == 0) {
			myFontIDZero = m_HUD_e_number_0;
		}
		else {
			myFontIDZero = m_HUD_number_0;
		}
	}
	else if (fontID == 1) {
		if (EASY_CVAR_GET(hud_version) == 0) {
			myFontIDZero = m_HUD_e_number_0_health;
		}
		else {
			myFontIDZero = m_HUD_number_0_health;
		}
	}
	else if(fontID == 2){
		myFontIDZero = m_HUD_number_1_tiny;
	}
	//MODDD - altgui
	else if(fontID == 3){
		myFontIDZero = m_HUD_number_0_E3R;
	}else if(fontID == 4){
		//SAME.
		myFontIDZero = m_HUD_number_0_E3R;
	}

	
	iWidth = GetSpriteRect(myFontIDZero).right - GetSpriteRect(myFontIDZero).left;
	
	int k;

	//MODDD - new var be more faded for filler "0" digits (transparency is melded into the numbers,
	//as the darker / closer to black they are, the more transparent they become).
	int fadedr = r;
	int fadedg = g;
	int fadedb = b;

	if( !(iFlags & (DHN_EMPTYDIGITSUNFADED) )) {
		//Fade filler 0 digits if the user hasn't told us to avoid doing that.
		//Without this action, "fadedr, g, b" remain the same as the starting r, g, b.
		ScaleColors(fadedr, fadedg, fadedb, 80);
		if(canDrawBrokenTrans){
			drawAltTransValue = 2;
		}
	}

	if(fontID != 2){
		if(iNumber > 0){
			// SPR_Draw 100's
			if (iNumber >= 100){
				k = iNumber/100;
				//MODDD
				drawAdditiveFilter(GetSprite(myFontIDZero + k), r, g, b, 0, x, y, &GetSpriteRect(myFontIDZero + k), canDrawBrokenTrans);
				x += iWidth;
			}
			else if (iFlags & (DHN_3DIGITS)){
				////SPR_DrawAdditive( 0, x, y, &rc );
				//x += iWidth;
				//MODDD - this means, draw the "0" anyways (always 3 digits)
				k = 0;
				//MODDD
			
				if(! (iFlags & (DHN_DRAWPLACE)) ){
					drawAdditiveFilter(GetSprite(myFontIDZero + k), fadedr, fadedg, fadedb, 0, x, y, &GetSpriteRect(myFontIDZero + k), drawAltTransValue);
				}
				x += iWidth;
				//////////////////////////////////////////////
			}

			// SPR_Draw 10's
			if (iNumber >= 10){
				k = (iNumber % 100)/10;
				//MODDD
				drawAdditiveFilter(GetSprite(myFontIDZero + k), r, g, b, 0, x, y, &GetSpriteRect(myFontIDZero + k), canDrawBrokenTrans);
				x += iWidth;
			}
			else if (iFlags & (DHN_3DIGITS | DHN_2DIGITS)){
				////SPR_DrawAdditive( 0, x, y, &rc );
				//x += iWidth;
				//MODDD - ditto, still draw the 2nd digit.
				k = 0;
				if(! (iFlags & (DHN_DRAWPLACE)) ){

					drawAdditiveFilter(GetSprite(myFontIDZero + k), fadedr, fadedg, fadedb, 0, x, y, &GetSpriteRect(myFontIDZero + k), drawAltTransValue);
				}
				x += iWidth;
			}
			// SPR_Draw ones
			k = iNumber % 10;
			drawAdditiveFilter(GetSprite(myFontIDZero + k), r, g, b, 0,  x, y, &GetSpriteRect(myFontIDZero + k), canDrawBrokenTrans);
			x += iWidth;
		}//END OF if (iNumber > 0)
		else if (iFlags & DHN_DRAWZERO){
			k = iNumber;
			int digits = 1;

			if(iFlags & (DHN_3DIGITS)){
				digits = 3;
			}else if(iFlags & (DHN_2DIGITS)){
				digits = 2;
			}

			//IF we're not just drawing a place, draw all the digit positions filled with 0's.
			if(!(iFlags & DHN_DRAWPLACE) ){
				//MODDD - create 3 zeroes with a simple loop.
				for(int i = 0; i < digits; i++){
					drawAdditiveFilter(GetSprite(myFontIDZero + k), fadedr, fadedg, fadedb, 0, x, y, &GetSpriteRect(myFontIDZero + k), drawAltTransValue);
					x += iWidth;
					k = iNumber;
				}
			//Otherwise, skip all digit positions but the one's place for a "0".
			}else{
				for(int i = 0; i < digits - 1; i++){
					x += iWidth;
				}
				drawAdditiveFilter(GetSprite(myFontIDZero + k), fadedr, fadedg, fadedb, 0, x, y, &GetSpriteRect(myFontIDZero + k), drawAltTransValue);
				x += iWidth;
				k = iNumber;
			}
		}//END OF else OF if (iFlags & DHN_DRAWZERO) 
	}//END OF if(fontID != 2)
	else{
		//This is the tiny font.  Only numbers 1 through 5 are written / could be ripped for it.
		//I assume "0" just means don't draw, and the syringes have a max of 5 for each type.
		k = iNumber;
		if(k >= 1 && k <= 6){
			//on drawing, apply an offset of - 1 (since the starting ID is for #1, not #0).
			drawAdditiveFilter(GetSprite(myFontIDZero + k - 1), r, g, b, 0, x, y, &GetSpriteRect(myFontIDZero + k - 1), canDrawBrokenTrans);
			x += iWidth;
		}
	}//END OF else OF if(myFontID != 2)
	return x;
}


int CHud::GetNumWidth( int iNumber, int iFlags )
{
	if (iFlags & (DHN_3DIGITS))
		return 3;

	if (iFlags & (DHN_2DIGITS))
		return 2;

	if (iNumber <= 0)
	{
		if (iFlags & (DHN_DRAWZERO))
			return 1;
		else
			return 0;
	}

	if (iNumber < 10)
		return 1;

	if (iNumber < 100)
		return 2;

	return 3;

}	


