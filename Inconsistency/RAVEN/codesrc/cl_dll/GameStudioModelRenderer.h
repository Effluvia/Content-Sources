//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( GAMESTUDIOMODELRENDERER_H )
#define GAMESTUDIOMODELRENDERER_H
#if defined( _WIN32 )
#pragma once
#endif

/*
====================
CGameStudioModelRenderer

====================
*/
class CGameStudioModelRenderer : public CStudioModelRenderer
{
public:
	CGameStudioModelRenderer( void );
};
extern CGameStudioModelRenderer g_StudioRenderer;
#endif // GAMESTUDIOMODELRENDERER_H