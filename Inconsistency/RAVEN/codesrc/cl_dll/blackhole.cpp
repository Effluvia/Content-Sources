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

#include "windows.h"
#include "hud.h"
#include "cl_util.h"
#include <gl/glu.h>

#include "const.h"
#include "studio.h"
#include "entity_state.h"
#include "triangleapi.h"
#include "event_api.h"
#include "pm_defs.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "blackhole.h"
#include "com_model.h"
#include "blackhole_shared.h"

// Class definition
CBlackHole gBlackHole;

// Shader types
#define SHADER_VERTEX 0
#define SHADER_FRAGMENT 1

//===========================================
//	ARB SHADERS
//===========================================
char blackhole_ps [] =
"!!ARBfp1.0"
"PARAM c[4] = { program.local[0..1], { 1, 0.5, 6.7599993, 0 }, { 2.5999999, 0.005 } };"
"TEMP R0;"
"TEMP R1;"
"TEMP R2;"
"RCP R0.x, fragment.texcoord[0].w;"
"MUL R0.xy, fragment.texcoord[0], R0.x;"
"MAD R2.xy, R0, c[2].y, c[2].y;"
"RCP R0.z, c[1].y;"
"MUL R0.z, R0, c[1].x;"
"ADD R0.xy, R2, -c[0];"
"RCP R0.z, R0.z;"
"MUL R0.y, R0, R0.z;"
"MUL R0.zw, R0.xyxy, R0.xyxy;"
"ADD R0.z, R0, R0.w;"
"RSQ R0.w, R0.z;"
"RCP R0.z, R0.w;"
"MUL R0.xy, R0.w, R0;"
"MOV R0.w, c[3].y;"
"MUL R0.z, R0, c[0];"
"MUL R0.z, R0, R0;"
"DP3 R1.x, R0, R0;"
"RSQ R1.x, R1.x;"
"MUL R0.xyz, R1.x, R0;"
"MUL R2.z, R0.w, c[0].w;"
"MAD R0.w, -R0.z, R0.z, c[2].x;"
"MUL R1.x, R2.z, R2.z;"
"MUL R1.w, R0, R1.x;"
"ADD R1.x, -R1.w, c[2];"
"RSQ R1.x, R1.x;"
"RCP R1.x, R1.x;"
"MAD R1.x, R0.z, R2.z, R1;"
"MUL R1.xyz, R0, R1.x;"
"SLT R1.w, -R1, -c[2].x;"
"ABS R1.w, R1;"
"MUL R0.w, R0, c[2].z;"
"CMP R1.w, -R1, c[2], c[2].x;"
"MAD R1.xyz, R2.z, c[2].wwxw, -R1;"
"CMP R1.xyz, -R1.w, R1, c[2].w;"
"DP3 R1.x, R1, R1;"
"ADD R1.w, -R0, c[2].x;"
"RSQ R1.y, R1.w;"
"RCP R1.y, R1.y;"
"MAD R1.y, R0.z, c[3].x, R1;"
"SLT R0.w, -R0, -c[2].x;"
"MUL R0.xy, R0, R1.y;"
"ABS R0.z, R0.w;"
"CMP R1.y, -R0.z, c[2].w, c[2].x;"
"MUL R0.zw, R2.xyxy, c[1].xyxy;"
"CMP R0.xy, -R1.y, -R0, c[2].w;"
"MAD R0.xy, R0, c[0].w, R0.zwzw;"
"RSQ R1.x, R1.x;"
"RCP R0.w, R1.x;"
"TEX R0.xyz, R0, texture[0], RECT;"
"MUL result.color.xyz, R0, R0.w;"
"MOV result.color.w, c[2].x;"
"END";

char blackhole_vs [] =
"!!ARBvp1.0"
"TEMP R0;"
"TEMP R1;"
"DP4 R0.x, program.local[0], vertex.attrib[0];"
"DP4 R0.y, program.local[1], vertex.attrib[0];"
"DP4 R0.z, program.local[2], vertex.attrib[0];"
"DP4 R0.w, program.local[3], vertex.attrib[0];"
"DP4 R1.x, program.local[4], R0;"
"DP4 R1.y, program.local[5], R0;"
"DP4 R1.z, program.local[6], R0;"
"DP4 R1.w, program.local[7], R0;"
"MOV result.position, R1;"
"MOV result.texcoord, R1;"
"END";

/*
==================
MatMult4

==================
*/
void MatMult4( float *flmatrix, float *vecin, float *vecout )
{
	vecout[0] = vecin[0]*flmatrix[0] + vecin[1]*flmatrix[4] + vecin[2]*flmatrix[8] + vecin[3]*flmatrix[12];
	vecout[1] = vecin[0]*flmatrix[1] + vecin[1]*flmatrix[5] + vecin[2]*flmatrix[9] + vecin[3]*flmatrix[13];
	vecout[2] = vecin[0]*flmatrix[2] + vecin[1]*flmatrix[6] + vecin[2]*flmatrix[10] + vecin[3]*flmatrix[14];
	vecout[3] = vecin[0]*flmatrix[3] + vecin[1]*flmatrix[7] + vecin[2]*flmatrix[11] + vecin[3]*flmatrix[15];
}

//====================================
//
//====================================
void Sys_Error ( char *fmt, ... )
{ 
	va_list	vArgPtr;
	char cMsg[MAX_PATH];
	
	va_start(vArgPtr,fmt);
	vsprintf(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	ShowWindow(GetActiveWindow(), SW_FORCEMINIMIZE);
	MessageBox(NULL, cMsg, "SYSTEM FATAL ERROR", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
	exit(-1);
}

//===========================================
//	ARB SHADERS
//===========================================

int __MsgFunc_Goatse(const char *pszName, int iSize, void *pbuf)
{
	return gBlackHole.MsgFunc_BlackHole(pszName, iSize, pbuf );
}

/*
====================
CBlackHole :: Init

====================
*/
void CBlackHole :: Init ( void )
{
	// Load the functions required
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)wglGetProcAddress("glGenProgramsARB");
	glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)wglGetProcAddress("glBindProgramARB");
	glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)wglGetProcAddress("glProgramStringARB");
	glGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)wglGetProcAddress("glGetProgramivARB");
	glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)wglGetProcAddress("glProgramLocalParameter4fARB");

	if( !glActiveTexture || !glGenProgramsARB || !glBindProgramARB || !glProgramStringARB || !glGetProgramivARB || !glProgramLocalParameter4fARB )
		Sys_Error("Couldn't load required OpenGL functions.\n");

	m_uiVertexShader = LoadShader(SHADER_VERTEX, blackhole_vs);
	if(!m_uiVertexShader)
		Sys_Error("Shader blackhole_vs failed to compile.\n");

	m_uiFragmentShader = LoadShader(SHADER_FRAGMENT, blackhole_ps);
	if(!m_uiFragmentShader)
		Sys_Error("Shader blackhole_ps failed to compile.\n");

	// Hook us up
	HOOK_MESSAGE(Goatse);
}

/*
====================
CBlackHole :: LoadShader

====================
*/
GLuint CBlackHole :: LoadShader ( int type, char* pstr )
{
	GLuint shaderId = NULL;
	GLint errorPos, isNative;

	if( type == SHADER_VERTEX )
	{
		glEnable(GL_VERTEX_PROGRAM_ARB);
		glGenProgramsARB(1, &shaderId);
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, shaderId);

		glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(pstr), pstr);

		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
		glDisable(GL_VERTEX_PROGRAM_ARB);
	}
	else
	{
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		glGenProgramsARB(1, &shaderId);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, shaderId);

		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(pstr), pstr);

		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}

	if ((errorPos == -1) && (isNative == 1))
		return shaderId;

	return NULL;
}

/*
====================
CBlackHole :: VidInit

====================
*/
void CBlackHole :: VidInit ( void )
{
	// Delete previous one
	if(m_uiScreenTexture)
	{
		glDeleteTextures(1, &m_uiScreenTexture);
	}

	// Set up screen texture - Just use a really large value, bleh
	m_uiScreenTexture = 131072;

	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_uiScreenTexture);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, ScreenWidth, ScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	memset(m_pBlackHoles, 0, sizeof(m_pBlackHoles));
}

/*
====================
CBlackHole :: ShutDown

====================
*/
void CBlackHole :: Shutdown ( void )
{
	glDeleteTextures(1, &m_uiScreenTexture);
}

/*
====================
CBlackHole :: CalcRefDef

====================
*/
void CBlackHole :: CalcRefDef ( ref_params_t* pparams )
{
	m_pParams = pparams;
}

/*
====================
CBlackHole :: AffectTempEnt

====================
*/
void CBlackHole :: AffectTempEnt( TEMPENTITY* pTemp )
{
	if(!pTemp->entity.model)
		return;

	if(pTemp->entity.model->type != mod_studio)
		return;

	float fltime = gEngfuncs.GetClientTime();
	for(int i = 0; i < MAX_BLACK_HOLES; i++)
	{
		blackhole_t *pHole = &m_pBlackHoles[i];

		if(pHole->life != -1)
		{
			if(pHole->life <= fltime)
				continue;
		}

		float flDist = (pTemp->entity.origin - pHole->origin).Length();
		float distStrength = 1.0-(flDist/(BLACK_HOLE_SIZE*4));
		if(distStrength < 0)
			distStrength = 0;

		float pullStrength = distStrength*BLACK_HOLE_SUCK_SPEED*pHole->strength;
		Vector vVelocityDir = (pHole->origin - pTemp->entity.origin).Normalize();
		pTemp->entity.baseline.origin = pTemp->entity.baseline.origin + vVelocityDir*pullStrength;

		// Kill it if it's too close
		if( flDist < BLACK_HOLE_SIZE/8.0f )
			pTemp->die = fltime;
	}
}

/*
====================
CBlackHole :: AffectTempEnt

====================
*/
void CBlackHole :: AffectParticle( w_particle_t* pParticle )
{
	float fltime = gEngfuncs.GetClientTime();
	for(int i = 0; i < MAX_BLACK_HOLES; i++)
	{
		blackhole_t *pHole = &m_pBlackHoles[i];

		if(pHole->life != -1)
		{
			if(pHole->life <= fltime)
				continue;
		}

		float flDist = (pParticle->origin - pHole->origin).Length();
		float distStrength = 1.0-(flDist/(BLACK_HOLE_SIZE*4));
		if(distStrength < 0)
			distStrength = 0;

		float pullStrength = distStrength*BLACK_HOLE_SUCK_SPEED*pHole->strength;
		Vector vVelocityDir = (pHole->origin - pParticle->origin).Normalize();
		pParticle->velocity = pParticle->velocity + vVelocityDir*pullStrength;

		if( !pParticle->gravity )
			pParticle->gravity = SNOW_GRAVITY;

		// Kill it if it's too close
		if( flDist < BLACK_HOLE_SIZE/8.0f )
			pParticle->life = 0;
	}
}


/*
====================
CBlackHole :: Render

====================
*/
void CBlackHole :: Render ( void )
{
	float fltime = gEngfuncs.GetClientTime();

	// Check if we have anything to render
	int i = 0;
	for(; i < MAX_BLACK_HOLES; i++)
	{
		if(m_pBlackHoles[i].life == -1)
			break;

		if(m_pBlackHoles[i].life > fltime)
			break;
	}

	if(i == MAX_BLACK_HOLES)
		return;

	vec3_t vForward, vRight, vUp;
	AngleVectors(m_pParams->viewangles, vForward, vRight, vUp);

	glEnable(GL_VERTEX_PROGRAM_ARB);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	// Set up basic rendering
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_uiVertexShader);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_uiFragmentShader);

	glDepthMask(GL_FALSE);

	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_uiScreenTexture);

	float flModelView[16];
	float flProjection[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, flModelView);
	glGetFloatv(GL_PROJECTION_MATRIX, flProjection);

	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, flModelView[0], flModelView[4], flModelView[8], flModelView[12]);
	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, flModelView[1], flModelView[5], flModelView[9], flModelView[13]);
	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 2, flModelView[2], flModelView[6], flModelView[10], flModelView[14]);
	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 3, flModelView[3], flModelView[7], flModelView[11], flModelView[15]);

	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 4, flProjection[0], flProjection[4], flProjection[8], flProjection[12]);
	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 5, flProjection[1], flProjection[5], flProjection[9], flProjection[13]);
	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 6, flProjection[2], flProjection[6], flProjection[10], flProjection[14]);
	glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 7, flProjection[3], flProjection[7], flProjection[11], flProjection[15]);

	for(int i = 0; i < MAX_BLACK_HOLES; i++)
	{
		if(m_pBlackHoles[i].life != -1)
		{
			if(m_pBlackHoles[i].life <= fltime)
				continue;
		}

		// Adjust strength
		m_pBlackHoles[i].strength += BLACK_HOLE_GROWTH_RATE * m_pParams->frametime;
		if(m_pBlackHoles[i].strength > 4)
			m_pBlackHoles[i].strength = 4;

		// Calculate position on screen
		float vOrigin [] = { m_pBlackHoles[i].origin[0], m_pBlackHoles[i].origin[1], m_pBlackHoles[i].origin[2], 1.0 };
		float vViewPos[4], vScreenCoords[4];
		MatMult4(flModelView, vOrigin, vViewPos);
		MatMult4(flProjection, vViewPos, vScreenCoords);

		if(vScreenCoords[3] <= 0)
			continue;

		// Get current screen contents
		glCopyTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, 0, 0, ScreenWidth, ScreenHeight, 0);

		// Calculate uniform values
		float flDistance = (Vector(m_pParams->vieworg)-m_pBlackHoles[i].origin).Length();

		float flCoordX = (vScreenCoords[0]/vScreenCoords[3])*0.5+0.5;
		float flCoordY = (vScreenCoords[1]/vScreenCoords[3])*0.5+0.5;

		float flGrow = (fltime-m_pBlackHoles[i].spawn)/BH_LERP_TIME;
		if(flGrow > 1) flGrow = 1;

		float flShrink = (m_pBlackHoles[i].life-fltime)/BH_LERP_TIME;
		if(flShrink > 1) flShrink = 1;

		if(m_pBlackHoles[i].life == -1)
			flShrink = flGrow = 1;

		glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, flCoordX, flCoordY, flDistance, m_pBlackHoles[i].radius*flGrow*flShrink);
		glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, ScreenWidth, ScreenHeight, 0, 0);

		glBegin(GL_QUADS);
		vec3_t vPoint = m_pBlackHoles[i].origin - vForward * 0.01 + vUp * m_pBlackHoles[i].radius * flGrow*flShrink;
		vPoint = vPoint - vRight * m_pBlackHoles[i].radius * flGrow*flShrink;
		glVertex3fv(vPoint);

		vPoint = m_pBlackHoles[i].origin - vForward * 0.01 + vUp * m_pBlackHoles[i].radius * flGrow*flShrink;
		vPoint = vPoint + vRight * m_pBlackHoles[i].radius * flGrow*flShrink;
		glVertex3fv(vPoint);

		vPoint = m_pBlackHoles[i].origin - vForward * 0.01 - vUp * m_pBlackHoles[i].radius * flGrow*flShrink;
		vPoint = vPoint + vRight * m_pBlackHoles[i].radius * flGrow*flShrink;
		glVertex3fv(vPoint);

		vPoint = m_pBlackHoles[i].origin - vForward * 0.01 - vUp * m_pBlackHoles[i].radius * flGrow*flShrink;
		vPoint = vPoint - vRight * m_pBlackHoles[i].radius * flGrow*flShrink;
		glVertex3fv(vPoint);
		glEnd();
	}

	glDepthMask(GL_TRUE);

	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glDisable(GL_VERTEX_PROGRAM_ARB);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
}

/*
====================
CBlackHole :: AllocBlackHole

====================
*/
blackhole_t *CBlackHole :: AllocBlackHole ( int key )
{
	// Find key
	blackhole_t *pSlot = NULL;
	for(int i = 0; i < MAX_BLACK_HOLES; i++)
	{
		if(m_pBlackHoles[i].key == key)
		{
			pSlot = &m_pBlackHoles[i];
			break;
		}
	}

	// Allocate an empty slot
	float fltime = gEngfuncs.GetClientTime();
	if(!pSlot)
	{
		for(int i = 0; i < MAX_BLACK_HOLES; i++)
		{
			if(m_pBlackHoles[i].life <= fltime && m_pBlackHoles[i].life != -1)
			{
				pSlot = &m_pBlackHoles[i];
				break;
			}
		}
	}

	return pSlot;
}

/*
====================
CBlackHole :: MsgFunc_BlackHole

====================
*/
int CBlackHole :: MsgFunc_BlackHole ( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ(pbuf, iSize);

	int key = READ_SHORT();
	float time = READ_COORD();
	int size = READ_LONG();
	float coordx = READ_COORD();
	float coordy = READ_COORD();
	float coordz = READ_COORD();

	blackhole_t *pSlot = AllocBlackHole(key);

	if(!pSlot)
		return 1;

	float fltime = gEngfuncs.GetClientTime();

	pSlot->key = key;
	if(time == -1) pSlot->life = -1;
	else pSlot->life = fltime+time;

	pSlot->spawn = fltime;
	pSlot->radius = size;
	pSlot->origin.x = coordx;
	pSlot->origin.y = coordy;
	pSlot->origin.z = coordz;
	pSlot->strength = 0;
	return 1;
}
