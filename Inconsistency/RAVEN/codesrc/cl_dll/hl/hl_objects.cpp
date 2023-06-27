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
#include "../hud.h"
#include "../cl_util.h"
#include "../demo.h"

#include "demo_api.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"

#include "pm_defs.h"
#include "event_api.h"
#include "entity_types.h"
#include "r_efx.h"

#include "com_model.h"
#include "../studio_util.h"
#include "r_studioint.h"

#include "../StudioModelRenderer.h"
#include "../GameStudioModelRenderer.h"
#include "../elightlist.h"

extern BEAM *pBeam;
extern BEAM *pBeam2;

void UpdateBeams ( void )
{
	vec3_t forward, vecSrc, vecEnd, origin, angles, right, up;
	vec3_t view_ofs;
	pmtrace_t tr;
	cl_entity_t *pthisplayer = gEngfuncs.GetLocalPlayer();
	int idx = pthisplayer->index;
		
	// Get our exact viewangles from engine
	gEngfuncs.GetViewAngles( (float *)angles );

	// Determine our last predicted origin
	origin = gHUD.m_vecOrigin;

	AngleVectors( angles, forward, right, up );

	VectorCopy( origin, vecSrc );
	
	VectorMA( vecSrc, 2048, forward, vecEnd );

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );	
						
	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();
					
	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( idx - 1 );	

	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr );

	gEngfuncs.pEventAPI->EV_PopPMStates();

	if ( pBeam )
	{
		pBeam->target = tr.endpos;
		pBeam->die	  = gEngfuncs.GetClientTime() + 0.1; // We keep it alive just a little bit forward in the future, just in case.
	}
		
	if ( pBeam2 )
	{
		pBeam2->target = tr.endpos;
		pBeam2->die	   = gEngfuncs.GetClientTime() + 0.1; // We keep it alive just a little bit forward in the future, just in case.
	}
}

void UpdateScopedRifleBeam ( cl_entity_t *pView )
{
	// Refresh attachments
	g_StudioRenderer.UpdateAttachments(pView);

	vec3_t vecOrigin, vecForward, vecEnd;
	
	VectorCopy(pView->attachment[1], vecOrigin);
	VectorSubtract(pView->attachment[2], vecOrigin, vecForward);
	VectorNormalize(vecForward);

	VectorMA(vecOrigin, 8196, vecForward, vecEnd);

	// Pointer to local player
	cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();

	// Do a trace to see where it impacts
	pmtrace_t tr;

	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecOrigin, vecEnd, PM_NORMAL, pPlayer->index, &tr );

	// Set beam
	int beamModelIdx = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/laserbeam.spr" );

	if( !pBeam || pBeam->die < gEngfuncs.GetClientTime() )
		pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint ( pPlayer->index | 0x2000, tr.endpos, beamModelIdx, gEngfuncs.GetClientTime() + 0.1, 0.2, 0, 0.25, 10, 0, 0, 255, 0, 0 );

	if( pBeam )
	{
		pBeam->target = tr.endpos;
		pBeam->die = gEngfuncs.GetClientTime() + 0.01; // We keep it alive just a little bit forward in the future, just in case.
	}
}

/*
=====================
Game_AddObjects

Add game specific, client-side objects here
=====================
*/
void Game_AddObjects( void )
{
	if( pBeam && pBeam2 )
		UpdateBeams();
}

/*
=====================
Game_UpdateScopeBeam

Add game specific, client-side objects here
=====================
*/
void Game_UpdateScopeBeam( void )
{
	cl_entity_t *pView = gEngfuncs.GetViewModel();
	if(!pView || !pView->model)
		return;

	if( !strcmp(pView->model->name, "models/v_scopedrifle.mdl") )
	{
		UpdateScopedRifleBeam( pView );

		// Add a small elight
		cl_entity_t* pView = gEngfuncs.GetViewModel();
		gELightList.AddEntityLight( pView->index, pView->attachment[1], Vector(1.0, 0.0, 0.0), 5, false );
	}
	else if ( pBeam && pBeam->die > gEngfuncs.GetClientTime() )
	{
		pBeam->die = 0;
		pBeam = NULL;
	}
}
