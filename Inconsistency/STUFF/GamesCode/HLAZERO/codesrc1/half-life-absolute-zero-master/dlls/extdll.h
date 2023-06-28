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

// MODDD NOTE -
// A lot of things from this file have been moved to util_shared.h/.cpp.
// Windows-specific things moved to 'external_lib_include.h'.
// Now, this file is more of a redirect for several other includes instead,
// most commonly serverside or clientside using shared weapon script.

#ifndef EXTDLL_H
#define EXTDLL_H
//
// Global header file for extension DLLs
//

//MODDD - simple way to get this across serverside at least.
#include "build_settings.h"
#include "external_lib_include.h"

// MODDD - big OS-check section moved to external_lib_include.h

// MODDD - the "external_lib_include.h" include above handles this.
//#include "stdio.h"
//#include "stdlib.h"
// Misc C-runtime library headers
//#include "math.h"

// Shared engine/DLL constants
#include "const.h"   //includes "common/vector.h" too!
#include "progdefs.h"
#include "edict.h"

// Shared header describing protocol between engine and DLLs
#include "eiface.h"
// Shared header between the client DLL and the game DLLs
#include "cdll_dll.h"

#endif //EXTDLL_H
