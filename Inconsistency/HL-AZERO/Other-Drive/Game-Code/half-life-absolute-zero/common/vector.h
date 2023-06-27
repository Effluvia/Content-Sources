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


// New file. New place for storing the Vector class server and clientside, which barely changes between
// the two.
// Looks like the "Vector" class being present clientside was a relatively recent edition late in the
// game's development.  Lots of C-level methods that assume the lack of a Vector class are used there
// anyway (especially seen in cl_util.h/.cpp).


// IMPORTANT - It's probably feasible to remove all the vector-math-related methods in cl_util.c/.hpp,
// as the game already compiles pm_shared/pm_math.c (most likely header'd by common/mathlib.h).
// Just gets a little tricky from the math-related files outside the game being C files, but the
// ones in cl_util.cpp are clearly for C++ like the rest of the cl_dlls and the dlls folders.  Weird.

// ALSO - no need to worry about where to include mathlib.h anymore, that's handled by
// this file too at the bottom.  mathlib.h requires this to be included first anyway (or would be
// including const.h which includes this).  Everything C/C++/client/serverside, whatever, it works out.


#ifndef VECTOR_H
#define VECTOR_H

#include "external_lib_include.h"

// NOTE - section used to be in the "#ifdef CLIENT_DLL" section below.
// Misc C-runtime library headers
//#include "STDIO.H"
//#include "STDLIB.H"
//#include "MATH.H"
////////////////////////////////////////

// does 'const' give C issues?    Nevermind, most of this file isn't for C anyway.
//#define const 


//original comment: 'needed before including progdefs.h'
// ...seen in pm_shared files, so shared b/w C and C++ it goes.



//***
// Bunch of typedefs/defines related to vectors from the C math files moved here
//   common/mathlib.h
//   pm_shared/pm_math.c
//   utils/common/mathlib.h
//   utils/common/mathlib.c

// Seems a lot are included in both C/C++ files though.
// vec3_t however, is a little special.  Client and serverside for C++, it may
// mean the "Vector" class, but it could also be the array of 3 floats
// (typedef vec_t vec3_t[3]).

typedef float vec_t;

// It MIGHT be more accurate to just go through every cl_dlls and dlls file and replace every
// single mention of "vec3_t" with "Vector".
// However, other common and util files from other folders also use vec3_t, and a consistent
// usage there (float array or Vector class) is harder to pin down.
// If something is included by both C and C++ files at some point (like pm_math.h), the meaning
// of 'vec3_t' is different in each case: array of 3 floats for C, 'Vector' class for C++.
// And C++ is littered with reliances on the Vector class, so that part isn't budging.
// In short, if there really isn't any guarantee that all 'vec3_t' mentions can be safely
// replaced with 'Vector' (always included by C++ which allows this), there isn't really a point.
#ifndef __cplusplus
	typedef vec_t vec3_t[3];	// x,y,z
#else
	#define vec3_t Vector
#endif

typedef vec_t vec4_t[4];	// x,y,z,w
typedef vec_t vec5_t[5];

typedef short vec_s_t;
typedef vec_s_t vec3s_t[3];
typedef vec_s_t vec4s_t[4];	// x,y,z,w
typedef vec_s_t vec5s_t[5];




#ifndef __cplusplus
// (moved here from utils/common/matlab.h)
// MODDD - wha.  nowhere else checks for DOUBLEVEC_T, this seems really inconsistent.
// Disabling the check.
// Nope, disable it all!  Plenty of other places make vec_t a 'float' without even
// involving this DOUBLEVEC_T constant.  Really inconsistent, going with a 'drop it'.
/*
//#ifdef DOUBLEVEC_T
//typedef double vec_t;
//#else
typedef float vec_t;
//#endif
*/
#endif




// If this is C++, get the majority of the script.
// NOTICE - a lot of the 'extern "C"' script below is prototyped in common/mathlib.h instead,
// included by C files like pm_shared.c.  extern "C"'s below are mostly prototypes of those
// for C++ to use.
// CHANGE, now in this file too, see the 'else' of this line
#ifdef __cplusplus

#ifdef CLIENT_DLL
    //has it.
    #define EXTRAPARSE (float)

	// Header file containing definition of globalvars_t and entvars_t
	// ...leftover comment copied from extdll.h methinks.

#else
    // server?

	// from extdll.h, what harm can it do being everywhere.
	
	// blank this
    #define EXTRAPARSE

	//original comment:  Defining it as a (bogus) struct helps enforce type-checking
	// From extdll.h
#endif


//=========================================================
// 2DVector - used for many pathfinding and many other 
// operations that are treated as planar rather than 3d.
//=========================================================
class Vector2D
{
public:

	//MODDD - moved to the top, paranoia
	vec_t	x, y;

	inline Vector2D(void)									{ }
	inline Vector2D(float X, float Y)						{ x = X; y = Y; }
	inline Vector2D operator+(const Vector2D& v)	const	{ return Vector2D(x+v.x, y+v.y);	}
	inline Vector2D operator-(const Vector2D& v)	const	{ return Vector2D(x-v.x, y-v.y);	}
	inline Vector2D operator*(float fl)				const	{ return Vector2D(x*fl, y*fl);	}
	inline Vector2D operator/(float fl)				const	{ return Vector2D(x/fl, y/fl);	}
	
	inline float Length(void)						const	{ return EXTRAPARSE sqrt(x*x + y*y );		}

	inline Vector2D Normalize ( void ) const
	{
		Vector2D vec2;

		float flLen = Length();
		if ( flLen == 0 )
		{
			return Vector2D( EXTRAPARSE 0, EXTRAPARSE 0 );
		}
		else
		{
			flLen = 1 / flLen;
			return Vector2D( x * flLen, y * flLen );
		}
	}

};


//=========================================================
// 3D Vector
//=========================================================
class Vector						// same data-layout as engine's vec3_t,
{								//		which is a vec_t[3]
public:

	//MODDD - moved to the top, paranoia
	// Members
	vec_t x, y, z;

	// Construction/destruction
	inline Vector(void)								{ }
	inline Vector(float X, float Y, float Z)		{ x = X; y = Y; z = Z;						}
	

////////////////////////////////////////////////////////////////////////
	//for whatever reason?
	//Yea, forget these.
//#ifdef CLIENT_DLL
	//inline Vector(double X, double Y, double Z)		{ x = (float)X; y = (float)Y; z = (float)Z;	}
	//inline Vector(int X, int Y, int Z)				{ x = (float)X; y = (float)Y; z = (float)Z;	}
//#endif
////////////////////////////////////////////////////////////////////////


	inline Vector(const Vector& v)					{ x = v.x; y = v.y; z = v.z;			} 
	inline Vector(float rgfl[3])					{ x = rgfl[0]; y = rgfl[1]; z = rgfl[2];}

	// Operators
	inline Vector operator-(void) const				{ return Vector(-x,-y,-z);				}
	inline int operator==(const Vector& v) const	{ return x==v.x && y==v.y && z==v.z;	}
	inline int operator!=(const Vector& v) const	{ return !(*this==v);					}
	inline Vector operator+(const Vector& v) const	{ return Vector(x+v.x, y+v.y, z+v.z);	}
	inline Vector operator-(const Vector& v) const	{ return Vector(x-v.x, y-v.y, z-v.z);	}
	inline Vector operator*(float fl) const			{ return Vector(x*fl, y*fl, z*fl);		}
	inline Vector operator/(float fl) const			{ return Vector(x/fl, y/fl, z/fl);		}
	
	// Methods
	inline void CopyToArray(float* rgfl) const		{ rgfl[0] = x, rgfl[1] = y, rgfl[2] = z; }
	inline float Length(void) const					{ return EXTRAPARSE sqrt(x*x + y*y + z*z); }
	operator float *()								{ return &x; } // Vectors will now automatically convert to float * when needed
	operator const float *() const					{ return &x; } // Vectors will now automatically convert to float * when needed
	inline Vector Normalize(void) const
	{
		float flLen = Length();
		if (flLen == 0) return Vector(0,0,1); // ????
		flLen = 1 / flLen;
		return Vector(x * flLen, y * flLen, z * flLen);
	}

	inline Vector2D Make2D ( void ) const
	{
		Vector2D	Vec2;

		Vec2.x = x;
		Vec2.y = y;

		return Vec2;
	}
	inline float Length2D(void) const				{ return EXTRAPARSE sqrt(x*x + y*y); }


	inline vec_t NormalizeInPlace( void ){
		float flLen = Length();
		if( flLen == 0 ){
			x = y = 0;
			z = 1;
			return flLen;
		}
		const float flInvertedLen = 1 / flLen;
		x *= flInvertedLen;
		y *= flInvertedLen;
		z *= flInvertedLen;
		return flLen;
	}

};


inline Vector operator*(float fl, const Vector& v) { return v * fl; }
inline Vector2D operator*(float fl, const Vector2D& v) { return v * fl; }

// These were found identical in the original cl_dlls/util_vector.h and dlls//vector.h.
inline Vector CrossProduct(const Vector& a, const Vector& b) { return Vector(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
inline float DotProduct(const Vector2D& a, const Vector2D& b) { return(a.x * b.x + a.y * b.y); }
inline float DotProduct(const Vector& a, const Vector& b) { return(a.x * b.x + a.y * b.y + a.z * b.z); }

// this was in cl_dlls/cl_util.h, but is more fitting here.
inline void VectorClear(float* a) { a[0] = 0.0; a[1] = 0.0; a[2] = 0.0; }


// A few externs from cl_dlls/view.cpp to pull out ordinarilly C-only functions from common/mathlib.h
// "NormalizeAngles" was also included by hud_spectator.cpp
extern "C" void InterpolateAngles(vec3_t start, vec3_t end, vec3_t output, float frac);
extern "C" void NormalizeAngles(vec3_t angles);

// to be more fitting with the actual pm_math.c implementation
//extern "C" float Distance(const float* v1, const float* v2);
// testing, disabled for C++ access.  Seems to crash when called from there anyway. No clue.
//extern "C" float Distance(const vec3_t v1, const vec3_t v2);
//extern "C" float Distance2D(const vec3_t v1, const vec3_t v2);

extern "C" float AngleBetweenVectors(const vec3_t v1, const vec3_t v2);
// And from cl_dlls/input.cpp.
extern "C" float anglemod( float a );


// from cl_dlls/cl_util.cpp, implemented in common/vector.cpp.  C++ only.
extern vec3_t vec3_origin;
extern float Length(const float* v);

//MODDD - NEW.  For C++.  cloned from C.   for some reason.
float Distance(const vec3_t v1, const vec3_t v2);
float Distance2D(const vec3_t v1, const vec3_t v2);
float DistanceFromDelta(const vec3_t vDelta);
float Distance2DFromDelta(const vec3_t vDelta);

extern void VectorAngles(const float* forward, float* angles);
extern float VectorNormalize(float* v);
extern void VectorMA(const float* veca, float scale, const float* vecb, float* vecc);
extern void VectorScale(const float* in, float scale, float* out);
extern void VectorInverse(float* v);


#else

// Base C?  From common/mathlib.h, implementations (all?) in pm_shared/pm_math.c
void VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc);
int VectorCompare(const vec3_t v1, const vec3_t v2);
float Length(const vec3_t v);
void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);

float Distance(const vec3_t v1, const vec3_t v2);
float Distance2D(const vec3_t v1, const vec3_t v2);
float DistanceFromDelta(const vec3_t vDelta);
float Distance2DFromDelta(const vec3_t vDelta);

float VectorNormalize(vec3_t v); // returns vector length
void VectorInverse(vec3_t v);
void VectorScale(const vec3_t in, vec_t scale, vec3_t out);

#endif //END OF C++ requirement



// for C and C++ again
////////////////////////////////////////////////////////////////////////////

// NOTICE - renamed the "DotProduct" macro to "DotProduct_f" to avoid a conflict with the DotProduct
// method above.
// Overloading does not play nicely with macros, especially not with methods that share the same name
// at the same time.
// cl_dlls/cl_util.h defined DotProduct, VectorSubtract, VectorAdd, and VectorCopy.
// Unsure why though, they're in the two mathlib.h files too.
// In short, these can be included client and serverside, as they always were (mathlib files are not
// client/server specific).


#define DotProduct_f(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
// Also, this was in studio_util.h.
// The "F" in front isn't for "float", it means to take the 'fabs' of each of the component
// multiplications in a dot product instead.  Never used though.
// Renamed with "_f" at the end for consistency like above.
#define FDotProduct_f( a, b ) (fabs((a[0])*(b[0])) + fabs((a[1])*(b[1])) + fabs((a[2])*(b[2])))


#define VectorAdd_f(a,b,c) {(c)[0]=(a)[0]+(b)[0];(c)[1]=(a)[1]+(b)[1];(c)[2]=(a)[2]+(b)[2];}
#define VectorCopy_f(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}
#define VectorSubtract_f(a,b,c) {(c)[0]=(a)[0]-(b)[0];(c)[1]=(a)[1]-(b)[1];(c)[2]=(a)[2]-(b)[2];}
#define VectorClear_f(a) {(a)[0]=0.0;(a)[1]=0.0;(a)[2]=0.0;}

#define VectorFill_f(a,b) { (a)[0]=(b); (a)[1]=(b); (a)[2]=(b);}
#define VectorAvg_f(a) ( ( (a)[0] + (a)[1] + (a)[2] ) / 3 )

// Of all as-is script that's part of the compile, seems to use the "VectorScale" version, no _f.
// uncertain in what places as-is script called VectorScale_f (define version) for non-compiled.
// So far it seems nowhere ever used the macro (_f now) version, the "VectorScale" method seen in
// mathlib.h/pm_math.c files or cl_dlls/cl_util.h/.cppcpp seems to be used.
#define VectorScale_f(a,b,c) {(c)[0]=(b)*(a)[0];(c)[1]=(b)*(a)[1];(c)[2]=(b)*(a)[2];}


// Only include if we're in the C Lang.  Otherwise there will be issues since C does not support
// method overloading (similar C++ methods from cl_dlls/cl_util.cpp are now in common/vector.cpp).
#ifndef __cplusplus
#include "mathlib.h"
#endif



#endif //END OF #ifndef VECTOR_H