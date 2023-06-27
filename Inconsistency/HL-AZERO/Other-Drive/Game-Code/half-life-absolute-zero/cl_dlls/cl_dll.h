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
//
//  cl_dll.h
//

// 4-23-98  JOHN

//
//  This DLL is linked by the client when they first initialize.
// This DLL is responsible for the following tasks:
//		- Loading the HUD graphics upon initialization
//		- Drawing the HUD graphics every frame
//		- Handling the custum HUD-update packets
//

#ifndef CL_DLL_H
#define CL_DLL_H

//MODDD - some typedef's moved to const.h
#include "const.h"
#include "vector.h"  //MODDD - used to be util_vector.h. Now the shared version.

//MODDD - Removed. Another case where the EXPORT define is completely ineffective, removed or derived to garbage.
//#define EXPORT	_declspec( dllexport )

#include "../engine/cdll_int.h"
#include "../dlls/cdll_dll.h"

extern cl_enginefunc_t gEngfuncs;


#endif //END OF CL_DLL_H
