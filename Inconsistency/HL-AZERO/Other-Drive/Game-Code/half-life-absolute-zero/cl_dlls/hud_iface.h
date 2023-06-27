//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( HUD_IFACEH )
#define HUD_IFACEH
#pragma once

//MODDD - incredible.  This define has no impact, guaranteed.
// Erase this or make it derive to nonsense, nothing about the compile is affected.
//#define EXPORT		_declspec( dllexport )

//MODDD - _DLLEXPORT define moved to UTIL_SHARED.  eh, why not.

#include "wrect.h"
#include "../engine/cdll_int.h"
extern cl_enginefunc_t gEngfuncs;

#endif