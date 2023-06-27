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
// Train.cpp
//
// implementation of CHudAmmo class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"

DECLARE_MESSAGE(m_Train, Train )

//MODDD - externs
EASY_CVAR_EXTERN(hud_version)


int CHudTrain::Init(void)
{
	HOOK_MESSAGE( Train );

	m_iPos = 0;
	m_iFlags = 0;
	gHUD.AddHudElem(this);

	return 1;
};

int CHudTrain::VidInit(void)
{
	m_SpriteHandle_t = 0;

	return 1;
};

int CHudTrain::Draw(float fTime)
{
	//MODDD - if the weapon select menu is on, can't draw the lower portion of the HUD.
	if(!gHUD.canDrawBottomStats){
		return 1;
	}

	if ( !m_SpriteHandle_t )
		m_SpriteHandle_t = LoadSprite("sprites/%d_train.spr");

	if (m_iPos)
	{
		int r, g, b, x, y;

		//MODDD - use generic GUI color.
		//UnpackRGB(r,g,b, RGB_YELLOWISH);
		gHUD.getGenericGUIColor(r, g, b);
		SPR_Set(m_SpriteHandle_t, r, g, b );


		//88, 48from bottom


		if(EASY_CVAR_GET(hud_version) < 3){
			// This should show up to the right and part way up the armor number
			y = ScreenHeight - SPR_Height(m_SpriteHandle_t,0) - gHUD.m_iFontHeight;
			//x = ScreenWidth/3 + SPR_Width(m_SpriteHandle_t,0)/4;
			//MODDD - closer to health.
			int healthWidth = gHUD.m_iFontWidthAlt;
			x = (healthWidth*4.5);

		}else{
			x = 216 - 50;
			y = ScreenHeight - 109 + 10;
		}

		//m_iPos KEY:
		//0: off?
		//1: stopped
		//2: forward 1
		//3: forward 2
		//4: forward 3 (also for speed forwards 4)
		//5: backwards 1 (also for speed backwards 2 3 4)
		SPR_DrawAdditive( m_iPos - 1,  x, y, NULL);

	}

	return 1;
}


int CHudTrain::MsgFunc_Train(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	// update Train data
	m_iPos = READ_BYTE();

	if (m_iPos)
		m_iFlags |= HUD_ACTIVE;
	else
		m_iFlags &= ~HUD_ACTIVE;

	return 1;
}
