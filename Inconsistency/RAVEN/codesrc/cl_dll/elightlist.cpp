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

#include "elightlist.h"

// Class declaration
CELightList gELightList;

// Testing function
void __CmdFunc_MakeLight( void )
{
	vec3_t lightColor;
	int entIndex = atoi(gEngfuncs.Cmd_Argv(1));
	lightColor[0] = atof(gEngfuncs.Cmd_Argv(2))/255.0f;
	lightColor[1] = atof(gEngfuncs.Cmd_Argv(3))/255.0f;
	lightColor[2] = atof(gEngfuncs.Cmd_Argv(4))/255.0f;
	float radius = atof(gEngfuncs.Cmd_Argv(5));

	gELightList.AddEntityLight(entIndex, gHUD.m_vecOrigin, lightColor, radius, false);
}

int __MsgFunc_ELight( const char *pszName, int iSize, void *pBuf )
{
	return gELightList.MsgFunc_ELight( pszName, iSize, pBuf );
}

/*
====================
Init

====================
*/
void CELightList::Init( void )
{
	gEngfuncs.pfnAddCommand("make_light", __CmdFunc_MakeLight);

	HOOK_MESSAGE(ELight);
}

/*
====================
VidInit

====================
*/
void CELightList::VidInit( void )
{
	memset(m_pEntityLights, 0, sizeof(m_pEntityLights));
	m_iNumEntityLights = NULL;

	memset(m_pTempEntityLights, 0, sizeof(m_pTempEntityLights));
	m_iNumTempEntityLights = NULL;

	// Get pointer to first elight
	m_pGoldSrcELights = gEngfuncs.pEfxAPI->CL_AllocElight(0);
}

/*
====================
AddEntityLight

====================
*/
void CELightList::AddEntityLight( int entindex, const vec3_t& origin, const vec3_t& color, float radius, bool isTemporary )
{
	elight_t* plight = NULL;
	for(int i = 0; i < m_iNumEntityLights; i++)
	{
		if(m_pEntityLights[i].entindex == entindex)
		{
			plight = &m_pEntityLights[i];
			break;
		}
	}

	if(!plight)
	{
		if(!isTemporary)
		{
			if(m_iNumEntityLights == MAX_ENTITY_LIGHTS)
				return;

			plight = &m_pEntityLights[m_iNumEntityLights];
			m_iNumEntityLights++;
		}
		else
		{
			if(m_iNumTempEntityLights == MAX_GOLDSRC_ELIGHTS)
				return;

			plight = &m_pTempEntityLights[m_iNumTempEntityLights];
			m_iNumTempEntityLights++;
		}
	}

	plight->origin = origin;
	plight->color = color;
	plight->radius = radius;
	plight->entindex = entindex;
	plight->temporary = isTemporary;

	for(int i = 0; i < 3; i++)
	{
		plight->mins[i] = plight->origin[i] - plight->radius;
		plight->maxs[i] = plight->origin[i] + plight->radius;
	}
}

/*
====================
GetLightList

====================
*/
void CELightList::GetLightList( vec3_t& origin, const vec3_t& mins, const vec3_t& maxs, elight_t** lightArray, unsigned int* numLights )
{
	// Set this to zero
	*numLights = NULL;

	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );

	for(int i = 0; i < m_iNumEntityLights; i++)
	{
		if((*numLights) == MAX_MODEL_ENTITY_LIGHTS)
			return;

		elight_t* plight = &m_pEntityLights[i];

		if(CheckBBox(plight, mins, maxs))
			continue;

		static pmtrace_t traceResult;
		gEngfuncs.pEventAPI->EV_PlayerTrace( (float *)origin, (float *)plight->origin, PM_WORLD_ONLY, -1, &traceResult );
		if(traceResult.fraction != 1.0 || traceResult.allsolid || traceResult.startsolid)
			continue;

		lightArray[*numLights] = plight;
		(*numLights)++;
	}

	for(int i = 0; i < m_iNumTempEntityLights; i++)
	{
		if((*numLights) == MAX_MODEL_ENTITY_LIGHTS)
			return;

		elight_t* plight = &m_pTempEntityLights[i];

		if(CheckBBox(plight, mins, maxs))
			continue;

		static pmtrace_t traceResult;
		gEngfuncs.pEventAPI->EV_PlayerTrace( (float *)origin, (float *)plight->origin, PM_WORLD_ONLY, -1, &traceResult );
		if(traceResult.fraction != 1.0 || traceResult.allsolid || traceResult.startsolid)
			continue;

		lightArray[*numLights] = plight;
		(*numLights)++;
	}
}

/*
====================
CheckBBox

====================
*/
bool CELightList::CheckBBox( elight_t* plight, const vec3_t& vmins, const vec3_t& vmaxs )
{
	if (vmins[0] > plight->maxs[0]) 
		return true;

	if (vmins[1] > plight->maxs[1]) 
		return true;

	if (vmins[2] > plight->maxs[2]) 
		return true;

	if (vmaxs[0] < plight->mins[0]) 
		return true;

	if (vmaxs[1] < plight->mins[1]) 
		return true;

	if (vmaxs[2] < plight->mins[2]) 
		return true;

	return false;
}

/*
====================
CalcRefDef

====================
*/
void CELightList::CalcRefDef( void )
{
	// Reset to zero
	m_iNumTempEntityLights = 0;

	float fltime = gEngfuncs.GetClientTime();

	dlight_t* pdlight = m_pGoldSrcELights;
	for(int i = 0; i < MAX_GOLDSRC_ELIGHTS; i++, pdlight++)
	{
		if(!pdlight->radius || pdlight->die < fltime)
			continue;

		vec3_t lightColor;
		lightColor.x = (float)pdlight->color.r/255.0f;
		lightColor.y = (float)pdlight->color.g/255.0f;
		lightColor.z = (float)pdlight->color.b/255.0f;

		AddEntityLight( pdlight->key, pdlight->origin, lightColor, pdlight->radius*8, true );
	}
}

/*
====================
MsgFunc_ELight

====================
*/
int CELightList::MsgFunc_ELight( const char *pszName, int iSize, void *pBuf )
{
	BEGIN_READ(pBuf, iSize);

	int entindex = READ_SHORT();

	vec3_t origin;
	for(int i = 0; i < 3; i++)
		origin[i] = READ_COORD();

	vec3_t color;
	for(int i = 0; i < 3; i++)
		color[i] = (float)READ_BYTE()/255.0f;

	float radius = READ_COORD()*9.5;
	AddEntityLight(entindex, origin, color, radius, false);

	return 1;
}