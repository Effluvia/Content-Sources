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
// Health.cpp
//
// implementation of CHudHealth class
//

#include "health.h"
#include "external_lib_include.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(timedDamage_debug)
EASY_CVAR_EXTERN(hud_version)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode)
EASY_CVAR_EXTERN(hud_weaponselecthideslower)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(itemFlashFadeMult)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(healthcolor_brightness)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage)
EASY_CVAR_EXTERN(timedDamage_extraBrightness)

EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashPrintouts)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashArmorBlock)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashDrownMode)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(painFlashDirTolerance)



int giDmgHeight;
int giDmgWidth;


int giDmgFlags[DMGICO_NUM_DMG_TYPES] =
{
	DMG_POISON,
	DMG_ACID,
	DMG_FREEZE | DMG_SLOWFREEZE,
	DMG_DROWN,
	DMG_BURN | DMG_SLOWBURN,
	DMG_NERVEGAS,
	DMG_RADIATION,
	DMG_SHOCK,

	// REMOVED!  Don't have the icons or use these damage types in HL anyway.
	//DMG_CALTROP,
	//DMG_TRANQ,
	//DMG_CONCUSS,
	//DMG_NUM_DMG_TYPES,

	//MODDD - addition.
	DMG_BLEEDING
};



DECLARE_MESSAGE(m_Health, Health)
DECLARE_MESSAGE(m_Health, HUDItemFsh)



CHudHealth::CHudHealth(void) {

	itemFlashGiven = 0;
}


int CHudHealth::Init(void)
{
	HOOK_MESSAGE(Health);
	HOOK_MESSAGE(HUDItemFsh);

	m_iHealth = 100;
	m_fFade = 0;
	m_iFlags = 0;
	m_bitsDamage = 0;

	giDmgHeight = 0;
	giDmgWidth = 0;

	// oh, sets everything in m_dmg to 0's, ok.
	memset(m_dmg, 0, sizeof(DAMAGE_IMAGE) * DMGICO_NUM_DMG_TYPES);


	gHUD.AddHudElem(this);
	return 1;
}


void CHudHealth::Reset(void)
{
	// force all the flashing damage icons to expire
	m_bitsDamage = 0;
	//MODDD - the new bitmask should be affected too.
	m_bitsDamageMod = 0;

	for (int i = 0; i < DMGICO_NUM_DMG_TYPES; i++)
	{
		m_dmg[i].fExpire = 0;
	}

	m_fFade = 0;
	m_bitsDamage = 0;
	m_bitsDamageMod = 0;
	itemFlashCumulative = 0;

}

int CHudHealth::VidInit(void)
{

	// The start of the damage icon list for all other icons to be found from.
	m_HUD_dmg_icon_start = gHUD.GetSpriteIndex("dmg_bio") + 1;

	m_HUD_cross = gHUD.GetSpriteIndex("cross");

	giDmgHeight = gHUD.GetSpriteRect(m_HUD_dmg_icon_start).right - gHUD.GetSpriteRect(m_HUD_dmg_icon_start).left;
	giDmgWidth = gHUD.GetSpriteRect(m_HUD_dmg_icon_start).bottom - gHUD.GetSpriteRect(m_HUD_dmg_icon_start).top;
	return 1;
}


int CHudHealth::MsgFunc_Health(const char* pszName, int iSize, void* pbuf)
{
	// TODO: update local health data
	BEGIN_READ(pbuf, iSize);
	int x = READ_BYTE();

	m_iFlags |= HUD_ACTIVE;

	// Only update the fade if we've changed health
	if (x != m_iHealth)
	{
		m_fFade = FADE_TIME;
		m_iHealth = x;
	}

	return 1;
}


int CHudHealth::MsgFunc_HUDItemFsh(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int determiner = READ_BYTE();


	//MODDD - asked to make this white instead.
	/*
	if(determiner == 0){
		//antidote. yellow (gold?)
		itemFlashColorStartR = 255;
		itemFlashColorStartG = 255;
		itemFlashColorStartB = 0;
		itemFlashGiven = 1;
		itemFlashCumulative = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump);

	}else if(determiner == 1){
		//radiation. green. (lime; bright?)
		itemFlashColorStartR = 0;
		itemFlashColorStartG = 255;
		itemFlashColorStartB = 0;
		itemFlashGiven = 1;
		itemFlashCumulative = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump);

	}else if(determiner == 2){
		//adrenaline. orange (red?)
		itemFlashColorStartR = 255;
		itemFlashColorStartG = 127;
		itemFlashColorStartB = 0;
		itemFlashGiven = 1;
		itemFlashCumulative = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump);
	}
	*/

	if (determiner >= 0 && determiner <= 2) {
		// white.
		itemFlashColorStartR = 255;
		itemFlashColorStartG = 255;
		itemFlashColorStartB = 255;
		itemFlashGiven = 1;
		itemFlashCumulative = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(itemFlashCumulativeJump);
	}


	return 1;
}


// Returns back a color from the
// Green <-> Yellow <-> Red ramp
void CHudHealth::GetPainColor(int& r, int& g, int& b)
{
	int iHealth = m_iHealth;

	if (iHealth > 25)
		iHealth -= 25;
	else if (iHealth < 0)
		iHealth = 0;
#if 0
	g = iHealth * 255 / 100;
	r = 255 - g;
	b = 0;
#else
	//easyPrintLine("MY HEALTH? %d", m_iHealth);
	if (m_iHealth > 25)
	{
		//MODDD - use our new generic orange instead:
		//UnpackRGB(r,g,b, RGB_YELLOWISH);
		gHUD.getGenericOrangeColor(r, g, b);
	}
	else
	{
		r = 250;
		g = 0;
		b = 0;
	}
#endif 
}

int CHudHealth::Draw(float flTime)
{
	int r, g, b;
	int a = 0, x, y;

	if (gHUD.frozenMem) {
		//Don't draw health while frozen.
		return 0;
	}

	//MODDD - only draw this if the weapon select screen is not on.
	//if(gHUD.canDrawBottomStats){
	if (!(!gHUD.canDrawBottomStats && EASY_CVAR_GET(hud_weaponselecthideslower) == 1)) {

		int HealthWidth;

		if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || gEngfuncs.IsSpectateOnly())
			return 1;


		//MODDD - BUG FIX - if going to a new map ("loading..."), maybe loading a game, the pain sprite will be one of the spritesheets.
		// Very strange.  Moved to pain.cpp's VidInit method.
		//if ( !m_painFlashSprite )
		//	m_painFlashSprite = LoadSprite(PAIN_NAME);

		// Has health changed? Flash the health #
		if (m_fFade)
		{
			m_fFade -= (gHUD.m_flTimeDelta * 20);
			if (m_fFade <= 0)
			{
				a = MIN_ALPHA;
				m_fFade = 0;
			}

			// Fade the health number back to dim

			a = MIN_ALPHA + (m_fFade / FADE_TIME) * 128;

		}
		else
			a = MIN_ALPHA;

		// If health is getting low, make it bright red
		if (m_iHealth <= 15)
			a = 255;

		deriveColorFromHealth(r, g, b, a);

		// Only draw health if we have the suit.
		if (gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT)))
		{
			HealthWidth = gHUD.m_iFontWidthAlt;
			//MODDD - new height var.
			int HealthHeight = gHUD.m_iFontHeightAlt;

			if (EASY_CVAR_GET(hud_version) < 3) {
				if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) <= 0 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 1 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 3 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 4 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 6) {
					//MODDD - say "true" for "useBoxedNumbers".  Also, set x to HealthWidth / 2 (commented out above) and set y properly.
					x = HealthWidth / 2;
					//MODDD - higher
					y = ScreenHeight - ((int)(HealthHeight * 1.5)) - 1;

					//If not dead, draw normally.
					x = gHUD.DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWZERO, m_iHealth, r, g, b, 1, 1);

					x += HealthWidth / 2;
				}
				else if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 2 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 5) {
					drawTimedDamageIcon(0, giDmgWidth / 8, ScreenHeight - giDmgHeight * 2 + giDmgHeight * 1.5, r, g, b);
				}
			}
			else {

				//(41, 544)
				//DHN_3DIGITS | DHN_DRAWZERO
				x = 16 + 19 - 24 - 1;
				y = ScreenHeight - 45;
				x = gHUD.DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWPLACE | DHN_DRAWZERO | DHN_EMPTYDIGITSUNFADED, m_iHealth, r, g, b, 3);

				gHUD.getGenericOrangeColor(r, g, b);

				//Hard-coded bar graphic.
				x = 79;
				y = ScreenHeight - 48;
				int iHeight = 33;
				int iWidth = 3;
				FillRGBA(x, y, iWidth, iHeight, r, g, b, a);
			}
		}

	}//END OF if(canDraw)


	DrawDamage(flTime);
	DrawItemFlash(flTime);
	// MODDD - call to DrawPain moved to hud_redraw.cpp
	//return DrawPain(flTime);
	return 1;
}


int CHudHealth::DrawItemFlash(float flTime) {
	if (itemFlashCumulative == 0) {
		//no cumulative?  skip this.
		return 1;
	}

	int r, g, b;
	int x, y, a, shade;

	//NOTE - drawing "itemFlash" here too.

	r = itemFlashColorStartR;
	g = itemFlashColorStartG;
	b = itemFlashColorStartB;
	a = 255;

	const float fFade = gHUD.m_flTimeDelta * EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(itemFlashFadeMult);

	//if(itemFlashCumulative > 0.00){
		//maxAttackFade += 0.2;
		//GetPainColor(r,g,b);
		//getPainColorMode((int)EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(painFlashColorMode), r, g, b);

		//Clip cumulativeFade to 1.00, as you can't shade anymore intensely than 255, opaque.
		//(as opaque as the engine will allow, it seems)

		//PENDING (???)
		//if(EASY_CVAR_GET(itemFlashColorMode) > 0){
			//draw it.

			//WAS 1.0 on the right!!
	shade = a * max(min(itemFlashCumulative * EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMax)), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(itemFlashDrawOpacityMin));
	//!!!!!

	//shade = a * 1;
	ScaleColors(r, g, b, shade);
	FillRGBA(0, 0, ScreenWidth, ScreenHeight, r, g, b, shade);
		//}

	//}

	//reduce the cumulative fade by "fFade", time.
	//if(itemFlashCumulative > cumulativeFadeMinimum){
	itemFlashCumulative = max(0, itemFlashCumulative - fFade);
	//}
	return 1;
}//END OF DrawItemFlash(...)



//MODDD - method added so that any changes to health-related colors across the program are consistent.
void CHudHealth::deriveColorFromHealth(int& r, int& g, int& b, int& a) {

	if (EASY_CVAR_GET(hud_version) < 3) {
		//NOTICE: not involving the "a" value (for scaling colors) in Pre E3.

		//How bright can all of the color components (r, g, b) potentially be?  (0 - 255)

		//At full health, how red is it? Adds a tiny bit of yellow alongside the bright green.

		//const float fullRedMin = 20;
		//const float brightness = (float)COLOR_PRE_E3_BRIGHTNESS;

		//At what point is the GUI going from green to yellow? Measure of health (0 - 100)
		//const float yellowMark = 70 + fullRedMin;

		const float fullRedMin = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin);
		const float brightness = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(healthcolor_brightness);
		const float yellowMark = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark);

		if (m_iHealth >= yellowMark) {
			//r = (int) (( ( -m_iHealth +yellowMark+100  ) /((float)yellowMark)) *175 );
			r = (int)(((-m_iHealth + fullRedMin + 100) / ((float)(100.0 + fullRedMin - yellowMark))) * brightness);
			g = brightness;
			b = 5;
		}
		else {
			r = brightness;
			g = (int)(((m_iHealth) / ((float)yellowMark)) * brightness);
			b = 5;
		}

	}
	else {
		GetPainColor(r, g, b);
		ScaleColors(r, g, b, a);
	}

}//END OF deriveColorFromHealth



//MODDD - version that accepts no alpha.
void CHudHealth::deriveColorFromHealth(int& r, int& g, int& b) {

	if (EASY_CVAR_GET(hud_version) < 3) {
		//NOTICE: not involving the "a" value (for scaling colors) in Pre E3.

		const float fullRedMin = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(healthcolor_fullRedMin);
		const float brightness = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(healthcolor_brightness);
		const float yellowMark = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(healthcolor_yellowMark);

		if (m_iHealth >= yellowMark) {
			//r = (int) (( ( -m_iHealth +yellowMark+100  ) /((float)yellowMark)) *175 );
			r = (int)(((-m_iHealth + fullRedMin + 100) / ((float)(100.0 + fullRedMin - yellowMark))) * brightness);
			g = brightness;
			b = 5;
		}
		else {
			r = brightness;
			g = (int)(((m_iHealth) / ((float)yellowMark)) * brightness);
			b = 5;
		}

	}
	else {

		GetPainColor(r, g, b);
	}
}



//Only draws the timed damage indicators to the bottom-left, like radiation, toxins, burning, freezing, bleeding, etc.
int CHudHealth::DrawDamage(float flTime)
{
	int r, g, b, a = 0;
	//MODDD - HAHA.  Whoops devs.
	// This is already declared below in the only for-loop it is relevant for.
	//DAMAGE_IMAGE *pdmg;

	//... nevermind, status indicators aren't too distracting and it's good to be aware of these going on.
	//We'd practically hide the GUI if we didn't want this.  ONLY if 2 instead now.
	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) <= 0 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(hideDamage) >= 2) {
		//don't do it.  If we're debugging clearly we want to see it though (above 0)
		return 1;
	}

	//MODDD - can't draw damage without a suit (these are part of the suit's UI)
	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 1;

	//MODDD
	//if (!m_bitsDamage)


	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) <= 0) {
		if (!m_bitsDamage && !m_bitsDamageMod)
			return 1;
	}


	//MODDD
	if (gHUD.m_fPlayerDead && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(timedDamageDeathRemoveMode) == 1) {
		//do not render anymore.
		return 1;
	}

	if (!gHUD.canDrawBottomStats) {
		//do not draw damage indicators if showing the weapon-select.
		return 1;
	}

	// Draw all the items
	int i;

	if ((EASY_CVAR_GET(hud_version) == 3 && EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(E3ShowsDamageIcons) == 1) || (EASY_CVAR_GET(hud_version) < 3 && EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(preE3ShowsDamageIcons) == 1)) {
		

		//UnpackRGB(r,g,b, RGB_YELLOWISH);
		//Not working out so well for whatever reason, using this instead:

		//gHUD.getGenericOrangeColor(r, g, b);
		deriveColorFromHealth(r, g, b);

		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) <= 0 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 4 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 5 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 6) {
			//MODDD - customizable.
			//a = (int)( fabs(sin(flTime*2)) * 256.0);
			float const brightnessRange = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMax) - EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin);
			a = (int)(fabs(sin(flTime * EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_flashSpeed))) * brightnessRange + EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessMin));

			if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap) >= 0) {
				//if "a" is smaller than the cap, it stays. If "a" is greater, it is forced to the cap.
				a = min(a, EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessCap));
			}
			if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor) >= 0) {
				//if "a" is greater than the floor, it stays. If "a" is less, it is forced to the floor.
				a = max(a, EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_brightnessFloor));
			}

			ScaleColors(r, g, b, a);
		}

		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) <= 0) {

			for (i = 0; i < DMGICO_NUM_DMG_TYPES; i++) {
				int* m_rgbTimeBasedDamageRef = NULL;
				if (i < DMGICO_FIRST_MOD_TYPE_INDEX) {
					//use the old bitmask.
					m_rgbTimeBasedDamageRef = &m_bitsDamage;
				}
				else {
					//use the new bitmask.
					m_rgbTimeBasedDamageRef = &m_bitsDamageMod;
				}

				if ((*m_rgbTimeBasedDamageRef) & giDmgFlags[i]) {
					//pass, proceed.
					drawTimedDamageIcon(i, r, g, b);
				}

			}//END OF for loop through damage types.

		}//END OF timedDamage_debug check
		else if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 1 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 2 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 4 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 5) {
			drawTimedDamageIcon(0, giDmgWidth / 8, ScreenHeight - giDmgHeight * 2, r, g, b);
		}
		else if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 3 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(timedDamage_debug) == 6) {
			//draw the health number in this place instead!
			int HealthWidth = gHUD.m_iFontWidthAlt;
			int HealthHeight = gHUD.m_iFontHeightAlt;
			int x;
			int y;

			//x = HealthWidth /2;
			//y = ScreenHeight - ((int)(HealthHeight*1.5)) - 1;

			x = HealthWidth / 2;
			y = ScreenHeight - giDmgHeight * 2 - 20;

			//If not dead, draw normally.
			x = gHUD.DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWZERO, m_iHealth, r, g, b, 1, 1);
		}

	}//END OF show damage icon allowed checks


	// check for bits that should be expired
	for (i = 0; i < DMGICO_NUM_DMG_TYPES; i++)
	{
		DAMAGE_IMAGE* pdmg = &m_dmg[i];


		int* m_rgbTimeBasedDamageRef = NULL;
		if (i < DMGICO_FIRST_MOD_TYPE_INDEX) {
			//use the old bitmask.
			m_rgbTimeBasedDamageRef = &m_bitsDamage;
		}
		else {
			//use the new bitmask.
			m_rgbTimeBasedDamageRef = &m_bitsDamageMod;
		}


		if ((*m_rgbTimeBasedDamageRef) & giDmgFlags[i])
		{
			pdmg->fExpire = min(flTime + DMGICO_DURATION, pdmg->fExpire);

			if (pdmg->fExpire <= flTime		// when the time has expired
				&& a < 40)						// and the flash is at the low point of the cycle
			{
				pdmg->fExpire = 0;

				int y = pdmg->y;
				pdmg->x = pdmg->y = 0;

				// move everyone above down
				for (int j = 0; j < DMGICO_NUM_DMG_TYPES; j++)
				{
					pdmg = &m_dmg[j];
					if ((pdmg->y) && (pdmg->y < y))
						pdmg->y += giDmgHeight;

				}

				(*m_rgbTimeBasedDamageRef) &= ~giDmgFlags[i];  // clear the bits
			}
		}
	}

	return 1;
}

//MODDD - includes "long bitsDamageMod".
//void CHudHealth::UpdateTiles(float flTime, long bitsDamage)
void CHudHealth::UpdateTiles(float flTime, long bitsDamage, long bitsDamageMod)
{
	DAMAGE_IMAGE* pdmg;

	// Which types are new?
	long bitsOn = ~m_bitsDamage & bitsDamage;
	long bitsOnMod = ~m_bitsDamageMod & bitsDamageMod;

	for (int i = 0; i < DMGICO_NUM_DMG_TYPES; i++)
	{
		pdmg = &m_dmg[i];


		int* m_rgbTimeBasedDamageRef = NULL;
		long* bitsOnRef = NULL;
		long* bitsModern = NULL;
		if (i < DMGICO_FIRST_MOD_TYPE_INDEX) {
			//use the old bitmask.
			m_rgbTimeBasedDamageRef = &m_bitsDamage;
			bitsOnRef = &bitsOn;
			bitsModern = &bitsDamage;
		}
		else {
			//use the new bitmask.
			m_rgbTimeBasedDamageRef = &m_bitsDamageMod;
			bitsOnRef = &bitsOnMod;
			bitsModern = &bitsDamageMod;
		}

		// Is this one already on?
		if ((*bitsModern) & giDmgFlags[i])
		{
			pdmg->fExpire = flTime + DMGICO_DURATION; // extend the duration
			if (!pdmg->fBaseline)
				pdmg->fBaseline = flTime;
		}

		// Are we just turning it on?
		if ((*bitsOnRef) & giDmgFlags[i])
		{
			// put this one at the bottom
			pdmg->x = giDmgWidth / 8;
			pdmg->y = ScreenHeight - giDmgHeight * 2;
			pdmg->fExpire = flTime + DMGICO_DURATION;

			// move everyone else up
			for (int j = 0; j < DMGICO_NUM_DMG_TYPES; j++)
			{
				if (j == i)
					continue;

				pdmg = &m_dmg[j];
				if (pdmg->y)
					pdmg->y -= giDmgHeight;

			}
			pdmg = &m_dmg[i];
		}
	}

	// damage bits are only turned on here;  they are turned off when the draw time has expired (in DrawDamage())
	m_bitsDamage |= bitsDamage;
	//MODDD - new
	m_bitsDamageMod |= bitsDamageMod;
}



void CHudHealth::drawTimedDamageIcon(int arg_index, const int& r, const int& g, const int& b) {
	//symbol to draw relying on the damage icon's own recorded X and Y instead. This is retail behavior
	//where the icon draw locations are set at the time damage is taken.
	drawTimedDamageIcon(arg_index, -1, -1, r, g, b);
}

//MODDD - Method for drawing a timed damage icon now that it happens in multiple places.
void CHudHealth::drawTimedDamageIcon(int arg_index, int arg_draw_x, int arg_draw_y, const int& r, const int& g, const int& b) {
	int imaoffset = 0;
	const int i = arg_index;
	int draw_x;
	int draw_y;
	DAMAGE_IMAGE* pdmg;

	int extraYOffset = 0;

	if (EASY_CVAR_GET(hud_version) <= 1) {
		extraYOffset = -41 + 35 - 6;
	}
	else if (EASY_CVAR_GET(hud_version) == 2) {
		extraYOffset = -41;
	}
	else {
		// at 3?  no change.
	}


	pdmg = &m_dmg[i];

	if (arg_draw_x == -1 && arg_draw_y == -1) {
		//just use where the icon already is.
		draw_x = pdmg->x;
		draw_y = pdmg->y;
	}
	else {
		//anything else? use that instead.
		draw_x = arg_draw_x;
		draw_y = arg_draw_y;
	}

	wrect_t* tempRect = &gHUD.GetSpriteRect(m_HUD_dmg_icon_start + i + imaoffset);
	gHUD.attemptDrawBrokenTrans(draw_x - 2, draw_y + 5 + extraYOffset, tempRect);

	SPR_Set(gHUD.GetSprite(m_HUD_dmg_icon_start + i + imaoffset), r, g, b);
	SPR_DrawAdditive(0, draw_x - 2, draw_y + 5 + extraYOffset, tempRect);


}//END OF drawTimedDamageIcon

