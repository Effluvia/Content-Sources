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
//  cdll_dll.h

// this file is included by both the game-dll and the client-dll,

#ifndef CDLL_DLL_H
#define CDLL_DLL_H

//MODDD - no includes, eh?  I too, live dangerously.
// ...not really.
#include "build_settings.h"
#include "const.h"



// MODDD - I didn't make that "???", don't look at me.
// Likely a hard limit by 'int' capacity in sending a bitmask of equipped items (yes/no per bit each)
// between the server and client.
#define MAX_WEAPONS		32		// ???


//MODDD - NOTE.
// This is the groups at the top of the screen for weapons to go to (1, 2, 3, 4, 5, etc.).
// See "MAX_WEAPON_POSITIONS" below for the max number of weapons are allowed to belong to the same
// slot (scroll down thru mousewheel or pressing the same number key).
// And, pretty sure the terms 'bucket' and 'slot' are interchangable here.
// If any weapon's 'iSlot' or 'iSlotPos' goes out of these limits, welcome to flava town.
// Or... horrific untracable C/C++ crashes.  Yay.
#if SPLIT_ALIEN_WEAPONS_INTO_NEW_SLOT != 1
	#define MAX_WEAPON_SLOTS		5	// hud item selection slots
#else
	// Have an extra slot for the alien throwables to go to.
	#define MAX_WEAPON_SLOTS		6
#endif


//MODDD - moved from weapons_resource.h (and also used to be in ammohistory.h).
// ALSO, it is a mystery to me why this got passed along the same "MAX_WEAPON_SLOTS" constant.
// Doing so makes it a square: one more or less weapon slot changes the positions by that same for 
// no real reason.  Just be a literal number too.
// this is the max number of items in each bucket
//#define MAX_WEAPON_POSITIONS		MAX_WEAPON_SLOTS
#define MAX_WEAPON_POSITIONS		5

#define MAX_ITEM_TYPES			6	// hud item selection slots


#define HIDEHUD_WEAPONS		( 1<<0 )
#define HIDEHUD_FLASHLIGHT	( 1<<1 )
#define HIDEHUD_ALL			( 1<<2 )
#define HIDEHUD_HEALTH		( 1<<3 )

//MODDD - MAX_AMMO_SLOTS constant removed, any references replaced with MAX_AMMO_TYPES.
// 'SLOTS' isn't a fitting way to refer to different ammo choices anyway.
#define MAX_AMMO_TYPES	32

#define HUD_PRINTNOTIFY		1
#define HUD_PRINTCONSOLE	2
#define HUD_PRINTTALK		3
#define HUD_PRINTCENTER		4


#define WEAPON_SUIT			31

#endif