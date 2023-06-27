//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( ELIGHTLIST_H )
#define ELIGHTLIST_H
#if defined( _WIN32 )
#pragma once
#endif

#include "elight.h"
#include "dlight.h"

#define MAX_ENTITY_LIGHTS		1024
#define MAX_GOLDSRC_ELIGHTS		256

/*
====================
CELightList

====================
*/
class CELightList
{
public:
	void Init( void );
	void VidInit( void );
	void CalcRefDef( void );

	int MsgFunc_ELight( const char *pszName, int iSize, void *pBuf );

	void AddEntityLight( int entindex, const vec3_t& origin, const vec3_t& color, float radius, bool isTemporary );
	void GetLightList( vec3_t& origin, const vec3_t& mins, const vec3_t& maxs, elight_t** lightArray, unsigned int* numLights );

	bool CheckBBox( elight_t* plight, const vec3_t& vmins, const vec3_t& vmaxs );

private:
	elight_t	m_pEntityLights[MAX_ENTITY_LIGHTS];
	int			m_iNumEntityLights;

	elight_t	m_pTempEntityLights[MAX_GOLDSRC_ELIGHTS];
	int			m_iNumTempEntityLights;

	dlight_t	*m_pGoldSrcELights;
};

extern CELightList gELightList;
#endif // ELIGHTLIST_H