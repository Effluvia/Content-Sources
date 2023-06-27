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
// Ammo.cpp
//
// implementation of CHudAmmo class
//

#include "external_lib_include.h"
#include "ammo.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

//MODDD - Already in external_lib_include.h
//#include <string.h>
//#include <stdio.h>

#include "ammohistory.h"
#include "vgui_TeamFortressViewport.h"

#include "hl/hl_weapons.h"
#include "dlls/entity/player.h"


// Note about all the 'filterSlot' calls.
// These are for support of the new CVar 'hud_swapFirstTwoBuckets', which switches the 1st and
// 2nd buckets/slots (interchangable terms) if turned on.
// It is best to interject 'filterSlot' any requests for a weapon's slot to exchange 0 for 1,
// 1 for 0 if necessary, to work the most cleanly with existing script without huge changes.
// filterSlot is a small global method in weapons_resource.h/.cpp.





extern int global2PSEUDO_playerHasGlockSilencer;
EASY_CVAR_EXTERN(hud_version)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)
EASY_CVAR_EXTERN(hud_drawammobar)
EASY_CVAR_EXTERN(hud_weaponselecthideslower)
EASY_CVAR_EXTERN(hud_moveselect_mousewheelsound)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(drawViewModel)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(drawHUD)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros)
EASY_CVAR_EXTERN(hud_rpg_clipless)



//MODDD - why were these define's down a ways??
// width of ammo fonts
#define AMMO_SMALL_WIDTH 10
#define AMMO_LARGE_WIDTH 20

#define HISTORY_DRAW_TIME	"5"



//MODDD - gpActiveSel and gpLastSel moved to the WeaponsResource (ammohistory.h) class for outside access.
// Also, the WeaponsResource instance, gWR, has been moved to the CHudAmmo class
// NOW, the WeaponsResource class has been moved to its own file, 'weaponsresource.h'.  Go figure.
// Having a class (method prototypes) and method implementations in a .h/.cpp that don't even share
// the same name (ammohistory.h/ ammo.cpp) was just wonky as fuck.  Both .h and .cpp files even exist.
// Simply... why.

// MODDD - read by input.cpp for sending to the server, I think?
// Now accessed across here and weapons_resource.cpp, so why not just be stored to input.cpp instead?
extern int g_weaponselect;


int giBucketHeight;
int giBucketWidth;
int giABHeight;
int giABWidth; // Ammo Bar width and height

SpriteHandle_t ghsprBuckets;					// Sprite for top row of weapons menu




//MODDD - new stuff
DECLARE_MESSAGE(m_Ammo, AntidoteP)
DECLARE_MESSAGE(m_Ammo, AdrenalineP)
DECLARE_MESSAGE(m_Ammo, RadiationP)
DECLARE_MESSAGE(m_Ammo, UpdTankTime)
DECLARE_MESSAGE(m_Ammo, UpdLJCharge)
DECLARE_MESSAGE(m_Ammo, ClearWpn)
//DECLARE_MESSAGE(m_Ammo, UpdateCam)

//that is, UpdateAlphaCrosshair
DECLARE_MESSAGE(m_Ammo, UpdACH)
DECLARE_MESSAGE(m_Ammo, UpdFrz )

DECLARE_MESSAGE( m_Ammo, HasGlockSil )

DECLARE_MESSAGE(m_Ammo, CurWeapon );	// Current weapon and clip
DECLARE_MESSAGE(m_Ammo, WeaponList);	// new weapon type
DECLARE_MESSAGE(m_Ammo, AmmoX);			// update known ammo type's count
DECLARE_MESSAGE(m_Ammo, AmmoPickup);	// flashes an ammo pickup record
DECLARE_MESSAGE(m_Ammo, WeapPickup);    // flashes a weapon pickup record
DECLARE_MESSAGE(m_Ammo, HideWeapon);	// hides the weapon, ammo, and crosshair displays temporarily
DECLARE_MESSAGE(m_Ammo, ItemPickup);

//MODDD CUSTOM MESSAGE



DECLARE_COMMAND(m_Ammo, Slot1);
DECLARE_COMMAND(m_Ammo, Slot2);
DECLARE_COMMAND(m_Ammo, Slot3);
DECLARE_COMMAND(m_Ammo, Slot4);
DECLARE_COMMAND(m_Ammo, Slot5);
DECLARE_COMMAND(m_Ammo, Slot6);
DECLARE_COMMAND(m_Ammo, Slot7);
DECLARE_COMMAND(m_Ammo, Slot8);
DECLARE_COMMAND(m_Ammo, Slot9);
DECLARE_COMMAND(m_Ammo, Slot10);
DECLARE_COMMAND(m_Ammo, Close);
DECLARE_COMMAND(m_Ammo, NextWeapon);
DECLARE_COMMAND(m_Ammo, PrevWeapon);



int CHudAmmo::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(CurWeapon);
	HOOK_MESSAGE(WeaponList);
	HOOK_MESSAGE(AmmoPickup);
	HOOK_MESSAGE(WeapPickup);
	HOOK_MESSAGE(ItemPickup);
	HOOK_MESSAGE(HideWeapon);
	HOOK_MESSAGE(AmmoX);

	//MODDD - CUSTOM MESSAGE
	HOOK_MESSAGE(AntidoteP);
	HOOK_MESSAGE(AdrenalineP);
	HOOK_MESSAGE(RadiationP);
	HOOK_MESSAGE(UpdTankTime);
	HOOK_MESSAGE(UpdLJCharge);
	HOOK_MESSAGE(ClearWpn);
	HOOK_MESSAGE(UpdACH);
	HOOK_MESSAGE(UpdFrz);


	HOOK_MESSAGE(HasGlockSil);


	HOOK_COMMAND("slot1", Slot1);
	HOOK_COMMAND("slot2", Slot2);
	HOOK_COMMAND("slot3", Slot3);
	HOOK_COMMAND("slot4", Slot4);
	HOOK_COMMAND("slot5", Slot5);
	HOOK_COMMAND("slot6", Slot6);
	HOOK_COMMAND("slot7", Slot7);
	HOOK_COMMAND("slot8", Slot8);
	HOOK_COMMAND("slot9", Slot9);
	HOOK_COMMAND("slot10", Slot10);
	HOOK_COMMAND("cancelselect", Close);
	HOOK_COMMAND("invnext", NextWeapon);
	HOOK_COMMAND("invprev", PrevWeapon);

	Reset();

	CVAR_CREATE( "hud_drawhistory_time", HISTORY_DRAW_TIME, 0 );
	CVAR_CREATE( "hud_fastswitch", "0", FCVAR_ARCHIVE );		// controls whether or not weapons can be selected in one keypress

	m_iFlags |= HUD_ACTIVE; //!!!

	gWR.Init();
	gHR.Init();

	return 1;
};

void CHudAmmo::Reset(void)
{
	m_fFade = 0;
	m_iFlags |= HUD_ACTIVE; //!!!

	gWR.gpActiveSel = NULL;

	gWR.gpLastSel = NULL; //MODDD - new, safety.
	

	gHUD.m_iHideHUDDisplay = 0;

	gWR.Reset();
	gHR.Reset();

	//	VidInit();
}



int CHudAmmo::VidInit(void)
{
	// Load sprites for buckets (top row of weapon menu)

	//may as well see if this needs re-loading.
	if(gHUD.alphaCrossHairIndex == -1){
		gHUD.alphaCrossHairIndex = gHUD.GetSpriteIndex( "alphacrosshair" );
		if(gHUD.alphaCrossHairIndex == -1){
			easyPrint("ALPHACROSSHAIR NOT LOADED B\n");
		}else{
			//easyPrint("ALPHACROSSHAIR LOADED\n");
		}
	}


	//MODDD - replaced, remove that too later, now completely unused either way.
	//m_HUD_bucket0 = gHUD.GetSpriteIndex( "bucket1" );
	m_HUD_weapons_categorybackground = gHUD.GetSpriteIndex( "weapons_categorybackground");

	
	m_HUD_selection = gHUD.GetSpriteIndex( "selection" );

	//MODDD - initialize new index
	m_HUD_slash = gHUD.GetSpriteIndex("number_slash");
	m_antidoteindex = gHUD.GetSpriteIndex("antidote");
	m_adrenalineindex = gHUD.GetSpriteIndex("adrenaline");
	m_radiationindex = gHUD.GetSpriteIndex("radiation");

	//m_antidote_emptyindex = gHUD.GetSpriteIndex("antidote_empty");
	//m_adrenaline_emptyindex = gHUD.GetSpriteIndex("adrenaline_empty");
	//m_radiation_emptyindex = gHUD.GetSpriteIndex("radiation_empty");


	m_longjump_empty = gHUD.GetSpriteIndex("longjump_empty");
	m_longjump_full = gHUD.GetSpriteIndex("longjump_full");
	m_airtank_empty = gHUD.GetSpriteIndex("airtank_empty");
	m_airtank_full = gHUD.GetSpriteIndex("airtank_full");
	
	
	// In this file, since the battery shows up in the right-hand sidebar
	// under certain hud_version choices
	m_e_battery_empty = gHUD.GetSpriteIndex("e_battery_empty");
	m_e_battery_full = gHUD.GetSpriteIndex("e_battery_full");
	

	//And the alphacrosshair

	/*
	alphaCrossHair = &gHUD.GetSprite(gHUD.GetSpriteIndex( "alphacrosshair" )) ;
	alphaCrossHairRect = &gHUD.GetSpriteRect(gHUD.GetSpriteIndex( "alphacrosshair" ));
	*/
	

	ghsprBuckets = gHUD.GetSprite(m_HUD_weapons_categorybackground);
	giBucketWidth = gHUD.GetSpriteRect(m_HUD_weapons_categorybackground).right - gHUD.GetSpriteRect(m_HUD_weapons_categorybackground).left;
	giBucketHeight = gHUD.GetSpriteRect(m_HUD_weapons_categorybackground).bottom - gHUD.GetSpriteRect(m_HUD_weapons_categorybackground).top;

	gHR.iHistoryGap = max( gHR.iHistoryGap, gHUD.GetSpriteRect(m_HUD_weapons_categorybackground).bottom - gHUD.GetSpriteRect(m_HUD_weapons_categorybackground).top);

	// If we've already loaded weapons, let's get new sprites
	gWR.LoadAllWeaponSprites();

	if (ScreenWidth >= 640)
	{
		giABWidth = 20;
		giABHeight = 4;
	}
	else
	{
		giABWidth = 10;
		giABHeight = 2;
	}

	return 1;
}



//
// Think:
//  Used for selection of weapon menu item.
//
void CHudAmmo::Think(void)
{
	//MODDD - redirect to the gWR (weapons_resource.cpp) Think method instead.
	// Almost everything involved the gWR, more at home there.
	gWR.Think();
}



//------------------------------------------------------------------------
// Message Handlers
//------------------------------------------------------------------------

//
// AmmoX  -- Update the count of a known type of ammo
// 
int CHudAmmo::MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	gWR.SetAmmo( iIndex, abs(iCount) );

	return 1;
}

int CHudAmmo::MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	// Add ammo to the history
	gHR.AddToHistory( HISTSLOT_AMMO, iIndex, abs(iCount) );

	return 1;
}

int CHudAmmo::MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iIndex = READ_BYTE();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_WEAP, iIndex );

	return 1;
}

int CHudAmmo::MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	const char *szName = READ_STRING();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_ITEM, szName );

	return 1;
}

int CHudAmmo::MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	
	//surprisingly, this seems not too important in regards to the crosshair.  Scripted stuff won't be using this I don't think.
	//easyPrintLine("!!!!!!!!!!HIDE WEAPON IS HERE!!!!   PAY ATTNETION !!!??!!!!!");

	gHUD.m_iHideHUDDisplay = READ_BYTE();

	if (gEngfuncs.IsSpectateOnly())
		return 1;

	if ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
	{
		static wrect_t nullrc;
		gWR.gpActiveSel = NULL;
		SetCrosshairFiltered( 0, nullrc, 0, 0, 0 );
	}
	else
	{
		//if ( m_pWeapon )
		//	SetCrosshairFiltered( m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255 );

		//MODDD TODO - is this replacement a good idea? Verify!
		updateCrosshair();


	}

	return 1;
}

//MODDD - constructor added.
CHudAmmo::CHudAmmo(){
	//easyPrint("CONSTRUCTA\n");

	//no, that's part of the gHUD (CHUD).
	//alphaCrossHairIndex = -1;
	
	m_antidotes = 0;
	m_adrenalines = 0;
	m_radiations = 0;
	//safe default, since it might be possible to refer to this var before picking up any antidotes.
	m_airTankAirTime = 0;
	m_longJumpCharge = -1;



}






	
int CHudAmmo::MsgFunc_ClearWpn(const char *pszName, int iSize, void *pbuf ){

	//???
	//BEGIN_READ( pbuf, iSize );
	//int x = READ_SHORT();


	//Forcing this reset!
	//easyPrintLine("*********WEAP RESET!!!!!!!!!!***********");
	m_pWeapon = NULL;

	//MODDD - this as well?
	gWR.gpActiveSel = NULL;

	updateCrosshair();

	return 1;
}



// 
//  CurWeapon: Update hud state with the current weapon and clip count. Ammo
//  counts are updated with AmmoX. Server assures that the Weapon ammo type 
//  numbers match a real ammo type.
//
int CHudAmmo::MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf )
{
	static wrect_t nullrc;
	int fOnTarget = FALSE;

	BEGIN_READ( pbuf, iSize );

	int iState = READ_BYTE();
	int iId = READ_CHAR();
	int iClip = READ_CHAR();

	// detect if we're also on target
	//MODDD - NOTICE.  This should be what counts WEAPON_IS_ONTARGET (clearly over 1)
	if ( iState > 1 )
	{
		fOnTarget = TRUE;
	}

	//save to instance-var for seeing this later.
	recentOnTarget = fOnTarget;


	if ( iId < 1 )
	{
		//MODDD - we do indeed want to force the crosshair off, don't just let the alpha crosshair being possible change anything.
		//SetCrosshairFiltered(0, nullrc, 0, 0, 0);
		//SetCrosshairFiltered(0, nullrc, 0, 0, 0, TRUE);
		//or... leave what to do up to "updateCrosshair" as usual? that is probably best.
		//but the point of this was to set the current weapon to NULL.
		m_pWeapon = NULL;
		gWR.gpActiveSel = NULL;
		updateCrosshair();


		// HACKY.  See if this helps with some issues.
		localPlayer.m_bHolstering = FALSE;
		localPlayer.m_chargeReady &= ~128;

		return 0;
	}

	if ( g_iUser1 != OBS_IN_EYE )
	{
		// Is player dead???
		if ((iId == -1) && (iClip == -1))
		{
			//MODDD - this is not a decent check to see if the player is dead or not.
			//At least, not since making a fairly influential change in player.cpp.
			//gHUD.m_fPlayerDead = TRUE;
			gWR.gpActiveSel = NULL;
			return 1;
		}
		//gHUD.m_fPlayerDead = FALSE;
	}

	WEAPON *pWeapon = gWR.GetWeapon( iId );
	

	if ( !pWeapon )
		return 0;


	//MODDD - wait... what.
	// If under -1 (-2, -3, etc.), interpret it as positive.    what.   w-why.  Why would any other negative be used.
	/*
	if ( iClip < -1 )
		pWeapon->iClip = abs(iClip);
	else
		pWeapon->iClip = iClip;
	*/

	if (iClip < -1) {
		int x = 45;    // NOT POSSIBLE???
	}

	// See if this casues any problems really.
	pWeapon->iClip = iClip;






	if ( iState == 0 )	// we're not the current weapon, so update no more
		return 1;

	m_pWeapon = pWeapon;

	updateCrosshair();
	
	//MODDD- this is forcing the alpha crosshair.
	m_fFade = 200.0f; //!!!
	m_iFlags |= HUD_ACTIVE;
	
	return 1;
}

//
// WeaponList -- Tells the hud about a new weapon type.
//
int CHudAmmo::MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	
	WEAPON Weapon;

	strcpy( Weapon.szName, READ_STRING() );
	Weapon.iAmmoType = (int)READ_CHAR();	
	
	Weapon.iMax1 = READ_BYTE();

	//MODDD - NOTE.  No edit, just mentioning.  255 is really -1 in disguise, as these BYTEs are unsigned,
	// -1 is forced to the highest int possible for the type instead.
	if (Weapon.iMax1 == 255) {
		Weapon.iMax1 = -1;
	}

	Weapon.iAmmo2Type = READ_CHAR();
	Weapon.iMax2 = READ_BYTE();

	if (Weapon.iMax2 == 255) {
		Weapon.iMax2 = -1;
	}

	Weapon.iSlot = READ_CHAR();
	Weapon.iSlotPos = READ_CHAR();
	Weapon.iId = READ_CHAR();
	Weapon.iFlags = READ_BYTE();
	Weapon.iClip = 0;
	
	//MODDD - off until told otherwise
	Weapon.fForceNoSelectOnEmpty = FALSE;

	gWR.AddWeapon( &Weapon );

	return 1;

}


//MODDD - needless to say, the next 3 methods are new.
int CHudAmmo::MsgFunc_AntidoteP(const char *pszName,  int iSize, void *pbuf )
{

	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();

	m_antidotes = x;

	/*if (x != m_iBat)
	{
		m_fFade = FADE_TIME;
		
	}
	*/

	return 1;
}


int CHudAmmo::MsgFunc_AdrenalineP(const char *pszName,  int iSize, void *pbuf )
{

	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();

	m_adrenalines = x;

	return 1;
}


int CHudAmmo::MsgFunc_RadiationP(const char *pszName,  int iSize, void *pbuf )
{

	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();

	m_radiations = x;

	return 1;
}

int CHudAmmo::MsgFunc_UpdTankTime(const char *pszName,  int iSize, void *pbuf )
{

	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();

	m_airTankAirTime = ((float)x) / 100;

	return 1;
}

int CHudAmmo::MsgFunc_UpdLJCharge(const char *pszName,  int iSize, void *pbuf )
{

	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();

	float xmod = ((float)x) / 100;

	//easyPrint("CONSTRUCTA %.3f\n", xmod);

	m_longJumpCharge = xmod;

	return 1;
}



/*
int CHudAmmo::MsgFunc_UpdateCam(const char *pszName, int iSize, void *pbuf){

	
	//gHUD.m_iLastCameraMode;

	if(gHUD.CVar_cameraModeMem != CL_IsThirdPerson()){

		if(CL_IsThirdPerson()){
			gHUD.CVar_cameraMode->value = 1;
		}else{
			gHUD.CVar_cameraMode->value = 0;
		}

		gHUD.CVar_cameraModeMem = CL_IsThirdPerson();
	}
	//gHUD.CVar_cameraMode->value = 1;


	return 1;
}
*/



void CHudAmmo::updateCrosshair(void){
	static wrect_t nullrc;
	int allowCrosshairUpdate = FALSE;

	//easyPrintLine("My Weapon?? %s flag:%d  count:%d clip:%d",  m_pWeapon!=NULL?m_pWeapon->szName:"NULL", m_pWeapon!=NULL?m_pWeapon->iFlags:-1, m_pWeapon!=NULL?m_pWeapon->iCount:-1, m_pWeapon!=NULL?m_pWeapon->iClip:-1);

	if(m_pWeapon  ){
		allowCrosshairUpdate = TRUE;
		//easyPrintLine("CROSSHAIRUPDATE: GUN NAME: %s", m_pWeapon->szName);
	}else{

		if(CVAR_GET_FLOAT("crosshair") == 1 && EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns) == TRUE ){
			allowCrosshairUpdate = TRUE;
		}
	}

	if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(alphaCrosshairBlockedOnFrozen)==TRUE && gHUD.frozenMem == TRUE){
		allowCrosshairUpdate = FALSE;
	}

	if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(drawHUD) != 0 && EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(drawHUD) != 1){
		allowCrosshairUpdate = FALSE;
	}

	//easyPrintLine("WHAT update %d %d %d %d", allowCrosshairUpdate, m_pWeapon==NULL, useAlphaCrosshairVal, allowAlphaCrosshairWithoutGunsVal);

	if(allowCrosshairUpdate){

		if(m_pWeapon){

			// CHECK FOR ALTERNATE ZOOMED CROSSHAIRS (only effective for retail crosshairs)
			//MODDD TODO - uh, is this crude "90" check okay?  if we never go back to retail crosshairs probably won't matter.
			// Got it!
			//if ( gHUD.m_iPlayerFOV >= 90 )
			if(gHUD.m_iPlayerFOV >= gHUD.getPlayerBaseFOV() )
			{ // normal crosshairs
				if (recentOnTarget && m_pWeapon->hAutoaim)
					SetCrosshairFiltered(m_pWeapon->hAutoaim, m_pWeapon->rcAutoaim, 255, 255, 255);
				else
					SetCrosshairFiltered(m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255);
			}
			else
			{ // zoomed crosshairs
				if (recentOnTarget && m_pWeapon->hZoomedAutoaim)
					SetCrosshairFiltered(m_pWeapon->hZoomedAutoaim, m_pWeapon->rcZoomedAutoaim, 255, 255, 255);
				else
					SetCrosshairFiltered(m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255);
			}

		}else{
			// send bogus info.
			// It will be overidden by "crosshair 1" if the alpha one is wanted
			SetCrosshairFiltered(0, nullrc, 0, 0, 0);
		}
	}else{
		SetCrosshairFiltered(0, nullrc, 0, 0, 0, TRUE);
	}

	// update these, no sense calling this method over and over
	// without a change since.
	gHUD.crosshairMem = CVAR_GET_FLOAT("crosshair");
	gHUD.allowAlphaCrosshairWithoutGunsMem = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(allowAlphaCrosshairWithoutGuns);

}//updateCrosshair



int CHudAmmo::MsgFunc_UpdACH(const char *pszName, int iSize, void *pbuf){
	updateCrosshair();

	return 1;
}

int CHudAmmo::MsgFunc_UpdFrz(const char *pszName, int iSize, void *pbuf){
	BEGIN_READ( pbuf, iSize );
	int x = READ_BYTE();
	gHUD.frozenMem = x;

	//okay???
	updateCrosshair();

	return 1;
}



int CHudAmmo::MsgFunc_HasGlockSil(const char *pszName, int iSize, void *pbuf){
	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();

	global2PSEUDO_playerHasGlockSilencer = x;

	return 1;
}



//------------------------------------------------------------------------
// Command Handlers
//------------------------------------------------------------------------
// Slot button pressed
void CHudAmmo::SlotInput( int iSlot )
{
	if(!gHUD.m_fPlayerDead){
		//normal behaviour.
		if ( gViewPort && gViewPort->SlotInput(iSlot) ){
			return;
		}

		gWR.SelectSlot(iSlot, FALSE, 1);
	}else{
		if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 1 ){
			if ( gViewPort && gViewPort->SlotInput(iSlot) ){
				return;
			}
			gWR.SelectSlot(iSlot, FALSE, 1);
		}else{
			if ( gViewPort && gViewPort->SlotInput(iSlot) ){
				return;
			}
			//no select attempt available.
		}
	}
	
	//gWR.SelectSlot(iSlot, FALSE, 1);
	/*
	if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 1 || !gHUD.m_fPlayerDead){
		// Let the Viewport use it first, for menus
		if ( gViewPort && gViewPort->SlotInput( iSlot ) ){
			return;
		}

		gWR.SelectSlot(iSlot, FALSE, 1);
	}
	*/
}

void CHudAmmo::UserCmd_Slot1(void)
{
	SlotInput(0);
}

void CHudAmmo::UserCmd_Slot2(void)
{
	SlotInput(1);
}

void CHudAmmo::UserCmd_Slot3(void)
{
	SlotInput( 2 );
}

void CHudAmmo::UserCmd_Slot4(void)
{
	SlotInput( 3 );
}

void CHudAmmo::UserCmd_Slot5(void)
{
	SlotInput( 4 );
}

void CHudAmmo::UserCmd_Slot6(void)
{
	SlotInput( 5 );
}

void CHudAmmo::UserCmd_Slot7(void)
{
	SlotInput( 6 );
}

void CHudAmmo::UserCmd_Slot8(void)
{
	SlotInput( 7 );
}

void CHudAmmo::UserCmd_Slot9(void)
{
	SlotInput( 8 );
}

void CHudAmmo::UserCmd_Slot10(void)
{
	SlotInput( 9 );
}

// Linked to 'cancelselect' in console.     ok.
void CHudAmmo::UserCmd_Close(void)
{
	//MODDD - similar to the new gWR (weapons_resource.cpp) method 'CancelSelection', but
	// leaving the "escape" call here in case that had a point.
	if (gWR.gpActiveSel)
	{
		gWR.gpLastSel = gWR.gpActiveSel;
		gWR.gpActiveSel = NULL;
		PlaySound("common/wpn_hudoff.wav", 1);
	}
	else {
		ClientCmd("escape");
	}
}


// Selects the next item in the weapon menu
void CHudAmmo::UserCmd_NextWeapon(void)
{
	//if ( gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
	if ( (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 0 && gHUD.m_fPlayerDead) || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
		return;

	if ( !gWR.gpActiveSel || gWR.gpActiveSel == (WEAPON*)1 )
		gWR.gpActiveSel = m_pWeapon;

	int pos = 0;
	int slot = 0;
	if ( gWR.gpActiveSel )
	{
		pos = gWR.gpActiveSel->iSlotPos + 1;
		slot = filterSlot(gWR.gpActiveSel->iSlot);
	}


	for ( int loop = 0; loop <= 1; loop++ )
	{
		for ( ; slot < MAX_WEAPON_SLOTS; slot++ )
		{
			for ( ; pos < MAX_WEAPON_POSITIONS; pos++ )
			{
				WEAPON *wsp = gWR.GetWeaponSlot(slot, pos );

				if ( wsp && gWR.HasAmmo(wsp) )
				{
					gWR.gpActiveSel = wsp;

					//MODDD - if this CVar is on, play the weapon-select sound on mousewheeling through.
					if(EASY_CVAR_GET(hud_moveselect_mousewheelsound) == 1){
						gHUD.playWeaponSelectMoveSound();
						
					}
					return;
				}
			}
			pos = 0;
		}
		slot = 0;  // start looking from the first slot again
	}

	gWR.gpActiveSel = NULL;

}

// Selects the previous item in the menu
void CHudAmmo::UserCmd_PrevWeapon(void)
{

	//if ( gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
	if ((EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 0 && gHUD.m_fPlayerDead) || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL))) {
		return;
	}

	if (!gWR.gpActiveSel || gWR.gpActiveSel == (WEAPON*)1) {
		gWR.gpActiveSel = m_pWeapon;
	}

	int pos = MAX_WEAPON_POSITIONS-1;
	int slot = MAX_WEAPON_SLOTS-1;
	if ( gWR.gpActiveSel )
	{
		pos = gWR.gpActiveSel->iSlotPos - 1;
		slot = filterSlot(gWR.gpActiveSel->iSlot);
	}
	
	for ( int loop = 0; loop <= 1; loop++ )
	{
		for ( ; slot >= 0; slot-- )
		{
			for ( ; pos >= 0; pos-- )
			{
				WEAPON *wsp = gWR.GetWeaponSlot(slot, pos );

				if ( wsp && gWR.HasAmmo(wsp) )
				{
					gWR.gpActiveSel = wsp;

					//MODDD - if this CVar is on, play the weapon-select sound on mousewheeling through.
					if(EASY_CVAR_GET(hud_moveselect_mousewheelsound) == 1){
						gHUD.playWeaponSelectMoveSound();
					}

					return;
				}
			}

			pos = MAX_WEAPON_POSITIONS-1;
		}
		
		slot = MAX_WEAPON_SLOTS-1;
	}

	gWR.gpActiveSel = NULL;
}



//-------------------------------------------------------------------------
// Drawing code
//-------------------------------------------------------------------------
int CHudAmmo::Draw(float flTime)
{
	//easyPrintLine("DERP??? hide: %d viewf: %d inter: %d lastcam: %d", gHUD.m_iHideHUDDisplay, gHUD.viewFlags, gHUD.m_iIntermission, gHUD.m_iLastCameraMode);

	int a, x, y, r, g, b;
	int AmmoWidth;

	BOOL ammoClipReserveBlend = FALSE;
	int ammoDrawPrimary = 0;    //Likely to be the primary ammo clip.
	int ammoDrawSecondary = 0;   //Likely to be the total primary ammo. Side by side with ammoDrawPrimary.
	int ammoDrawTertiary = 0;  //Likely to be the total secondary ammo. A separate line from Primary and Secondary.
	BOOL ammoDrawPrimary_draw = FALSE;
	BOOL ammoDrawSecondary_draw = FALSE;
	BOOL ammoDrawTertiary_draw = FALSE;
	int iFlags = DHN_DRAWZERO; // draw 0 values
	WEAPON* pw = m_pWeapon; // shorthand
	int primaryAmmoClip;
	int primaryAmmoTotal;
	int secondaryAmmoTotal;


	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return 1;


	if ( (gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL )) )
		return 1;

	// Draw Weapon Menu
	DrawWList(flTime);

	//Seems to be a preference if these two should go below or above the "if(!gHUD.canDrawBottomStats)" block.
	if (!(m_iFlags & HUD_ACTIVE))
		return 0;


	BOOL forceShowZero = FALSE;
	float sidebarY;

	//if dead, and cannot show the weapon select screen, don't bother drawing ammo.
	//Can depend on "hud_version" too, I think the E3 (yellow) may just never draw the ammo at death or something.
	if(gHUD.m_fPlayerDead){
		
		if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 1){
			//If the weapon select screen can be shown, just show 0 for ammo.
			forceShowZero = TRUE;
		}else{
			//otherwise, don't draw anything here.
			return 0;
		}
		
	}


	//if the weapon select is on, don't draw anything else... except the side-GUI (# of antidotes? adrenaline syringes? radiation syringes?)
	//if(!gHUD.canDrawBottomStats){
	if( gHUD.canDrawSidebar() ){
		
		//each is the same size.
		int inventoryIconWidth = gHUD.GetSpriteRect(m_antidoteindex).right - gHUD.GetSpriteRect(m_antidoteindex).left;
		int inventoryIconHeight = gHUD.GetSpriteRect(m_antidoteindex).bottom - gHUD.GetSpriteRect(m_antidoteindex).top;

		gHUD.getGenericGUIColor(r, g, b);


		//////////////////////////////////////////////////////////////////////
		// Where do I draw the next sidebar square, y coord?
		if (EASY_CVAR_GET(hud_version) <= 1) {
			// drawing the battery icon here in the sidebar?
			// Have a little extra space.  Start a little higher.
			// But see if we have a little more veritcal space,

			// If hud_weaponselecthideslower is 2, this opportunity to show the 
			// ammo/reserve numbers at the same time as weapon select should be taken.
			if (EASY_CVAR_GET(hud_weaponselecthideslower) == 2 && ScreenHeight >= 550) {
				sidebarY = ScreenHeight - (inventoryIconHeight * 7 + (9 * 5)) - 73;
			}
			else {
				// push up slightly more to resemble pre-release sizes at 480 high res,
				// from the bottom at least.
				sidebarY = ScreenHeight - (inventoryIconHeight * 6 + (8 * 5)) - 73 - 17 + 16;
			}
		}
		else if(EASY_CVAR_GET(hud_version) == 2){
			// normal space.
			sidebarY = ScreenHeight - (inventoryIconHeight * 6 + (8 * 5)) - 73;
		}
		else {
			// E3?  hm.
			if (EASY_CVAR_GET(hud_weaponselecthideslower) == 2 && ScreenHeight >= 520) {
				// extra boost then.
				sidebarY = ScreenHeight - (inventoryIconHeight * 6 + (8 * 5)) - 73 - 32;
			}
			else {
				// normal
				sidebarY = ScreenHeight - (inventoryIconHeight * 6 + (8 * 5)) - 73;
			}
		}
		//////////////////////////////////////////////////////////////////////


		//AIR TANK
		//////////////////////////////////////////////////////////////////////
		x = ScreenWidth - inventoryIconWidth - 20;
		y = sidebarY;

		SPR_Set(gHUD.GetSprite(m_airtank_empty), r, g, b );
		SPR_DrawAdditive( 0,  x, y, &gHUD.GetSpriteRect(m_airtank_empty));
		

		SpriteHandle_t spritehandle = gHUD.GetSprite( m_airtank_full );
		wrect_t* spriterect = &gHUD.GetSpriteRect( m_airtank_full );

		if(m_airTankAirTime > 0){
			gHUD.drawPartialFromBottom(spritehandle, spriterect, ( m_airTankAirTime / (float)PLAYER_AIRTANK_TIME_MAX ), x + 11, y + 3, r, g, b);
		}
		
		//////////////////////////////////////////////////////////////////////
		//LONG JUMP CHARGE
		//////////////////////////////////////////////////////////////////////
		
		x = ScreenWidth - inventoryIconWidth - 20;
		sidebarY += (inventoryIconHeight + 8);
		y = sidebarY;

		SPR_Set(gHUD.GetSprite(m_longjump_empty), r, g, b );
		SPR_DrawAdditive( 0,  x, y, &gHUD.GetSpriteRect(m_longjump_empty));
		

		spritehandle = gHUD.GetSprite( m_longjump_full );
		spriterect = &gHUD.GetSpriteRect( m_longjump_full );
		
		if(m_longJumpCharge > 0){
			gHUD.drawPartialFromBottom(spritehandle, spriterect, ( (float)m_longJumpCharge / (float)PLAYER_LONGJUMPCHARGE_MAX ), x + 11, y + 4, r, g, b);
		}
		
		//////////////////////////////////////////////////////////////////////
		// OPTIONAL: BATTERY?  For certain hud_version choices
		//////////////////////////////////////////////////////////////////////
		if(EASY_CVAR_GET(hud_version) <= 1){
			
			x = ScreenWidth - inventoryIconWidth - 20;
			sidebarY += (inventoryIconHeight + 8);
			y = sidebarY;


			SPR_Set(gHUD.GetSprite(m_e_battery_empty), r, g, b );
			SPR_DrawAdditive( 0,  x, y, &gHUD.GetSpriteRect(m_e_battery_empty));
			
			spritehandle = gHUD.GetSprite( m_e_battery_full );
			spriterect = &gHUD.GetSpriteRect( m_e_battery_full );


			int iBatDraw;
			//MODDD - if "batteryHiddenDead" is on and the player is dead, hide the battery amount (show 0).
			if (!gHUD.m_fPlayerDead || EASY_CVAR_GET(hud_batteryhiddendead) != 1) {
				iBatDraw = gHUD.m_Battery.m_iBat;
			}
			else {
				// dead and the CVar demands hiding battery?  Show 0 here.
				iBatDraw = 0;
			}

			if(iBatDraw > 0){
				float fillPortion = ((float)iBatDraw / 100.0f) * (0.95 - 0.15) + 0.15;
				gHUD.drawPartialFromBottom(spritehandle, spriterect, fillPortion, x + 0, y + 0, r, g, b);
			}
		}


		//RADIATION
		//////////////////////////////////////////////////////////////////////
		x = ScreenWidth - inventoryIconWidth - 20;
		sidebarY += (inventoryIconHeight + 8);
		y = sidebarY;

		//if (m_radiations > 0) {
			SPR_Set(gHUD.GetSprite(m_radiationindex), r, g, b);
			SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_radiationindex));
		//}
		//else {
		//	SPR_Set(gHUD.GetSprite(m_radiation_emptyindex), r, g, b);
		//	SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_radiation_emptyindex));
		//}

		x += 4;
		y += 38;

		//Draw the radiation number (adjust these coords if needed)
		gHUD.DrawHudNumber(x, y, DHN_DRAWZERO, m_radiations, r, g, b, 2);
		//////////////////////////////////////////////////////////////////////


		//ANTIDOTE
		//////////////////////////////////////////////////////////////////////
		x = ScreenWidth - inventoryIconWidth - 20;
		sidebarY += (inventoryIconHeight + 8);
		y = sidebarY;

		//if (m_antidotes > 0) {
			SPR_Set(gHUD.GetSprite(m_antidoteindex), r, g, b);
			SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_antidoteindex));
		//}
		//else {
		//	SPR_Set(gHUD.GetSprite(m_antidote_emptyindex), r, g, b);
		//	SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_antidote_emptyindex));
		//}

		x += 4;
		y += 38;

		//Draw the antidote number (adjust these coords if needed)
		gHUD.DrawHudNumber(x, y, DHN_DRAWZERO, m_antidotes, r, g, b, 2);
		//////////////////////////////////////////////////////////////////////


		//ADRENALINE
		//////////////////////////////////////////////////////////////////////
		x = ScreenWidth - inventoryIconWidth - 20;
		sidebarY += (inventoryIconHeight + 8);
		y = sidebarY;

		//if (m_adrenalines > 0) {
			SPR_Set(gHUD.GetSprite(m_adrenalineindex), r, g, b);
			SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_adrenalineindex));
		//}
		//else {
		//	SPR_Set(gHUD.GetSprite(m_adrenaline_emptyindex), r, g, b);
		//	SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_adrenaline_emptyindex));
		//}

		x += 4;
		y += 38;

		//Draw the adrenaline number (adjust these coords if needed)
		gHUD.DrawHudNumber(x, y, DHN_DRAWZERO, m_adrenalines, r, g, b, 2);
		//////////////////////////////////////////////////////////////////////

		//FLASHLIGHT.
		x = ScreenWidth - inventoryIconWidth - 20;
		sidebarY += (inventoryIconHeight + 8);
		y = sidebarY;

		gHUD.m_Flash.drawFlashlightSidebarIcon(x, y);

	}//END OF gHUD.canDrawSidebar()


	if (!m_pWeapon) {
		return 0;
	}
	///////////////////////////////////////////////////////////////


	if(!forceShowZero){
		//NOTE: the later seen "pw" is just shorthand for "m_pWeapon".
		primaryAmmoClip = m_pWeapon->iClip;

		/*
		if(m_pWeapon->iAmmoType > 0){
			primaryAmmoTotal = gWR.CountAmmo(m_pWeapon->iAmmoType);
		}else{
			primaryAmmoTotal = 0;
		}
		*/
		primaryAmmoTotal = gWR.CountAmmo(m_pWeapon->iAmmoType);

		//It is safe to assume that "secondaryAmmoTotal" is referred to only if the secondary ammo type is above 0.
		if(m_pWeapon->iAmmo2Type > 0){
			secondaryAmmoTotal = gWR.CountAmmo(m_pWeapon->iAmmo2Type);
		}else{
			secondaryAmmoTotal = 0;
		}
	}else{
		//force all 0's.
		primaryAmmoClip = 0;
		primaryAmmoTotal = 0;
		secondaryAmmoTotal = 0;
	}


	//easyForcePrintLine("WHO AREREEEEE YOU %d::%.2f", gHUD.canDrawBottomStats, EASY_CVAR_GET(hud_weaponselecthideslower));

	

	// Can I draw the ammo / reserve numbers?
	// If canDrawBottomStats is on, yes, but there are exceptions that may be forced.
	if(
		gHUD.canDrawBottomStats ||
		(
			EASY_CVAR_GET(hud_weaponselecthideslower) == 0 &&
			EASY_CVAR_GET(hud_version) == 2
		)
		||
		(
			EASY_CVAR_GET(hud_weaponselecthideslower) == 2
			&&
			(
				(EASY_CVAR_GET(hud_version) <= 1 && ScreenHeight >= 550) ||
				EASY_CVAR_GET(hud_version) == 2 ||
				(EASY_CVAR_GET(hud_version) == 3 && ScreenHeight >= 520)
			)
			
		)
	){
		// pass
	}
	else {
		return 1;
	}

	// Draw ammo pickup history
	//MODDD - removed.
	/*
	gHR.DrawAmmoHistory( flTime );
	*/

	//I will assume this is what cuts off the crowbar's attempt at drawing ammo.
	//MODDD - don't do this check anymore. We even already check for iAmmoType being < 0. Want to draw extra zeros
	//        possibly in place of all blank ammo counters too.
	// SPR_Draw Ammo
	//if ((pw->iAmmoType < 0) && (pw->iAmmo2Type < 0))
	//	return 0;


	//m_HUD_number_0   is just the plain font, not the boxed or tiny one.
	AmmoWidth = gHUD.m_iFontWidth;

	a = (int) max( MIN_ALPHA, m_fFade );

	if (m_fFade > 0) {
		m_fFade -= (gHUD.m_flTimeDelta * 20);
	}

	//MODDD - CHANGE COLOR - use green instead.
	//UnpackRGB(r,g,b, RGB_YELLOWISH);
	gHUD.getGenericGUIColor(r, g, b);

	//MODDD - not used.
	//ScaleColors(r, g, b, a );

	//MODDD - swap primary & secondary ammo Y positions.
	// Does this weapon have a clip?
	//y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight/2;


	//MODDD - this is how we check to see if the currently equipped weapon is the rpg.
	// NOTICE - this check is only for drawing the alpha "clip and total ammo" combined trick,
	// no other weapon does this.  Turned into a CVar since that defaults to 'on' to allow this.
	if(EASY_CVAR_GET(hud_rpg_clipless) == 1 && FStrEq(m_pWeapon->szName, "weapon_rpg")){
		ammoClipReserveBlend = TRUE;
	}

	//In either HUD there are three possible places to draw numbers. Those places will be determined by HUD version.
	//These are not necessarly named after whether they are for "primary" ammo (mp5 bullets) or "secondary" ammo (mp5 grenades).
	ammoDrawPrimary = 0;    //Likely to be the primary ammo clip.
	ammoDrawSecondary = 0;   //Likely to be the total primary ammo. Side by side with ammoDrawPrimary.
	ammoDrawTertiary = 0;  //Likely to be the total secondary ammo. A separate line from Primary and Secondary.
	ammoDrawPrimary_draw = FALSE;
	ammoDrawSecondary_draw = FALSE;
	ammoDrawTertiary_draw = FALSE;


	if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros) == 1){
		//draw all number places, even if it stays to the default of 0.
		//ammoDrawPrimary_draw = TRUE;
		//but not you... strangely.
		ammoDrawSecondary_draw = TRUE;
		ammoDrawTertiary_draw = TRUE;
	}else if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cl_drawExtraZeros) == 2){
		//all three.
		ammoDrawPrimary_draw = TRUE;
		ammoDrawSecondary_draw = TRUE;
		ammoDrawTertiary_draw = TRUE;
	}

	if(EASY_CVAR_GET(hud_version) < 3){
		int iNumberHeight = gHUD.m_iFontHeight;
		//y = (ScreenHeight - gHUD.m_iFontHeight*2.75);
		y = (ScreenHeight - (iNumberHeight*2.5) );
		
		//// Does weapon have any ammo at all?
		if(m_pWeapon->iAmmoType > 0){

			if(m_pWeapon->iAmmoType > 0){
				// most weapons in general with any kind of ammo (not the crowbar)
				if(primaryAmmoClip >= 0){
					if(ammoClipReserveBlend){
						// the RPG gets special rules.
						ammoDrawTertiary_draw = TRUE;
						ammoDrawTertiary = primaryAmmoClip + primaryAmmoTotal;
					}else{
						// Weapons with a clip and total ammo count to draw from when reloading.
						ammoDrawPrimary_draw = TRUE;
						ammoDrawPrimary = primaryAmmoClip;
						
						ammoDrawSecondary_draw = TRUE;
						ammoDrawSecondary = primaryAmmoTotal;

						// Does weapon have seconday ammo?
						if(pw->iAmmo2Type > 0){
							ammoDrawTertiary_draw = TRUE;
							ammoDrawTertiary = secondaryAmmoTotal;
						}
					}
				}else{
					// clipless weapons that only count and draw from total ammo.
					ammoDrawTertiary_draw = TRUE;
					ammoDrawTertiary = primaryAmmoTotal;
				}
			}
		}else{
			// no primary ammo type? is there a secondary ammo count though?
			if(pw->iAmmo2Type > 0){
				ammoDrawTertiary_draw = TRUE;
				ammoDrawTertiary = secondaryAmmoTotal;
			}
		}//END OF iAmmoType check
		


		x = ScreenWidth - (8 * AmmoWidth) - 1;

		if(ammoDrawPrimary_draw){
			x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, ammoDrawPrimary, r, g, b, 0, 1);
		}else{
			// advance the x anyways.
			x += gHUD.DrawHUDNumber_widthOnly(iFlags | DHN_3DIGITS, ammoDrawPrimary, 0);
		}


		if(ammoDrawPrimary_draw && ammoDrawSecondary_draw){
			// only draw a slash if there are two numbers to be seprated (primary & secondary)
			gHUD.drawAdditiveFilter( gHUD.GetSprite(m_HUD_slash), r, g, b, 0, x, y, &gHUD.GetSpriteRect(m_HUD_slash), 1);
		}
		// advance the X regardless of drawing the bar.
		int iBarWidth = gHUD.GetSpriteRect(m_HUD_slash).right - gHUD.GetSpriteRect(m_HUD_slash).left;
		x += iBarWidth;


		if(ammoDrawSecondary_draw){
			x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, ammoDrawSecondary, r, g, b, 0, 1);
		}

		if(ammoDrawTertiary_draw){
			x = ScreenWidth - (5 * AmmoWidth) - 1;
			y = (ScreenHeight - (iNumberHeight*1.5) ) + 5;
			x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, ammoDrawTertiary, r, g, b, 0, 1);
		}

	}//END OF if(EASY_CVAR_GET(hud_version) < 3)
	else{

		if(m_pWeapon->iAmmoType > 0){

			if(m_pWeapon->iAmmoType > 0){
				// most weapons in general with any kind of ammo (not the crowbar)
				if(primaryAmmoClip >= 0){
					if(ammoClipReserveBlend){
						// the RPG gets special rules.
						ammoDrawSecondary_draw = TRUE;
						ammoDrawSecondary = primaryAmmoClip + primaryAmmoTotal;
					}else{
						// Weapons with a clip and total ammo count to draw from when reloading.
						ammoDrawPrimary_draw = TRUE;
						ammoDrawPrimary = primaryAmmoClip;
						
						ammoDrawSecondary_draw = TRUE;
						ammoDrawSecondary = primaryAmmoTotal;

						// Does weapon have seconday ammo?
						if(pw->iAmmo2Type > 0){
							ammoDrawTertiary_draw = TRUE;
							ammoDrawTertiary = secondaryAmmoTotal;
						}
					}
				}else{
					//clipless weapons that only count and draw from total ammo.
					ammoDrawSecondary_draw = TRUE;
					ammoDrawSecondary = primaryAmmoTotal;
				}
			}
		}else{
			//no primary ammo type? is there a secondary ammo count though?
			if(pw->iAmmo2Type > 0){
				ammoDrawTertiary_draw = TRUE;
				ammoDrawTertiary = secondaryAmmoTotal;
			}
		}//END OF iAmmoType check


		//used again.
		ScaleColors(r, g, b, a );
		
		if(ammoDrawPrimary_draw){
			x = ScreenWidth - 114 - (24 * 3) + 5 + 21 + 1;
			y = ScreenHeight - 45;
			gHUD.DrawHudNumber(x, y, DHN_DRAWZERO|DHN_3DIGITS|DHN_DRAWPLACE|DHN_EMPTYDIGITSUNFADED , ammoDrawPrimary, r, g, b, 4);
		}

		if(ammoDrawPrimary_draw && ammoDrawSecondary_draw){
			//only draw a bar if there are two numbers to be seprated (primary & secondary)
			//Hard-coded bar graphic.
			x = ScreenWidth - 90;
			y = ScreenHeight - 48;
			int iHeight = 33;
			int iWidth = 3;
			FillRGBA(x, y, iWidth, iHeight, r, g, b, a);
		}
		
		if(ammoDrawSecondary_draw){
			x = ScreenWidth - (30 + 24*3) + 13 + 8 -1 + 2 + 1;
			y = ScreenHeight - 45;
			gHUD.DrawHudNumber(x, y, DHN_DRAWZERO|DHN_3DIGITS|DHN_DRAWPLACE|DHN_EMPTYDIGITSUNFADED , ammoDrawSecondary, r, g, b, 4);
		}

		if(ammoDrawTertiary_draw){
			x = ScreenWidth - (30 + 24*3 + 0) + 13 + 8 + 2;
			y = ScreenHeight - 59 - 33 + 20 - 4 - 10;
			gHUD.DrawHudNumber(x, y, DHN_DRAWZERO|DHN_2DIGITS|DHN_DRAWPLACE|DHN_EMPTYDIGITSUNFADED , ammoDrawTertiary, r, g, b, 4);
		}

	}//END OF hud_version checks


	return 1;
}


//
// Draws the ammo bar on the hud
//
int DrawBar(int x, int y, int width, int height, float f)
{
	int r, g, b;

	if (f < 0)
		f = 0;
	if (f > 1)
		f = 1;

	if (f)
	{
		int w = f * width;

		// Always show at least one pixel if we have ammo.
		if (w <= 0)
			w = 1;
		UnpackRGB(r, g, b, RGB_GREENISH);
		FillRGBA(x, y, w, height, r, g, b, 255);
		x += w;
		width -= w;
	}

	UnpackRGB(r, g, b, RGB_YELLOWISH);

	FillRGBA(x, y, width, height, r, g, b, 128);

	return (x + width);
}


void DrawAmmoBar(WEAPON *p, int x, int y, int width, int height)
{
	//MODDD - removed.
	
	if(EASY_CVAR_GET(hud_drawammobar) == 1){
		//proceed to draw.
	}else{
		//No.
		return;
	}

	if ( !p )
		return;
	
	if (p->iAmmoType != -1)
	{
		//MODDD - what.
		if (!gHUD.m_Ammo.gWR.CountAmmo(p->iAmmoType))
			return;

		float f = (float)gHUD.m_Ammo.gWR.CountAmmo(p->iAmmoType)/(float)p->iMax1;
		
		x = DrawBar(x, y, width, height, f);


		// Do we have secondary ammo too?

		if (p->iAmmo2Type != -1)
		{
			f = (float)gHUD.m_Ammo.gWR.CountAmmo(p->iAmmo2Type)/(float)p->iMax2;

			x += 5; //!!!

			DrawBar(x, y, width, height, f);
		}
	}
	
}


void setWeaponMenuColor(int &r, int &g, int &b){
	//UnpackRGB(r,g,b, RGB_YELLOWISH);
	gHUD.getGenericGUIColor(r, g, b);
}

void setWeaponMenuColorOutOfAmmo(int &r, int &g, int &b){
	gHUD.getGenericEmptyColor(r, g, b);
}

//
// Draw Weapon Menu
//
int CHudAmmo::DrawWList(float flTime)
{
	int r;
	int g;
	int b;
	int x;
	int y;
	int a;
	int itrSlot;


	//MODDD - added.  Force the selected weapon null if the player is dead and cannot see the weapon menu.
	if(gHUD.m_fPlayerDead && EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 0){
		gWR.gpLastSel = gWR.gpActiveSel;
		gWR.gpActiveSel = NULL;
		//can't select a weapon in this state.
	}

	if ( !gWR.gpActiveSel ){
		gHUD.canDrawBottomStats = TRUE;
		return 0;
	}else{
		gHUD.canDrawBottomStats = FALSE;
	}

	int iActiveSlot;

	if (gWR.gpActiveSel == (WEAPON*)1) {
		iActiveSlot = -1;	// current slot has no weapons
	}
	else {
		//iActiveSlot = filterSlot(gWR.gpActiveSel->iSlot);
		iActiveSlot = filterSlot(gWR.gpActiveSel->iSlot);
	}

	//MODDD - 
	//x = 55; //!!!
	//y = 10; //!!!
	x = 28;
	y = 4;


	// Ensure that there are available choices in the active slot
	if ( iActiveSlot > 0 )
	{
		if ( !gWR.GetFirstPos( iActiveSlot ) )
		{
			gWR.gpActiveSel = (WEAPON *)1;
			iActiveSlot = -1;
		}
	}
		
	// Draw top line
	for (itrSlot = 0; itrSlot < MAX_WEAPON_SLOTS; itrSlot++ )
	{
		int iWidth;
		setWeaponMenuColor(r, g, b);
	
		if (itrSlot == iActiveSlot ){
			//a = 255;
			//MODDD
			a = 100;
		}else{
			//a = 192;
			//MODDD
			a = 100;
		}

		ScaleColors(r, g, b, 255);

		//MODDD No, background, then number separately.
		//SPR_Set(gHUD.GetSprite(m_HUD_bucket0 + itrSlot), r, g, b );

		SPR_Set(gHUD.GetSprite(m_HUD_weapons_categorybackground), r, g, b );
		
		// make active slot wide enough to accomodate gun pictures
		if (itrSlot == iActiveSlot )
		{
			WEAPON *p = gWR.GetFirstPos(itrSlot);
			if ( p )
				iWidth = p->rcActive.right - p->rcActive.left;
			else
				iWidth = giBucketWidth;
		}
		else
			iWidth = giBucketWidth;


		//hm, try this here instead?
		FillRGBA( x, y, giBucketWidth, giBucketHeight, r, g, b, a );

		//MODDD  see above
		//SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_weapons_categorybackground));
		
		//SPR_Set(gHUD.GetSprite(number_1tiny), r, g, b );
		//SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(number_1tiny));
		
		a = 224;
		gHUD.DrawHudNumber(x + 6, y + 6, 0, itrSlot+1, r, g, b, 2);

		//x += iWidth + 5;
		x += iWidth + 4;
	}

	a = 128; //!!!
	//MODDD - new.
	//x = 10;
	x = 28;

	// Draw all of the buckets
	for (itrSlot = 0; itrSlot < MAX_WEAPON_SLOTS; itrSlot++)
	{
		//MODDD
		//y = giBucketHeight + 10;
		y = giBucketHeight + 8;

		// If this is the active slot, draw the bigger pictures,
		// otherwise just draw boxes
		if (itrSlot == iActiveSlot )
		{
			WEAPON *p = gWR.GetFirstPos(itrSlot);
			int iWidth = giBucketWidth;
			if ( p )
				iWidth = p->rcActive.right - p->rcActive.left;

			for ( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
			{
				p = gWR.GetWeaponSlot(itrSlot, iPos );

				if ( !p || !p->iId )
					continue;

				setWeaponMenuColor(r, g, b);
			
				// if active, then we must have ammo.

				int fallToDefault = 1;
				//easyPrintLine("YOU ARE AMAZING %d %s", iPos, p->szName);
				if(strcmp(p->szName, "weapon_9mmhandgun") == 0){

					if(global2PSEUDO_playerHasGlockSilencer == 1){
						fallToDefault = 0;
						
						if ( gWR.gpActiveSel == p ){
							//MODDD - Broken trans?
							gHUD.attemptDrawBrokenTrans(x, y, p->rcActive.right - p->rcActive.left, p->rcActive.bottom - p->rcActive.top);

							SPR_Set( gHUD.GetSprite(gHUD.m_glockSilencerWpnIcoActive), r, g, b );
							SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(gHUD.m_glockSilencerWpnIcoActive) );

							SPR_Set(gHUD.GetSprite(m_HUD_selection), r, g, b );
							SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_selection));
							
						}
						else{
							// Draw Weapon if Red if no ammo
							if (gWR.HasAmmo(p)) {
								ScaleColors(r, g, b, 192);
							}
							else
							{
								//UnpackRGB(r,g,b, RGB_REDISH);
								setWeaponMenuColorOutOfAmmo(r, g, b);
								ScaleColors(r, g, b, 128);
							}
							//MODDD - Broken trans?
							gHUD.attemptDrawBrokenTrans(x, y, p->rcInactive.right - p->rcInactive.left, p->rcInactive.bottom - p->rcInactive.top);

							SPR_Set( gHUD.GetSprite(gHUD.m_glockSilencerWpnIcoInactive), r, g, b );
							SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect(gHUD.m_glockSilencerWpnIcoInactive) );
						}

					}else{
						fallToDefault = 1;
					}//END OF else OF if(global2PSEUDO_playerHasGlockSilencer == 1)
				}

				if(fallToDefault){
					if ( gWR.gpActiveSel == p )
					{
						//MODDD - Broken trans?
						gHUD.attemptDrawBrokenTrans(x, y, p->rcActive.right - p->rcActive.left, p->rcActive.bottom - p->rcActive.top);

						SPR_Set(p->hActive, r, g, b );
						SPR_DrawAdditive(0, x, y, &p->rcActive);

						SPR_Set(gHUD.GetSprite(m_HUD_selection), r, g, b );
						SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_selection));
					}
					else
					{
						// Draw Weapon if Red if no ammo
						if ( gWR.HasAmmo(p) )
							ScaleColors(r, g, b, 192);
						else
						{
							//UnpackRGB(r,g,b, RGB_REDISH);
							setWeaponMenuColorOutOfAmmo(r, g, b);
							ScaleColors(r, g, b, 128);
						}
					
						//MODDD - Broken trans?
						gHUD.attemptDrawBrokenTrans(x, y, p->rcInactive.right - p->rcInactive.left, p->rcInactive.bottom - p->rcInactive.top);

						SPR_Set( p->hInactive, r, g, b );
						SPR_DrawAdditive( 0, x, y, &p->rcInactive );

					}
				}//END OF special glock check.

				// Draw Ammo Bar
				DrawAmmoBar(p, x + giABWidth/2, y, giABWidth, giABHeight);
				
				//MODDD
				//y += p->rcActive.bottom - p->rcActive.top + 5; ???
				y += p->rcActive.bottom - p->rcActive.top + 4;
			}
			//MODDD - was + 5.
			x += iWidth + 4;

		}
		else
		{
			// Draw Row of weapons.

			//UnpackRGB(r,g,b, RGB_YELLOWISH);
			setWeaponMenuColor(r, g, b);

			for ( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
			{
				WEAPON *p = gWR.GetWeaponSlot(itrSlot, iPos );
				
				if ( !p || !p->iId )
					continue;

				setWeaponMenuColor(r, g, b);

				if ( gWR.HasAmmo(p) )
				{
					//UnpackRGB(r,g,b, RGB_YELLOWISH);
					setWeaponMenuColor(r, g, b);
					//a = 128;
					//MODDD - now 100.
					a = 100;
				}
				else
				{
					//UnpackRGB(r,g,b, RGB_REDISH);
					setWeaponMenuColorOutOfAmmo(r, g, b);
					a = 96;
				}

				//MODDD - Broken trans?
				gHUD.attemptDrawBrokenTrans(x, y-1, giBucketWidth, giBucketHeight);
		
				FillRGBA( x, y, giBucketWidth, giBucketHeight, r, g, b, a );

				//MODDD
				//y += giBucketHeight + 5;
				y += giBucketHeight + 4;
			}

			//MODDD
			//x += giBucketWidth + 5;
			x += giBucketWidth + 4;
		}// END OF (i == active slot check)
	}//END OF for loop through buckets/slots (I think they're interchangable terms?).

	return 1;
}


/* =================================
	GetSpriteList

Finds and returns the matching 
sprite name 'psz' and resolution 'iRes'
in the given sprite list 'pList'
iCount is the number of items in the pList
================================= */
client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount)
{
	if (!pList)
		return NULL;

	int i = iCount;
	client_sprite_t *p = pList;

	while(i--)
	{
		if ((!strcmp(psz, p->szName)) && (p->iRes == iRes))
			return p;
		p++;
	}

	return NULL;
}
