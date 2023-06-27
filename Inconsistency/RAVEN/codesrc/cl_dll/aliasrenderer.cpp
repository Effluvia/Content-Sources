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
#include "aliasrenderer.h"
#include "fog.h"

qboolean R_CullBox ( const vec3_t& mins, const vec3_t& maxs );

// Class declaration
CAliasRenderer gAliasRenderer;

// Global engine <-> studio model rendering code interface
extern engine_studio_api_t IEngineStudio;

// Max renderable entities
const int CAliasRenderer::MAX_ENTITIES = 2048;

// Array of normals
const float CAliasRenderer::g_aliasNormals[][3] = { 
	{-0.525731, 0.000000, 0.850651}, {-0.442863, 0.238856, 0.864188},  {-0.295242, 0.000000, 0.955423}, 
	{-0.309017, 0.500000, 0.809017}, {-0.162460, 0.262866, 0.951056}, {0.000000, 0.000000, 1.000000}, 
	{0.000000, 0.850651, 0.525731}, {-0.147621, 0.716567, 0.681718}, {0.147621, 0.716567, 0.681718}, 
	{0.000000, 0.525731, 0.850651}, {0.309017, 0.500000, 0.809017}, {0.525731, 0.000000, 0.850651}, 
	{0.295242, 0.000000, 0.955423}, {0.442863, 0.238856, 0.864188}, {0.162460, 0.262866, 0.951056}, 
	{-0.681718, 0.147621, 0.716567}, {-0.809017, 0.309017, 0.500000}, {-0.587785, 0.425325, 0.688191}, 
	{-0.850651, 0.525731, 0.000000}, {-0.864188, 0.442863, 0.238856}, {-0.716567, 0.681718, 0.147621}, 
	{-0.688191, 0.587785, 0.425325}, {-0.500000, 0.809017, 0.309017}, {-0.238856, 0.864188, 0.442863}, 
	{-0.425325, 0.688191, 0.587785}, {-0.716567, 0.681718, -0.147621}, {-0.500000, 0.809017, -0.309017}, 
	{-0.525731, 0.850651, 0.000000}, {0.000000, 0.850651, -0.525731}, {-0.238856, 0.864188, -0.442863}, 
	{0.000000, 0.955423, -0.295242}, {-0.262866, 0.951056, -0.162460}, {0.000000, 1.000000, 0.000000}, 
	{0.000000, 0.955423, 0.295242}, {-0.262866, 0.951056, 0.162460}, {0.238856, 0.864188, 0.442863}, 
	{0.262866, 0.951056, 0.162460}, {0.500000, 0.809017, 0.309017}, {0.238856, 0.864188, -0.442863}, 
	{0.262866, 0.951056, -0.162460}, {0.500000, 0.809017, -0.309017}, {0.850651, 0.525731, 0.000000},
	{0.716567, 0.681718, 0.147621}, {0.716567, 0.681718, -0.147621}, {0.525731, 0.850651, 0.000000}, 
	{0.425325, 0.688191, 0.587785}, {0.864188, 0.442863, 0.238856}, {0.688191, 0.587785, 0.425325}, 
	{0.809017, 0.309017, 0.500000}, {0.681718, 0.147621, 0.716567}, {0.587785, 0.425325, 0.688191}, 
	{0.955423, 0.295242, 0.000000}, {1.000000, 0.000000, 0.000000}, {0.951056, 0.162460, 0.262866}, 
	{0.850651, -0.525731, 0.000000}, {0.955423, -0.295242, 0.000000}, {0.864188, -0.442863, 0.238856}, 
	{0.951056, -0.162460, 0.262866}, {0.809017, -0.309017, 0.500000}, {0.681718, -0.147621, 0.716567}, 
	{0.850651, 0.000000, 0.525731}, {0.864188, 0.442863, -0.238856}, {0.809017, 0.309017, -0.500000}, 
	{0.951056, 0.162460, -0.262866}, {0.525731, 0.000000, -0.850651}, {0.681718, 0.147621, -0.716567}, 
	{0.681718, -0.147621, -0.716567}, {0.850651, 0.000000, -0.525731}, {0.809017, -0.309017, -0.500000}, 
	{0.864188, -0.442863, -0.238856}, {0.951056, -0.162460, -0.262866}, {0.147621, 0.716567, -0.681718}, 
	{0.309017, 0.500000, -0.809017}, {0.425325, 0.688191, -0.587785}, {0.442863, 0.238856, -0.864188}, 
	{0.587785, 0.425325, -0.688191}, {0.688191, 0.587785, -0.425325}, {-0.147621, 0.716567, -0.681718}, 
	{-0.309017, 0.500000, -0.809017}, {0.000000, 0.525731, -0.850651}, {-0.525731, 0.000000, -0.850651}, 
	{-0.442863, 0.238856, -0.864188}, {-0.295242, 0.000000, -0.955423}, {-0.162460, 0.262866, -0.951056}, 
	{0.000000, 0.000000, -1.000000}, {0.295242, 0.000000, -0.955423}, {0.162460, 0.262866, -0.951056}, 
	{-0.442863, -0.238856, -0.864188}, {-0.309017, -0.500000, -0.809017}, {-0.162460, -0.262866, -0.951056}, 
	{0.000000, -0.850651, -0.525731}, {-0.147621, -0.716567, -0.681718}, {0.147621, -0.716567, -0.681718}, 
	{0.000000, -0.525731, -0.850651}, {0.309017, -0.500000, -0.809017}, {0.442863, -0.238856, -0.864188}, 
	{0.162460, -0.262866, -0.951056}, {0.238856, -0.864188, -0.442863}, {0.500000, -0.809017, -0.309017}, 
	{0.425325, -0.688191, -0.587785}, {0.716567, -0.681718, -0.147621}, {0.688191, -0.587785, -0.425325}, 
	{0.587785, -0.425325, -0.688191}, {0.000000, -0.955423, -0.295242}, {0.000000, -1.000000, 0.000000}, 
	{0.262866, -0.951056, -0.162460}, {0.000000, -0.850651, 0.525731}, {0.000000, -0.955423, 0.295242}, 
	{0.238856, -0.864188, 0.442863}, {0.262866, -0.951056, 0.162460}, {0.500000, -0.809017, 0.309017}, 
	{0.716567, -0.681718, 0.147621}, {0.525731, -0.850651, 0.000000}, {-0.238856, -0.864188, -0.442863}, 
	{-0.500000, -0.809017, -0.309017}, {-0.262866, -0.951056, -0.162460}, {-0.850651, -0.525731, 0.000000}, 
	{-0.716567, -0.681718, -0.147621}, {-0.716567, -0.681718, 0.147621}, {-0.525731, -0.850651, 0.000000}, 
	{-0.500000, -0.809017, 0.309017}, {-0.238856, -0.864188, 0.442863}, {-0.262866, -0.951056, 0.162460}, 
	{-0.864188, -0.442863, 0.238856}, {-0.809017, -0.309017, 0.500000}, {-0.688191, -0.587785, 0.425325}, 
	{-0.681718, -0.147621, 0.716567}, {-0.442863, -0.238856, 0.864188}, {-0.587785, -0.425325, 0.688191}, 
	{-0.309017, -0.500000, 0.809017}, {-0.147621, -0.716567, 0.681718}, {-0.425325, -0.688191, 0.587785}, 
	{-0.162460, -0.262866, 0.951056}, {0.442863, -0.238856, 0.864188}, {0.162460, -0.262866, 0.951056}, 
	{0.309017, -0.500000, 0.809017}, {0.147621, -0.716567, 0.681718}, {0.000000, -0.525731, 0.850651}, 
	{0.425325, -0.688191, 0.587785}, {0.587785, -0.425325, 0.688191}, {0.688191, -0.587785, 0.425325}, 
	{-0.955423, 0.295242, 0.000000}, {-0.951056, 0.162460, 0.262866}, {-1.000000, 0.000000, 0.000000}, 
	{-0.850651, 0.000000, 0.525731}, {-0.955423, -0.295242, 0.000000}, {-0.951056, -0.162460, 0.262866}, 
	{-0.864188, 0.442863, -0.238856}, {-0.951056, 0.162460, -0.262866}, {-0.809017, 0.309017, -0.500000}, 
	{-0.864188, -0.442863, -0.238856}, {-0.951056, -0.162460, -0.262866}, {-0.809017, -0.309017, -0.500000}, 
	{-0.681718, 0.147621, -0.716567}, {-0.681718, -0.147621, -0.716567}, {-0.850651, 0.000000, -0.525731}, 
	{-0.688191, 0.587785, -0.425325}, {-0.587785, 0.425325, -0.688191}, {-0.425325, 0.688191, -0.587785}, 
	{-0.425325, -0.688191, -0.587785}, {-0.587785, -0.425325, -0.688191}, {-0.688191, -0.587785, -0.425325} 
};

/*
====================
Init

====================
*/
void CAliasRenderer::Init( void )
{
	// Get opengl draw funcs
	glActiveTexture	= (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");

	// Get lambert cvar
	m_pCvarLambert = gEngfuncs.pfnGetCvarPointer( "lambert" );

	// Alias offset on z
	m_pCvarAliasOffset = CVAR_CREATE("r_aliasoffset", "24", FCVAR_ARCHIVE);
	// Alias offset on z
	m_pCvarAliasScale = CVAR_CREATE("r_aliasscale", "1", FCVAR_ARCHIVE);
}

/*
====================
VidInit

====================
*/
void CAliasRenderer::VidInit( void )
{
	if(!m_pDefaultPalette)
		m_pDefaultPalette = LoadPalette("gfx/palette.lmp");

	// Clear extradatas
	for(int i = 0; i < m_numAliasExtraData; i++)
	{
		// Make sure this gets cleared
		if(m_aliasExtraDatas[i].pmodel)
			m_aliasExtraDatas[i].pmodel->visdata = NULL;
	}

	memset(m_aliasExtraDatas, 0, sizeof(m_aliasExtraDatas));
	m_numAliasExtraData = 0;
}

/*
====================
Shutdown

====================
*/
void CAliasRenderer::Shutdown( void )
{
	if(m_pDefaultPalette)
	{
		delete[] m_pDefaultPalette;
		m_pDefaultPalette = NULL;
	}
}

/*
====================
LoadPalette

====================
*/
byte* CAliasRenderer::LoadPalette( const char* szPath, bool prompt )
{
	int length = 0;
	byte *pfile = gEngfuncs.COM_LoadFile((char*)szPath, 5, &length);

	if(!pfile || length < 768)
	{
		if(prompt)
			gEngfuncs.Con_Printf("Failed to load: %s", szPath);
		else
			gEngfuncs.Con_DPrintf("Failed to load: %s", szPath);

		return NULL;
	}

	// Allocate and load it
	int size = 256*3;
	byte* pPalette = new byte[size];
	memcpy(pPalette, pfile, size);
	
	gEngfuncs.COM_FreeFile(pfile);
	return pPalette;
}

/*
====================
DrawNormal

====================
*/
void CAliasRenderer::DrawNormal( void )
{
	cl_entity_t* pplayer = gEngfuncs.GetLocalPlayer();

	for(int i = 0; i < MAX_ENTITIES; i++)
	{
		cl_entity_t* pentity = gEngfuncs.GetEntityByIndex(i);
		if(!pentity)
			break;

		if(!pentity->model)
			continue;

		if(pentity->model->type != mod_alias)
			continue;

		if(pentity->curstate.rendermode != kRenderNormal)
			continue;

		if(pentity->curstate.messagenum != pplayer->curstate.messagenum)
			continue;

		DrawAliasModel(pentity);
	}
}

/*
====================
DrawTransparent

====================
*/
void CAliasRenderer::DrawTransparent( void )
{
	cl_entity_t* pplayer = gEngfuncs.GetLocalPlayer();

	for(int i = 0; i < MAX_ENTITIES; i++)
	{
		cl_entity_t* pentity = gEngfuncs.GetEntityByIndex(i);
		if(!pentity)
			break;

		if(!pentity->model)
			continue;

		if(pentity->model->type != mod_alias)
			continue;

		if(pentity->curstate.rendermode == kRenderNormal)
			continue;

		if(pentity->curstate.messagenum != pplayer->curstate.messagenum)
			continue;

		DrawAliasModel(pentity);
	}
}

/*
====================
SetupRenderer

====================
*/
void CAliasRenderer::SetupRenderer( int rendermode )
{
	glDisable(GL_BLEND);
	gEngfuncs.pTriAPI->RenderMode( m_pCurrentEntity->curstate.rendermode );

	// Push texture state
	glPushAttrib(GL_TEXTURE_BIT);

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE3);
	glDisable(GL_TEXTURE_2D);

	// Set the active texture unit
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	// Set up texture state
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

	// set smoothing
	glShadeModel(GL_SMOOTH);

	glColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
	glDepthFunc(GL_LEQUAL);

	// Set this to 0 first
	m_uiActiveTextureId = 0;
}

/*
====================
ResetRenderer

====================
*/
void CAliasRenderer::ResetRenderer( void )
{
	glShadeModel(GL_FLAT);
	glDisable(GL_BLEND);
	glPopAttrib();
}

/*
====================
Lighting

====================
*/
__forceinline void CAliasRenderer::Lighting( vec3_t& origin, vec3_t& normal, vec3_t& color )
{
	float lightcos = DotProduct(normal, m_lightingInfo.plightvec); // -1 colinear, 1 opposite
	if (lightcos > 1.0)
		lightcos = 1;

	float illum = m_flAmbientLight + m_flDirectLight;

	lightcos = (lightcos + (m_pCvarLambert->value - 1.0)) / m_pCvarLambert->value; 		// do modified hemispherical lighting
	if (lightcos > 0.0) 
		illum -= m_flDirectLight * lightcos; 

	if (illum <= 0)
		illum = 0;

	VectorScale(m_lightingInfo.color, illum, color);

	for(unsigned int i = 0; i < m_iNumEntityLights; i++)
	{
		elight_t *plight = m_pEntityLights[i];
		
		// Inverse square radius
		vec3_t dir;
		float radius = plight->radius*plight->radius;
		VectorSubtract(m_vLocalLightOrigins[i], origin, dir);
		
		float dist = DotProduct(dir, dir);
		float attn = max((dist/radius-1)*-1, 0);

		VectorNormalizeFast(dir);

		float fldot = max(DotProduct(normal, dir), 0);
		VectorMA(color, attn*fldot, plight->color, color);
	}
}

/*
====================
GetInterpolatedPose

====================
*/
__forceinline void CAliasRenderer::GetInterpolatedPose( trivertx_t* pvert1, trivertx_t *pvert2,  float interp, vec3_t& origin, vec3_t& normal )
{
	vec3_t v1, v2;
	for(int i = 0; i < 3; i++)
	{
		v1[i] = m_pAliasHeader->scale_origin[i] + pvert1->v[i] * m_pAliasHeader->scale[i];
		v2[i] = m_pAliasHeader->scale_origin[i] + pvert2->v[i] * m_pAliasHeader->scale[i];
	}

	VectorScale(v1, 1.0-interp, origin);
	VectorMA(origin, interp, v2, origin);

	origin[2] += m_flAliasOffset;
	VectorScale(origin, m_pCvarAliasScale->value, origin);

	VectorScale(g_aliasNormals[pvert1->lightnormalindex], 1.0-interp, normal);
	VectorMA(normal, interp, g_aliasNormals[pvert2->lightnormalindex], normal);
}

/*
====================
GetSinglePose

====================
*/
__forceinline void CAliasRenderer::GetSinglePose( trivertx_t* pvert, vec3_t& origin, vec3_t& normal )
{
	for(int i = 0; i < 3; i++)
		origin[i] = m_pAliasHeader->scale_origin[i] + pvert->v[i] * m_pAliasHeader->scale[i];

	origin[2] += m_flAliasOffset;
	VectorScale(origin, m_pCvarAliasScale->value, origin);
	VectorCopy(g_aliasNormals[pvert->lightnormalindex], normal);
}

/*
====================
UploadTexture

====================
*/
void CAliasRenderer::UploadTexture( GLuint texindex, int width, int height, byte* pdata, byte* ppal )
{
	int row1[1024], row2[1024], col1[1024], col2[1024];
	byte *pix1, *pix2, *pix3, *pix4;
	byte alpha1, alpha2, alpha3, alpha4;

	// convert texture to power of 2
	int outwidth;
	for (outwidth = 1; outwidth < width; outwidth <<= 1);
	if (outwidth > 1024) 
		outwidth = 1024;

	int outheight;
	for (outheight = 1; outheight < height; outheight <<= 1);
	if (outheight > 1024) 
		outheight = 1024;

	byte *out, *tex;
	tex = out = (byte *)malloc(outwidth*outheight*4*sizeof(byte));

	for (int i = 0; i < outwidth; i++)
	{
		col1[i] = (int) ((i + 0.25) * (width / (float)outwidth));
		col2[i] = (int) ((i + 0.75) * (width / (float)outwidth));
	}

	for (int i = 0; i < outheight; i++)
	{
		row1[i] = (int) ((i + 0.25) * (height / (float)outheight)) * width;
		row2[i] = (int) ((i + 0.75) * (height / (float)outheight)) * width;
	}

	for (int i = 0; i<outheight; i++)
	{
		for (int j = 0; j<outwidth; j++, out += 4)
		{
			pix1 = &ppal[pdata[row1[i] + col1[j]] * 3];
			pix2 = &ppal[pdata[row1[i] + col2[j]] * 3];
			pix3 = &ppal[pdata[row2[i] + col1[j]] * 3];
			pix4 = &ppal[pdata[row2[i] + col2[j]] * 3];
			alpha1 = 0xFF; alpha2 = 0xFF; alpha3 = 0xFF; alpha4 = 0xFF;

			out[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			out[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			out[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			out[3] = (alpha1 + alpha2 + alpha3 + alpha4)>>2;
		}
	}

	glBindTexture(GL_TEXTURE_2D, texindex); 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outwidth, outheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	free(tex);
}

/*
====================
ReloadTextures

====================
*/
void CAliasRenderer::ReloadTextures( model_t* pModel )
{
	int length = 0;
	byte* pfile = gEngfuncs.COM_LoadFile( pModel->name, 5, &length );
	if( !length || !pfile )
		return;

	mdl_t* paliasmdl = (mdl_t*)pfile;
	aliashdr_t* phdr = (aliashdr_t*)IEngineStudio.Mod_Extradata(pModel);

	// what sort of ugly wizardry is this?
	daliasskintype_t* pskintype = (daliasskintype_t *)&paliasmdl[1];
	byte *pskin = (byte *)(pskintype + 1);

	if (paliasmdl->numskins < 1 || paliasmdl->numskins > MAX_SKINS)
		gEngfuncs.Con_Printf("CAliasRenderer::ReloadTextures: Invalid # of skins: %d\n", paliasmdl->numskins);

	// Custom palette support
	char szPath[MAX_PATH];
	strcpy(szPath, pModel->name);
	strcpy(&szPath[strlen(szPath)-3], "lmp");

	// Load the custom palette
	byte* pPalette = LoadPalette(szPath, false);

	for (int i = 0; i < paliasmdl->numskins; i++)
	{
		// Only support single skin groups
		if (pskintype->type == ALIAS_SKIN_SINGLE) 
		{
			UploadTexture(m_pAliasHeader->gl_texturenum[i], paliasmdl->skinwidth, paliasmdl->skinheight, (byte *)(pskintype + 1), pPalette ? pPalette : m_pDefaultPalette);
			pskintype = (daliasskintype_t *)((byte *)(pskintype+1) + (paliasmdl->skinwidth * paliasmdl->skinheight));
		} 
		else
		{
			gEngfuncs.Con_Printf("CAliasRenderer::ReloadTextures: Texture not loaded for %s, skin groups are not supported.\n", pModel->name);
		}
	}

	if(pPalette)
		delete[] pPalette;

	gEngfuncs.COM_FreeFile(pfile);
}

/*
=================
DrawAliasFrame

=================
*/
void CAliasRenderer::DrawAliasFrame ( int curPose, int nextPose, float interp, int latchedCurPose, int latchedNextPose, float latchedInterp )
{
	vec3_t vorigin, vnormal, vcolor, vlorigin, vlnormal, vlcolor;
	trivertx_t* pvertexes = (trivertx_t *)((byte *)m_pAliasHeader + m_pAliasHeader->posedata);
	trivertx_t* pverts1 = pvertexes + curPose * m_pAliasHeader->poseverts;

	trivertx_t* pverts2 = NULL;
	if(nextPose != -1)
		pverts2 = pvertexes + nextPose * m_pAliasHeader->poseverts;

	trivertx_t* plverts1 = NULL;
	trivertx_t* plverts2 = NULL;
	if( latchedCurPose != -1 )
	{
		plverts1 = pvertexes + latchedCurPose * m_pAliasHeader->poseverts;

		if(latchedNextPose != -1)
			plverts2 = pvertexes + latchedNextPose * m_pAliasHeader->poseverts;
	}

	int* order = (int *)((byte *)m_pAliasHeader + m_pAliasHeader->commands);

	float seqinterp = 0.0;
	if(plverts1 && plverts2)
	{
		// Estimate interpolant
		seqinterp = 1.0 - (gEngfuncs.GetClientTime() - m_pCurrentEntity->latched.sequencetime) / 0.2;
	}

	while (1)
	{
		// get the vertex count and primitive type
		int count = (*order);
		order++;

		if (!count)
			break;		// done

		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
			glBegin (GL_TRIANGLE_STRIP);

		do
		{
			// texture coordinates come from the draw list
			glTexCoord2f(((float *)order)[0], ((float *)order)[1]);
			order += 2;

			// Calculate pose
			if( pverts2 )
				GetInterpolatedPose(pverts1++, pverts2++, interp, vorigin, vnormal);
			else
				GetSinglePose(pverts1++, vorigin, vnormal);

			// Calculate lighting
			Lighting(vorigin, vnormal, vcolor);

			// Interpolate if needed
			if( plverts1 )
			{
				// Calcolate pose
				if( plverts2 )
					GetInterpolatedPose(plverts1++, plverts2++, latchedInterp, vlorigin, vlnormal);
				else
					GetSinglePose(plverts1++, vlorigin, vlnormal);

				// Calculate lighting
				Lighting(vlorigin, vlnormal, vlcolor);

				// Interpolate animation change
				VectorScale(vorigin, (1.0-seqinterp), vorigin);
				VectorMA(vorigin, seqinterp, vlorigin, vorigin);

				VectorScale(vcolor, (1.0-seqinterp), vcolor);
				VectorMA(vcolor, seqinterp, vlcolor, vcolor);
			}

			// Draw
			glColor3fv(vcolor);
			glVertex3fv(vorigin);
		} while (--count);

		glEnd ();
	}
}

/*
=================
GetEntityLighting

=================
*/
void CAliasRenderer::GetEntityLighting ( vec3_t& mins, vec3_t& maxs )
{
	// Get elight list
	gELightList.GetLightList(m_pCurrentEntity->origin, mins, maxs, m_pEntityLights, &m_iNumEntityLights);

	float angleMatrix[3][4];
	AngleMatrix(m_pCurrentEntity->angles, angleMatrix);

	// Rotate lights into reference
	for(unsigned int i = 0; i < m_iNumEntityLights; i++)
	{
		vec3_t vtemp;
		VectorSubtract(m_pEntityLights[i]->origin, m_pCurrentEntity->origin, vtemp);
		VectorIRotate(vtemp, angleMatrix, m_vLocalLightOrigins[i]);
	}
}

/*
=================
GetPoseForFrame

=================
*/
int CAliasRenderer::GetPoseForFrame ( float time, alias_sequence_t* psequence, int frame )
{
	int outframe = frame;
	if(outframe < 0)
	{
		outframe = 0;
	}
	else if(frame >= psequence->numframes)
	{
		if(psequence->looped)
			outframe = outframe % psequence->numframes;
		else
			outframe = (psequence->numframes-1);
	}

	outframe = psequence->startframe + outframe;

	int curPose = m_pAliasHeader->frames[outframe].firstpose;
	int numPoses = m_pAliasHeader->frames[outframe].numposes;

	if (numPoses > 1)
	{
		float interval = m_pAliasHeader->frames[outframe].interval;
		curPose += (int)(time / interval) % numPoses;
	}

	return curPose;
}

/*
====================
EstimateFrame

====================
*/
float CAliasRenderer::EstimateFrame( void )
{
	float time = gEngfuncs.GetClientTime();
	int sequence = m_pCurrentEntity->curstate.sequence;
	if(sequence >= m_pExtraData->numsequences)
	{
		gEngfuncs.Con_Printf ("CAliasRenderer::DrawModel: no such sequence %d\n", sequence);
		sequence = 0;
	}

	// Get sequence data
	alias_sequence_t* psequence = &m_pExtraData->sequences[sequence];
	int numframes = psequence->looped ? (psequence->numframes+1) : psequence->numframes;

	double dfdt, f;
	if ( time < m_pCurrentEntity->curstate.animtime )
		dfdt = 0;
	else
		dfdt = (time - m_pCurrentEntity->curstate.animtime) * m_pCurrentEntity->curstate.framerate * psequence->fps;

	if (psequence->numframes <= 1)
		f = 0;
	else
		f = (m_pCurrentEntity->curstate.frame * (numframes - 1)) / 256.0;
 	
	f += dfdt;

	if (psequence->looped) 
	{
		if (psequence->numframes > 1)
		{
			f -= (int)(f / (numframes - 1)) *  (numframes - 1);
		}
		if (f < 0) 
		{
			f += (numframes - 1);
		}
	}
	else 
	{
		if (f >= numframes - 1.001) 
		{
			f = numframes - 1.001;
		}
		if (f < 0.0) 
		{
			f = 0.0;
		}
	}

	return f;
}

/*
====================
GetExtraData

====================
*/
alias_extradata_t* CAliasRenderer::GetExtraData( void )
{
	// Check if it was already set
	if(m_pRenderModel->visdata)
		return (alias_extradata_t*)m_pRenderModel->visdata;

	// See if we have it loaded
	for(int i = 0; i < m_numAliasExtraData; i++)
	{
		if(!strcmp(m_aliasExtraDatas[i].name, m_pRenderModel->name))
		{
			m_pRenderModel->visdata = (byte*)&m_aliasExtraDatas[i];
			return &m_aliasExtraDatas[i];
		}
	}

	if(m_numAliasExtraData == MAX_ALIAS_MODELS)
	{
		gEngfuncs.Con_Printf("Error: Exceeded MAX_ALIAS_MODELS\n");
		return NULL;
	}

	// Allocate a new one
	alias_extradata_t* pextradata = &m_aliasExtraDatas[m_numAliasExtraData];
	m_numAliasExtraData++;

	// Set info
	strcpy(pextradata->name, m_pRenderModel->name);
	pextradata->pmodel = m_pRenderModel;

	// Fill sequence data
	Alias_GetSequenceInfo(m_pAliasHeader, pextradata);
	Alias_LoadSequenceInfo(m_pRenderModel->name, gEngfuncs.pfnGetGameDirectory(), pextradata);

	// TODO create SVD

	// Set ptr in model_t
	m_pRenderModel->visdata = (byte*)pextradata;

	return pextradata;
}

/*
====================
SetupTransform

====================
*/
void CAliasRenderer::SetupTransform ( void )
{
	int				i;
	vec3_t			angles;
	vec3_t			modelpos;

	VectorCopy( m_pCurrentEntity->origin, modelpos );

	angles[ROLL] = m_pCurrentEntity->curstate.angles[ROLL];
	angles[PITCH] = m_pCurrentEntity->curstate.angles[PITCH];
	angles[YAW] = m_pCurrentEntity->curstate.angles[YAW];

	float time = gEngfuncs.GetClientTime();
	if (m_pCurrentEntity->curstate.movetype == MOVETYPE_STEP) 
	{
		float			f = 0;
		float			d;

		if ( ( time < m_pCurrentEntity->curstate.animtime + 1.0f ) &&
			 ( m_pCurrentEntity->curstate.animtime != m_pCurrentEntity->latched.prevanimtime ) )
			f = (time - m_pCurrentEntity->curstate.animtime) / (m_pCurrentEntity->curstate.animtime - m_pCurrentEntity->latched.prevanimtime);

		f = f - 1.0;

		for (i = 0; i < 3; i++)
			modelpos[i] += (m_pCurrentEntity->origin[i] - m_pCurrentEntity->latched.prevorigin[i]) * f;

		for (i = 0; i < 3; i++)
		{
			float ang1, ang2;

			ang1 = m_pCurrentEntity->angles[i];
			ang2 = m_pCurrentEntity->latched.prevangles[i];

			d = ang1 - ang2;
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;

			angles[i] += d * f;
		}
	}
	else if ( m_pCurrentEntity->curstate.movetype != MOVETYPE_NONE ) 
	{
		VectorCopy( m_pCurrentEntity->angles, angles );
	}

    glTranslatef(modelpos[0],  modelpos[1],  modelpos[2]);

    glRotatef(angles[1],  0, 0, 1);
    glRotatef(-angles[0],  0, 1, 0);
    glRotatef(angles[2],  1, 0, 0);
}

/*
=================
DrawAliasModel

=================
*/
void CAliasRenderer::DrawAliasModel ( cl_entity_t* pentity )
{
	if(!pentity->model)
		return;

	if(pentity->model->type != mod_alias)
		return;

	// Set pointers
	m_pCurrentEntity = pentity;
	m_pRenderModel = m_pCurrentEntity->model;

	vec3_t mins, maxs;
	VectorAdd (m_pCurrentEntity->origin, m_pCurrentEntity->curstate.mins, mins);
	VectorAdd (m_pCurrentEntity->origin, m_pCurrentEntity->curstate.maxs, maxs);

	if(m_pCurrentEntity != gEngfuncs.GetViewModel())
	{
		if(R_CullBox(mins, maxs))
			return;
	}

	if(gFog.CullFogBBox(mins, maxs))
		return;

	VectorCopy (m_pCurrentEntity->origin, m_vEntityOrigin);

	//
	// locate the proper data
	//
	m_pAliasHeader = (aliashdr_t *)IEngineStudio.Mod_Extradata (m_pCurrentEntity->model);

	// Set modelview matrix
	glMatrixMode(GL_MODELVIEW);
    glPushMatrix ();

	// Set transform matrix
	SetupTransform();

	// We need to reload the texture, as HL doesn't actually
	// load it, because he pPal pointer is NULL when passed
	// to the texture loading function
	// HL only gives it an unused texture index
	if(!(m_pAliasHeader->flags & AF_RELOADED))
	{
		ReloadTextures( m_pRenderModel );
		m_pAliasHeader->flags |= AF_RELOADED;
	}

	// Get extra animation info
	m_pExtraData = GetExtraData();
	if(!m_pExtraData)
		return;

	// Get lighting info
	m_lightingInfo.plightvec = m_vLightDirection;
	IEngineStudio.StudioDynamicLight( m_pCurrentEntity, &m_lightingInfo );

	m_flAmbientLight = (float)m_lightingInfo.ambientlight/255.0f;
	m_flDirectLight = (float)m_lightingInfo.shadelight/255.0f;

	// Get entity lights
	GetEntityLighting( mins, maxs );

	// Set render mode
	SetupRenderer( m_pCurrentEntity->curstate.rendermode );

	// View models don't need this offset
	if(m_pCurrentEntity != gEngfuncs.GetViewModel())
		m_flAliasOffset = m_pCvarAliasOffset->value;
	else
		m_flAliasOffset = 0;

	// Bind texture
	GLuint textureIndex = m_pAliasHeader->gl_texturenum[m_pCurrentEntity->curstate.skin];
	if(m_uiActiveTextureId != textureIndex)
	{
		glBindTexture(GL_TEXTURE_2D, textureIndex);
		m_uiActiveTextureId = textureIndex;
	}

	int sequenceidx = m_pCurrentEntity->curstate.sequence;
	if(sequenceidx >= m_pExtraData->numsequences)
	{
		gEngfuncs.Con_Printf ("CAliasRenderer::DrawModel: no such sequence %d\n", sequenceidx);
		sequenceidx = 0;
	}

	// Get sequence data
	alias_sequence_t* psequence = &m_pExtraData->sequences[sequenceidx];

	float time = gEngfuncs.GetClientTime();
	float nextTime = time + 1.0f / psequence->fps;

	// Get frames
	float floatFrame = EstimateFrame();
	int curFrame = (int)floor(floatFrame);
	int nextFrame = curFrame + 1;

	// Get poses
	float interp = 0;
	int curPose = GetPoseForFrame(time, psequence, curFrame);
	int nextPose = GetPoseForFrame(nextTime, psequence, nextFrame);
	
	if(curPose == nextPose)
		nextPose = -1;
	else
		interp = floatFrame - curFrame;

	// For sequence changes
	float latchedInterp = 0;
	int latchedCurPose = -1;
	int latchedNextPose = -1;

	// Check if we need to interpolate
	if(m_pCurrentEntity->latched.sequencetime &&
		( m_pCurrentEntity->latched.sequencetime + 0.2 > gEngfuncs.GetClientTime() ) && 
		( m_pCurrentEntity->latched.prevsequence < m_pExtraData->numsequences ))
	{
		// Get sequence data
		int latchedSequence = m_pCurrentEntity->latched.prevsequence;
		alias_sequence_t* pprevsequence = &m_pExtraData->sequences[latchedSequence];

		// Get frames
		float latchedFrame = m_pCurrentEntity->latched.prevframe;
		int curLatchedFrame = (int)floor(latchedFrame);
		int nextLatchedFrame = curLatchedFrame + 1;

		// Get poses
		latchedCurPose = GetPoseForFrame(time, pprevsequence, curLatchedFrame);
		latchedNextPose = GetPoseForFrame(time, pprevsequence, nextLatchedFrame);

		if(latchedCurPose == latchedNextPose)
			latchedNextPose = -1;
		else
			latchedInterp = latchedFrame - curLatchedFrame;
	}
	else
	{
		// Save for interp
		m_pCurrentEntity->latched.prevframe = floatFrame;
	}

	// Draw the frame
	DrawAliasFrame(curPose, nextPose, interp, latchedCurPose, latchedNextPose, latchedInterp);

	glPopMatrix ();

	ResetRenderer();
}