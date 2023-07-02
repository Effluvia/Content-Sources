
#include "weapons_resource.h"

// includes from ammo.cpp, where these implementations used to be.
#include "external_lib_include.h"
#include "ammo.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"
#include "ammohistory.h"
#include "vgui_TeamFortressViewport.h"

#include "hl/hl_weapons.h"
#include "dlls/entity/player.h"
#include "dlls/weapons.h"


EASY_CVAR_EXTERN(hud_swapFirstTwoBuckets)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath)

extern int g_weaponselect;




// !!!
// A word about all the "filterSlot" calls and 


// otherwise repetitive logic for seeing whether to switch "1" for "2" and vice
// versa if a CVar is on.
int filterSlot(int iSlot) {
	if (EASY_CVAR_GET(hud_swapFirstTwoBuckets) != 1) {
		// that's it.
		return iSlot;
	}
	else {
		if (iSlot == 0) {
			//swap
			return 1;
		}
		else if (iSlot == 1) {
			//swap
			return 0;
		}
		else {
			// oh, carry on then.
			return iSlot;
		}
	}
}//END OF filterSlot



void WeaponsResource::LoadAllWeaponSprites(void)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (rgWeapons[i].iId) {
			//easyForcePrintLine("OH YEAH!!! %d", rgWeapons[i].iId);
			LoadWeaponSprites(&rgWeapons[i]);
		}
	}
}

int WeaponsResource::CountAmmo(int iId)
{
	//MODDD - no upper bound check?
	if (iId < 0 || iId >= MAX_AMMO_TYPES)
		return 0;

	return riAmmo[iId];
}

int WeaponsResource::HasAmmo(WEAPON* p)
{
	if (!p)
		return FALSE;

	// weapons with no max ammo can always be selected
	if (p->iMax1 == -1)
		return TRUE;


	BOOL hasAnyReserveAmmo = (CountAmmo(p->iAmmoType) > 0 || CountAmmo(p->iAmmo2Type) > 0);

	/*
	if (hasAnyReserveAmmo) {
		// new ammo since?  Flick this flag off.
		p->fForceNoSelectOnEmpty = FALSE;

		if (FStrEq(p->szName, "weapon_satchel")) {
			int x = 45;
		}
	}
	*/



	if (FStrEq(p->szName, "weapon_satchel")) {
		BOOL hasflag = (p->iFlags & ITEM_FLAG_SELECTONEMPTY);
		BOOL noselectOnEmpty = p->fForceNoSelectOnEmpty;
		BOOL combo = hasflag && !noselectOnEmpty;
		int x = 45;
	}



	//MODDD - fForceNoSelectOnEmpty created so that, if the satchel is out of deployed charges
	// and has no ammo, this can be detected and shown on the client as red.
	// Otherwise the client can't tell and will always show it green (even though picking another
	// weapon in that state would remove it)
	return (
		(p->iAmmoType == -1) ||
		p->iClip > 0 ||
		hasAnyReserveAmmo ||
		(
			(p->iFlags & ITEM_FLAG_SELECTONEMPTY) &&
			!(p->fForceNoSelectOnEmpty)
		)
	);

}


void WeaponsResource::LoadWeaponSprites(WEAPON* pWeapon)
{
	//easyPrintLine("HELP LoadWeaponSprites!!! %s", pWeapon->szName);
	int i, iRes;

	if (ScreenWidth < 640)
		iRes = 320;
	else
		iRes = 640;

	char sz[128];

	if (!pWeapon)
		return;

	memset(&pWeapon->rcActive, 0, sizeof(wrect_t));
	memset(&pWeapon->rcInactive, 0, sizeof(wrect_t));
	memset(&pWeapon->rcAmmo, 0, sizeof(wrect_t));
	memset(&pWeapon->rcAmmo2, 0, sizeof(wrect_t));
	pWeapon->hInactive = 0;
	pWeapon->hActive = 0;
	pWeapon->hAmmo = 0;
	pWeapon->hAmmo2 = 0;

	sprintf(sz, "sprites/%s.txt", pWeapon->szName);
	client_sprite_t* pList = SPR_GetList(sz, &i);

	if (!pList)
		return;

	client_sprite_t* p;

	p = GetSpriteList(pList, "crosshair", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hCrosshair = SPR_Load(sz);
		pWeapon->rcCrosshair = p->rc;
	}
	else
		pWeapon->hCrosshair = NULL;

	p = GetSpriteList(pList, "autoaim", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAutoaim = SPR_Load(sz);
		pWeapon->rcAutoaim = p->rc;
	}
	else
		pWeapon->hAutoaim = 0;

	p = GetSpriteList(pList, "zoom", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedCrosshair = SPR_Load(sz);
		pWeapon->rcZoomedCrosshair = p->rc;
	}
	else
	{
		pWeapon->hZoomedCrosshair = pWeapon->hCrosshair; //default to non-zoomed crosshair
		pWeapon->rcZoomedCrosshair = pWeapon->rcCrosshair;
	}

	p = GetSpriteList(pList, "zoom_autoaim", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedAutoaim = SPR_Load(sz);
		pWeapon->rcZoomedAutoaim = p->rc;
	}
	else
	{
		pWeapon->hZoomedAutoaim = pWeapon->hZoomedCrosshair;  //default to zoomed crosshair
		pWeapon->rcZoomedAutoaim = pWeapon->rcZoomedCrosshair;
	}

	p = GetSpriteList(pList, "weapon", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		//easyPrintLine("HELP what is the sprinte A?? %s", sz);
		pWeapon->hInactive = SPR_Load(sz);
		pWeapon->rcInactive = p->rc;

		gHR.iHistoryGap = max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hInactive = 0;

	p = GetSpriteList(pList, "weapon_s", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		//easyPrintLine("HELP what is the sprinte B?? %s", sz);
		pWeapon->hActive = SPR_Load(sz);
		pWeapon->rcActive = p->rc;
	}
	else
		pWeapon->hActive = 0;

	p = GetSpriteList(pList, "ammo", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo = SPR_Load(sz);
		pWeapon->rcAmmo = p->rc;

		gHR.iHistoryGap = max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hAmmo = 0;

	p = GetSpriteList(pList, "ammo2", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo2 = SPR_Load(sz);
		pWeapon->rcAmmo2 = p->rc;

		gHR.iHistoryGap = max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hAmmo2 = 0;

}


void WeaponsResource::PickupWeapon(WEAPON* wp){
	//for breakpoints to latch onto.
	/*
	if (wp->iSlot < 0 || wp->iSlot >= MAX_WEAPON_SLOTS) {
		int x = 0;
	}
	if (wp->iSlotPos < 0 || wp->iSlotPos >= MAX_WEAPON_POSITIONS) {
		int x = 0;
	}
	*/

	//if (wp->iSlotPos >= MAX_WEAPON_POSITIONS || wp->iSlot >= MAX_WEAPON_SLOTS)
	//	return;

	rgSlots[wp->iSlot][wp->iSlotPos] = wp;
}


WEAPON* WeaponsResource::GetWeaponSlot(int slot, int pos) {
	//for breakpoints to latch onto.
	/*
	if (slot < 0 || slot >= MAX_WEAPON_SLOTS) {
		int x = 0;
	}
	if (pos < 0 || pos >= MAX_WEAPON_POSITIONS) {
		int x = 0;
	}
	*/

	// And there wasn't boundary checking on this, because?
	// No longer needed!  Got a bug out.
	//if (pos >= MAX_WEAPON_POSITIONS || slot >= MAX_WEAPON_SLOTS)
	//	return NULL;

	//int iLogicSlot = filterSlot(slot);
	//MODDD - if a CVar demands it, switchout 0 for 1 and 1 for 0 (swap display slots 1 and 2).
	return rgSlots[ filterSlot(slot) ][pos];
}


// Returns the first weapon for a given slot.
WEAPON* WeaponsResource::GetFirstPos(int iSlot)
{
	WEAPON* pret = NULL;
	
	for (int i = 0; i < MAX_WEAPON_POSITIONS; i++)
	{
		if (GetWeaponSlot(iSlot,i) && HasAmmo(GetWeaponSlot(iSlot,i)))
		{
			pret = GetWeaponSlot(iSlot,i);
			break;
		}
		
	}
	return pret;
}



WEAPON* WeaponsResource::GetNextActivePos(int iSlot, int iSlotPos)
{
	//MODDD - aha!  Check to see if 'iSlotPos + 1' has a conflict with the bounds instead.
	// This stops the 'GetwWeaponSlot' call further below from hitting the max of the
	// array (MAX_WEAPON_POSITIONS + 1), which required having their sizes + 1 to have
	// unused postfix elements (always NULL; never used).  Yep... only reason.  Wow.
	if (iSlotPos + 1 >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS)
		return NULL;

	//MODDD - what? Just making referecnes to gWR more specific to the gHUD.m_Ammo.gWR WeaponsResource instance.
	// Which should be this one, right? Why not just drop gWR entirely? oh well.
	
	//WEAPON* p = gHUD.m_Ammo.gWR.rgSlots[iSlot][iSlotPos + 1];
	WEAPON* p = gHUD.m_Ammo.gWR.GetWeaponSlot(iSlot,iSlotPos + 1);
	

	if (!p || !gHUD.m_Ammo.gWR.HasAmmo(p))
		return GetNextActivePos(iSlot, iSlotPos + 1);

	return p;
}



//
// Helper function to return a Ammo pointer from id
//

SpriteHandle_t* WeaponsResource::GetAmmoPicFromWeapon(int iAmmoId, wrect_t& rect)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (rgWeapons[i].iAmmoType == iAmmoId)
		{
			rect = rgWeapons[i].rcAmmo;
			return &rgWeapons[i].hAmmo;
		}
		else if (rgWeapons[i].iAmmo2Type == iAmmoId)
		{
			rect = rgWeapons[i].rcAmmo2;
			return &rgWeapons[i].hAmmo2;
		}
	}

	return NULL;
}


// Menu Selection Code

void WeaponsResource::SelectSlot(int iSlot, int fAdvance, int iDirection)
{
	if (gHUD.m_Menu.m_fMenuDisplayed && (fAdvance == FALSE) && (iDirection == 1))
	{ // menu is overriding slot use commands
		gHUD.m_Menu.SelectMenuItem(iSlot + 1);  // slots are one off the key numbers
		return;
	}

	//MODDD - and why was this only '>' instead of '>=' like another place?  weird.
	if (iSlot >= MAX_WEAPON_SLOTS)
		return;

	//MODDD
	//if ( gHUD.m_fPlayerDead || gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
	if ((EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 0 && gHUD.m_fPlayerDead) || gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL))
		return;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return;

	if (!(gHUD.m_iWeaponBits & ~(1 << (WEAPON_SUIT))))
		return;

	WEAPON* p = NULL;
	BOOL fastSwitch = CVAR_GET_FLOAT("hud_fastswitch") != 0;



	// First time opening the menu instead of going through slots while already open? Note that
	BOOL openSinceClose = (gpActiveSel == NULL);

	if (openSinceClose || (gpActiveSel == (WEAPON*)1) || (iSlot != filterSlot(gpActiveSel->iSlot))  )
	{
		//MODDD - play the wpn_hudon.wav sound only if the sound wasn't absorbed by fastswitch playing wpn_select.wav now instead.
		p = GetFirstPos(iSlot);

		if (p && fastSwitch) // check for fast weapon switch mode
		{
			// if fast weapon switch is on, then weapons can be selected in a single keypress
			// but only if there is only one item in the bucket
			WEAPON* p2 = GetNextActivePos(filterSlot(p->iSlot), p->iSlotPos);
			if (!p2)
			{	// only one active item in bucket, so change directly to weapon
				ServerCmd(p->szName);
				g_weaponselect = p->iId;

				//MODDD
				// play this sound instead, as though the weapon were clicked while weapon select was open

				if (gHUD.m_Ammo.m_pWeapon != p) {
					// changed weapon?
					PlaySound("common/wpn_select.wav", 1);

					// why keep weapon select up when we decided to equip a weapon?
					gpActiveSel = NULL;
				}
				else {
					// weapon-select wasn't even open for this, don't make any noise
					// ...unless it was?
					CancelSelection();
				}

				return;
			}
		}


		if (openSinceClose) {
			//MODDD -  hud_fastwtich wasn't on or didn't work (0 or many items in slot)?  Play for the open menu.
			// AND only if the menu was opened at all. Slots without any items don't open the menu.
			// AND only since opening since a close.  Otherwise...
			PlaySound("common/wpn_hudon.wav", 1);
		}

		if (p) {
			if(!openSinceClose){
				// yep.
				gHUD.playWeaponSelectMoveSound();
			}
		}

	}
	else
	{
		gHUD.playWeaponSelectMoveSound();
		if (gpActiveSel)
			p = GetNextActivePos(filterSlot(gpActiveSel->iSlot), gpActiveSel->iSlotPos);
		if (!p)
			p = GetFirstPos(iSlot);
	}


	if (!p)  // no selection found
	{
		// just display the weapon list, unless fastswitch is on just ignore it

		//MODDD - why refuse to show the menu for an empty slot when  fastSwitch is on?
		// Seems like odd behavior irrelevant to the point of fastSwitch.
		//if (!fastSwitch){
			gpActiveSel = (WEAPON*)1;
		//}else{
		//	gpActiveSel = NULL;
		//}
	}
	else {
		gpActiveSel = p;
	}
}



// NEW. stop showing weapon-select, don't commit the current selection.
// Play the close-menu sound if it was open.
// Oh.  Mostly copied from CHudAmmo::UserCmd_Close(void) actually.
// Without the "escape" call in there, I have no idea what that was.
// And... funny I named it this, I didn't even see that UserCmd_Close was
// linked by console text 'cancelselect'.
void WeaponsResource::CancelSelection(void) {
	if (gpActiveSel != NULL)
	{
		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		if (gpActiveSel != (WEAPON*)1) {
			PlaySound("common/wpn_hudoff.wav", 1);
		}
	}
}


// NEW.  This is not tied to any engine events or overridable method,
// this must be called by gHUD.m_Ammo (ammo.cpp)'s Think method, used to be there in fact.
void WeaponsResource::Think(void) {

	//MODDD - still okay to do weapon selection during death, IF the option is on.
	//if ( gHUD.m_fPlayerDead )
	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(canShowWeaponSelectAtDeath) == 0 && gHUD.m_fPlayerDead) {
		return;
	}

	if (gHUD.m_iWeaponBits != iOldWeaponBits)
	{
		iOldWeaponBits = gHUD.m_iWeaponBits;

		for (int i = MAX_WEAPONS - 1; i > 0; i--)
		{
			WEAPON* p = GetWeapon(i);

			if (p)
			{
				if (gHUD.m_iWeaponBits & (1 << p->iId))
					PickupWeapon(p);
				else
					DropWeapon(p);
			}
		}
	}

	if (!gpActiveSel)
		return;

	
	

	// has the player selected one?
	// (this is a mouse-click to close the menu with whatever is picked in mind)
	if (gHUD.m_iKeyBits & IN_ATTACK)
	{
		//MODDD - player must also not be dead to make a selection (in case weapons menu is still enabled)
		//MODDD - new condition...?
		//if(!gHUD.m_fPlayerDead ){


		//MODDD - convoluted attempt to see if the same weapon has already been selected
		// from looking at the mostly dummied clientside copy of the player.
		// That isnt' very smart though, it is better to keep track of the existing
		// selected weapon (gHUD.m_Ammo.m_pWeapon?) before making the change.    But this is a thing I suppose.
		/*
		BOOL isAlreadySelected = FALSE;

		if (
			gpActiveSel != NULL &&
			gpActiveSel != (WEAPON*)1 &&
			localPlayer.m_pActiveItem != NULL
			) {

			// test?
			const char* a = STRING(localPlayer.m_pActiveItem->pev->classname);
			const char* b = gpActiveSel->szName;

			// so strcmp does not like NULL strings...    okay.

			if (a != NULL && b != NULL) {
				isAlreadySelected = (strcmp(a, b) == 0);

				if (isAlreadySelected) {
					int x = 4;
				}
				int x = 4;
			}
		}
		else {
			int x = 4;
		}
		*/

		if (gpActiveSel != (WEAPON*)1)
		{
			ServerCmd(gpActiveSel->szName);
			g_weaponselect = gpActiveSel->iId;

			if (gHUD.m_Ammo.m_pWeapon != gpActiveSel) {
				//MODDD - before, sound was played anytime the current selection was 'picked' (even if nothing was selected; empty slot).
				// Also not played on selecting the same weapon.  Call still sent to the server for safety in case it isn't in synch.
				PlaySound("common/wpn_select.wav", 1);
			}
			else {
				// off it goes, mundanely.
				PlaySound("common/wpn_hudoff.wav", 1);
			}
		}
		else {
			// for an empty slot too now
			PlaySound("common/wpn_hudoff.wav", 1);
		}

		//} gHUD.m_fPlayerDead check



		//Misunderstood something, nevermind this.
		/*
		if(CVAR_GET_FLOAT("deployingWeaponPlaysOtherSound") == 1){
			PlaySound("items/gunpickup4.wav", 1);
		}else{
			PlaySound("common/wpn_select.wav", 1);
		}
		*/


		gHUD.m_iKeyBits &= ~IN_ATTACK;

		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
	}//END OF click check



	if (gHUD.m_iKeyBits & IN_ATTACK2) {


		CancelSelection();

		gHUD.m_iKeyBits &= ~IN_ATTACK2;
	}


}//Think




