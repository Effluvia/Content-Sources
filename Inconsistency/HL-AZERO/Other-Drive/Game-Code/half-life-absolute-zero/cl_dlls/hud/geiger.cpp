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
// Geiger.cpp
//
// implementation of CHudAmmo class
//

#include "external_lib_include.h"

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_Geiger, Geiger )

int CHudGeiger::Init(void)
{
	HOOK_MESSAGE( Geiger );

	m_iGeigerRange = 0;
	m_iFlags = 0;

	gHUD.AddHudElem(this);

	srand( (unsigned)time( NULL ) );

	return 1;
};

int CHudGeiger::VidInit(void)
{
	return 1;
};

int CHudGeiger::MsgFunc_Geiger(const char *pszName,  int iSize, void *pbuf)
{

	BEGIN_READ( pbuf, iSize );

	// update geiger data
	m_iGeigerRange = READ_BYTE();
	m_iGeigerRange = m_iGeigerRange << 2;
	
	m_iFlags |= HUD_ACTIVE;

	return 1;
}

int CHudGeiger::Draw (float flTime)
{
	//MODDD - dummied.
	// This has been moved to player.cpp for better control over channels
	// (method 'UpdateGeigerCounter').
	return 1;
}
