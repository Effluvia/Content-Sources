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

// Constants shared by the engine and dlls
// This header file included by engine files and DLL files.
// Most came from server.h

//MODDD NOTE - const.h is included in so many places, client/serverside, and even
// the C lang (pm_shared files are in that), putting any #includes here ensures the
// files are included nearly everywhere.



#ifndef CONST_H
#define CONST_H

#include "external_lib_include.h"
// A cheap way to include custom CVars throughout all files in the entire project.
#include "cvar_custom.h"
// And vectors, so common they even show up in this very file.
// This will also include "mathlib.h", if it makes sense to
// (always for serverside, only for the C Lang for clientside, as there are already
//  C++ versions in common/vector.cpp.  The C versions don't support method overloading).
// never mind the client/server restriction, just not being C++ is enough.
#include "vector.h"








// !!! BUNCHA DEFINES.  Why not go everywhere, at least primarily stuff that doesn't seem
// Half-Life specific (DLLEXPORT, etc.)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



// from utils/vgui/include/VGUI.h.
// Why only that file?
#define null 0L


// C functions for external declarations that call the appropriate C++ methods
//NOTE - looks Like it derives something similar to DLLEXPORT, but notice that
// "declspec" begins with one underscore instead of two.  Just keep it this way.

// "EXPORT" has to go before methods of entities tied to the engine
// as events, such as Think, Touch, and Use  (methods ever intended to be set
// to those, like 'setTouch(&thatMethod);', etc.
// I don't expect any new usage of DLLEXPORT though.
#ifdef _WIN32
#define EXPORT _declspec( dllexport )
#else
#define EXPORT /* */
#endif


// Not sure why DLLEXPORT was re-defined in so many files:
//  This was defined the same in the following files (and removed from since):
//  cl_dlls/cdll_int.cpp
//  cl_dlls/demo.cpp
//  cl_dlls/entity.cpp
//  cl_dlls/GameStudioModelRenderer.cpp
//  cl_dlls/in_defs.h
//  cl_dlls/tri.cpp
// These files rely on that same definition anyway:
//  cl_dlls/input.cpp
//  cl_dlls/inputw32.cpp
//  cl_dlls/in_camera.cpp
//  cl_dlls/view.cpp

// The version used in those files has been moved here.
// Every single mention of this is for prototypes in 'extern "c"' space.
// Implementations of those don't do 'extern "c"' that but still get the DLLEXPORT tag.
// There was another define that derived the exact same result called "_DLLEXPORT".
// Yes, that similar in name.  No idea.
#define DLLEXPORT __declspec( dllexport )

// HOWEVER, engine/eiface.h had this instead:
#ifdef _WIN32
#define DLL_CALL_CONV __stdcall
#else
#define DLL_CALL_CONV
#endif





// ARRAYSIZE macro now won't happen if already defined, such as by Windows.
// If overriding those turns out to be necssary, forget these 'ifndef' checks.
#ifndef ARRAYSIZE
// Was in ev_hldm.cpp... strangely enough.
#define ARRAYSIZE(p) (sizeof(p)/sizeof(p[0]))
#endif



// Makes these more explicit, and easier to find
#define FILE_GLOBAL static
#define DLL_GLOBAL

// Until we figure out why "const" gives the compiler problems, we'll just have to use
// this bogus "empty" define to mark things as constant.
#define CONSTANT


// undefing min, max, and fabs is a terrible idea apparently.
//#undef min
//#undef max
//#undef fabs
//#undef ARRAYSIZE


// some stuff from util.cpp
// Testing strings for nullity
#define iStringNull 0
// Use this instead of ALLOC_STRING on constant strings
#define STRING(offset)		(const char *)(gpGlobals->pStringBase + (int)offset)
#define MAKE_STRING(str)	((int)str - (int)STRING(0))






/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////








// PM-SHARED RELATED.  May as well go here in common space to go absolutely everywhere.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forget everything you know about TRUE and FALSE.
// Also, no idea why dlls/extdll.h tried to do "define TRUE (!FALSE)".  What?
// It would have to be the exact same 1 as anywhere else since FALSE was defined 0,
// and (!0) must be 1.
//#undef TRUE
//#undef FALSE
#define TRUE 1
#define FALSE 0


// not really sure what else to call this?  Max number of entities possible in the game at a time, going
// past index 899 crashes if I remember right.  (Not be confused with MAX_MAP_ENTITIES from a barely related,
// if at all, file)
#define GAME_MAX_ENTITIES 900

#define MAX_CLIENTS 32

//MODDD - version better for working with floats.
// It begins losing accuracy after the first 5 decimal places, but even visual studio added
// a few extra anyway on a preview of 3.14...f.
#define M_PI 3.141592741f

// OLD PI CONSTANT
// A printout of the as-is PI constant, even as a proper double, gave this instead printed out (for comparison):
// Accurate up to 15 places.
//           3.141592653589793115997963...
//#define M_PI 3.14159265358979323846

//MODDD - Some extra math stuff because hey, why not.
#define M_180_RAD 3.141592741f
#define M_2PI 6.283185482f

// may as well be floats  (changed to approximations they end up as anyway)
//#define CONST_DEG_TO_RAD_CONV 0.017453293
//#define CONST_RAD_TO_DEG_CONV 57.295779506
#define CONST_DEG_TO_RAD_CONV 0.01745329238f
#define CONST_RAD_TO_DEG_CONV 57.29578018f







#define PLAYER_FATAL_FALL_SPEED		1024// approx 60 feet
#define PLAYER_MAX_SAFE_FALL_SPEED	580// approx 20 feet
#define DAMAGE_FOR_FALL_SPEED		(float) 100 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED )// damage per unit per second.
#define PLAYER_MIN_BOUNCE_SPEED		200


#define PLAYER_FALL_PUNCH_THRESHHOLD (float)350 // won't punch player's screen/make scrape noise unless player falling at least this fast.

#define PLAYER_FALL_PUNCH_TINY_THRESHHOLD (float)0.6f //MODDD : unused.

#define PLAYER_LONGJUMP_SPEED 350 // how fast we longjump



//MODDD - from clientside's cl_util.h.
// ...what a journey. Now it's everywhere.
// Also surrounded by 'ifndef''s.  If some system library defines these, just use that.
#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef fabs
// NOTE - supplied by base libaries too now in modern VS, but doesn't seem to hurt here.
#define fabs(x)    ((x) > 0 ? (x) : 0 - (x))
#endif


#define PM_DEAD_VIEWHEIGHT	-8

#define VEC_VIEW			Vector( 0, 0, 28 )
#define VEC_VIEW_Z			28


#define VEC_HULL_MIN		Vector(-16, -16, -36)
#define VEC_HULL_MIN_Z		-36
#define VEC_HULL_MAX		Vector( 16,  16,  36)
#define VEC_HULL_MAX_Z		36

#define VEC_HUMAN_HULL_MIN	Vector( -16, -16, 0 )
#define VEC_HUMAN_HULL_MAX	Vector( 16, 16, 72 )
#define VEC_HUMAN_HULL_DUCK	Vector( 16, 16, 36 )

#define VEC_DUCK_HULL_MIN	Vector(-16, -16, -18 )
#define VEC_DUCK_HULL_MIN_Z	-18
#define VEC_DUCK_HULL_MAX	Vector( 16,  16,  18)
#define VEC_DUCK_HULL_MAX_Z	18

#define VEC_DUCK_VIEW		Vector( 0, 0, 12 )
#define VEC_DUCK_VIEW_Z		12

// defaults for clientinfo messages

// up / down
#define PITCH	0
// left / right
#define YAW		1
// fall over
#define ROLL	2 

// (sound.cpp)
#define CTEXTURESMAX		512			// max number of textures loade


// See common/vector.h for re-defines of DotProduct, VectorSubtract, VectorAdd, and VectorCopy
// compared to the common/mathlib.h and utils/common/matlih.b files.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////




// Special sequence number for animations to use to force no update on the client.  Still using this?
// If a sequence number ever reaches 63 (doubtful), this won't make sense.
#define ANIM_NO_UPDATE 63






// some pretty danged important things.
typedef int BOOL;
typedef int func_t;
typedef int string_t;

typedef unsigned char byte;
typedef unsigned short word;
#define _DEF_BYTE_

typedef float vec_t;

// careful, 'qboolean' shows up in C files mostly, and
// this came from util_model.cpp (C++).
//WAIT STOP.  Already handled further down in this file.
//#ifdef __cplusplus
//typedef int qboolean;
//#endif


//MODDD
// for pre-release player ladder logic. Needs to be here to reach pm_shared.h.
// was 0.3833
#define LADDER_CYCLE_BASE (0.4000 * 10000)

//MODDD - edit
//#define MAX_CLIMB_SPEED_ALPHA	200
//130?
// previous build, was 268 and 153.
#define MAX_CLIMB_SPEED_ALPHA 240  // fastest vertical climbing speed possible
							 // (used for the alpha ladder, or cl_ladder 0 instead)
#define MAX_CLIMB_SPEED_RETAIL 125    //old speed for being forced off a ladder sometimes?
								  // Mainly the speed of the retail ladder style (cl_ladder 1).
								  // Keep in mind, slower than retail even at default ladderspeedmulti
								  // (was 200)
// OLD LADDER SPEED DEFAULTS:
// MAX_CLIMB_SPEED_ALPHA 110
// MAX_CLIMB_SPEED_RETAIL 200

// this used to be the same as MAX_CLIMB_SPEED_RETAIL, new constant for this.
#define JUMP_OFF_FORCE 120




//MODDD - also seen by pm_shared.c.
// flags for iuser4.
#define FLAG_JUMPED	8192	//2^13.
#define FLAG_RESET_RECEIVED 16384 //2^14.
#define FLAG_CYCLE_PASSED 32768 //2^15.




//MODDD - flags moved from weapons.h to be available clientside as well
#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16 // A player can totally exhaust their ammo supply and lose this weapon

#define WEAPON_IS_ONTARGET 0x40









// edict->flags
#define FL_FLY					(1<<0)	// Changes the SV_Movestep() behavior to not need to be on ground
#define FL_SWIM					(1<<1)	// Changes the SV_Movestep() behavior to not need to be on ground (but stay in water)
#define FL_CONVEYOR				(1<<2)
#define FL_CLIENT				(1<<3)
#define FL_INWATER				(1<<4)
#define FL_MONSTER				(1<<5)
#define FL_GODMODE				(1<<6)
#define FL_NOTARGET				(1<<7)
#define FL_SKIPLOCALHOST		(1<<8)	// Don't send entity to local host, it's predicting this entity itself
#define FL_ONGROUND				(1<<9)	// At rest / on the ground
#define FL_PARTIALGROUND		(1<<10)	// not all corners are valid
#define FL_WATERJUMP			(1<<11)	// player jumping out of water
#define FL_FROZEN				(1<<12) // Player is frozen for 3rd person camera
#define FL_FAKECLIENT			(1<<13)	// JAC: fake client, simulated server side; don't send network messages to them
#define FL_DUCKING				(1<<14)	// Player flag -- Player is fully crouched
#define FL_FLOAT				(1<<15)	// Apply floating force to this entity when in water
#define FL_GRAPHED				(1<<16) // worldgraph has this ent listed as something that blocks a connection

// UNDONE: Do we need these?
#define FL_IMMUNE_WATER			(1<<17)
#define FL_IMMUNE_SLIME			(1<<18)
#define FL_IMMUNE_LAVA			(1<<19)

#define FL_PROXY				(1<<20)	// This is a spectator proxy
#define FL_ALWAYSTHINK			(1<<21)	// Brush model flag -- call think every frame regardless of nextthink - ltime (for constantly changing velocity/path)
#define FL_BASEVELOCITY			(1<<22)	// Base velocity has been applied this frame (used to convert base velocity into momentum)
#define FL_MONSTERCLIP			(1<<23)	// Only collide in with monsters who have FL_MONSTERCLIP set
#define FL_ONTRAIN				(1<<24) // Player is _controlling_ a train, so movement commands should be ignored on client during prediction.
#define FL_WORLDBRUSH			(1<<25)	// Not moveable/removeable brush entity (really part of the world, but represented as an entity for transparency or something)
#define FL_SPECTATOR            (1<<26) // This client is a spectator, don't run touch functions, etc.
#define FL_CUSTOMENTITY			(1<<29)	// This is a custom entity
#define FL_KILLME				(1<<30)	// This entity is marked for death -- This allows the engine to kill ents at the appropriate time
#define FL_DORMANT				(1<<31)	// Entity is dormant, no updates to client


// Goes into globalvars_t.trace_flags
#define FTRACE_SIMPLEBOX		(1<<0)	// Traceline with a simple box


// walkmove modes
#define WALKMOVE_NORMAL		0 // normal walkmove
#define WALKMOVE_WORLDONLY	1 // doesn't hit ANY entities, no matter what the solid type
#define WALKMOVE_CHECKONLY	2 // move, but don't touch triggers

// edict->movetype values
#define MOVETYPE_NONE			0		// never moves
//#define MOVETYPE_ANGLENOCLIP	1
//#define MOVETYPE_ANGLECLIP		2
#define MOVETYPE_WALK			3		// Player only - moving on the ground

//MODDD REALLY IMPORTANT NOTE
// This move flag locks the monster to the ground. That isn't too surprising though.
// The surprising thing is, this also interpolates monster movement so that they don't snap to the 0.1 second increments that it would usually be limited 
// to due to MonsterThink's limit of 0.1 seconds. MOVETYPE_TOSS can also move against the ground, but it has this issue.
#define MOVETYPE_STEP			4		// gravity, special edge handling -- monsters use this

#define MOVETYPE_FLY			5		// No gravity, but still collides with stuff
#define MOVETYPE_TOSS			6		// gravity/collisions
#define MOVETYPE_PUSH			7		// no clip to world, push and crush
#define MOVETYPE_NOCLIP			8		// No gravity, no collisions, still do velocity/avelocity
#define MOVETYPE_FLYMISSILE		9		// extra size to monsters
#define MOVETYPE_BOUNCE			10		// Just like Toss, but reflect velocity when contacting surfaces
#define MOVETYPE_BOUNCEMISSILE	11		// bounce w/o gravity
#define MOVETYPE_FOLLOW			12		// track movement of aiment
#define MOVETYPE_PUSHSTEP		13		// BSP model that needs physics/world collisions (uses nearest hull for world collision)

// edict->solid values
// NOTE: Some movetypes will cause collisions independent of SOLID_NOT/SOLID_TRIGGER when the entity moves
// SOLID only effects OTHER entities colliding with this one when they move - UGH!
#define SOLID_NOT				0		// no interaction with other objects
#define SOLID_TRIGGER			1		// touch on edge, but not blocking
#define SOLID_BBOX				2		// touch on edge, block
#define SOLID_SLIDEBOX			3		// touch on edge, but not an onground
#define SOLID_BSP				4		// bsp clip, touch on edge, block

// edict->deadflag values
#define DEAD_NO					0 // alive
#define DEAD_DYING				1 // playing death animation or still falling off of a ledge waiting to hit ground
#define DEAD_DEAD				2 // dead. lying still.
#define DEAD_RESPAWNABLE		3
#define DEAD_DISCARDBODY		4

#define DAMAGE_NO				0
#define DAMAGE_YES				1
#define DAMAGE_AIM				2

// entity effects
#define EF_BRIGHTFIELD			1	// swirling cloud of particles
#define EF_MUZZLEFLASH 			2	// single frame ELIGHT on entity attachment 0
#define EF_BRIGHTLIGHT 			4	// DLIGHT centered at entity origin
#define EF_DIMLIGHT 			8	// player flashlight
#define EF_INVLIGHT				16	// get lighting from ceiling
#define EF_NOINTERP				32	// don't interpolate the next frame
#define EF_LIGHT				64	// rocket flare glow sprite
#define EF_NODRAW				128	// don't draw entity

// entity flags
#define EFLAG_SLERP				1	// do studio interpolation of this entity

		
//
// temp entity events
//
#define TE_BEAMPOINTS		0		// beam effect between two points
// coord coord coord (start position) 
// coord coord coord (end position) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_BEAMENTPOINT		1		// beam effect between point and entity
// short (start entity) 
// coord coord coord (end position) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_GUNSHOT			2		// particle effect plus ricochet sound
// coord coord coord (position) 

#define TE_EXPLOSION		3		// additive sprite, 2 dynamic lights, flickering particles, explosion sound, move vertically 8 pps
// coord coord coord (position) 
// short (sprite index)
// byte (scale in 0.1's)
// byte (framerate)
// byte (flags)
//
// The Explosion effect has some flags to control performance/aesthetic features:
#define TE_EXPLFLAG_NONE		0	// all flags clear makes default Half-Life explosion
#define TE_EXPLFLAG_NOADDITIVE	1	// sprite will be drawn opaque (ensure that the sprite you send is a non-additive sprite)
#define TE_EXPLFLAG_NODLIGHTS	2	// do not render dynamic lights
#define TE_EXPLFLAG_NOSOUND		4	// do not play client explosion sound
#define TE_EXPLFLAG_NOPARTICLES	8	// do not draw particles


#define TE_TAREXPLOSION		4		// Quake1 "tarbaby" explosion with sound
// coord coord coord (position) 

#define TE_SMOKE			5		// alphablend sprite, move vertically 30 pps
// coord coord coord (position) 
// short (sprite index)
// byte (scale in 0.1's)
// byte (framerate)

#define TE_TRACER			6		// tracer effect from point to point
// coord, coord, coord (start) 
// coord, coord, coord (end)

#define TE_LIGHTNING		7		// TE_BEAMPOINTS with simplified parameters
// coord, coord, coord (start) 
// coord, coord, coord (end) 
// byte (life in 0.1's) 
// byte (width in 0.1's) 
// byte (amplitude in 0.01's)
// short (sprite model index)

#define TE_BEAMENTS			8		
// short (start entity) 
// short (end entity) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_SPARKS			9		// 8 random tracers with gravity, ricochet sprite
// coord coord coord (position) 

#define TE_LAVASPLASH		10		// Quake1 lava splash
// coord coord coord (position) 

#define TE_TELEPORT			11		// Quake1 teleport splash
// coord coord coord (position) 

#define TE_EXPLOSION2		12		// Quake1 colormaped (base palette) particle explosion with sound
// coord coord coord (position) 
// byte (starting color)
// byte (num colors)

#define TE_BSPDECAL			13		// Decal from the .BSP file 
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// short (texture index of precached decal texture name)
// short (entity index)
// [optional - only included if previous short is non-zero (not the world)] short (index of model of above entity)

#define TE_IMPLOSION		14		// tracers moving toward a point
// coord, coord, coord (position)
// byte (radius)
// byte (count)
// byte (life in 0.1's) 

#define TE_SPRITETRAIL		15		// line of moving glow sprites with gravity, fadeout, and collisions
// coord, coord, coord (start) 
// coord, coord, coord (end) 
// short (sprite index)
// byte (count)
// byte (life in 0.1's) 
// byte (scale in 0.1's) 
// byte (velocity along vector in 10's)
// byte (randomness of velocity in 10's)

#define TE_BEAM				16		// obsolete

#define TE_SPRITE			17		// additive sprite, plays 1 cycle
// coord, coord, coord (position) 
// short (sprite index) 
// byte (scale in 0.1's) 
// byte (brightness)

#define TE_BEAMSPRITE		18		// A beam with a sprite at the end
// coord, coord, coord (start position) 
// coord, coord, coord (end position) 
// short (beam sprite index) 
// short (end sprite index) 

#define TE_BEAMTORUS		19		// screen aligned beam ring, expands to max radius over lifetime
// coord coord coord (center position) 
// coord coord coord (axis and radius) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_BEAMDISK			20		// disk that expands to max radius over lifetime
// coord coord coord (center position) 
// coord coord coord (axis and radius) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_BEAMCYLINDER		21		// cylinder that expands to max radius over lifetime
// coord coord coord (center position) 
// coord coord coord (axis and radius) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_BEAMFOLLOW		22		// create a line of decaying beam segments until entity stops moving
// short (entity:attachment to follow)
// short (sprite index)
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte,byte,byte (color)
// byte (brightness)

#define TE_GLOWSPRITE		23		
// coord, coord, coord (pos) short (model index) byte (scale / 10)

#define TE_BEAMRING			24		// connect a beam ring to two entities
// short (start entity) 
// short (end entity) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_STREAK_SPLASH	25		// oriented shower of tracers
// coord coord coord (start position) 
// coord coord coord (direction vector) 
// byte (color)
// short (count)
// short (base speed)
// short (ramdon velocity)

#define TE_BEAMHOSE			26		// obsolete

#define TE_DLIGHT			27		// dynamic light, effect world, minor entity effect
// coord, coord, coord (pos) 
// byte (radius in 10's) 
// byte byte byte (color)
// byte (brightness)
// byte (life in 10's)
// byte (decay rate in 10's)

#define TE_ELIGHT			28		// point entity light, no world effect
// short (entity:attachment to follow)
// coord coord coord (initial position) 
// coord (radius)
// byte byte byte (color)
// byte (life in 0.1's)
// coord (decay rate)

#define TE_TEXTMESSAGE		29
// short 1.2.13 x (-1 = center)
// short 1.2.13 y (-1 = center)
// byte Effect 0 = fade in/fade out
			// 1 is flickery credits
			// 2 is write out (training room)

// 4 bytes r,g,b,a color1	(text color)
// 4 bytes r,g,b,a color2	(effect color)
// ushort 8.8 fadein time
// ushort 8.8  fadeout time
// ushort 8.8 hold time
// optional ushort 8.8 fxtime	(time the highlight lags behing the leading text in effect 2)
// string text message		(512 chars max sz string)
#define TE_LINE				30
// coord, coord, coord		startpos
// coord, coord, coord		endpos
// short life in 0.1 s
// 3 bytes r, g, b

#define TE_BOX				31
// coord, coord, coord		boxmins
// coord, coord, coord		boxmaxs
// short life in 0.1 s
// 3 bytes r, g, b

#define TE_KILLBEAM			99		// kill all beams attached to entity
// short (entity)

#define TE_LARGEFUNNEL		100
// coord coord coord (funnel position)
// short (sprite index) 
// short (flags) 

#define TE_BLOODSTREAM		101		// particle spray
// coord coord coord (start position)
// coord coord coord (spray vector)
// byte (color)
// byte (speed)

#define TE_SHOWLINE			102		// line of particles every 5 units, dies in 30 seconds
// coord coord coord (start position)
// coord coord coord (end position)

#define TE_BLOOD			103		// particle spray
// coord coord coord (start position)
// coord coord coord (spray vector)
// byte (color)
// byte (speed)

#define TE_DECAL			104		// Decal applied to a brush entity (not the world)
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name)
// short (entity index)

#define TE_FIZZ				105		// create alpha sprites inside of entity, float upwards
// short (entity)
// short (sprite index)
// byte (density)

#define TE_MODEL			106		// create a moving model that bounces and makes a sound when it hits
// coord, coord, coord (position) 
// coord, coord, coord (velocity)
// angle (initial yaw)
// short (model index)
// byte (bounce sound type)
// byte (life in 0.1's)

#define TE_EXPLODEMODEL		107		// spherical shower of models, picks from set
// coord, coord, coord (origin)
// coord (velocity)
// short (model index)
// short (count)
// byte (life in 0.1's)

#define TE_BREAKMODEL		108		// box of models or sprites
// coord, coord, coord (position)
// coord, coord, coord (size)
// coord, coord, coord (velocity)
// byte (random velocity in 10's)
// short (sprite or model index)
// byte (count)
// byte (life in 0.1 secs)
// byte (flags)

#define TE_GUNSHOTDECAL		109		// decal and ricochet sound
// coord, coord, coord (position)
// short (entity index???)
// byte (decal???)

#define TE_SPRITE_SPRAY		110		// spay of alpha sprites
// coord, coord, coord (position)
// coord, coord, coord (velocity)
// short (sprite index)
// byte (count)
// byte (speed)
// byte (noise)

#define TE_ARMOR_RICOCHET	111		// quick spark sprite, client ricochet sound. 
// coord, coord, coord (position)
// byte (scale in 0.1's)

#define TE_PLAYERDECAL		112		// ???
// byte (playerindex)
// coord, coord, coord (position)
// short (entity???)
// byte (decal number???)
// [optional] short (model index???)

#define TE_BUBBLES			113		// create alpha sprites inside of box, float upwards
// coord, coord, coord (min start position)
// coord, coord, coord (max start position)
// coord (float height)
// short (model index)
// byte (count)
// coord (speed)

#define TE_BUBBLETRAIL		114		// create alpha sprites along a line, float upwards
// coord, coord, coord (min start position)
// coord, coord, coord (max start position)
// coord (float height)
// short (model index)
// byte (count)
// coord (speed)

#define TE_BLOODSPRITE		115		// spray of opaque sprite1's that fall, single sprite2 for 1..2 secs (this is a high-priority tent)
// coord, coord, coord (position)
// short (sprite1 index)
// short (sprite2 index)
// byte (color)
// byte (scale)

#define TE_WORLDDECAL		116		// Decal applied to the world brush
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name)

#define TE_WORLDDECALHIGH	117		// Decal (with texture index > 256) applied to world brush
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name - 256)

#define TE_DECALHIGH		118		// Same as TE_DECAL, but the texture index was greater than 256
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name - 256)
// short (entity index)

#define TE_PROJECTILE		119		// Makes a projectile (like a nail) (this is a high-priority tent)
// coord, coord, coord (position)
// coord, coord, coord (velocity)
// short (modelindex)
// byte (life)
// byte (owner)  projectile won't collide with owner (if owner == 0, projectile will hit any client).

#define TE_SPRAY			120		// Throws a shower of sprites or models
// coord, coord, coord (position)
// coord, coord, coord (direction)
// short (modelindex)
// byte (count)
// byte (speed)
// byte (noise)
// byte (rendermode)

#define TE_PLAYERSPRITES	121		// sprites emit from a player's bounding box (ONLY use for players!)
// byte (playernum)
// short (sprite modelindex)
// byte (count)
// byte (variance) (0 = no variance in size) (10 = 10% variance in size)

#define TE_PARTICLEBURST	122		// very similar to lavasplash.
// coord (origin)
// short (radius)
// byte (particle color)
// byte (duration * 10) (will be randomized a bit)

#define TE_FIREFIELD			123		// makes a field of fire.
// coord (origin)
// short (radius) (fire is made in a square around origin. -radius, -radius to radius, radius)
// short (modelindex)
// byte (count)
// byte (flags)
// byte (duration (in seconds) * 10) (will be randomized a bit)
//
// to keep network traffic low, this message has associated flags that fit into a byte:
#define TEFIRE_FLAG_ALLFLOAT	1 // all sprites will drift upwards as they animate
#define TEFIRE_FLAG_SOMEFLOAT	2 // some of the sprites will drift upwards. (50% chance)
#define TEFIRE_FLAG_LOOP		4 // if set, sprite plays at 15 fps, otherwise plays at whatever rate stretches the animation over the sprite's duration.
#define TEFIRE_FLAG_ALPHA		8 // if set, sprite is rendered alpha blended at 50% else, opaque
#define TEFIRE_FLAG_PLANAR		16 // if set, all fire sprites have same initial Z instead of randomly filling a cube. 

#define TE_PLAYERATTACHMENT			124 // attaches a TENT to a player (this is a high-priority tent)
// byte (entity index of player)
// coord (vertical offset) ( attachment origin.z = player origin.z + vertical offset )
// short (model index)
// short (life * 10 );

#define TE_KILLPLAYERATTACHMENTS	125 // will expire all TENTS attached to a player.
// byte (entity index of player)

#define TE_MULTIGUNSHOT				126 // much more compact shotgun message
// This message is used to make a client approximate a 'spray' of gunfire.
// Any weapon that fires more than one bullet per frame and fires in a bit of a spread is
// a good candidate for MULTIGUNSHOT use. (shotguns)
//
// NOTE: This effect makes the client do traces for each bullet, these client traces ignore
//		 entities that have studio models.Traces are 4096 long.
//
// coord (origin)
// coord (origin)
// coord (origin)
// coord (direction)
// coord (direction)
// coord (direction)
// coord (x noise * 100)
// coord (y noise * 100)
// byte (count)
// byte (bullethole decal texture index)

#define TE_USERTRACER				127 // larger message than the standard tracer, but allows some customization.
// coord (origin)
// coord (origin)
// coord (origin)
// coord (velocity)
// coord (velocity)
// coord (velocity)
// byte ( life * 10 )
// byte ( color ) this is an index into an array of color vectors in the engine. (0 - )
// byte ( length * 10 )



#define MSG_BROADCAST		0		// unreliable to all
#define MSG_ONE				1		// reliable to one (msg_entity)
#define MSG_ALL				2		// reliable to all
#define MSG_INIT			3		// write to the init string
#define MSG_PVS				4		// Ents in PVS of org
#define MSG_PAS				5		// Ents in PAS of org
#define MSG_PVS_R			6		// Reliable to PVS
#define MSG_PAS_R			7		// Reliable to PAS
#define MSG_ONE_UNRELIABLE	8		// Send to one client, but don't put in reliable stream, put in unreliable datagram ( could be dropped )
#define MSG_SPEC			9		// Sends to all spectator proxies




// contents of a spot in the world
#define CONTENTS_EMPTY		-1
#define CONTENTS_SOLID		-2
#define CONTENTS_WATER		-3
#define CONTENTS_SLIME		-4
#define CONTENTS_LAVA		-5
#define CONTENTS_SKY		-6

// These additional contents constants are defined in bspfile.h
// MODDD - uncommented out.  All here now to show up in pm_shared.c without needing
// its own re-defines.
// Even weirder when you consider that sveral macro constants were defined
// both here and in bspfile.h anyway.
// All contents will begin with "CONTENTS_" instead of "CONTENT_" like some did,
// looks like this way is more common, may as well be consistent.
#define CONTENTS_ORIGIN		-7		// removed at csg time
#define CONTENTS_CLIP		-8		// changed to contents_solid
#define CONTENTS_CURRENT_0		-9
#define CONTENTS_CURRENT_90		-10
#define CONTENTS_CURRENT_180	-11
#define CONTENTS_CURRENT_270	-12
#define CONTENTS_CURRENT_UP		-13
#define CONTENTS_CURRENT_DOWN	-14
#define CONTENTS_TRANSLUCENT	-15

#define CONTENTS_LADDER		-16

#define CONTENTS_FLYFIELD			-17
#define CONTENTS_GRAVITY_FLYFIELD	-18
#define CONTENTS_FOG				-19




// channels
#define CHAN_AUTO			0
#define CHAN_WEAPON			1
#define CHAN_VOICE			2
#define CHAN_ITEM			3
#define CHAN_BODY			4
#define CHAN_STREAM			5			// allocate stream channel from the static or dynamic area
#define CHAN_STATIC			6			// allocate channel from the static area 
#define CHAN_NETWORKVOICE_BASE	7		// voice data coming across the network
#define CHAN_NETWORKVOICE_END	500		// network voice data reserves slots (CHAN_NETWORKVOICE_BASE through CHAN_NETWORKVOICE_END).


//MODDD - for reference, 4 seems acceptable too.  Not sure how much higher is unacceptable (crashes).
// attenuation values
#define ATTN_NONE		0
#define ATTN_NORM		(float)0.8
#define ATTN_IDLE		(float)2
#define ATTN_STATIC		(float)1.25 

// pitch values
#define PITCH_NORM		100			// non-pitch shifted
#define PITCH_LOW		95			// other values are possible - 0-255, where 255 is very high
#define PITCH_HIGH		120


// volume values    (woa there guys, don't overwhelm me with choices)
#define VOL_NORM		1.0

// plats
#define PLAT_LOW_TRIGGER	1

// Trains
#define SF_TRAIN_WAIT_RETRIGGER	1
#define SF_TRAIN_START_ON		4		// Train is initially moving
#define SF_TRAIN_PASSABLE		8		// Train is not solid -- used to make water trains


//MODDD - another place for custom spawnflags.  See monsters.h for a bunch too.  Some ones specific to individual monsters are in their own files.
#define SF_GLOCK_HAS_SILENCER	8





// buttons
#ifndef IN_BUTTONS_H
#include "in_buttons.h"
#endif

// Break Model Defines

#define BREAK_TYPEMASK	0x4F
#define BREAK_GLASS		0x01
#define BREAK_METAL		0x02
#define BREAK_FLESH		0x04
#define BREAK_WOOD		0x08

#define BREAK_SMOKE		0x10
#define BREAK_TRANS		0x20
#define BREAK_CONCRETE	0x40
#define BREAK_2			0x80

// Colliding temp entity sounds

#define BOUNCE_GLASS	BREAK_GLASS
#define BOUNCE_METAL	BREAK_METAL
#define BOUNCE_FLESH	BREAK_FLESH
#define BOUNCE_WOOD		BREAK_WOOD
#define BOUNCE_SHRAP	0x10
#define BOUNCE_SHELL	0x20
#define BOUNCE_CONCRETE BREAK_CONCRETE
#define BOUNCE_SHOTSHELL 0x80

// Temp entity bounce sound types
#define TE_BOUNCE_NULL		0
#define TE_BOUNCE_SHELL		1
#define TE_BOUNCE_SHOTSHELL	2

// Rendering constants
enum 
{	
	kRenderNormal,			// src
	kRenderTransColor,		// c*a+dest*(1-a)
	kRenderTransTexture,	// src*a+dest*(1-a)
	kRenderGlow,			// src*a+dest -- No Z buffer checks
	kRenderTransAlpha,		// src*srca+dest*(1-srca)
	kRenderTransAdd,		// src*a+dest
};

//VALUES FOR RENDERFX.
//Note that, since enumeration goes up to 20, it may also act as a bitmask for 32, 64, 128, 256, 512, 1024, 2048, etc.
//UPDATE!!!   NO, that is not the case.  Unfortunately, anything above 255 gets capped to 255.  Just do it in singles beyond that I guess.

//RESERVED BITMASK PLACES (working down)
//16: NO, used.
//32: !!!available!!!
//64: do / don't draw shadows (off means do draw)
//128: player glock muzzle flash on & off (depends on silencer by logic)


enum 
{	
	kRenderFxNone = 0, 
	kRenderFxPulseSlow, 
	kRenderFxPulseFast, 
	kRenderFxPulseSlowWide, 
	kRenderFxPulseFastWide, 
	kRenderFxFadeSlow, 
	kRenderFxFadeFast, 
	kRenderFxSolidSlow, 
	kRenderFxSolidFast, 	   
	kRenderFxStrobeSlow, 
	kRenderFxStrobeFast, 
	kRenderFxStrobeFaster, 
	kRenderFxFlickerSlow, 
	kRenderFxFlickerFast,
	kRenderFxNoDissipation,
	kRenderFxDistort,			// Distort/scale/translate flicker
	kRenderFxHologram,			// kRenderFxDistort + distance fade
	kRenderFxDeadPlayer,		// kRenderAmt is the player index
	kRenderFxExplode,			// Scale up really big!
	kRenderFxGlowShell,			// Glowing Shell
	kRenderFxClampMinScale,		// Keep this sprite from getting very small (SPRITES only!)
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//MODDD - HOLY. MOTHERloving. stuff. DO NOT. INSERT. ANY. kRenderFx ENUM CHOICES. ABOVE. THIS. POINT.
	//        OR I WILL SKULLlove YOUR golly darned GOLDFISH UNTIL THAT LITTLE buddy DROWNS IN AN OCEAN
	//        OF RAGE-INFUSED ACIDIC milk THAT EATS THROUGH lovely METAL.
	//        ...Or don't because the engine sometimes acts on kRenderFx's from the as-is SDK state
	//        being what they are for some bizarre reason.  Even if studioModelRenderer.cpp refers to
	//        them explicitly already in the as-is SDK. I got nothing.
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	kRenderFxImplode,           //MODDD - new. Opposite of Explode, shrink fast instead.
	kRenderFxDummy,


	//MODDDMIRROR - all unused?
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//kRenderFxReflection,		//LRC - draw a reflection under my feet
	//kRenderFxEntInPVS,			//G-Cont - this style for entity, which draw always
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//REMOVED?
	//kRenderFxMirror,			//G-Cont - mirror transform



	//note that positions after 32 probably don't clash with new flags...

};

//NOTE - going to take advantage of the other possible bits - values after the last enum list above.
//       the "renderfx" variable used to have nothing to do with bitmasking, but now can include any values below
//       to have those effects in addition to the setting above.
//       Some bits can be reused for special circumstances, like only viewmodels using the 3rd bit (4, NOMUZZLEFLASH).

// IMPORTANT - renderflags ISVIEWMODEL and ISPLAYER removed.  There are other ways to determine these in
// studiomodelrenderer.cpp and other clientside places.  These can be freed up for other ideas.

//This is in binary for, the first 5 bits (powers 0 to 4).  Those cover values from 1 to 31 inclusive.
//This mask can be used to remove the new FX bits used for combinations of choices.
//For instance, to see if the choice for renderFX is kRenderFxExplode, it would be best to do this:
//    (m_pCurrentEntity->curstate.renderfx & RENDERFX_PRIMARY_BITS) == kRenderFxExplode
#define RENDERFX_PRIMARY_BITS ( 0x0000001F  )
//all other bits, from powers 5 to 31 (last), counting starting at 0.
#define RENDERFX_SECONDARY_BITS ( 0xFFFFFFE0  )

#define FORCE_NOLOOP (1 << 0) //ONLY use with "ISVIEWMODEL" !!!
					//becomes  (16 | ISVIEWMODEL) or 176, total.

//BYE. Not done this way anymore, not responsive enough and just plain unnecessary. Probably the same case for FORCE_NOLOOP above later.
//#define ANIMATEBACKWARDS (1 << 1) //ONLY use with "ISVIEWMODEL", otherwise no effect. Anything else (server-side) need not rely on this for playing backwards.


#define STOPINTR (1 << 5) //32
//NOTICE!!! This is a flag for pev->renderfx to be seen directly in StudioModelRendered.cpp clientside and acted on directly there,
//          no implied behavior.
//          This may be redundant with the pev->effects flag EF_NOINTERP which should do the exact same thing? Unsure, test before removing this renderflag.


#define ISNPC (1 << 6) //64
//#define NOREFLECT (DONOTDRAWSHADOW | ISNPC | ISPLAYER) //224 (exclusive combo)
#define ISWORLDAFFILIATED (1 << 7)

#define ISMETALNPC (ISNPC | (1 << 0)) //  (exclusive combo... hopefully.)



// Removing either of these when both are present would be a little awkward but shouldn't be possible anyway.
// Mainly keep the (1 << 3) and (1 << 4) bits after removing either to keep the other (remove only (1 << 1) or (1 << 2),  unless both NOMUZZLEFLASH
// and DONOTDRAWSHADOW are being removed,  then (1 << 3) and (1 << 4) go too).
#define NOMUZZLEFLASH ( (1 << 1) | (1 << 3) | (1 << 4) ) //with non-view models, this prevents the muzzle flash. This is not for viewmodels, they use a different way. Or bit(s).
#define DONOTDRAWSHADOW ( (1 << 2) | (1 << 3) | (1 << 4) )

// These used to be uncommented before the ISPLAYER / ISVIEWMODEL removal
//#define ISPLAYER (1 << 7) //128
//#define ISVIEWMODEL ( (1 << 2) | (1 << 3) | (1 << 4) ) //  (exclusive combo). This is 100011 in binary, which should not interfere with anything else.
//#define DONOTDRAWSHADOW (ISVIEWMODEL | ISPLAYER) //  (exclusive combo)

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// exclusive combo ((1 << 2) | (1 << 3) | (1 << 4) | (1 << 7)) is still available,  maybe?



//!!! Make sure this is ok!
#define FX_DUMMY kRenderFxDummy
//(DONOTDRAWSHADOW | ISNPC | ISPLAYER | ANIMATEBACKWARDS | NOMUZZLEFLASH)






#undef true
#undef false

#ifndef __cplusplus
typedef enum {false, true} qboolean;
#else 
typedef int qboolean;
#endif

typedef struct
{
	byte r, g, b;
} color24;

typedef struct
{
	unsigned r, g, b, a;
} colorVec;

#ifdef _WIN32
#pragma pack(push,2)
#endif

typedef struct
{
	unsigned short r, g, b, a;
} PackedColorVec;

#ifdef _WIN32
#pragma pack(pop)
#endif
typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;

typedef struct edict_s edict_t;

typedef struct
{
	vec3_t	normal;
	float dist;
} plane_t;

typedef struct
{
	qboolean	allsolid;	// if true, plane is not valid
	qboolean	startsolid;	// if true, the initial point was in a solid area
	qboolean	inopen, inwater;
	float fraction;		// time completed, 1.0 = didn't hit anything
	vec3_t	endpos;			// final position
	plane_t	plane;			// surface normal at impact
	edict_t	*ent;			// entity the surface is on
	int	hitgroup;		// 0 == generic, non zero is specific body part
} trace_t;

#endif

