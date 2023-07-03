//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "windows.h"
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

#include "pmtrace.h"
#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"
#include "pm_defs.h"
#include "elightlist.h"
#include "svdformat.h"
#include "svd_render.h"
#include "fog.h"

// Quake definitions
#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40
#define SURF_UNDERWATER		0x80
#define SURF_DONTWARP		0x100
#define BACKFACE_EPSILON	0.01

// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2

// Globals used by shadow rendering
model_t*	g_pWorld;
int			g_visFrame;
int			g_frameCount;
vec3_t		g_viewOrigin;

/*
====================
Mod_PointInLeaf

====================
*/
mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model) // quake's func
{
	mnode_t *node = model->nodes;
	while (1)
	{
		if (node->contents < 0)
			return (mleaf_t *)node;
		mplane_t *plane = node->plane;
		float d = DotProduct (p,plane->normal) - plane->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}

	return NULL;	// never reached
}

/*
====================
SVD_RecursiveDrawWorld

====================
*/
void SVD_RecursiveDrawWorld ( mnode_t *node )
{
	if (node->contents == CONTENTS_SOLID)
		return;

	if (node->visframe != g_visFrame)
		return;
	
	if (node->contents < 0)
		return;		// faces already marked by engine

	// recurse down the children, Order doesn't matter
	SVD_RecursiveDrawWorld (node->children[0]);
	SVD_RecursiveDrawWorld (node->children[1]);

	// draw stuff
	int c = node->numsurfaces;
	if (c)
	{
		msurface_t	*surf = g_pWorld->surfaces + node->firstsurface;

		for ( ; c ; c--, surf++)
		{
			if (surf->visframe != g_frameCount)
				continue;

			if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB|SURF_UNDERWATER))
				continue;

			glpoly_t *p = surf->polys;
			float *v = p->verts[0];

			glBegin (GL_POLYGON);			
			for (int i = 0; i < p->numverts; i++, v+= VERTEXSIZE)
			{
				glTexCoord2f (v[3], v[4]);
				glVertex3fv (v);
			}
			glEnd ();
		}
	}
}

/*
====================
SVD_CalcRefDef

====================
*/
void SVD_CalcRefDef ( ref_params_t* pparams )
{
	if(IEngineStudio.IsHardware() != 1)
		return;

	SVD_CheckInit();

	if(g_StudioRenderer.m_pCvarDrawShadows->value < 2)
		return;

	glClear( GL_STENCIL_BUFFER_BIT );

	g_StudioRenderer.StudioSetBuffer();
}

/*
====================
SVD_DrawNormalTriangles

====================
*/
void SVD_DrawNormalTriangles ( void )
{
	if(g_StudioRenderer.m_pCvarDrawShadows->value < 2)
		return;

	if(IEngineStudio.IsHardware() != 1)
		return;

	glPushAttrib(GL_TEXTURE_BIT);

	// Draw black fog
	gFog.RenderFog();

	// buz: workaround half-life's bug, when multitexturing left enabled after
	// rendering brush entities
	g_StudioRenderer.glActiveTexture( GL_TEXTURE1_ARB );
	glDisable(GL_TEXTURE_2D);
	g_StudioRenderer.glActiveTexture( GL_TEXTURE0_ARB );

	glDepthMask(GL_FALSE);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(GL_ZERO, GL_ZERO, GL_ZERO, 0.5);

	glStencilFunc(GL_NOTEQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_STENCIL_TEST);

	// Set view origin
	g_viewOrigin = g_StudioRenderer.m_vRenderOrigin;

	// get current visframe number
	g_pWorld = IEngineStudio.GetModelByIndex(1);
	mleaf_t *pleaf = Mod_PointInLeaf ( g_StudioRenderer.m_vRenderOrigin, g_pWorld );
	g_visFrame = pleaf->visframe;

	// get current frame number
	g_frameCount = g_StudioRenderer.m_nFrameCount;

	// draw world
	SVD_RecursiveDrawWorld( g_pWorld->nodes );

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);

	g_StudioRenderer.StudioClearBuffer();
	glPopAttrib();
}