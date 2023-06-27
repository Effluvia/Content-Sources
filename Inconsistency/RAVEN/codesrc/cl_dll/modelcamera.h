//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( MODELCAMERA_H )
#define MODELCAMERA_H
#if defined( _WIN32 )
#pragma once
#endif

#include "ref_params.h"

enum attachments_e
{
	ATTACH_ORIGIN = 0,
	ATTACH_FORWARD,
	ATTACH_LEFT,

	// Must be last
	NUM_ATTACHMENTS
};

/*
====================
CModelCamera

====================
*/
class CModelCamera
{
public:
	void Init( void );
	void VidInit( void );
	bool IsActive( void );

	int MsgFunc_ModelCamera( const char *pszName, int iSize, void *pBuf );

	void CalcRefDef( ref_params_t* pparams );
	void MouseMove( float mousex, float mousey );

private:
	void ResetValues( void );
	float SplineFraction( float value, float scale );
	vec3_t VecToAngles( vec3_t forward, vec3_t left );

private:
	bool m_bActive;

	float m_flLastMouseMove;
	float m_flDeviationTimeout;

	float m_flMaxDeviationX;
	float m_flMaxDeviationY;

	vec3_t m_vAddDeviationAngles;
	vec3_t m_vDeviationAngles;

	ref_params_t* m_pParams;
	cl_entity_t* m_pViewEntity;

	int m_iAttachments[NUM_ATTACHMENTS];
};

extern CModelCamera gModelCamera;
#endif // MODELCAMERA_H