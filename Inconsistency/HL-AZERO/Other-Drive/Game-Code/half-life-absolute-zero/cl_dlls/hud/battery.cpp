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
// battery.cpp
//
// implementation of CHudBattery class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE(m_Battery, Battery)


//MODDD - externs
EASY_CVAR_EXTERN(hud_version)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber)
EASY_CVAR_EXTERN(hud_batterydraw)
EASY_CVAR_EXTERN(hud_weaponselecthideslower)
EASY_CVAR_EXTERN(blastExtraArmorDamageMode)


int CHudBattery::Init(void)
{
	m_iBat = 0;
	m_fFade = 0;
	m_iFlags = 0;

	HOOK_MESSAGE(Battery);

	gHUD.AddHudElem(this);

	return 1;
};


int CHudBattery::VidInit(void)
{
	int HUD_suit_empty = gHUD.GetSpriteIndex( "suit_empty" );
	int HUD_suit_full = gHUD.GetSpriteIndex( "suit_full" );


	//MODDD - initialize new index
	m_HUD_battery_empty = gHUD.GetSpriteIndex( "battery_empty" );
	m_HUD_battery_full = gHUD.GetSpriteIndex( "battery_full" );

	m_SpriteHandle_t1 = m_SpriteHandle_t2 = 0;  // delaying get sprite handles until we know the sprites are loaded
	


	//not needed here?
	//alphaCrossHairIndex = gHUD.GetSpriteIndex( "alphacrosshair" );


	//MODDD - now refers to the battery instead.
	/*
	m_prc1 = &gHUD.GetSpriteRect( HUD_suit_empty );
	m_prc2 = &gHUD.GetSpriteRect( HUD_suit_full );
	*/

	m_prc1 = &gHUD.GetSpriteRect( m_HUD_battery_empty );
	m_prc2 = &gHUD.GetSpriteRect( m_HUD_battery_full );

	//MODDD - changed
	//m_iHeight = m_prc2->bottom - m_prc1->top;
	m_iHeight = m_prc2->bottom - m_prc2->top;
	m_fFade = 0;
	return 1;
};

int CHudBattery::MsgFunc_Battery(const char *pszName,  int iSize, void *pbuf )
{
	m_iFlags |= HUD_ACTIVE;

	
	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();

	if (x != m_iBat)
	{
		m_fFade = FADE_TIME;
		m_iBat = x;
	}

	return 1;
}




int CHudBattery::Draw(float flTime)
{
	if(gHUD.frozenMem ){
		//Don't draw while frozen.
		return 0;
	}


	//MODDD - only draw if the weapon select screen is not on.
	if( (!gHUD.canDrawBottomStats && EASY_CVAR_GET(hud_weaponselecthideslower) == 1) ){
		return 1;
	}

	if ( gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH )
		return 1;


	if (EASY_CVAR_GET(hud_version) <= 1) {
		// versions 0 and 1 do not draw the battery here, already handled in
		// weapon sidebar
		return 1;
	}

	int iBatDraw;
	int r, g, b, x, y, a;

	//MODDD - not quite that simple now.  Use formula, factoring in "m_iHealth",
	//        AFTER we get the alpha from looking at "m_fFade":
	//UnpackRGB(r,g,b, RGB_YELLOWISH);
	

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return 1;


	//MODDD - if "batteryHiddenDead" is on and the player is dead, hide the battery amount (show 0).
	if (!gHUD.m_fPlayerDead || EASY_CVAR_GET(hud_batteryhiddendead) != 1) {
		iBatDraw = m_iBat;
	}
	else {
		// dead and the CVar demands hiding battery?  Show 0 here.
		iBatDraw = 0;
	}



	// Has health changed? Flash the health #
	if (m_fFade)
	{
		if (m_fFade > FADE_TIME)
			m_fFade = FADE_TIME;

		m_fFade -= (gHUD.m_flTimeDelta * 20);
		if (m_fFade <= 0)
		{
			a = 128;
			m_fFade = 0;
		}

		// Fade the health number back to dim
		a = MIN_ALPHA +  (m_fFade/FADE_TIME) * 128;
	}
	else {
		a = MIN_ALPHA;
	}



	//MODDD - Not sure what to do here.  Always make the alpha 255 or rely on pain colors?
	//ScaleColors(r, g, b, a );

	int iOffset = (m_prc1->bottom - m_prc1->top)/6;


	if(EASY_CVAR_GET(hud_version) < 3){

		//MODDD - here to use the health colors.
		gHUD.m_Health.deriveColorFromHealth(r, g, b, a);
		//Note that "deriveColorFromHealth" does not factor in alpha in the pre-E3 GUI.
		//Must be afterwards if necessary.

		//MODDD - at death, fade.
		if(gHUD.m_fPlayerDead){
			a = 68;
			ScaleColors(r, g, b, a);
		}

		//int HealthHeight = gHUD.m_iFontHeightAlt;
		int BatteryHeight = gHUD.GetSpriteRect(m_HUD_battery_empty).bottom - gHUD.GetSpriteRect(m_HUD_battery_empty).top;


		//MODDD - for debugging the battery's real value with a bit more accuracy than "about 1/3".
		if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber) == 1){
			y = ScreenHeight - ((int)(BatteryHeight*4)) + 3 - 28;
			x = (gHUD.m_iFontWidthAlt)/2 + 5;
			gHUD.DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWZERO, m_iBat, r, g, b, 0, 1);
		}

		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//BATTERY DRAWING COORDS HERE. The rest will be relative to these!!!

		//y = ScreenHeight - ((int)(HealthHeight*1.5)) - ((int) BatteryHeight*1.5);
		y = ScreenHeight - ((int)(BatteryHeight*4)) + 3 + 8;

		x = (gHUD.m_iFontWidthAlt)/2 - 1;



		//Also, the "x" above is just the width of the 0 boxed number divided by 2.  This lines it up to the left 
		//with the health below.

		//MODDD - replacing with the battery icon.
		/*
		// make sure we have the right sprite handles
		if ( !m_SpriteHandle_t1 )
			m_SpriteHandle_t1 = gHUD.GetSprite( gHUD.GetSpriteIndex( "suit_empty" ) );
		if ( !m_SpriteHandle_t2 )
			m_SpriteHandle_t2 = gHUD.GetSprite( gHUD.GetSpriteIndex( "suit_full" ) );

		*/


		//ALWAYS update to be safe.
		//if ( !m_SpriteHandle_t1 )
		m_SpriteHandle_t1 = gHUD.GetSprite( m_HUD_battery_empty );
		//if ( !m_SpriteHandle_t2 )
		m_SpriteHandle_t2 = gHUD.GetSprite( m_HUD_battery_full );

		//MODDD - important to make sure..
		m_prc1 = &gHUD.GetSpriteRect( m_HUD_battery_empty );
		m_prc2 = &gHUD.GetSpriteRect( m_HUD_battery_full );


		int emptyBatteryX = x;
		int emptyBatteryY = y - iOffset;

		gHUD.attemptDrawBrokenTrans(emptyBatteryX, emptyBatteryY, m_prc1->right-m_prc1->left+3, m_prc1->bottom-m_prc1->top);

		// no trans yet.  Do it after everything else...
		gHUD.drawAdditiveFilter(m_SpriteHandle_t1, r, g, b, 0,  emptyBatteryX, emptyBatteryY, m_prc1, 0);

		x += 5;
		y += 5;



		if(EASY_CVAR_GET(hud_batterydraw) == 0){
			//across.
			float fillPortion = ((float)gHUD.m_Battery.m_iBat / 100.0f) * (0.98 - 0.02) + 0.02;
			gHUD.drawPartialFromLeft(m_SpriteHandle_t2, m_prc2, fillPortion, x, y - iOffset , r, g, b);
		}else if(EASY_CVAR_GET(hud_batterydraw) == 1){
			//vertically.
			float fillPortion = ((float)gHUD.m_Battery.m_iBat / 100.0f) * (0.95 - 0.05) + 0.05;
			gHUD.drawPartialFromBottom(m_SpriteHandle_t2, m_prc2, fillPortion, x, y - iOffset , r, g, b);
		}else if(EASY_CVAR_GET(hud_batterydraw) == 2){
			float fillPortion = ((float)gHUD.m_Battery.m_iBat / 100.0f) * (1.00 - 0.00) + 0.00;
			gHUD.drawPartialFromRight(m_SpriteHandle_t2, m_prc2, fillPortion, x, y - iOffset , r, g, b);
		}
		
	}else{
		//a = 255;
		gHUD.getGenericOrangeColor(r, g, b);

		iOffset = 0;

		//MODDD - for debugging the battery's real value with a bit more accuracy than "about 1/3".
		if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(forceDrawBatteryNumber) == 1){

			//gHUD.m_Health.deriveColorFromHealth(r, g, b, a);
			gHUD.getGenericOrangeColor(r, g, b);
			ScaleColors(r, g, b, a);
			x = 123 - 5 - 24 + 5;
			y = ScreenHeight - 52 - 5 + 11 - 29;
			gHUD.DrawHudNumber(x, y, DHN_3DIGITS|DHN_DRAWPLACE|DHN_DRAWZERO|DHN_EMPTYDIGITSUNFADED, m_iBat, r, g, b, 4);
		}

		//MODDD - at death, fade.
		if(gHUD.m_fPlayerDead){
			a = 68;
		}


		x = 123 - 5 - 24;
		y = ScreenHeight - 52 - 5 + 11;

		//ALWAYS update to be safe.
		//if ( !m_SpriteHandle_t1 )
		m_SpriteHandle_t1 = gHUD.GetSprite( gHUD.m_HUD_battery_empty_E3 );
		//if ( !m_SpriteHandle_t2 )
		m_SpriteHandle_t2 = gHUD.GetSprite( gHUD.m_HUD_battery_full_E3 );

		//MODDD - important to make sure..
		m_prc1 = &gHUD.GetSpriteRect( gHUD.m_HUD_battery_empty_E3 );
		m_prc2 = &gHUD.GetSpriteRect( gHUD.m_HUD_battery_full_E3 );


		gHUD.drawAdditiveFilter(m_SpriteHandle_t1, r, g, b, 0,  x, y - iOffset, m_prc1);

		x += 9;
		y += 9;
		//(float)(100-(min(100,iBatDraw))) * 0.01

		if(EASY_CVAR_GET(hud_batterydraw) == 0){
			//across.
			gHUD.drawPartialFromLeft(m_SpriteHandle_t2, m_prc2, ((float)iBatDraw / (float)100 ), x, y, r, g, b);
		}else{
			//vertically.
			gHUD.drawPartialFromBottom(m_SpriteHandle_t2, m_prc2, ((float)iBatDraw / (float)100 ), x, y, r, g, b);
		}


	}//END OF hud_version CHECK

	//alphaCrossHairIndex

	//MODDD - don't draw suit battery number.
	/*
	x += (m_prc1->right - m_prc1->left);
	x = gHUD.DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWZERO, m_iBat, r, g, b);
	*/

	return 1;
}