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
// mathlib.h


// The file "pm_shared/pm_math.c" is incuded by the compile, but utils/common/mathlib.c is not.
// This one's the important one, this prototypes a lot of things for pm_math.c

// !!! IMPORTANT!  This file is for C only.  A note below goes into more detail.
// Nothing in here is ever needed in C++ files, if anything in here looks that useful
// try taking it and moving it to const.h, especially struct definitions.
// The same method can work, see common/vector.h for some 'extern "c"' business, but
// no overloading allowed as this is coming from C.  And still be careful crossing types
// like even so much as "float*" (pointer to an array of 3 floats) and "vec3_t"
// (interpreted as a Vector-instance in C++ but an array of 3 floats in C).
// Or maybe that isn't even an issue without the crude "#ifdef __cplusplus" being forced
// below.
// ...don't force this, just going by the honor system.  I can't tell if this makes
// Visual Studio error previews freak out with a bunch of fake errors.
///#ifndef __cplusplus

//MODDD - single-use include restriction, unsure why it wasn't here.
#ifndef COMMON_MATHLIB_H
#define COMMON_MATHLIB_H
#pragma once


//MODDD - new include.  Leave any vector-related defines/typedefs to this file instead.
//#include "vector.h"  //common/vector.h
// nah, go all-out.  const it is.  it has M_PI, need it.
#include "const.h"


//MODDD - NEW!  Testing something.
// Borrowed from utils/common/mathlib.h
// ***NEVERMIND!  Do not attempt forcing these to work with method overloading.
// You get run-time errors of methods in cl_dlls sending 'vec3_t' vectors using the Vector
// class that sometimes use one of the pm_math.c methods with C's vec3_t (the array of 3
// floats), so it's interpreted of NULL.  And that's still a huge guess why parameters
// from cl_dlls methods come to pm_shared.c methods as "NULL", even though they clearly
// weren't on cl_dlls's side.  Either the class difference or a C/C++ difference issue.
// Point is, just don't include mathlib.h in C++ files, they have their own methods
// in the new common/vector.cpp already that go client and serverside now.
// Although that both pm_shared/pm_math.c and common/vector.cpp are still part of the
// same compile without causing any issues is... interesting, no issues so long as 
// they aren't both prototype'd everywhere.
// as NULL instead.
//#ifdef __cplusplus
//extern "C" {
//#endif


#define IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)


typedef	int fixed4_t;
typedef	int fixed8_t;
typedef	int fixed16_t;

struct mplane_s;

extern vec3_t vec3_origin;
extern	int nanmask;

//MODDD - removed, and any other similar mentions throught related files.
// None of the underscore-prefix'd vector math methods were ever called.
// Probably a few others weren't either, common/vector.h handles a lot of those
// just fine anyway.  Good to check for other redundancies later as vector.h
// is meant to be included everywhere (yes, even C and C++).
/*
vec_t _DotProduct (vec3_t v1, vec3_t v2);
void _VectorSubtract (vec3_t veca, vec3_t vecb, vec3_t out);
void _VectorAdd (vec3_t veca, vec3_t vecb, vec3_t out);
void _VectorCopy (vec3_t in, vec3_t out);
*/

// MODDD - vector method prototypes moved to vector.h


int Q_log2(int val);

void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);

// Here are some "manual" INLINE routines for doing floating point to integer conversions
extern short new_cw, old_cw;

typedef union DLONG {
	int	i[2];
	double d;
	float f;
	} DLONG;

extern DLONG	dlong;

#ifdef _WIN32
void __inline set_fpu_cw(void)
{
_asm	
	{		wait
			fnstcw	old_cw
			wait
			mov		ax, word ptr old_cw
			or		ah, 0xc
			mov		word ptr new_cw,ax
			fldcw	new_cw
	}
}

int __inline quick_ftol(float f)
{
	_asm {
		// Assumes that we are already in chop mode, and only need a 32-bit int
		fld		DWORD PTR f
		fistp	DWORD PTR dlong
	}
	return dlong.i[0];
}

void __inline restore_fpu_cw(void)
{
	_asm	fldcw	old_cw
}
#else
#define set_fpu_cw() /* */
#define quick_ftol(f) ftol(f)
#define restore_fpu_cw() /* */
#endif

void FloorDivMod (double numer, double denom, int *quotient,
		int *rem);
fixed16_t Invert24To16(fixed16_t val);
int GreatestCommonDivisor (int i1, int i2);

void AngleVectors (const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void AngleVectorsTranspose (const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
#define AngleIVectors	AngleVectorsTranspose

void AngleMatrix (const vec3_t angles, float (*matrix)[4] );
void AngleIMatrix (const vec3_t angles, float (*matrix)[4] );
void VectorTransform (const vec3_t in1, float in2[3][4], vec3_t out);

// wait..   
void NormalizeAngles( vec3_t angles );

void InterpolateAngles( vec3_t start, vec3_t end, vec3_t output, float frac );
float AngleBetweenVectors( const vec3_t v1, const vec3_t v2 );


void VectorMatrix( vec3_t forward, vec3_t right, vec3_t up);
void VectorAngles( const vec3_t forward, vec3_t angles );

int InvertMatrix( const float * m, float *out );

int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct mplane_s *plane);
float anglemod(float a);



#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))



//MODDD - complementary end.
//#ifdef __cplusplus
//}
//#endif


#endif //COMMON_MATHLIB_H

//#endif //__cplusplus
