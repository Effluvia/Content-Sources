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
// modelcamera.cpp
//
// implementation of CModelCamera class
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

#include "ref_params.h"
#include "modelcamera.h"

#include "com_model.h"
#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#ifndef RAD2DEG
	#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI) )
#endif

#define MOUSEMOVE_TIMEOUT_BLEND 2

int __MsgFunc_ModelCamera(const char *pszName, int iSize, void *pbuf )
{
	return gModelCamera.MsgFunc_ModelCamera( pszName, iSize, pbuf );
}

// Class declaration
CModelCamera gModelCamera;

/*
====================
Init

====================
*/
void CModelCamera::Init( void )
{
	HOOK_MESSAGE( ModelCamera );
}

/*
====================
VidInit

====================
*/
void CModelCamera::VidInit( void )
{
	m_pViewEntity = NULL;
	m_bActive = false;

	m_flDeviationTimeout = 0;
	m_flMaxDeviationX = 0;
	m_flMaxDeviationY = 0;

	ResetValues();
}

/*
====================
ResetValues

====================
*/
void CModelCamera::ResetValues( void )
{
	m_flLastMouseMove = 0;

	VectorClear(m_vAddDeviationAngles);
	VectorClear(m_vDeviationAngles);
}

/*
====================
MsgFunc_ModelCamera

====================
*/
int CModelCamera::MsgFunc_ModelCamera( const char *pszName, int iSize, void *pBuf )
{
	BEGIN_READ( pBuf, iSize );

	int entIndex = READ_SHORT();
	if( !entIndex )
	{
		m_pViewEntity = NULL;
		m_bActive = false;
		return 1;
	}

	for(int i = 0; i < NUM_ATTACHMENTS; i++)
		m_iAttachments[i] = READ_BYTE();

	m_pViewEntity = gEngfuncs.GetEntityByIndex( entIndex );
	if(!m_pViewEntity)
	{
		m_pViewEntity = NULL;
		m_bActive = false;
	}

	m_flDeviationTimeout = READ_COORD();
	m_flMaxDeviationX = READ_COORD();
	m_flMaxDeviationY = READ_COORD();

	// Reset everything to basics
	ResetValues();

	m_bActive = true;
	return 1;
}

/*
====================
CalcRefDef

====================
*/
void CModelCamera::CalcRefDef( ref_params_t* pparams )
{
	vec3_t up, left, forward;
	vec3_t vorigin, vtemp;

	g_StudioRenderer.UpdateAttachments(m_pViewEntity);
	VectorCopy(m_pViewEntity->attachment[m_iAttachments[ATTACH_ORIGIN]], vorigin);

	// Get forward
	VectorSubtract(m_pViewEntity->attachment[m_iAttachments[ATTACH_FORWARD]], m_pViewEntity->attachment[m_iAttachments[ATTACH_ORIGIN]], forward);
	VectorNormalize(forward);

	// Get left
	VectorSubtract(m_pViewEntity->attachment[m_iAttachments[ATTACH_LEFT]], m_pViewEntity->attachment[m_iAttachments[ATTACH_ORIGIN]], left);
	VectorNormalize(left);

	// Get the viewing angle
	vec3_t vangles = VecToAngles(forward, left);

	// degrate the view add value
	if(m_flDeviationTimeout && (m_flMaxDeviationX || m_flMaxDeviationY))
	{
		if(m_flLastMouseMove && (m_flLastMouseMove+m_flDeviationTimeout) <= pparams->time)
		{
			float time = min(MOUSEMOVE_TIMEOUT_BLEND, ( (pparams->time-(m_flLastMouseMove+m_flDeviationTimeout)) ));
			float flfrac = SplineFraction( time, (1.0/MOUSEMOVE_TIMEOUT_BLEND) );

			if(flfrac >= 1.0)
			{
				VectorClear(m_vDeviationAngles);
				m_flLastMouseMove = NULL;
			}

			VectorScale(m_vDeviationAngles, (1.0-flfrac), m_vAddDeviationAngles);
		}
		else if(m_flLastMouseMove)
		{
			VectorCopy(m_vDeviationAngles, m_vAddDeviationAngles);
		}
	}

	VectorAdd( vangles, m_vAddDeviationAngles, vangles );

	if(vangles[0] > 360) vangles[0] -= 360;
	if(vangles[0] < -360) vangles[0] += 360;

	if(vangles[1] > 360) vangles[1] -= 360;
	if(vangles[1] < -360) vangles[1] += 360;

	if(vangles[2] > 360) vangles[2] -= 360;
	if(vangles[2] < -360) vangles[2] += 360;

	VectorCopy(vorigin, pparams->vieworg);
	VectorCopy(vangles, pparams->viewangles);
	
	AngleVectors(pparams->viewangles, pparams->forward, pparams->right, pparams->up);
}

/*
====================
IsActive

====================
*/
bool CModelCamera::IsActive( void )
{
	return m_bActive;
}

/*
==================
VecToAngles

==================
*/
vec3_t CModelCamera::VecToAngles( vec3_t forward, vec3_t left )
{
	vec3_t angles;
	float xyDist = sqrtf( forward[0] * forward[0] + forward[1] * forward[1] );
	if ( xyDist > 0.001f )
	{
		angles[1] = RAD2DEG( atan2f( forward[1], forward[0] ) );
		angles[0] = RAD2DEG( atan2f( -forward[2], xyDist ) );

		float up_z = (left[1] * forward[0]) - (left[0] * forward[1]);
		angles[2] = RAD2DEG( atan2f( left[2], up_z ) );
	}
	else
	{
		angles[1] = RAD2DEG( atan2f( -left[0], left[1] ) );
		angles[0] = RAD2DEG( atan2f( -forward[2], xyDist ) );
		angles[2] = 0;
	}	

	return angles;
}

/*
=====================
MouseMove

=====================
*/
void CModelCamera::MouseMove( float mousex, float mousey )
{
	if(!m_flDeviationTimeout || !m_flMaxDeviationX && !m_flMaxDeviationY) 
		return;
	
	if(!mousex && !mousey)
		return;

	if(m_flLastMouseMove+m_flDeviationTimeout < gEngfuncs.GetClientTime())
		VectorCopy(m_vAddDeviationAngles, m_vDeviationAngles);

	m_vDeviationAngles.x += mousey; // wtf this is switched for some reason
	if(m_vDeviationAngles.x > m_flMaxDeviationX) m_vDeviationAngles.x = m_flMaxDeviationX;
	if(m_vDeviationAngles.x < -m_flMaxDeviationX) m_vDeviationAngles.x = -m_flMaxDeviationX;

	m_vDeviationAngles.y -= mousex;
	if(m_vDeviationAngles.y > m_flMaxDeviationY) m_vDeviationAngles.y = m_flMaxDeviationY;
	if(m_vDeviationAngles.y < -m_flMaxDeviationY) m_vDeviationAngles.y = -m_flMaxDeviationY;

	m_flLastMouseMove = gEngfuncs.GetClientTime();
}

/*
==================
SplineFraction

==================
*/
float CModelCamera::SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}