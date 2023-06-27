//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( ALIASRENDERER_H )
#define ALIASRENDERER_H
#if defined( _WIN32 )
#pragma once
#endif

#include "windows.h"
#include "gl/gl.h"
#include "gl/glext.h"
#include "elight.h"
#include "alias.h"
#include "alias_shared.h"

/*
====================
CAliasRenderer

====================
*/
class CAliasRenderer
{
public:
	// Max renderable entities
	static const int MAX_ENTITIES;
	
	// Vertex normals
	static const float g_aliasNormals[][3];

public:
	void Init( void );
	void VidInit( void );
	void Shutdown( void );

	void DrawNormal( void );
	void DrawTransparent( void );
	void DrawAliasModel( cl_entity_t* pentity );

	void SetupRenderer( int rendermode );
	void ResetRenderer( void );

private:
	byte* LoadPalette( const char* szPath, bool prompt = true );
	void ReloadTextures( model_t* pmodel );
	void UploadTexture( GLuint texindex, int width, int height, byte* pdata, byte* ppal );

	void DrawAliasFrame( int curPose, int nextPose, float interp, int latchedCurPose, int latchedNextPose, float latchedInterp );
	void SetupTransform( void );

	alias_extradata_t* GetExtraData( void );

	int GetPoseForFrame( float time, alias_sequence_t* psequence, int frame );
	float EstimateFrame( void );

	void GetEntityLighting( vec3_t& mins, vec3_t& maxs );
	__forceinline void Lighting( vec3_t& origin, vec3_t& normal, vec3_t& color );
	__forceinline void GetInterpolatedPose( trivertx_t* pvert1, trivertx_t *pvert2, float interp, vec3_t& origin, vec3_t& normal );
	__forceinline void GetSinglePose( trivertx_t* pvert1, vec3_t& origin, vec3_t& normal );

private:
	// Currently rendered entity
	cl_entity_t* m_pCurrentEntity;
	// Current model
	model_t* m_pRenderModel;
	// Alias model header
	aliashdr_t* m_pAliasHeader;
	// Extra data for model
	alias_extradata_t* m_pExtraData;

	// Entity's origin
	vec3_t m_vEntityOrigin;
	// Last bound texture id
	GLuint m_uiActiveTextureId;

	// Alias z offset
	float m_flAliasOffset;
	// Ambient light strength
	float m_flAmbientLight;
	// Direct light strength
	float m_flDirectLight;
	// Light vector
	vec3_t m_vLightDirection;
	// Basic lighting info
	alight_t m_lightingInfo;

	// Entity lights
	elight_t *m_pEntityLights[MAX_MODEL_ENTITY_LIGHTS];
	// Local light origins
	vec3_t m_vLocalLightOrigins[MAX_MODEL_ENTITY_LIGHTS];
	// Number of ent lights
	unsigned int m_iNumEntityLights;
	// Lambertian factor cvar
	cvar_t*	m_pCvarLambert;

	// Alias offset on z
	cvar_t* m_pCvarAliasOffset;
	// Alias scale
	cvar_t* m_pCvarAliasScale;

	// Quake 1 texture palette
	byte* m_pDefaultPalette;

	// Opengl functions
	PFNGLACTIVETEXTUREPROC			glActiveTexture;

private:
	// Array of alias extradatas
	alias_extradata_t m_aliasExtraDatas[MAX_ALIAS_MODELS];
	int m_numAliasExtraData;
};

extern CAliasRenderer gAliasRenderer;
#endif // ALIASRENDERER_H