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
/*

===== globals.cpp ========================================================

  DLL-wide global variable definitions.
  They're all defined here, for convenient centralization.
  Source files that need them should "extern ..." declare each
  variable, to better document what globals they care about.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "soundent.h"

DLL_GLOBAL ULONG	g_ulFrameCount;
DLL_GLOBAL ULONG	g_ulModelIndexEyes;
DLL_GLOBAL ULONG	g_ulModelIndexPlayer;
DLL_GLOBAL Vector	g_vecAttackDir;
DLL_GLOBAL int		g_iSkillLevel;
DLL_GLOBAL int		gDisplayTitle;
DLL_GLOBAL BOOL		g_fGameOver;

//MODDD - ugh.  Hate doing it this way but we do rarely ever need to see bitsDamageType's in Killed or BecomeDead.
DLL_GLOBAL int      g_bitsDamageType;
DLL_GLOBAL int      g_bitsDamageTypeMod;
// And a var to record whether this BecomeDead call should even involve g_vecAttackDir or the damage types.
// If they're not set first, it doesn't make sense to trust them.
// And why not default our globals anyway?
DLL_GLOBAL BOOL		g_tossKilledCall = FALSE;

DLL_GLOBAL float	g_rawDamageCumula = 0;


//MODDD - now in util_shared.cpp.
//DLL_GLOBAL const Vector	g_vecZero = Vector(0,0,0);

//MODDD - NOTICE: g_Language is linked to the non-existent CVar "sv_language".  Disregard this.
//Looks like the only point was to remove gore / use alternate models for humans when g_Language (sv_language) is German.
//Using the new CVar "sv_germancensorship" instead, just 0 or 1 (off or on).
DLL_GLOBAL int		g_Language;
