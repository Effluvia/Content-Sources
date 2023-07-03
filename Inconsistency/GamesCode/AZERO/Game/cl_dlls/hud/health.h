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

#ifndef HEALTH_H
#define HEALTH_H

#include "hudbase.h"


// !!! IMPORTANT!   Keep this in synch with the number of things in
// giDmgFlags.
// Aha!  How about just piggybacking off other constants that count this correctly anyway.
#define DMGICO_NUM_DMG_TYPES		itbd_COUNT

// ALSO, do not assume a direct connection between the giDmgFlags array of
// damage types for icons and the list of timed damages in 
// cbase.h (itbd_...) simply from order alone.
// Many of the damage types associated with those are here there
// (itbd_Poison -> DMG_POISON), but not necessarily in the same 
// order and not all of them.  DMG_SHOCK is in giDmgFlags but not 
// in the 'itbd' list.  DMY_PARALYZE is in the itbd_ list (itbd_paralyze),
// but not giDmgFlags.


#define DMGICO_FIRST_MOD_TYPE_INDEX itbd_BITMASK2_FIRST
// Keeps track of the divide between damage types that should be checked
// from m_bitsDamage or m_bitsDamageMod.
// It is a count, not a power of 2, from the first item in giDmgFlags.
// If the first 8 types come from m_bitsDamage (0 thru 7), a choice of 8
// here makes sense to start checking m_bitsDamageMod instead for further
// damage types.


#define DMGICO_DURATION		2	// seconds that image is up


// In short, keep giDmgFlags up-to-date with new damage types (DMG_...)
// that have corresponding hud icons to show the user when that damage
// type is present.  A damage type being present in 'giDmgFlags' allows
// it to be checked and use that same offset into hud.txt as the hud icon
// to show.
// giDmgFlags[0] member uses 'm_HUD_dmg_icon_start ' itself,
// giDmgFlags[1] uses the icon after, m_HUD_dmg_icon_start[3] uses the 
// fourth icon, etc. See what they are in sprites/hud.txt, starting with
// dmg_bio.


//MODDD - damage type info (DMG_) moved to util_shared.h.



typedef struct
{
	float fExpire;
	float fBaseline;
	int x, y;
} DAMAGE_IMAGE;


//
//-----------------------------------------------------
//

// OK. HOW IN THE HELL DOES THIS KNOW WHAT "CHudBase" IS??? NO INCLUDES IN THIS FILE. I'M LOSING MY FECKIN' MIND OVER THIS.
// Apparently this is only included by hud.h after it has declared the CHudBase class.  ...HACKY. but it works.
// Nah, just include hudbase.h with only the essentials so we don't run into redinition issues. Now CHudHealth can be
// included anywhere in hud.h instead, like at the top for neatness.
class CHudHealth: public CHudBase
{
private:
	DAMAGE_IMAGE m_dmg[DMGICO_NUM_DMG_TYPES];
	int m_bitsDamage;
	int m_bitsDamageMod;

	int itemFlashGiven;
	int itemFlashColorStartR;
	int itemFlashColorStartG;
	int itemFlashColorStartB;
	float itemFlashCumulative;

public:
	int m_iHealth;
	int m_HUD_dmg_icon_start;
	int m_HUD_cross;
	float m_fFade;

	//MODDD - constructor given.
	CHudHealth(void);

	virtual int Init( void );
	virtual int VidInit( void );
	virtual int Draw(float fTime);
	virtual void Reset( void );
	int MsgFunc_Health(const char *pszName,  int iSize, void *pbuf);
	int MsgFunc_HUDItemFsh(const char *pszName,  int iSize, void *pbuf);


	void GetPainColor( int &r, int &g, int &b );

	//MODDD - new
	void deriveColorFromHealth(int &r, int &g, int &b, int &a);
	void deriveColorFromHealth(int &r, int &g, int &b);
	

	//MODDD
	//void UpdateTiles(float fTime, long bits);
	void UpdateTiles(float fTime, long bits, long bitsMod);

private:
	int DrawDamage(float fTime);
	int DrawItemFlash(float flTime);

	void drawTimedDamageIcon(int arg_index, const int& r, const int& g, const int& b);
	void drawTimedDamageIcon(int arg_index, int arg_draw_x, int arg_draw_y, const int& r, const int& g, const int& b);

};	


#endif //END OF HEALTH_H
