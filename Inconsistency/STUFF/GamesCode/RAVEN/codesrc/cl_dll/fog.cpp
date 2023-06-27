/***+
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
// Ammo.cpp
//
// implementation of CHudAmmo class
//

#include "windows.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

#include <string.h>
#include <stdio.h>
#include "elightlist.h"

#include "pmtrace.h"
#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"
#include "pm_defs.h"
#include "fog.h"

#include "gl/gl.h"
#include "gl/glext.h"

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"

// Class declaration
CFog gFog;

// Fog color variable
vec3_t CFog::m_vFogColor;

int __MsgFunc_Fog( const char *pszName, int iSize, void *pBuf )
{
	return gFog.MsgFunc_Fog( pszName, iSize, pBuf );
}

/*
====================
Init

====================
*/
void CFog::Init( void )
{
	HOOK_MESSAGE( Fog );
}

/*
====================
VidInit

====================
*/
void CFog::VidInit( void )
{
	m_iEndDist = NULL;
	m_iStartDist = NULL;
}

/*
====================
RenderFog

====================
*/
void CFog::RenderFog( vec3_t color )
{
	if(!m_iStartDist && !m_iEndDist)
	{
		gEngfuncs.pTriAPI->Fog ( Vector(0, 0, 0), 0, 0, FALSE );
		//glDisable( GL_FOG );
		return;
	}

	glEnable( GL_FOG );
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_DENSITY, GL_ONE);
	glFogi(GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);

	glFogfv(GL_FOG_COLOR, color);
	glFogf(GL_FOG_START, m_iStartDist);
	glFogf(GL_FOG_END, m_iEndDist);

	// Tell the engine too
	gEngfuncs.pTriAPI->Fog ( color, m_iStartDist, m_iEndDist, TRUE );	
}

/*
====================
BlackFog

====================
*/
void CFog::BlackFog( void )
{
	static float fColorBlack[3] = {0,0,0};
	RenderFog(fColorBlack);

	if(!m_iStartDist && !m_iEndDist)
	{
		gEngfuncs.pTriAPI->Fog ( fColorBlack, m_iStartDist, m_iEndDist, FALSE );
		return;
	}

	//Not in water and we want fog.
	gEngfuncs.pTriAPI->Fog ( fColorBlack, m_iStartDist, m_iEndDist, TRUE );	
}

/*
====================
CalcRefDef

====================
*/
void CFog::CalcRefDef( ref_params_t* pparams )
{
	for(int i = 0; i < 3; i++)
	{
		m_vFogBBoxMin[i] = pparams->vieworg[i] - m_iEndDist;
		m_vFogBBoxMax[i] = pparams->vieworg[i] + m_iEndDist;
	}

	RenderFog();
}

/*
====================
CullFogBBox

====================
*/
bool CFog::CullFogBBox ( const vec3_t& mins, const vec3_t& maxs )
{
	if(!m_iStartDist && !m_iEndDist)
		return false;

	if (mins[0] > m_vFogBBoxMax[0]) 
		return true;

	if (mins[1] > m_vFogBBoxMax[1]) 
		return true;

	if (mins[2] > m_vFogBBoxMax[2]) 
		return true;

	if (maxs[0] < m_vFogBBoxMin[0]) 
		return true;

	if (maxs[1] < m_vFogBBoxMin[1]) 
		return true;

	if (maxs[2] < m_vFogBBoxMin[2]) 
		return true;

	return false;
}

/*
====================
MsgFunc_Fog

====================
*/
int CFog::MsgFunc_Fog( const char *pszName, int iSize, void *pBuf )
{
	BEGIN_READ(pBuf, iSize);

	m_iStartDist = READ_COORD();
	m_iEndDist = READ_COORD();

	for(int i = 0; i < 3; i++)
		m_vFogColor[i] = (float)READ_BYTE()/255.0f;

	return 1;
}