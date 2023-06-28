/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

//MODDD - IMPORTANT NOTE!
// This file is no longer included by the cl_dlls or dlls projects.
// Instead see common/mathlib.h.
// The file "pm_shared/pm_math.c" is incuded by the compile, but utils/common/mathlib.c is not.



#ifndef __MATHLIB__
#define __MATHLIB__

// mathlib.h

#include <math.h>
//MODDD - new include. Leave any vector-related defines/typedefs to this file instead.
//#include "vector.h"  //common/vector.h
// nah, go all-out.  const it is.  it has M_PI now, need it.
#include "const.h"


#ifdef __cplusplus
extern "C" {
#endif

//MODDD - vector-related defines moved to common/vector.h

#define SIDE_FRONT		0
#define SIDE_ON			2
#define SIDE_BACK		1
#define SIDE_CROSS		-2

extern vec3_t vec3_origin;

// Use this definition globally
#define ON_EPSILON		0.01
#define EQUAL_EPSILON	0.001


//MODDD - many vector-related defines removed.  These below were never even called.
// See more notes in common/mathlib.h (not utils/...).
/*
vec_t _DotProduct (vec3_t v1, vec3_t v2);
void _VectorSubtract (vec3_t va, vec3_t vb, vec3_t out);
void _VectorAdd (vec3_t va, vec3_t vb, vec3_t out);
void _VectorCopy (vec3_t in, vec3_t out);
void _VectorScale (vec3_t v, vec_t scale, vec3_t out);
*/


int VectorCompare (vec3_t v1, vec3_t v2);
double VectorLength(vec3_t v);
void VectorMA (vec3_t va, double scale, vec3_t vb, vec3_t vc);
void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross);
vec_t VectorNormalize (vec3_t v);
void VectorInverse (vec3_t v);



vec_t Q_rint(vec_t in);

void ClearBounds (vec3_t mins, vec3_t maxs);
void AddPointToBounds (vec3_t v, vec3_t mins, vec3_t maxs);

void AngleMatrix (const vec3_t angles, float matrix[3][4] );
void AngleIMatrix (const vec3_t angles, float matrix[3][4] );
void R_ConcatTransforms (const float in1[3][4], const float in2[3][4], float out[3][4]);

void VectorIRotate (const vec3_t in1, const float in2[3][4], vec3_t out);
void VectorRotate (const vec3_t in1, const float in2[3][4], vec3_t out);

void VectorTransform (const vec3_t in1, const float in2[3][4], vec3_t out);

void AngleQuaternion( const vec3_t angles, vec4_t quaternion );
void QuaternionMatrix( const vec4_t quaternion, float (*matrix)[4] );
void QuaternionSlerp( const vec4_t p, vec4_t q, float t, vec4_t qt );


#ifdef __cplusplus
}
#endif

#endif
