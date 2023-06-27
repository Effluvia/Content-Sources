//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( FOG_H )
#define FOG_H
#if defined( _WIN32 )
#pragma once
#endif

#include "ref_params.h"

/*
====================
CFog

====================
*/
class CFog
{
public:
	void Init( void );
	void VidInit( void );

	int MsgFunc_Fog( const char *pszName, int iSize, void *pBuf );

	void RenderFog( vec3_t color = m_vFogColor );
	void BlackFog( void );

	void CalcRefDef( ref_params_t* pparams );
	bool CullFogBBox ( const vec3_t& mins, const vec3_t& maxs );

private:
	int	m_iEndDist;
	int m_iStartDist;
	
	vec3_t m_vFogBBoxMin;
	vec3_t m_vFogBBoxMax;

	static vec3_t m_vFogColor;
};

extern CFog gFog;
#endif // FOG_H