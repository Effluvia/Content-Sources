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

#if !defined( BLACKHOLE_H )
#define BLACKHOLE_H
#if defined( _WIN32 )
#pragma once
#endif

#include "windows.h"
#include "gl/gl.h"
#include "gl/glext.h"
#include "pm_defs.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "dlight.h"
#include "parsemsg.h"
#include "cvardef.h"
#include "r_efx.h"
#include "weather.h"

#define MAX_BLACK_HOLES 8
#define BH_LERP_TIME	1

struct blackhole_t
{
	int key;
	float spawn;
	float life;
	float radius;
	float strength;
	vec3_t origin;
};

/*
====================
CBlackHole

====================
*/
class CBlackHole
{
public:
	void Init( void );
	void VidInit( void );
	void Shutdown( void );

	void Render ( void );
	int MsgFunc_BlackHole( const char *pszName, int iSize, void *pbuf );

	void CalcRefDef ( ref_params_t* pparams );
	GLuint LoadShader ( int type, char* pstr );
	void AffectTempEnt( TEMPENTITY* pTemp );
	void AffectParticle( w_particle_t* pParticle );

private:
	blackhole_t *AllocBlackHole( int key );

public:
	blackhole_t	m_pBlackHoles[MAX_BLACK_HOLES];

private:
	// Current screen contents
	GLuint		m_uiScreenTexture;

	// Shaders for the effect
	GLuint		m_uiVertexShader;
	GLuint		m_uiFragmentShader;

	// Pointer to ref_params object
	ref_params_t *m_pParams;

private:
	// OpenGL functions used
	PFNGLACTIVETEXTUREPROC				glActiveTexture;
	PFNGLGENPROGRAMSARBPROC				glGenProgramsARB;
	PFNGLBINDPROGRAMARBPROC				glBindProgramARB;

	PFNGLPROGRAMSTRINGARBPROC			glProgramStringARB;
	PFNGLGETPROGRAMIVARBPROC			glGetProgramivARB;
	PFNGLPROGRAMLOCALPARAMETER4FARBPROC	glProgramLocalParameter4fARB;
};
extern CBlackHole gBlackHole;
#endif