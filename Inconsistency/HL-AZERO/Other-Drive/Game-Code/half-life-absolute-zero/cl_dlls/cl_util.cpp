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
// cl_util.cpp
//
// implementation of class-less helper functions
//

#include "external_lib_include.h"
//#include "STDIO.H"
//#include "STDLIB.H"
//#include "MATH.H"

#include "hud.h"
#include "cl_util.h"
//#include <string.h>


#include "cvar_custom_info.h"
#include "cvar_custom_list.h"



cvar_t* cl_lw = NULL;
cvar_t* cl_viewrollangle;
cvar_t* cl_viewrollspeed;

// from hud.cpp
float g_lastFOV = 0.0;

// don't mind these.
int playingMov = FALSE;
float movieStartTime = -1;


extern BOOL g_queueCVarHiddenSave;

// from inputw32.cpp
extern void determineMouseParams(float argNew);
extern void onUpdateRawInput(void);


//MODDD - no need!  Including common/mathlib.h, associated pm_shared/pm_math.c has vec3_origina already.
// 
//vec3_t vec3_origin( 0, 0, 0 );

//MODDD - IMPORTANT!
float globalPSEUDO_autoDeterminedFOV = -1;
float globalPSEUDO_ScreenWidth = -1;
float globalPSEUDO_ScreenHeight = -1;
int global2PSEUDO_playerHasGlockSilencer = -1;
float global2PSEUDO_IGNOREcameraMode = -1;
//float global2PSEUDO_determinedFOV = -1;
float global2PSEUDO_grabbedByBarancle = 0;
// If cl_fvox has ever been changed, we have to let the server-version of the player know about
// this up-to-date preference.
float global2PSEUDO_cl_fvox = -1;
// Ditto.
float global2PSEUDO_cl_holster = -1;
float global2PSEUDO_cl_breakholster = -1;
float global2PSEUDO_cl_ladder = -1;
float globalPSEUDO_m_rawinputMem = -1;
float global2PSEUDO_default_fov = -1;
float global2PSEUDO_auto_adjust_fov = -1;

float g_cl_nextCVarUpdate = -1;

BOOL g_lateCVarInit_called = FALSE;



//is this accessible everywhere?
EASY_CVAR_DECLARATION_CLIENT_MASS


// Easy place for any other CVar init stuff after loading hidden vars (if applicble).
// Called from cl_dlls/cdll_int.cpp
// no. Now called from a custom message sent when the player's "OnFirstAppearance" is called form joining
// a server or loading a game.  This keeps the info in sync at startup since possibly falling out of sync between games.
void lateCVarInit(void){
	
	// force an update now for safety.
	// The second "1" is a signal to not play any message alongside the set, like on starting a map/joining a server.
	global2PSEUDO_cl_fvox = EASY_CVAR_GET(cl_fvox);
	if (global2PSEUDO_cl_fvox == 0) {
		easyClientCommand("_cl_fvox 0 1");
	}else {
		easyClientCommand("_cl_fvox 1 1");
	}

	global2PSEUDO_cl_holster = EASY_CVAR_GET(cl_holster);
	if (global2PSEUDO_cl_holster == 0) {
		easyClientCommand("_cl_holster 0");
	}else {
		easyClientCommand("_cl_holster 1");
	}

	global2PSEUDO_cl_breakholster = EASY_CVAR_GET(cl_breakholster);
	if (global2PSEUDO_cl_breakholster == 0) {
		easyClientCommand("_cl_breakholster 0");
	}else {
		easyClientCommand("_cl_breakholster 1");
	}

	global2PSEUDO_cl_ladder = EASY_CVAR_GET(cl_ladder);
	easyClientCommand("_cl_ladder %f", global2PSEUDO_cl_ladder);

	// NOTE: m_rawinput does not need startup script here.
	// It is already read by inputw32.cpp at startup as needed and acted on there.
	// Can force the mem to refer to the current m_rawinput value to avoid re-updating though.
	globalPSEUDO_m_rawinputMem = EASY_CVAR_GET(DEFAULT_m_rawinput);


//	char aryChrToSend[128];//	const char* szFmt = "%s %d";
//	sprintf(arg_dest, szFmt, arg_label, arg_arg);

	global2PSEUDO_default_fov = EASY_CVAR_GET(default_fov);
	//gEngfuncs.pfnClientCmd("_default_fov %f", global2PSEUDO_default_fov);
	easyClientCommand("_default_fov %f", global2PSEUDO_default_fov);

	global2PSEUDO_auto_adjust_fov = EASY_CVAR_GET_CLIENTONLY(auto_adjust_fov);
	easyClientCommand("_auto_adjust_fov %f", global2PSEUDO_auto_adjust_fov);
	
	updateAutoFOV();  //do we even need to do this here?
	easyClientCommand("_auto_determined_fov %f", globalPSEUDO_autoDeterminedFOV);
	
	g_lateCVarInit_called = TRUE;

}//END OF lateCVarInit


void updateClientCVarRefs(void){

	if (!g_lateCVarInit_called) {
		// not allowed to run until all CVars have been hooked up to starting defaults, particulary broadcasted  ones.
		return;
	}
	

	//nope, this is what EASY_CVAR should be doing.  Or something better.
	//if(cvar2_cl_interp_entity != NULL && cvar2_cl_interp_entity->value != global2_cl_interp_entity){\
	//	global2_cl_interp_entity = cvar2_cl_interp_entity->value;\
	//}

	
	//just keeping this in sync...   NO, see hud_redraw for where this is updated. that works too.
	//global2PSEUDO_IGNOREcameraMode = gHUD.CVar_cameraModeMem;

	if (EASY_CVAR_GET(cl_fvox) != global2PSEUDO_cl_fvox) {
		global2PSEUDO_cl_fvox = EASY_CVAR_GET(cl_fvox);
		if (global2PSEUDO_cl_fvox == 0) {
			easyClientCommand("_cl_fvox 0");
		}else{
			easyClientCommand("_cl_fvox 1");
		}
	}
	if (EASY_CVAR_GET(cl_holster) != global2PSEUDO_cl_holster) {
		global2PSEUDO_cl_holster = EASY_CVAR_GET(cl_holster);
		if (global2PSEUDO_cl_holster == 0) {
			easyClientCommand("_cl_holster 0");
		}else {
			easyClientCommand("_cl_holster 1");
		}
	}
	if (EASY_CVAR_GET(cl_breakholster) != global2PSEUDO_cl_breakholster) {
		global2PSEUDO_cl_breakholster = EASY_CVAR_GET(cl_breakholster);
		if (global2PSEUDO_cl_breakholster == 0) {
			easyClientCommand("_cl_breakholster 0");
		}else {
			easyClientCommand("_cl_breakholster 1");
		}
	}
	if (EASY_CVAR_GET(cl_ladder) != global2PSEUDO_cl_ladder) {
		global2PSEUDO_cl_ladder = EASY_CVAR_GET(cl_ladder);
		easyClientCommand("_cl_ladder %f", global2PSEUDO_cl_ladder);
	}
	
	if (EASY_CVAR_GET(m_rawinput) != globalPSEUDO_m_rawinputMem) {
		globalPSEUDO_m_rawinputMem = EASY_CVAR_GET(m_rawinput);
		// this CVar can modify originalmouseparms and newmouseparms, and apply immediately.
		determineMouseParams(globalPSEUDO_m_rawinputMem);
		onUpdateRawInput();
	}






	
	
	if(EASY_CVAR_GET(default_fov) != global2PSEUDO_default_fov){
		global2PSEUDO_default_fov = EASY_CVAR_GET(default_fov);
		easyClientCommand("_default_fov %f", global2PSEUDO_default_fov);
		// this changes the globalPSEUDO_autoDeterminedFOV
		updateAutoFOV();
		easyClientCommand("_auto_determined_fov %f", globalPSEUDO_autoDeterminedFOV);
	}
	
	if(EASY_CVAR_GET_CLIENTONLY(auto_adjust_fov) != global2PSEUDO_auto_adjust_fov){
		global2PSEUDO_auto_adjust_fov = EASY_CVAR_GET_CLIENTONLY(auto_adjust_fov);
		easyClientCommand("_auto_adjust_fov %f", global2PSEUDO_auto_adjust_fov);
	}
	
	if(globalPSEUDO_ScreenWidth != ScreenWidth || globalPSEUDO_ScreenHeight != globalPSEUDO_ScreenHeight){
		updateAutoFOV();
		easyClientCommand("_auto_determined_fov %f", globalPSEUDO_autoDeterminedFOV);
	}
	


	if (gEngfuncs.GetClientTime() >= g_cl_nextCVarUpdate) {
		g_cl_nextCVarUpdate = gEngfuncs.GetClientTime() + 1;
		EASY_CVAR_UPDATE_CLIENT_MASS
		
		if(g_queueCVarHiddenSave == TRUE){
			saveHiddenCVars();
		}
		
	}//g_cl_nextCVarUpdate check



}//END OF updateClientCVarRefs


void updateAutoFOV(void){
	
	// keep track of these, no need to update until they change again.
	globalPSEUDO_ScreenWidth = ScreenWidth;
	globalPSEUDO_ScreenHeight = ScreenHeight;
	
	//MODDD - determine the FOV, given the screen size available at this point (by a ratio of width/height, plugged
	//        into a linear function).
	//cvar_t* aspectratio_determined_fov = CVAR_GET("aspectratio_determined_fov");
	//aspectratio_determined_fov->value = (int) (  ((float)ScreenWidth / (float)ScreenHeight) * 45 + 30   );
	//new formula.  ratio of 1.333 still yeilds FOV of 90, but ratio of 1.777 yields FOV of 105 instead of 110.
	

	float defaultFOV_value = CVAR_GET_FLOAT( "default_fov" );

	if(defaultFOV_value < 1){
		//CVar not assigned yet?  Just assume it was 90.
		defaultFOV_value = 90;
	}else{
		//ok, just use whatever it is then.
	}
	
	//Now see how the screen size would influence this choice.
	globalPSEUDO_autoDeterminedFOV = ( (defaultFOV_value/90) * (  ((float)ScreenWidth / (float)ScreenHeight) * 33.75f + 45   ));
	
	
	
	//aspectratio_determined_fov->value = determinedFOV;
	//gEngfuncs.Cvar_SetValue("aspectratio_determined_fov", determinedFOV );
	//send it off!
	
	//global2PSEUDO_determinedFOV = determinedFOV;
	
	
	//easyPrintLine("HHHHHHHHHHHHHHHHHHHHHH %d %d, %.2f", ScreenWidth, ScreenHeight, (float)determinedFOV);

}//END OF updateAutoFOV





//MODDD - what was the point of this??
// If we rely on the OS to give this method, fine.  Doing this without ever defining it
// in the project's code just seems wonky.
//double sqrt(double x);

//MODDD - vector-math-related methods moved to common/vector.cpp.





SpriteHandle_t LoadSprite(const char *pszName)
{
	int i;
	char sz[256]; 

	if (ScreenWidth < 640)
		i = 320;
	else
		i = 640;

	sprintf(sz, pszName, i);

	return SPR_Load(sz);
}




void SetCrosshairFiltered( SpriteHandle_t hspr, wrect_t rc, int r, int g, int b ){

	SetCrosshairFiltered(hspr, rc, r, g, b, 0);
}


//MODDD - NOTE: all calls to "SetCrosshair" have been changed to "SetCrosshairFiltered" to go through this process first.
//If the alpha crosshair was loaded, use that.  Otherwise, use the normal crosshair (exepcted per weapon, given from the args here).
void SetCrosshairFiltered( SpriteHandle_t hspr, wrect_t rc, int r, int g, int b, int forceException ){
	//MODDD - player crosshair is always this instead.
	

	if(CVAR_GET_FLOAT("crosshair") == 1 && forceException == 0){
		//if we don't know 
		//easyPrint("YAYY\n");

		if(gHUD.alphaCrossHairIndex != -1){
			SetCrosshair(gHUD.GetSprite(gHUD.alphaCrossHairIndex), gHUD.GetSpriteRect(gHUD.alphaCrossHairIndex), 255, 255, 255);
		}else{
			easyPrintLine("ERROR: could not set alpha crosshair; not loaded!");
		}

	}else{

		SetCrosshair(hspr, rc, r, g, b);
	}
	
}



//fill an array of 3 integers ("ary", assumed) with integers from 0 to 255.  Could these be bytes?
//~Colors of the brightest intensity only allowed.
void generateColor(int* ary){

	int hue = gEngfuncs.pfnRandomLong(0, 239);

	int changeType = 1;
	int deltaColor = 0;
	int progress = 0;

	int rFull = 0;
	int gFull = 0;
	int bFull = 0;

	if(hue >= 0 && hue < 40){
		rFull = 1;
		changeType = 1;
		deltaColor = 1;
	}else if(hue < 80){
		gFull = 1;
		changeType = -1;
		deltaColor = 0;
	}else if(hue < 120){
		gFull = 1;
		changeType = 1;
		deltaColor = 2;
	}else if(hue < 160){
		bFull = 1;
		changeType = -1;
		deltaColor = 1;
	}else if(hue < 200){
		bFull = 1;
		changeType = 1;
		deltaColor = 0;
	}else if(hue < 240){
		rFull = 1;
		changeType = -1;
		deltaColor = 2;
	}

	//255 / 40 = 6.375
	
	progress = (int)((hue % 40) * 6.375f) ;

	//  255 * 0 = 0       (1)
	//  255 * 1 = 255      (-1)

	//   -1, 1
	//    (1 - 1)/-2 = 0
	//    (-1 - 1)/-2 = 1

	if(rFull){
		ary[0] = 255;
	}else{
		ary[0] = 0;
	}
	if(gFull){
		ary[1] = 255;
	}else{
		ary[1] = 0;
	}
	if(bFull){
		ary[2] = 255;
	}else{
		ary[2] = 0;
	}
	ary[deltaColor] = 255 * ( changeType - 1)/-2 + changeType * progress;

}



//fill an array of 3 values ("ary", assumed) with decimals from 0 to 1.
//~Colors of the brightest intensity only allowed.
void generateColorDec(float* ary){

	int hue = gEngfuncs.pfnRandomLong(0, 239);

	int changeType = 1;
	int deltaColor = 0;
	float progress = 0;

	int rFull = 0;
	int gFull = 0;
	int bFull = 0;

	if(hue >= 0 && hue < 40){
		rFull = 1;
		changeType = 1;
		deltaColor = 1;
	}else if(hue < 80){
		gFull = 1;
		changeType = -1;
		deltaColor = 0;
	}else if(hue < 120){
		gFull = 1;
		changeType = 1;
		deltaColor = 2;
	}else if(hue < 160){
		bFull = 1;
		changeType = -1;
		deltaColor = 1;
	}else if(hue < 200){
		bFull = 1;
		changeType = 1;
		deltaColor = 0;
	}else if(hue < 240){
		rFull = 1;
		changeType = -1;
		deltaColor = 2;
	}

	//1 / 40 = 0.025
	
	progress = (float)((hue % 40) * 0.025f) ;

	//  255 * 0 = 0       (1)
	//  255 * 1 = 255      (-1)

	//   -1, 1
	//    (1 - 1)/-2 = 0
	//    (-1 - 1)/-2 = 1

	if(rFull){
		ary[0] = 1;
	}else{
		ary[0] = 0;
	}
	if(gFull){
		ary[1] = 1;
	}else{
		ary[1] = 0;
	}
	if(bFull){
		ary[2] = 1;
	}else{
		ary[2] = 0;
	}
	ary[deltaColor] = 1 * ( changeType - 1)/-2 + (changeType) * progress;

}



//MODDD - utility random invert.  Good tool for a random that is so far from 0, but can be positive or negative from it.
//Such as, say I want a random number in range  |30, 80|  or  ( [-80, -30] or [30, 80] ).  This makes sure the absolute value isn't under 30 or over 80.
//Below even does the absolute range all-in-one (randomAbsoluteValue)
inline
float randomInvert(const float& arg_flt){
	if(gEngfuncs.pfnRandomLong(0, 1) == 0){
		return arg_flt;
	}else{
		return -arg_flt;
	}
}
inline
float randomAbsoluteValue(const float& arg_fltMin, const float& arg_fltMax){

	float fltRange = arg_fltMax - arg_fltMin;

	//just get from 0 to "range", both ways.
	//that is, -range to +range  (or, -range to 0 and 0 to +range)

	float fltReturn = gEngfuncs.pfnRandomFloat(-fltRange, fltRange);

	//add back the minimum (or, subtract?)

	if(fltReturn < 0){
		fltReturn -= arg_fltMin;
	}else if(fltReturn > 0){
		fltReturn += arg_fltMin;
	}else{
		//exactly 0?  strange, but not impossible (I think)
		fltReturn += randomInvert(arg_fltMin);
	}
	return fltReturn;
}
inline
int randomValueInt(const int& arg_min, const int& arg_max){
	//nothing too special.
	return gEngfuncs.pfnRandomLong(arg_min, arg_max);
}
inline
float randomValue(const float& arg_fltMin, const float& arg_fltMax){
	//nothing too special.
	return gEngfuncs.pfnRandomFloat(arg_fltMin, arg_fltMax);
}

inline
float modulusFloat(const float& arg_dividend, const float& arg_divisor ){
	//368.7 -> 368
	//int trunc = (int)arg_dividend;
	
	//int mod = trunc % arg_divisor;

	return fmod(arg_dividend, arg_divisor);
}

inline
float getTimePeriod(const float& arg_time, const float& arg_period, const float& arg_extentMin, const float& arg_extentMax){
	float timeFactor = modulusFloat(arg_time, arg_period) / arg_period;

	//get in terms of extents.

	float range = arg_extentMax - arg_extentMin;
	float finalResult = timeFactor * range + arg_extentMin;

	return finalResult;
}

inline
float getTimePeriodAndBack(const float& arg_time, const float& arg_period, const float& arg_extentMin, const float& arg_extentMax){
	
	//split in half.
	float timeFactor = modulusFloat(arg_time, arg_period*2) / (arg_period*2) ;

	//get in terms of extents.
	if(timeFactor <= 0.5){

		float interpretedTimeFactor = timeFactor/0.5;

		float range = arg_extentMax - arg_extentMin;
		float finalResult = interpretedTimeFactor * range + arg_extentMin;

		return finalResult;
	}else{
		//backwards!

		float interpretedTimeFactor = (1.0 - timeFactor)/0.5;

		float range = arg_extentMax - arg_extentMin;
		float finalResult = interpretedTimeFactor * range + arg_extentMin;

		return finalResult;
	}
}

inline
float getTimePeriodAndBackSmooth(const float& arg_time, const float& arg_period, const float& arg_extentMin, const float& arg_extentMax){
	
	//split in half.
	float timeFactor = modulusFloat(arg_time, arg_period*2) / (arg_period*2) ;

	//get in terms of extents.
	if(timeFactor <= 0.5){

		float interpretedTimeFactor = pow( (timeFactor/0.5f), 0.5f  );

		float range = arg_extentMax - arg_extentMin;
		float finalResult = interpretedTimeFactor * range + arg_extentMin;

		return finalResult;
	}else{
		//backwards!

		float interpretedTimeFactor = pow(( (1.0f - timeFactor)/0.5f), 2.0f);

		float range = arg_extentMax - arg_extentMin;
		float finalResult = interpretedTimeFactor * range + arg_extentMin;

		return finalResult;

	}

	
}

inline
float getTimePeriodAndBackSmooth(const float& arg_time, const float& arg_period, const float& arg_period2, const float& arg_extentMin, const float& arg_extentMax){
	
	//split in half.

	float totalPeriod = arg_period + arg_period2;
	float timeFactor = modulusFloat(arg_time, totalPeriod) / (totalPeriod) ;

	float portionBreaker = arg_period / (totalPeriod);
	float portionBreaker2 = 1 - portionBreaker;

	//get in terms of extents.
	if(timeFactor <= portionBreaker){

		float interpretedTimeFactor = pow( (timeFactor/portionBreaker), 2.5f  );

		float range = arg_extentMax - arg_extentMin;
		float finalResult = interpretedTimeFactor * range + arg_extentMin;

		return finalResult;
	}else{
		//backwards!

		float interpretedTimeFactor = pow(( (1.0f - timeFactor)/portionBreaker2), 2.5f);

		float range = arg_extentMax - arg_extentMin;
		float finalResult = interpretedTimeFactor * range + arg_extentMin;

		return finalResult;

	}

	
}







void drawStringFabulous(int arg_x, int arg_y, const char* arg_str){
	int i = 0;
	int currentOffX = 0;
	int characterWidth = 10;
	while(i < 120){
		if(arg_str[i] == '\0'){
			//done.
			break;
		}else{
			float color[3];
			generateColorDec(color);
			char tempChar[2];
			tempChar[0] = arg_str[i];
			tempChar[1] = '\0';
			gEngfuncs.pfnDrawSetTextColor(color[0], color[1], color[2]);
			gEngfuncs.pfnDrawConsoleString( arg_x + currentOffX, arg_y, tempChar );
			currentOffX += characterWidth;
		}
		i++;
	}//END
}


	
void drawStringFabulousVert(int arg_x, int arg_y, const char* arg_str){
	int i = 0;
	int currentOffY = 0;
	int characterHeight = 17;
	while(i < 120){
		if(arg_str[i] == '\0'){
			//done.
			break;
		}else{
			float color[3];
			generateColorDec(color);
			char tempChar[2];
			tempChar[0] = arg_str[i];
			tempChar[1] = '\0';
			gEngfuncs.pfnDrawSetTextColor(color[0], color[1], color[2]);
			gEngfuncs.pfnDrawConsoleString( arg_x, arg_y + currentOffY, tempChar );
			currentOffY += characterHeight;
		}
		i++;
	}//END
}

void drawString(int arg_x, int arg_y, const char* arg_str, const float& r, const float& g, const float& b){
	gEngfuncs.pfnDrawSetTextColor(r, g, b);
	gEngfuncs.pfnDrawConsoleString( arg_x, arg_y, (char*) arg_str );
}











int firstCrazyShit = TRUE;
int crazyRandomShitX[44];
int crazyRandomShitY[44];


float nextTimeeeee = -1;



void drawCrazyShit(float flTime)
{
	
	if(firstCrazyShit){
		firstCrazyShit = FALSE;
		for(int i = 0; i < 44; i++){
			crazyRandomShitX[i] = randomValueInt(0, ScreenWidth);
			crazyRandomShitY[i] = randomValueInt(0, ScreenHeight);
		}
	}

	int randomFudgeX = 0;
	int randomFudgeY = 0;
	
	//AMMO DRAWN HERE IN "Ammo.cpp"'s Draw method, as I don't want to create any new files.
	//Re-location to hud_redraw to be less specific to ammo may be considered, if that really matters.
	
	/*
	if(firstTime){
		easyPrintLine("SCREEN PRINTOUT1 %.2f %.2f %.2f", (float)ScreenWidth, (float)ScreenHeight,  ((float)ScreenWidth / (float)ScreenHeight) );
		easyPrintLine("SCREEN PRINTOUT2 %d %d %f", ScreenWidth, ScreenHeight,  ((float)ScreenWidth / (float)ScreenHeight) );
		
		easyPrintLine("SCREEN PRINTOUT3 %d %d %f", (int)((float)ScreenWidth), (int)((float)ScreenHeight),  ((float)ScreenWidth / (float)ScreenHeight) );
		firstTime = FALSE;
	}
	*/

	//gEngfuncs.pfnConsolePrint( "YAYAYAYAYA" );
	
	//gEngfuncs.pfnDrawCharacter(66, 66, 4, 255, 255, 255);
	//TextMessageDrawChar(66, 66, 24, 255, 255, 255);
	//gHUD.DrawHudNumberString(24, 24, 24, 24, 255, 255, 255);
	
	
	gEngfuncs.pfnDrawCharacter(36, 36, 215, 1, 1, 1);


	drawString( ScreenWidth - 240, 24, (char*) "ARE YOU FEELIN\' IT NOW", 0.8, 0, 0 );
	drawString( ScreenWidth - 140, 44, (char*) "MR. KRABS", 0.8, 0, 0 );
	
	drawString( 60, ScreenHeight - 170, (char*) "Consequences will never be the same", 0.5, 0.5, 0.4 );


	drawStringFabulous( ScreenWidth - 210, ScreenHeight - 110, "8===========D" );
	drawStringFabulous( ScreenWidth - 210, ScreenHeight - 125, "8===========D" );
	drawStringFabulous( ScreenWidth - 210, ScreenHeight - 140, "8===========D" );

	//depend on time.

	//float timeFactor = (modulusFloat(flTime, 3.6f) / 3.6f);
	//int xVal = (int) (timeFactor * (ScreenWidth *1.5) - ScreenWidth*0.5 ) ;

	//int xVal = (int) (getTimePeriod(flTime, 5.6f, -ScreenWidth, ScreenWidth) );

	int xVal = (int) (getTimePeriod(flTime, 8.6f, -ScreenWidth, ScreenWidth) );
	int yValXXX = (int) (getTimePeriod( (int)(flTime / 8.6f), 5.6f, 80, ScreenHeight - 80) );
	//drawString( xVal, ScreenHeight - 200, (char*) "FUCK YO COUCH", 0.5, 0.5, 0.4 );
	//drawString( xVal, ScreenHeight - 280, (char*) "OH SHIT!!!", 0.5, 0.9, 0.4 );
	drawString( xVal, yValXXX * 1, (char*) "FUCK YO COUCH", 0.5, 0.5, 0.4 );
	drawString( xVal, 60 + yValXXX * 1, (char*) "OH SHIT!!!", 0.5, 0.9, 0.4 );
	

	int xVal2 = (int) (getTimePeriod(flTime, 2.8f, -ScreenWidth, 0) );
	int yVal = (int) (getTimePeriod(flTime, 2.8f, -ScreenHeight, 0) );
	
	randomFudgeX = randomValueInt(-2, 2);
	randomFudgeY = randomValueInt(-2, 2);
	drawStringFabulous( xVal + 40 + randomFudgeX, ScreenHeight - 250 + randomFudgeY, "8===========D" );

	randomFudgeX = randomValueInt(-1, 1);
	randomFudgeY = randomValueInt(-1, 1);

	drawStringFabulous( xVal2+ScreenWidth + randomFudgeX, ScreenHeight/2 + randomFudgeY, "LOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOL" );
	drawStringFabulous( xVal2 + randomFudgeX, ScreenHeight/2 + randomFudgeY, "LOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOL" );
	drawStringFabulousVert( ScreenWidth/2 + randomFudgeX, yVal+ScreenHeight + randomFudgeY, "LOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOL" );
	drawStringFabulousVert( ScreenWidth/2 + randomFudgeX, yVal + randomFudgeY, "LOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOLOL" );
	

	for(int i = 0; i < 44; i++){
		if(crazyRandomShitX[i] <= 0){
			crazyRandomShitX[i] ++;
		}else if(crazyRandomShitX[i] >= ScreenWidth){
			crazyRandomShitX[i] --;
		}else{
			//random.
			if(randomValueInt(0, 1) == 0){
				crazyRandomShitX[i] += 5;
			}else{
				crazyRandomShitX[i] -= 5;
			}
		}
		if(crazyRandomShitY[i] <= 0){
			crazyRandomShitY[i] ++;
		}else if(crazyRandomShitY[i] >= ScreenHeight){
			crazyRandomShitY[i] --;
		}else{
			//random.
			if(randomValueInt(0, 1) == 0){
				crazyRandomShitY[i] += 5;
			}else{
				crazyRandomShitY[i] -= 5;
			}
		}

		int choice = randomValueInt(33, 126);
		char strTemp[2];
		strTemp[0] = (char) choice;
		strTemp[1] = '\0';
		drawStringFabulous(crazyRandomShitX[i], crazyRandomShitY[i], strTemp);

	}

	drawStringFabulous(30, 80, "FABULOUS");

	randomFudgeX = randomValueInt(-2, 2);
	randomFudgeY = randomValueInt(-2, 2);

	drawStringFabulous(40 + randomFudgeX, 140 + randomFudgeY, "TASTE THE RAINBOW");
	drawStringFabulous(70 + randomFudgeX, 157 + randomFudgeY, "MOTHERFUCKER");

	
	randomFudgeX = randomValueInt(-1, 1);
	randomFudgeY = randomValueInt(-1, 1);
	
	int xVal3 = (int) (getTimePeriod(flTime, 5.1f, -80, ScreenWidth + 80) );
	int yVal2 = (int) (getTimePeriod(flTime, 5.1f, -60, ScreenHeight + 20) );
	
	drawStringFabulous( -80 + xVal3 + randomFudgeX, -20 + yVal2*0.5 + randomFudgeY, "BITCHES" );
	
	drawStringFabulous(ScreenWidth - 360 + randomFudgeX + cos(flTime * 3) * 80, 190 + randomFudgeY + cos(flTime * 8) * 40, "MOTHERFUCKERS GONNA FUCK");

	drawStringFabulous(ScreenWidth/2 - 400 + randomFudgeX + cos(flTime * 1.2f) * 480, ScreenHeight + sin(flTime * 1.2f) * 160, "I\'M LITTLE TEAPOT SUCK MY DICK");

	int xVal4 = (int) (getTimePeriod(flTime, 14.1f, -80, ScreenWidth + 80) );
	drawString(xVal4, sin(flTime * 0.7)*400 + ScreenHeight/2, "DICKS", 1, 1, 0);

	int xVal5 = (int) (getTimePeriod(flTime, 12.0f, -80, ScreenWidth + 80) );
	drawString(xVal5, sin(flTime * 1.2)*70 + ScreenHeight - 300, "BITCHES DIG MY SINE WAVE", 1, 0, 1);

	float rad = 0;
	
	//int originX = ScreenWidth - 220;
	//int originY = 170;


	float radius = randomValue(150, 180);
	float anotherPeriod = getTimePeriodAndBack(flTime, 0.6f, 180, 224);


	int originX = ScreenWidth / 2 - 5;
	int originY = ScreenHeight / 2 - 8;


	for(rad = 0.0f; rad < M_PI * 2.0f; rad += (M_PI / 48.0f)){
		int tempx = (int) (cos(rad) * anotherPeriod + originX);
		int tempy = (int) (sin(rad) * anotherPeriod + originY);

		int choice = randomValueInt(33, 126);
		char strTemp[2];
		strTemp[0] = (char) choice;
		strTemp[1] = '\0';
		drawStringFabulous(tempx, tempy, strTemp);

	}

	drawString( ScreenWidth/2 + 18, ScreenHeight/2- 24-18, "WE GOT MATH", 0.35, 0.53, 0.91 );
	drawString( ScreenWidth/2 + 39, ScreenHeight/2- 24, "N\' SHIT", 0.35, 0.53, 0.91 );

}//END OF drawCrazyShit



char global2PSEUDO_halflifePath[512];
char global2PSEUDO_gamePath[512];

void testForHelpFile(void){
	//gEngfuncs.pfnGetGameDirectory();

	global2PSEUDO_halflifePath[0] = '\0';

	
	int recentSlashPos = -1;
	int i = 0;
	while(i < MAX_PATH){
		if(global2PSEUDO_halflifePath[i] == '\0'){
			break;
		}else if(global2PSEUDO_halflifePath[i] == '\\' || global2PSEUDO_halflifePath[i] == '/'){
			recentSlashPos = i;
		}
		i++;
	}
	
	//the "hl.exe" in here is unhelpful, cut it (substring, cut off from the last slash onwards)
	//UTIL_substring(globalPSEUDO_halflifePath, 0, recentSlashPos + 1);
	global2PSEUDO_halflifePath[recentSlashPos + 1] = '\0';  //termination... same effect.
	
	

	const char* gameName; //this will not end in a slash.
	gameName = gEngfuncs.pfnGetGameDirectory();


	copyString(global2PSEUDO_halflifePath, global2PSEUDO_gamePath);
	UTIL_appendToEnd(global2PSEUDO_gamePath, gameName);

	UTIL_appendToEnd(global2PSEUDO_gamePath, "\\"); //end in a slash.

	//Now, does our help fule exist?


	
	if(checkSubFileExistence("helpme.txt")){
		//huh, is this a good idea?
		//easyForcePrintLine("CLIENTSIDE: I DID FIND YOUR FILE!!");
		
		//"gets the point across"
		//global2_hiddenMemPrintout = 1;
		EASY_CVAR_SET_DEBUGONLY(hiddenMemPrintout, 1);

		//easyForcePrintLine("QQQQEEEEEEEYUYYERYERYERYERY");
	}else{
		//easyForcePrintLine("CLIENTSIDE: I did not find your file and am sad.");
	}



}//END OF testForHelpFile




// called by UTIL_ServerMassCVarReset in dlls/util.cpp
void resetModCVarsClientOnly(){
	EASY_CVAR_RESET_MASS
	// if applicable
	saveHiddenCVars();
}


