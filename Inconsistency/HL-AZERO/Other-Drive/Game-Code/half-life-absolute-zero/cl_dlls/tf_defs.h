/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#ifndef __TF_DEFS_H
#define __TF_DEFS_H

//MODDD - most of this file cleared, never involved.
// Particularly class-stuff, although it's likely safe to remove even more like the whole team menu.
// Just be careful about that, Half-Life does support team-based multiplayer.  Who knows what logic it still
// depends on from there, I've been surprised.
// MENU_INTRO is the MOTD by the way.


/*======================*/
//      Menu stuff      //
/*======================*/

#define MENU_DEFAULT				1
#define MENU_TEAM 					2
//#define MENU_CLASS 					3
#define MENU_MAPBRIEFING			4
#define MENU_INTRO 					5
//#define MENU_CLASSHELP				6
//#define MENU_CLASSHELP2 			7
#define MENU_REPEATHELP 			8

#define MENU_SPECHELP				9
/*
#define MENU_SPY					12
#define MENU_SPY_SKIN				13
#define MENU_SPY_COLOR				14
#define MENU_ENGINEER				15
#define MENU_ENGINEER_FIX_DISPENSER	16
#define MENU_ENGINEER_FIX_SENTRYGUN	17
#define MENU_ENGINEER_FIX_MORTAR	18
#define MENU_DISPENSER				19
#define MENU_CLASS_CHANGE			20
*/
#define MENU_TEAM_CHANGE			21

#define MENU_REFRESH_RATE 			25

#define MENU_VOICETWEAK				50

#endif // __TF_DEFS_H


