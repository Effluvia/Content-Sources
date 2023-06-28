//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( STUDIO_UTIL_H )
#define STUDIO_UTIL_H
#if defined( WIN32 )
#pragma once
#endif

//MODDD - new include.
#include "vector.h"



void AngleMatrix (const float *angles, float (*matrix)[4] );
int	VectorCompare (const float *v1, const float *v2);
void CrossProduct (const float *v1, const float *v2, float *cross);
void VectorTransform (const float *in1, float in2[3][4], float *out);
void ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);
void MatrixCopy( float in[3][4], float out[3][4] );
void QuaternionMatrix( vec4_t quaternion, float (*matrix)[4] );
void QuaternionSlerp( vec4_t p, vec4_t q, float t, vec4_t qt );
void AngleQuaternion( float *angles, vec4_t quaternion );

#endif // STUDIO_UTIL_H