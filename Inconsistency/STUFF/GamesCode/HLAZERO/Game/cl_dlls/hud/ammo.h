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

#ifndef __AMMO_H__
#define __AMMO_H__

#include "hudbase.h"

#include "ammohistory.h"
#include "weapons_resource.h"
//all constants / struct stuff moved to ammohistory.h. It's the only place that uses that stuff and ammohistory.h is already included by ammo.cpp for its weapon information.



extern client_sprite_t* GetSpriteList(client_sprite_t* pList, const char* psz, int iRes, int iCount);


//
//-----------------------------------------------------
//
class CHudAmmo: public CHudBase
{
public:

	// private, huh?               no
//private:
public:
	float m_fFade;
	RGBA  m_rgba;
	WEAPON* m_pWeapon;
	int m_HUD_bucket0;
	int m_HUD_selection;
	//MODDD - new sprite index
	int m_HUD_slash;

	//MODDD - new
	int recentOnTarget;

	//MODDD - Sprite Graphics for power canisters
	int m_antidoteindex;
	int m_adrenalineindex;
	int m_radiationindex;

	//int m_antidote_emptyindex;
	//int m_adrenaline_emptyindex;
	//int m_radiation_emptyindex;

	//MODDD - new to refer to the number of each power canister the player has
	int m_antidotes;
	int m_adrenalines;
	int m_radiations;
	//MODDD - other new things...
	float m_airTankAirTime;
	float m_longJumpCharge;

	float m_e_battery_empty;
	float m_e_battery_full;

	int m_longjump_empty;
	int m_longjump_full;
	int m_airtank_empty;
	int m_airtank_full;

	int m_HUD_weapons_categorybackground;

	//SpriteHandle_t* alphaCrossHair;
	//wrect_t* alphaCrossHairRect;
	//int alphaCrossHairIndex;  this is no longer an instance variable.  It is global, as seen in "cl_util.h".

	//MODDD - moved from ammo.cpp.
	WeaponsResource gWR;



public:

	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	void Think(void);
	void Reset(void);
	
	//MODDD
	void updateCrosshair(void);

	 

	int DrawWList(float flTime);
	int MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf );

	int MsgFunc_HasGlockSil(const char *pszName, int iSize, void *pbuf);

	//MODDD - NEW EVENTS FOR POWER CANISTERS
	int MsgFunc_AntidoteP(const char *pszName, int iSize, void *pbuf );
	int MsgFunc_AdrenalineP(const char *pszName, int iSize, void *pbuf );
	int MsgFunc_RadiationP(const char *pszName, int iSize, void *pbuf );
	//MODDD - for other new things...
	int MsgFunc_UpdTankTime(const char *pszName, int iSize, void *pbuf );
	int MsgFunc_UpdLJCharge(const char *pszName,  int iSize, void *pbuf );
	
	int MsgFunc_ClearWpn(const char *pszName,  int iSize, void *pbuf );


	//int MsgFunc_UpdateCam(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_UpdACH(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_UpdFrz(const char *pszName, int iSize, void *pbuf);
	
	
	void SlotInput( int iSlot );
	void _cdecl UserCmd_Slot1( void );
	void _cdecl UserCmd_Slot2( void );
	void _cdecl UserCmd_Slot3( void );
	void _cdecl UserCmd_Slot4( void );
	void _cdecl UserCmd_Slot5( void );
	void _cdecl UserCmd_Slot6( void );
	void _cdecl UserCmd_Slot7( void );
	void _cdecl UserCmd_Slot8( void );
	void _cdecl UserCmd_Slot9( void );
	void _cdecl UserCmd_Slot10( void );
	void _cdecl UserCmd_Close( void );
	void _cdecl UserCmd_NextWeapon( void );
	void _cdecl UserCmd_PrevWeapon( void );

	//MODDD - constructor added.
	CHudAmmo::CHudAmmo();


};




#endif