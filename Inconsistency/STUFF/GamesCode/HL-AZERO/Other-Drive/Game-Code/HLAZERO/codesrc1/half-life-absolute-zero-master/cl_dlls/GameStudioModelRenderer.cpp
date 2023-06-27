//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

//MODDD - library stuff.  externallibinclude.h maybe ?
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>


#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

//
// Override the StudioModelRender virtual member functions here to implement custom bone
// setup, blending, etc.
//

// Global engine <-> studio model rendering code interface
extern engine_studio_api_t IEngineStudio;



//MODDD - NEW.  Set me at the start of the drawcalls below.
//extern drawtype_e g_drawType;
// ...actually nevermind doing it in this file, to determine whether an entity is ordinary or the
// viewmodel, need m_pCurrentEntity.  Which is grabbed in StudioModelRenderer.cpp...


// It would've been really nice to have a definite start point of a draw frame at least, but nope.
// Every single thing is just drawn one after the other.  When does another frame actually happen?
// Who knows.
// No idea if something hacky like seeing when the same reliably drawn entity (#0? #1?) happens to
// judge that maybe.  Time check, unless beginning to end of the same frame reports a different time
// and the difference can't reliably tell whether a frame passed since last rendered object or not?

// ... oh.  Spirit of HL might have already done this, see the "m_nCachedFrameCount != m_nFrameCount"
// line in StudioModelRenderer.    SIGH, having to check a frame-count for change to see if it's
// a new frame, what a world.  Just gimmie engine access dammit.




// The renderer object, created on the stack.
CGameStudioModelRenderer g_StudioRenderer;
/*
====================
CGameStudioModelRenderer

====================
*/
CGameStudioModelRenderer::CGameStudioModelRenderer( void )
{
}

////////////////////////////////////
// Hooks to class implementation
////////////////////////////////////

/*
====================
R_StudioDrawPlayer

====================
*/
int R_StudioDrawPlayer( int flags, entity_state_t *pplayer )
{
	return g_StudioRenderer.StudioDrawPlayer( flags, pplayer );
}

/*
====================
R_StudioDrawModel

====================
*/
int R_StudioDrawModel( int flags )
{
	return g_StudioRenderer.StudioDrawModel( flags );
}

/*
====================
R_StudioInit

====================
*/
void R_StudioInit( void )
{
	g_StudioRenderer.Init();
}

// The simple drawing interface we'll pass back to the engine
r_studio_interface_t studio =
{
	STUDIO_INTERFACE_VERSION,
	R_StudioDrawModel,
	R_StudioDrawPlayer,
};

/*
====================
HUD_GetStudioModelInterface

Export this function for the engine to use the studio renderer class to render objects.
====================
*/
extern "C" int DLLEXPORT HUD_GetStudioModelInterface( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio )
{
	if ( version != STUDIO_INTERFACE_VERSION )
		return 0;

	// Point the engine to our callbacks
	*ppinterface = &studio;

	// Copy in engine helper functions
	memcpy( &IEngineStudio, pstudio, sizeof( IEngineStudio ) );

	// Initialize local variables, etc.
	R_StudioInit();

	// Success
	return 1;
}
