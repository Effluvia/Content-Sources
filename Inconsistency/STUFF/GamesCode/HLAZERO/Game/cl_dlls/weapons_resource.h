
// NEW FILE.
// Why this used to be split across ammohistory.h and ammo.cpp, especially since
// both .h/.cpp pairs exist, I'll never know.

#ifndef WEAPONSRESOURCE_H
#define WEAPONSRESOURCE_H

#include "ammohistory.h"
// includes from ammohistory.h
#include "cl_dll.h"
#include <string.h>



//MODDD - new.
extern int filterSlot(int iSlot);


class WeaponsResource
{
	//MODDD - moved from ammo.cpp.
public:

	WEAPON* gpActiveSel;	// NULL means off, 1 means just the menu bar, otherwise
							// this points to the active weapon menu item
	WEAPON* gpLastSel;		// Last weapon menu selection 

private:
	// Information about weapons & ammo
	WEAPON		rgWeapons[MAX_WEAPONS];	// Weapons Array

	// MODDD - why were these 'one beyond' the max's when nearly all logic everywhere else
	// (assuming one contradiction was a mistake) work only for 'MAX - 1' like any other
	// array logic?  I got nothing, removing the " + 1".
	//----------------------------------
	// counts of weapons * ammo
	// The slots currently in use by weapons.  The value is a pointer to the weapon;  if it's NULL, no weapon is there
	//WEAPON* rgSlots[MAX_WEAPON_SLOTS+1][MAX_WEAPON_POSITIONS+1];
	WEAPON* rgSlots[MAX_WEAPON_SLOTS][MAX_WEAPON_POSITIONS];	// The slots currently in use by weapons.  The value is a pointer to the weapon;  if it's NULL, no weapon is there

	int riAmmo[MAX_AMMO_TYPES];							// count of each ammo type

public:
	void Init(void)
	{
		memset(rgWeapons, 0, sizeof rgWeapons);
		Reset();
	}

	void Reset(void)
	{
		iOldWeaponBits = 0;
		memset(rgSlots, 0, sizeof rgSlots);
		memset(riAmmo, 0, sizeof riAmmo);
	}

	///// WEAPON /////
	int		iOldWeaponBits;

	WEAPON* GetWeapon(int iId) { return &rgWeapons[iId]; }

	void AddWeapon(WEAPON* wp)
	{
		rgWeapons[wp->iId] = *wp;
		LoadWeaponSprites(&rgWeapons[wp->iId]);
	}

	void PickupWeapon(WEAPON* wp);

	void DropWeapon(WEAPON* wp)
	{
		rgSlots[wp->iSlot][wp->iSlotPos] = NULL;
	}

	void DropAllWeapons(void)
	{
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			if (rgWeapons[i].iId)
				DropWeapon(&rgWeapons[i]);
		}
	}

	void LoadWeaponSprites(WEAPON* wp);
	void LoadAllWeaponSprites(void);

	WEAPON* GetWeaponSlot(int slot, int pos);
	WEAPON* GetFirstPos(int iSlot);
	void SelectSlot(int iSlot, int fAdvance, int iDirection);
	WEAPON* GetNextActivePos(int iSlot, int iSlotPos);

	int HasAmmo(WEAPON* p);

	///// AMMO /////
	AMMO GetAmmo(int iId) { return iId; }

	void SetAmmo(int iId, int iCount) { riAmmo[iId] = iCount; }

	int CountAmmo(int iId);

	SpriteHandle_t* GetAmmoPicFromWeapon(int iAmmoId, wrect_t& rect);


	void CancelSelection(void);
	void Think(void);

};




#endif //WEAPONSRESOURCE_H