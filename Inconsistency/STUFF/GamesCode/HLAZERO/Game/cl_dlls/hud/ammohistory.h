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
// ammohistory.h
//

#ifndef AMMOHISTORY_H
#define AMMOHISTORY_H

#include "cl_dll.h"
#include <string.h>


class HistoryResource;


#define MAX_WEAPON_NAME 128

//MODDD - and why is this flag redefined like this?  It's set as ITEM_FLAG_SELECTONEMPTY in specific
// weapon's files (the satchel is the only one that uses this though).
// weapons.h is even included client & serverside.  But not for weapons_resource.cpp to see?
// Fine, ITEM_FLAG's moved to const.h then to be useable absolutely everywhere.
//#define WEAPON_FLAGS_SELECTONEMPTY 1

typedef int AMMO;
extern HistoryResource gHR;



struct WEAPON
{
	char szName[MAX_WEAPON_NAME];
	int iAmmoType;
	int iAmmo2Type;
	int iMax1;
	int iMax2;
	int iSlot;
	int iSlotPos;
	int iFlags;
	int iId;
	int iClip;

	int iCount;  // # of itesm in plist
	
	//MODDD - new
	BOOL fForceNoSelectOnEmpty;

	SpriteHandle_t hActive;
	wrect_t rcActive;
	SpriteHandle_t hInactive;
	wrect_t rcInactive;
	SpriteHandle_t hAmmo;
	wrect_t rcAmmo;
	SpriteHandle_t hAmmo2;
	wrect_t rcAmmo2;
	SpriteHandle_t hCrosshair;
	wrect_t rcCrosshair;
	SpriteHandle_t hAutoaim;
	wrect_t rcAutoaim;
	SpriteHandle_t hZoomedCrosshair;
	wrect_t rcZoomedCrosshair;
	SpriteHandle_t hZoomedAutoaim;
	wrect_t rcZoomedAutoaim;
};


//MODDD - WeaponsResource class moved to the new 'weaponsresource.h'.


#define MAX_HISTORY 12
enum {
	HISTSLOT_EMPTY,
	HISTSLOT_AMMO,
	HISTSLOT_WEAP,
	HISTSLOT_ITEM,
};

class HistoryResource
{
private:
	struct HIST_ITEM {
		int type;
		float DisplayTime;  // the time at which this item should be removed from the history
		int iCount;
		int iId;
	};

	HIST_ITEM rgAmmoHistory[MAX_HISTORY];

public:

	void Init( void )
	{
		Reset();
	}

	void Reset( void )
	{
		memset( rgAmmoHistory, 0, sizeof rgAmmoHistory );
	}

	int iHistoryGap;
	int iCurrentHistorySlot;

	void AddToHistory( int iType, int iId, int iCount = 0 );
	void AddToHistory( int iType, const char *szName, int iCount = 0 );

	void CheckClearHistory( void );
	int DrawAmmoHistory( float flTime );
};


#endif //END OF AMMOHISTORY_H


