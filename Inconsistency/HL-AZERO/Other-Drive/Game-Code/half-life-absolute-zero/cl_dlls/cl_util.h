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
// cl_util.h
//

#ifndef CL_UTIL_H
#define CL_UTIL_H


#include "cvardef.h"

#include "util_shared.h"  //includes util_printout.h




extern cvar_t* cl_lw;
extern cvar_t* cl_viewrollangle;
extern cvar_t* cl_viewrollspeed;

extern float g_lastFOV;

extern float globalPSEUDO_m_rawinputMem;




// don't mind these.
extern int playingMov;
extern float movieStartTime;
////////////////////////////////////////////////////////////////////




//MODDD- accessible anywhere.?
extern void lateCVarInit(void);
extern void updateClientCVarRefs(void);
extern void updateAutoFOV(void);
////////////////////////////////////////////////////////////////////


// Macros to hook function calls into the HUD object
#define HOOK_MESSAGE(x) gEngfuncs.pfnHookUserMsg(#x, __MsgFunc_##x );

#define DECLARE_MESSAGE(y, x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
							{ \
							return gHUD.##y.MsgFunc_##x(pszName, iSize, pbuf ); \
							}

//MODDD - NEW.  Similar to above, but for the base gHUD instance itself, no sub-class members.
#define DECLARE_MESSAGE_HUD(x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
							{ \
							return gHUD.MsgFunc_##x(pszName, iSize, pbuf ); \
							}
							
//MODDD - for messages defined without the base gHUD instance nor a gHUD sub-class member.
// Put a bunch of PROTOTYPE_MESSAGE near the top of the file, HOOK_MESSAGE them somewhere
// in the middle in some 'init' method, and IMPLEMENT_MESSAGE below that.  Or wherever really.
#define PROTOTYPE_MESSAGE(x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf);
#define IMPLEMENT_MESSAGE(x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf)


#define HOOK_COMMAND(x, y) gEngfuncs.pfnAddCommand( x, __CmdFunc_##y );
#define DECLARE_COMMAND(y, x) void __CmdFunc_##x( void ) \
							{ \
								gHUD.##y.UserCmd_##x( ); \
							}

//CVar defines moved to util_shared.



#define SPR_Load (*gEngfuncs.pfnSPR_Load)
#define SPR_Set (*gEngfuncs.pfnSPR_Set)
#define SPR_Frames (*gEngfuncs.pfnSPR_Frames)
#define SPR_GetList (*gEngfuncs.pfnSPR_GetList)

// SPR_Draw  draws a the current sprite as solid
#define SPR_Draw (*gEngfuncs.pfnSPR_Draw)
// SPR_DrawHoles  draws the current sprites,  with color index255 not drawn (transparent)
#define SPR_DrawHoles (*gEngfuncs.pfnSPR_DrawHoles)
// SPR_DrawAdditive  adds the sprites RGB values to the background  (additive transulency)
#define SPR_DrawAdditive (*gEngfuncs.pfnSPR_DrawAdditive)

// SPR_EnableScissor  sets a clipping rect for HUD sprites.  (0,0) is the top-left hand corner of the screen.
#define SPR_EnableScissor (*gEngfuncs.pfnSPR_EnableScissor)
// SPR_DisableScissor  disables the clipping rect
#define SPR_DisableScissor (*gEngfuncs.pfnSPR_DisableScissor)
//
#define FillRGBA (*gEngfuncs.pfnFillRGBA)


// ScreenHeight returns the height of the screen, in pixels
#define ScreenHeight (gHUD.m_scrinfo.iHeight)
// ScreenWidth returns the width of the screen, in pixels
#define ScreenWidth (gHUD.m_scrinfo.iWidth)

// Use this to set any co-ords in 640x480 space
#define XRES(x)		((int)(float(x)  * ((float)ScreenWidth / 640.0f) + 0.5f))
#define YRES(y)		((int)(float(y)  * ((float)ScreenHeight / 480.0f) + 0.5f))

// use this to project world coordinates to screen coordinates
#define XPROJECT(x)	( (1.0f+(x))*ScreenWidth*0.5f )
#define YPROJECT(y) ( (1.0f-(y))*ScreenHeight*0.5f )

#define GetScreenInfo (*gEngfuncs.pfnGetScreenInfo)
#define ServerCmd (*gEngfuncs.pfnServerCmd)
#define ClientCmd (*gEngfuncs.pfnClientCmd)
#define SetCrosshair (*gEngfuncs.pfnSetCrosshair)
#define AngleVectors (*gEngfuncs.pfnAngleVectors)


// returns the players name of entity no.
#define GetPlayerInfo (*gEngfuncs.pfnGetPlayerInfo)


//global index of the crosshair index.  Load this early on.
extern int alphaCrossHairIndex;

extern void SetCrosshairFiltered( SpriteHandle_t hspr, wrect_t rc, int r, int g, int b );
extern void SetCrosshairFiltered( SpriteHandle_t hspr, wrect_t rc, int r, int g, int b, int forceException );





// Gets the height & width of a sprite,  at the specified frame
inline int SPR_Height( SpriteHandle_t x, int f )	{ return gEngfuncs.pfnSPR_Height(x, f); }
inline int SPR_Width( SpriteHandle_t x, int f )	{ return gEngfuncs.pfnSPR_Width(x, f); }

inline 	client_textmessage_t	*TextMessageGet( const char *pName ) { return gEngfuncs.pfnTextMessageGet( pName ); }
inline 	int					TextMessageDrawChar( int x, int y, int number, int r, int g, int b ) 
{ 
	return gEngfuncs.pfnDrawCharacter( x, y, number, r, g, b ); 
}


//MODDD - look all below, any mentions of "string" changed to "par_string".
inline int DrawConsoleString( int x, int y, const char *par_string )
{
	return gEngfuncs.pfnDrawConsoleString( x, y, (char*) par_string );
}

inline void GetConsoleStringSize( const char *par_string, int *width, int *height )
{
	gEngfuncs.pfnDrawConsoleStringLen( par_string, width, height );
}

inline int ConsoleStringLen( const char *par_string )
{
	int _width, _height;
	GetConsoleStringSize( par_string, &_width, &_height );
	return _width;
}

inline void ConsolePrint( const char *par_string )
{
	gEngfuncs.pfnConsolePrint( par_string );
}

inline void CenterPrint( const char *par_string )
{
	gEngfuncs.pfnCenterPrint( par_string );
}

// sound functions
inline void PlaySound( char *szSound, float vol ) { gEngfuncs.pfnPlaySoundByName( szSound, vol ); }
inline void PlaySound( int iSound, float vol ) { gEngfuncs.pfnPlaySoundByIndex( iSound, vol ); }

void ScaleColors( int &r, int &g, int &b, int a );



//MODDD -
//MODDD - Moved to common/vector.h
//inline void VectorClear(float* a) { a[0] = 0.0; a[1] = 0.0; a[2] = 0.0; }

/*
// and wait.. these are all defined in pm_shared/pm_math.c slightly differently too?
float Length(const float* v);
void VectorAngles(const float* forward, float* angles);
float VectorNormalize(float* v);
void VectorMA(const float* veca, float scale, const float* vecb, float* vecc);
void VectorScale(const float* in, float scale, float* out);
void VectorInverse(float* v);
*/

extern vec3_t vec3_origin;



inline void UnpackRGB(int &r, int &g, int &b, unsigned long ulRGB)\
{\
	r = (ulRGB & 0xFF0000) >>16;\
	g = (ulRGB & 0xFF00) >> 8;\
	b = ulRGB & 0xFF;\
}

SpriteHandle_t LoadSprite(const char *pszName);




extern void generateColor(int* ary);
extern void generateColorDec(float* ary);


extern float randomInvert(const float& arg_flt);
extern float randomAbsoluteValue(const float& arg_fltMin, const float& arg_fltMax);
extern int randomValueInt(const int& arg_min, const int& arg_max);
extern float randomValue(const float& arg_fltMin, const float& arg_fltMax);
extern float modulusFloat(const float& arg_dividend, const float& arg_divisor );
extern float getTimePeriod(const float& arg_time, const float& arg_period, const float& arg_extentMin, const float& arg_extentMax);
extern float getTimePeriodAndBack(const float& arg_time, const float& arg_period, const float& arg_extentMin, const float& arg_extentMax);

extern float getTimePeriodAndBackSmooth(const float& arg_time, const float& arg_period, const float& arg_extentMin, const float& arg_extentMax);
extern float getTimePeriodAndBackSmooth(const float& arg_time, const float& arg_period, const float& arg_period2, const float& arg_extentMin, const float& arg_extentMax);
	

extern void drawStringFabulous(int arg_x, int arg_y, const char* arg_str);
extern void drawStringFabulousVert(int arg_x, int arg_y, const char* arg_str);
extern void drawString(int arg_x, int arg_y, const char* arg_str, const float& r, const float& g, const float& b);
extern void drawCrazyShit(float flTime);


extern void testForHelpFile(void);

extern void resetModCVarsClientOnly(void);


//MODDD - used to be prototyped (externed?) in weapons.h serverside for whatever reason (weapons.h is still included clientside... huh)
// And removed, made obsolete by IsMutliplayer in util_shared.h/.cpp
//extern bool bIsMultiplayer(void);


#endif //END OF CL_UTIL_H