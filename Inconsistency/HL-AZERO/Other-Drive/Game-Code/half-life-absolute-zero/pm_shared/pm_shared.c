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

// easyPrint methods are now available for C!

#include <assert.h>

//MODDD - NO YA DONT.  To external_lib_include.h
/*
#include <stdio.h>  // NULL
#include <math.h>   // sqrt
#include <string.h> // strcpy
#include <stdlib.h> // atoi
#include <ctype.h>  // isspace
*/


#include "pm_shared.h"

#include "external_lib_include.h"
#include "mathlib.h"
#include "const.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_movevars.h"
#include "pm_debug.h"


//MODDD - new file.
#include "pm_printout.h"
// MODDD NOTE - we have a perfectly good source of several macro constants
// and yet still, it wasn't included?
#include "pm_materials.h"



// IDEA: slightly more midair-movement for unstuck jumps?
// Better detection for false positives (don't allow if a tall wall is a huge factor in getting midair movement blocked, an extra linetrace from the model center to the velocity's floor-wise (2D) direction finding something or not might do the trick)?

// Note about pmove->multiplayer.  It looks like it stays on (1) after running multiplayer,
// even after disconnecting and running/loading a singleplayer game.  Odd.
// Unsure but it looks like it's forced to 0 or 1 in some areas, unsure.

// pmove->movevars->footsteps is set by built-in CVar "mp_footsteps" (serverside CVar,
// must have been ingame at least once since booting the game up to see it).




//MODDD - debug constant.  set to "1" (yes) to print out information during a fall concerning how far the player fell.
#define DEBUG_PRINTFALL 0


// Was testing with 0.48, but reverted.
#define DUCK_SPEED_MULTI 0.333

// 35% volume if ducking
// MODDD - 30% now.
// Unaffected by ladder-steps, that's cut in half instead
#define DUCK_STEPVOLUME_MULTI 0.3





// Ducking time
#define TIME_TO_DUCK	0.4

#define PM_CHECKSTUCK_MINTIME 0.05  // Don't check again too quickly.
// Only allow bunny jumping up to 1.7x server / player maxspeed setting
#define BUNNYJUMP_MAX_SPEED_FACTOR 1.7f

#define WJ_HEIGHT 8

#define STUCK_MOVEUP 1
#define STUCK_MOVEDOWN -1
#define STOP_EPSILON	0.1


#define STEP_CONCRETE	0		// default step sound
#define STEP_METAL		1		// metal floor
#define STEP_DIRT		2		// dirt, sand, rock
#define STEP_VENT		3		// ventillation duct
#define STEP_GRATE		4		// metal grating
#define STEP_TILE		5		// floor tiles
#define STEP_SLOSH		6		// shallow liquid puddle
#define STEP_WADE		7		// wading in liquid
#define STEP_LADDER		8		// climbing ladder


typedef struct
{
	int		planenum;
	short		children[2];	// negative numbers are contents
} dclipnode_t;

typedef struct mplane_s
{
	vec3_t	normal;			// surface normal
	float dist;			// closest appoach to origin
	byte type;			// for texture axis selection and fast side tests
	byte signbits;		// signx + signy<<1 + signz<<1
	byte pad[2];
} mplane_t;

typedef struct hull_s
{
	dclipnode_t	*clipnodes;
	mplane_t	*planes;
	int		firstclipnode;
	int		lastclipnode;
	vec3_t		clip_mins;
	vec3_t		clip_maxs;
} hull_t;

typedef enum { mod_brush, mod_sprite, mod_alias, mod_studio } modtype_t;


static int pm_shared_initialized = 0;

static vec3_t rgv3tStuckTable[54];
static int rgStuckLast[MAX_CLIENTS][2];

// Texture names
static int gcTextures = 0;
static char grgszTextureName[CTEXTURESMAX][CBTEXTURENAMEMAX];	
static char grgchTextureType[CTEXTURESMAX];

playermove_t* pmove = NULL;
int g_onladder = 0;

#ifdef CLIENT_DLL
// Spectator Mode
int	iJumpSpectator;
float vJumpOrigin[3];
float vJumpAngles[3];
#endif


//MODDD - NEW, per player.
static int ary_iJumpOffDenyLadderFrames[MAX_CLIENTS];
static int ary_iDenyLadderFrames[MAX_CLIENTS];
static float ary_flGravityModMulti[MAX_CLIENTS];

static int ary_iMidAirMoveBlocked[MAX_CLIENTS];
static int ary_iTallSlopeBelow[MAX_CLIENTS];
//static float ary_flUnstuckJumpTimer[MAX_CLIENTS];




//MODDD - utility function.
// Usefullness outside of this file unknown though.
float getSafeSqureRoot(float fltInput){
	float toReturn;
	if(fltInput < 0){
		toReturn = -sqrt(-fltInput);
	}else{
		toReturn = sqrt(fltInput);
	}
	return toReturn;
}



void PM_SwapTextures( int i, int j )
{
	char chTemp;
	char szTemp[ CBTEXTURENAMEMAX ];

	strcpy( szTemp, grgszTextureName[ i ] );
	chTemp = grgchTextureType[ i ];
	
	strcpy( grgszTextureName[ i ], grgszTextureName[ j ] );
	grgchTextureType[ i ] = grgchTextureType[ j ];

	strcpy( grgszTextureName[ j ], szTemp );
	grgchTextureType[ j ] = chTemp;
}

void PM_SortTextures( void )
{
	// Bubble sort, yuck, but this only occurs at startup and it's only 512 elements...
	//
	int i, j;

	for ( i = 0 ; i < gcTextures; i++ )
	{
		for ( j = i + 1; j < gcTextures; j++ )
		{
			if ( stricmp( grgszTextureName[ i ], grgszTextureName[ j ] ) > 0 )
			{
				// Swap
				//
				PM_SwapTextures( i, j );
			}
		}
	}
}

void PM_InitTextureTypes()
{
	char buffer[512];
	int i, j;
	byte *pMemFile;
	int fileSize, filePos;
	static qboolean bTextureTypeInit = false;

	if ( bTextureTypeInit )
		return;

	memset(&(grgszTextureName[0][0]), 0, CTEXTURESMAX * CBTEXTURENAMEMAX);
	memset(grgchTextureType, 0, CTEXTURESMAX);

	gcTextures = 0;
	memset(buffer, 0, 512);

	fileSize = pmove->COM_FileSize( "sound/materials.txt" );
	pMemFile = pmove->COM_LoadFile( "sound/materials.txt", 5, NULL );
	if ( !pMemFile )
		return;

	filePos = 0;
	// for each line in the file...
	while ( pmove->memfgets( pMemFile, fileSize, &filePos, buffer, 511 ) != NULL && (gcTextures < CTEXTURESMAX) )
	{
		// skip whitespace
		i = 0;
		while(buffer[i] && isspace(buffer[i]))
			i++;
		
		if (!buffer[i])
			continue;

		// skip comment lines
		if (buffer[i] == '/' || !isalpha(buffer[i]))
			continue;

		// get texture type
		grgchTextureType[gcTextures] = toupper(buffer[i++]);

		// skip whitespace
		while(buffer[i] && isspace(buffer[i]))
			i++;
		
		if (!buffer[i])
			continue;

		// get sentence name
		j = i;
		while (buffer[j] && !isspace(buffer[j]))
			j++;

		if (!buffer[j])
			continue;

		// null-terminate name and save in sentences array
		j = min (j, CBTEXTURENAMEMAX-1+i);
		buffer[j] = 0;
		strcpy(&(grgszTextureName[gcTextures++][0]), &(buffer[i]));
	}

	// Must use engine to free since we are in a .dll
	pmove->COM_FreeFile ( pMemFile );

	PM_SortTextures();

	bTextureTypeInit = true;
}

char PM_FindTextureType( char *name )
{
	int left, right, pivot;
	int val;

	assert( pm_shared_initialized );

	left = 0;
	right = gcTextures - 1;

	while ( left <= right )
	{
		pivot = ( left + right ) / 2;

		val = strnicmp( name, grgszTextureName[ pivot ], CBTEXTURENAMEMAX-1 );
		if ( val == 0 )
		{
			return grgchTextureType[ pivot ];
		}
		else if ( val > 0 )
		{
			left = pivot + 1;
		}
		else if ( val < 0 )
		{
			right = pivot - 1;
		}
	}

	return CHAR_TEX_CONCRETE;
}


void PM_PlayStepSound_LadderAlpha(int playerLadderMovement) {
	int irand;
	float fvol;
	//MODDD - new.
	//int playerLadderMovement;
	//MODDD - TODO.  Make this per player instead!
	//static int alreadyPassedLadderCheck = 0;


	// NOTE - 'playerLadderMovement' is the old name of CVar 'cl_ladder_choice'.
	//playerLadderMovement = atoi(pmove->PM_Info_ValueForKey(pmove->physinfo, "plm"));

	//MODDD - needs to be in player.cpp instead for playerLadderMovement other than 0,
	// because the pev->punchangle has 
	//if (step == STEP_LADDER && playerLadderMovement != 0) {
		// If playerLadderMovement is anything but 0, we use player.cpp to play ladder sounds instead.
		// CHANGED.  Do it here now, turns out there is a way.  Still special logic done differently from
		// usual step logic below then, so skip after this area.

		int filterediuser4 = pmove->iuser4 & ~(FLAG_JUMPED | FLAG_RESET_RECEIVED | FLAG_CYCLE_PASSED);
		float ladderCycleMulti = atof(pmove->PM_Info_ValueForKey(pmove->physinfo, "lcm"));

		// this means, play one of the sounds & punch.
		// OLD CONDITION FOR player.cpp.   Rely on iuser4's FLAG_CYCLE_PASSED instead.
		//if (filterediuser4 > LADDER_CYCLE_BASE* ladderCycleMulti && !alreadyPassedLadderCheck) {
		if (pmove->iuser4 & FLAG_CYCLE_PASSED) {
			int iStepLeftFrame = pmove->iStepLeft;

			pmove->iuser4 &= ~FLAG_CYCLE_PASSED;  // and don't play me again without a reason

			// alternate.
			pmove->iStepLeft = !pmove->iStepLeft;

			// can't do this again until another frame passes that recognizes "filterediuser4" was below the threshold at some point before passing it again.
			// NO NEED ANYMORE
			//alreadyPassedLadderCheck = TRUE;

			// not possible to reach here for playerLadderMovement of 0, normal step logic below handles that.


			if (playerLadderMovement == 1 || playerLadderMovement == 3) {

				// little higher
				//fvol = 0.35f;
				fvol = 0.60f;
				if (pmove->flags & FL_DUCKING) {
					// little harsh, half is plenty
					//flvol *= 0.35f;  //again.
					fvol *= 0.5f;
				}

				irand = pmove->RandomLong(0, 1) + (iStepLeftFrame * 2);

				// play retail's sounds.
				// why did I use CHAN_VOICE for the player.cpp version?
				switch (irand) {
					// right foot
					case 0:	pmove->PM_PlaySound(CHAN_BODY, "player/pl_ladder1.wav", fvol, ATTN_NORM, 0, PITCH_NORM); break;
					case 1:	pmove->PM_PlaySound(CHAN_BODY, "player/pl_ladder3.wav", fvol, ATTN_NORM, 0, PITCH_NORM); break;
					// left foot
					case 2:	pmove->PM_PlaySound(CHAN_BODY, "player/pl_ladder2.wav", fvol, ATTN_NORM, 0, PITCH_NORM); break;
					case 3:	pmove->PM_PlaySound(CHAN_BODY, "player/pl_ladder4.wav", fvol, ATTN_NORM, 0, PITCH_NORM); break;
				}
			}

			if (playerLadderMovement == 2 || playerLadderMovement == 3) {
				// play a random pain sound, don't factor in whether this is the right or left step (as far as I know)

				// not alternating random pairs like most other places, but verify if that was the case for these.
				// Even if it was, I don't know which two two pain sounds were alternating for each left and right.
				irand = pmove->RandomLong(0, 3);

				// And override fvol!  They're pain sounds unrelated to foot-movement (speed).
				fvol = 1.0;

				// Should this use CHAN_VOICE?  They are 'pain' sounds but any other step sounds use CHAN_BODY.
				// Keeping it VOICE for now.
				// Well I'll be damned!  sentences work from pm_shared.  Still seems a little odd as nowhere else does this,
				// maybe for a reason...
				/*
				switch (irand) {
				case 0:	pmove->PM_PlaySound(CHAN_VOICE, "!player_pl_pain2", fvol, ATTN_NORM, 0, PITCH_NORM); break;
				case 1:	pmove->PM_PlaySound(CHAN_VOICE, "!player_pl_pain4", fvol, ATTN_NORM, 0, PITCH_NORM); break;
				case 2:	pmove->PM_PlaySound(CHAN_VOICE, "!player_pl_pain5", fvol, ATTN_NORM, 0, PITCH_NORM); break;
				case 3:	pmove->PM_PlaySound(CHAN_VOICE, "!player_pl_pain6", fvol, ATTN_NORM, 0, PITCH_NORM); break;
				}//END OF switch(rndSound)
				*/

				switch (irand) {
					case 0:	pmove->PM_PlaySound(CHAN_VOICE, "player/pl_pain2.wav", fvol, ATTN_NORM, 0, PITCH_NORM); break;
					case 1:	pmove->PM_PlaySound(CHAN_VOICE, "player/pl_pain4.wav", fvol, ATTN_NORM, 0, PITCH_NORM); break;
					case 2:	pmove->PM_PlaySound(CHAN_VOICE, "player/pl_pain5.wav", fvol, ATTN_NORM, 0, PITCH_NORM); break;
					case 3:	pmove->PM_PlaySound(CHAN_VOICE, "player/pl_pain6.wav", fvol, ATTN_NORM, 0, PITCH_NORM); break;
				}//END OF switch(rndSound)
			}

			//#'s 1 and 2 will give the view punch.  And the new 3 then.
			// So anything that makes it here (not 0).
			//if (playerLadderMovement == 1 || playerLadderMovement == 2) {
			if (iStepLeftFrame == 0) {
				pmove->punchangle[2] = 5.6;
			}
			else {
				pmove->punchangle[2] = -5.6;
			}
			//}
			// iStepLeft is already alternated above
		}//END OF cycle-check
		//MODDD - area is unnecessary after the move from player.cpp
		//else {
		//	if (filterediuser4 < LADDER_CYCLE_BASE * ladderCycleMulti) {
		//		// reset!
		//		alreadyPassedLadderCheck = FALSE;
		//	}
		//}

		//return;
	//}//END OF special ladder movement check


}//PM_PlayStepSound_LadderAlpha


void PM_PlayStepSound( int step, float fvol )
{
	static int iSkipStep = 0;
	int irand;
	vec3_t hvel;
	int iStepLeftFrame = pmove->iStepLeft;


	//MODDD - use the old version of iStepLeft instead, want to start with choice #0, right?
	// And doing the change after the 'pmove->runfuncs' check below, why alternate if it won't
	// be used this frame?
	if ( !pmove->runfuncs )
	{
		return;
	}

	// FIXME mp_footsteps needs to be a movevar
	//MODDD - wait.  But...  isn't 'pmove->movevars->footsteps' "mp_footsteps" as a move-var then?
	// Change mp_footsteps, and movevars->footsteps reflects the change.  I guess that comment
	// was out of date even as-is?
	if (pmove->multiplayer && !pmove->movevars->footsteps) {
		return;
	}

	VectorCopy_f( pmove->velocity, hvel );
	hvel[2] = 0.0;

	//MODDD - why was this block even a thing?  Now with pl_jump sounds introduced elsewhere,
	// those get played regardless of this section anyway.
	// To be consistent, those sounds also shouldn't be played if this gets blocked (this would block
	// the jump sounds caused by doubling step-sound on jumping, which still happens).
	// When on, and without pl_jump sounds being played, this caused jumping in place to be silent.
	//if (pmove->multiplayer && (!g_onladder && Length(hvel) <= 220)) {
 	//	return;
	//}


	// alternate.
	pmove->iStepLeft = !pmove->iStepLeft;
	
	irand = pmove->RandomLong(0, 1) + (iStepLeftFrame * 2);

	// irand - 0,1 for right foot, 2,3 for left foot
	// used to alternate left and right foot
	// FIXME, move to player state

	switch (step)
	{
	default:
	case STEP_CONCRETE:
		switch (irand)
		{
		// right foot
		case 0:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_step1.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 1:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_step3.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		// left foot
		case 2:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_step2.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 3:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_step4.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		}
		break;
	case STEP_METAL:
		switch(irand)
		{
		// right foot
		case 0:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_metal1.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 1:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_metal3.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		// left foot
		case 2:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_metal2.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 3:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_metal4.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		}
		break;
	case STEP_DIRT:
		switch(irand)
		{
		// right foot
		case 0:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_dirt1.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 1:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_dirt3.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		// left foot
		case 2:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_dirt2.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 3:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_dirt4.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		}
		break;
	case STEP_VENT:
		switch(irand)
		{
		// right foot
		case 0:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_duct1.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 1:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_duct3.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		// left foot
		case 2:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_duct2.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 3:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_duct4.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		}
		break;
	case STEP_GRATE:
		switch(irand)
		{
		// right foot
		case 0:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_grate1.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 1:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_grate3.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		// left foot
		case 2:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_grate2.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 3:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_grate4.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		}
		break;
	case STEP_TILE:
		if ( !pmove->RandomLong(0,4) )
			irand = 4;
		switch(irand)
		{
		// right foot
		case 0:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_tile1.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 1:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_tile3.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		// left foot
		case 2:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_tile2.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 3:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_tile4.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 4: pmove->PM_PlaySound( CHAN_BODY, "player/pl_tile5.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		}
		break;
	case STEP_SLOSH:
		switch(irand)
		{
		// right foot
		case 0:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_slosh1.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 1:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_slosh3.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		// left foot
		case 2:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_slosh2.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 3:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_slosh4.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		}
		break;
	case STEP_WADE:
		if ( iSkipStep == 0 )
		{
			iSkipStep++;
			break;
		}

		if ( iSkipStep++ == 3 )
		{
			iSkipStep = 0;
		}

		switch (irand)
		{
		// right foot
		case 0:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade1.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 1:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade2.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		// left foot
		case 2:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade3.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		case 3:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade4.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		}
		break;
	case STEP_LADDER:
		switch(irand)
		{
			// right foot
			case 0:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_ladder1.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
			case 1:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_ladder3.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
			// left foot
			case 2:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_ladder2.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
			case 3:	pmove->PM_PlaySound( CHAN_BODY, "player/pl_ladder4.wav", fvol, ATTN_NORM, 0, PITCH_NORM );	break;
		}
		break;
	}
}	

int PM_MapTextureTypeStepType(char chTextureType)
{
	switch (chTextureType)
	{
		default:
		case CHAR_TEX_CONCRETE:	return STEP_CONCRETE;	
		case CHAR_TEX_METAL: return STEP_METAL;	
		case CHAR_TEX_DIRT: return STEP_DIRT;	
		case CHAR_TEX_VENT: return STEP_VENT;	
		case CHAR_TEX_GRATE: return STEP_GRATE;	
		case CHAR_TEX_TILE: return STEP_TILE;
		case CHAR_TEX_SLOSH: return STEP_SLOSH;
	}
}

/*
====================
PM_CatagorizeTextureType

Determine texture info for the texture we are standing on.
====================
*/
void PM_CatagorizeTextureType( void )
{
	vec3_t start, end;
	const char *pTextureName;

	VectorCopy_f( pmove->origin, start );
	VectorCopy_f( pmove->origin, end );

	Distance(pmove->origin, pmove->origin);

	// Straight down
	end[2] -= 64;

	// Fill in default values, just in case.
	pmove->sztexturename[0] = '\0';
	pmove->chtexturetype = CHAR_TEX_CONCRETE;

	pTextureName = pmove->PM_TraceTexture( pmove->onground, start, end );
	if ( !pTextureName )
		return;

	// strip leading '-0' or '+0~' or '{' or '!'
	if (*pTextureName == '-' || *pTextureName == '+')
		pTextureName += 2;

	if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
		pTextureName++;
	// '}}'
	
	strcpy( pmove->sztexturename, pTextureName);
	pmove->sztexturename[ CBTEXTURENAMEMAX - 1 ] = 0;
		
	// get texture type
	pmove->chtexturetype = PM_FindTextureType( pmove->sztexturename );	
}

void PM_UpdateStepSound( void )
{
	int fWalking;
	float fvol;
	vec3_t knee;
	vec3_t feet;
	vec3_t center;
	float height;
	float speed;
	float velrun;
	float velwalk;
	float flduck;
	int fLadder;
	int step;
	//MODDD - new
	int playerLadderMovement;



	// determine if we are on a ladder
	fLadder = (pmove->movetype == MOVETYPE_FLY);// IsOnLadder();




	if (fLadder) {
		// NOTE - 'playerLadderMovement' is the old name of CVar 'cl_ladder_choice'.
		playerLadderMovement = atoi(pmove->PM_Info_ValueForKey(pmove->physinfo, "plm"));

		// do a check.  Is the playerLadderMovement 1, 2, or 3?  If soo, use the LadderAlpha step-logic instead.
		// it ignores the pmove->flTimeStepSound requirement.  Skip the rest of this method.
		if (playerLadderMovement == 1 || playerLadderMovement == 2 || playerLadderMovement == 3) {
			PM_PlayStepSound_LadderAlpha(playerLadderMovement);
			return;
		}

	}//END OF FladderCheck




	if ( pmove->flTimeStepSound > 0 )
		return;

	if ( pmove->flags & FL_FROZEN )
		return;

	PM_CatagorizeTextureType();

	speed = Length( pmove->velocity );






	// UNDONE: need defined numbers for run, walk, crouch, crouch run velocities!!!!	
	if ( ( pmove->flags & FL_DUCKING) || fLadder )
	{
		velwalk = 60;		// These constants should be based on cl_movespeedkey * cl_forwardspeed somehow
		velrun = 80;		// UNDONE: Move walking to server
		flduck = 100;
	}
	else
	{
		velwalk = 120;
		velrun = 210;
		flduck = 0;
	}

	// If we're on a ladder or on the ground, and we're moving fast enough,
	//  play step sound.  Also, if pmove->flTimeStepSound is zero, get the new
	//  sound right away - we just started moving in new level.
	if ( (fLadder || ( pmove->onground != -1 ) ) &&
		( Length( pmove->velocity ) > 0.0 ) &&
		( speed >= velwalk || !pmove->flTimeStepSound ) )
	{
		fWalking = speed < velrun;		

		VectorCopy_f( pmove->origin, center );
		VectorCopy_f( pmove->origin, knee );
		VectorCopy_f( pmove->origin, feet );

		height = pmove->player_maxs[ pmove->usehull ][ 2 ] - pmove->player_mins[ pmove->usehull ][ 2 ];

		knee[2] = pmove->origin[2] - 0.3 * height;
		feet[2] = pmove->origin[2] - 0.5 * height;

		// find out what we're stepping in or on...
		if (fLadder)
		{
			step = STEP_LADDER;
			fvol = 0.35;
			pmove->flTimeStepSound = 350;
		}
		else if ( pmove->PM_PointContents ( knee, NULL ) == CONTENTS_WATER )
		{
			step = STEP_WADE;
			fvol = 0.65;
			pmove->flTimeStepSound = 600;
		}
		else if ( pmove->PM_PointContents ( feet, NULL ) == CONTENTS_WATER )
		{
			step = STEP_SLOSH;
			fvol = fWalking ? 0.2 : 0.5;
			pmove->flTimeStepSound = fWalking ? 400 : 300;		
		}
		else
		{
			// find texture under player, if different from current texture, 
			// get material type
			step = PM_MapTextureTypeStepType( pmove->chtexturetype );

			switch ( pmove->chtexturetype )
			{
			default:
			case CHAR_TEX_CONCRETE:						
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_METAL:	
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_DIRT:	
				fvol = fWalking ? 0.25 : 0.55;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_VENT:	
				fvol = fWalking ? 0.4 : 0.7;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_GRATE:
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_TILE:	
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_SLOSH:
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;
			}
		}
		
		pmove->flTimeStepSound += flduck; // slower step time if ducking



		// play the sound
		if ( pmove->flags & FL_DUCKING )
		{
			fvol *= DUCK_STEPVOLUME_MULTI;
		}

		PM_PlayStepSound( step, fvol );
	}
}

/*
================
PM_AddToTouched

Add's the trace result to touch list, if contact is not already in list.
================
*/
qboolean PM_AddToTouched(pmtrace_t tr, vec3_t impactvelocity)
{
	int i;

	for (i = 0; i < pmove->numtouch; i++)
	{
		if (pmove->touchindex[i].ent == tr.ent)
			break;
	}
	if (i != pmove->numtouch)  // Already in list.
		return false;

	VectorCopy_f( impactvelocity, tr.deltavelocity );

	if (pmove->numtouch >= MAX_PHYSENTS)
		pmove->Con_DPrintf("Too many entities were touched!\n");

	pmove->touchindex[pmove->numtouch++] = tr;
	return true;
}

/*
================
PM_CheckVelocity

See if the player has a bogus velocity value.
================
*/
void PM_CheckVelocity ()
{
	int	i;

//
// bound velocity
//
	for (i=0 ; i<3 ; i++)
	{
		// See if it's bogus.
		if (IS_NAN(pmove->velocity[i]))
		{
			pmove->Con_Printf ("PM  Got a NaN velocity %i\n", i);
			pmove->velocity[i] = 0;
		}
		if (IS_NAN(pmove->origin[i]))
		{
			pmove->Con_Printf ("PM  Got a NaN origin on %i\n", i);
			pmove->origin[i] = 0;
		}

		// Bound it.
		if (pmove->velocity[i] > pmove->movevars->maxvelocity) 
		{
			pmove->Con_DPrintf ("PM  Got a velocity too high on %i\n", i);
			pmove->velocity[i] = pmove->movevars->maxvelocity;
		}
		else if (pmove->velocity[i] < -pmove->movevars->maxvelocity)
		{
			pmove->Con_DPrintf ("PM  Got a velocity too low on %i\n", i);
			pmove->velocity[i] = -pmove->movevars->maxvelocity;
		}
	}
}

/*
==================
PM_ClipVelocity

Slide off of the impacting object
returns the blocked flags:
0x01 == floor
0x02 == step / wall
==================
*/
int PM_ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float backoff;
	float change;
	float angle;
	int	i, blocked;
	
	angle = normal[ 2 ];

	blocked = 0x00;            // Assume unblocked.
	if (angle > 0)      // If the plane that is blocking us has a positive z component, then assume it's a floor.
		blocked |= 0x01;		// 
	if (!angle)         // If the plane has no Z, it is vertical (wall/step)
		blocked |= 0x02;		// 
	
	// Determine how far along plane to slide based on incoming direction.
	// Scale by overbounce factor.
	backoff = DotProduct_f (in, normal) * overbounce;

	for (i=0 ; i<3 ; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
		// If out velocity is too small, zero it out.
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
	
	// Return blocking flags.
	return blocked;
}



/*
============
PM_AddGravity
============
*/
void PM_AddGravity ()
{
	float ent_gravity;

	if (pmove->gravity)
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0;


	//MODDD - involve ary_flGravityModMulti[pmove->player_index]
	ent_gravity *= ary_flGravityModMulti[pmove->player_index];

	// Add gravity incorrectly
	pmove->velocity[2] -= (ent_gravity * pmove->movevars->gravity * pmove->frametime );
	pmove->velocity[2] += pmove->basevelocity[2] * pmove->frametime;
	pmove->basevelocity[2] = 0;
	PM_CheckVelocity();
}

void PM_AddCorrectGravity ()
{
	float ent_gravity;

	if ( pmove->waterjumptime )
		return;

	if (pmove->gravity)
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0;

	//MODDD - involve ary_flGravityModMulti[pmove->player_index]
	ent_gravity *= ary_flGravityModMulti[pmove->player_index];

	// Add gravity so they'll be in the correct position during movement
	// yes, this 0.5 looks wrong, but it's not.  
	pmove->velocity[2] -= (ent_gravity * pmove->movevars->gravity * 0.5 * pmove->frametime );
	pmove->velocity[2] += pmove->basevelocity[2] * pmove->frametime;
	pmove->basevelocity[2] = 0;

	PM_CheckVelocity();
}


void PM_FixupGravityVelocity ()
{
	float ent_gravity;

	if ( pmove->waterjumptime )
		return;

	if (pmove->gravity)
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0;

	//MODDD - involve ary_flGravityModMulti[pmove->player_index]
	ent_gravity *= ary_flGravityModMulti[pmove->player_index];

	// Get the correct velocity for the end of the dt 
  	pmove->velocity[2] -= (ent_gravity * pmove->movevars->gravity * pmove->frametime * 0.5 );

	PM_CheckVelocity();
}




/*
============
PM_FlyMove

The basic solid body movement clip that slides along multiple planes
============
*/
int PM_FlyMove (void)
{
	int		bumpcount, numbumps;
	vec3_t		dir;
	float	d;
	int		numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity, original_velocity;
	vec3_t      new_velocity;
	int		i, j;
	pmtrace_t	trace;
	vec3_t		end;
	float	time_left, allFraction;
	int		blocked;
		
	numbumps  = 4;           // Bump up to four times
	
	blocked   = 0;           // Assume not blocked
	numplanes = 0;           //  and not sliding along any planes
	VectorCopy_f (pmove->velocity, original_velocity);  // Store original velocity
	VectorCopy_f (pmove->velocity, primal_velocity);
	
	allFraction = 0;
	time_left = pmove->frametime;   // Total time for this movement operation.

	for (bumpcount=0 ; bumpcount<numbumps ; bumpcount++)
	{
		if (!pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2])
			break;

		// Assume we can move all the way from the current origin to the
		//  end point.
		for (i=0 ; i<3 ; i++)
			end[i] = pmove->origin[i] + time_left * pmove->velocity[i];

		// See if we can make it from origin to end point.
		trace = pmove->PM_PlayerTrace (pmove->origin, end, PM_NORMAL, -1 );

		allFraction += trace.fraction;
		// If we started in a solid object, or we were in solid space
		//  the whole way, zero out our velocity and return that we
		//  are blocked by floor and wall.
		if (trace.allsolid)
		{	// entity is trapped in another solid
			VectorCopy_f (vec3_origin, pmove->velocity);
			//Con_DPrintf("Trapped 4\n");
			return 4;
		}

		// If we moved some portion of the total distance, then
		//  copy the end position into the pmove->origin and 
		//  zero the plane counter.
		if (trace.fraction > 0)
		{	// actually covered some distance
			VectorCopy_f (trace.endpos, pmove->origin);
			VectorCopy_f (pmove->velocity, original_velocity);
			numplanes = 0;
		}

		// If we covered the entire distance, we are done
		//  and can return.
		if (trace.fraction == 1)
			 break;		// moved the entire distance

		//if (!trace.ent)
		//	Sys_Error ("PM_PlayerTrace: !trace.ent");

		// Save entity that blocked us (since fraction was < 1.0)
		//  for contact
		// Add it if it's not already in the list!!!
		PM_AddToTouched(trace, pmove->velocity);

		// If the plane we hit has a high z component in the normal, then
		//  it's probably a floor
		if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1;		// floor
		}
		// If the plane has a zero z component in the normal, then it's a 
		//  step or wall
		if (!trace.plane.normal[2])
		{
			blocked |= 2;		// step / wall
			//Con_DPrintf("Blocked by %i\n", trace.ent);
		}

		// Reduce amount of pmove->frametime left by total time left * fraction
		//  that we covered.
		time_left -= time_left * trace.fraction;
		
		// Did we run out of planes to clip against?
		if (numplanes >= MAX_CLIP_PLANES)
		{	// this shouldn't happen
			//  Stop our movement if so.
			VectorCopy_f (vec3_origin, pmove->velocity);
			//Con_DPrintf("Too many planes 4\n");

			break;
		}

		// Set up next clipping plane
		VectorCopy_f (trace.plane.normal, planes[numplanes]);
		numplanes++;
//

// modify original_velocity so it parallels all of the clip planes
//
		if ( pmove->movetype == MOVETYPE_WALK &&
			((pmove->onground == -1) || (pmove->friction != 1)) )	// relfect player velocity
		{
			for ( i = 0; i < numplanes; i++ )
			{
				if ( planes[i][2] > 0.7  )
				{// floor or slope
					PM_ClipVelocity( original_velocity, planes[i], new_velocity, 1 );
					VectorCopy_f( new_velocity, original_velocity );
				}
				else															
					PM_ClipVelocity( original_velocity, planes[i], new_velocity, 1.0 + pmove->movevars->bounce * (1-pmove->friction) );
			}

			VectorCopy_f( new_velocity, pmove->velocity );
			VectorCopy_f( new_velocity, original_velocity );
		}
		else
		{
			for (i=0 ; i<numplanes ; i++)
			{
				PM_ClipVelocity (
					original_velocity,
					planes[i],
					pmove->velocity,
					1);
				for (j=0 ; j<numplanes ; j++)
					if (j != i)
					{
						// Are we now moving against this plane?
						if (DotProduct_f (pmove->velocity, planes[j]) < 0)
							break;	// not ok
					}
				if (j == numplanes)  // Didn't have to clip, so we're ok
					break;
			}
			
			// Did we go all the way through plane set
			if (i != numplanes)
			{	// go along this plane
				// pmove->velocity is set in clipping call, no need to set again.
				;  
			}
			else
			{	// go along the crease
				if (numplanes != 2)
				{
					//Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
					VectorCopy_f (vec3_origin, pmove->velocity);
					//Con_DPrintf("Trapped 4\n");

					break;
				}
				CrossProduct (planes[0], planes[1], dir);
				d = DotProduct_f (dir, pmove->velocity);
				VectorScale (dir, d, pmove->velocity );
			}

	//
	// if original velocity is against the original velocity, stop dead
	// to avoid tiny occilations in sloping corners
	//
			if (DotProduct_f (pmove->velocity, primal_velocity) <= 0)
			{
				//Con_DPrintf("Back\n");
				VectorCopy_f (vec3_origin, pmove->velocity);
				break;
			}
		}
	}

	if ( allFraction == 0 )
	{
		VectorCopy_f (vec3_origin, pmove->velocity);
		//Con_DPrintf( "Don't stick\n" );
	}

	return blocked;
}




/*
//MODDD - NEW.  A simple version that says:  is there anything in front of me, yes or no?
// A lot of PM_FlyMove isn't needed for testing whether the space in front of the player is
// valid for going down as stairs or a downward incline.
int PM_FlyMoveTest(void) {


	if (!pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2])
		break;

	// Assume we can move all the way from the current origin to the
	//  end point.
	for (i = 0; i < 3; i++)
		end[i] = pmove->origin[i] + time_left * pmove->velocity[i];

	// See if we can make it from origin to end point.
	trace = pmove->PM_PlayerTrace(pmove->origin, end, PM_NORMAL, -1);

}//PM_FlyMoveTest
*/




/*
==============
PM_Accelerate
==============
*/
void PM_Accelerate (vec3_t wishdir, float wishspeed, float accel)
{
	int		i;
	float	addspeed, accelspeed, currentspeed;

	// Dead player's don't accelerate
	if (pmove->dead)
		return;

	// If waterjumping, don't accelerate
	if (pmove->waterjumptime)
		return;

	// See if we are changing direction a bit
	currentspeed = DotProduct_f (pmove->velocity, wishdir);

	// Reduce wishspeed by the amount of veer.
	addspeed = wishspeed - currentspeed;

	// If not going to add any speed, done.
	if (addspeed <= 0)
		return;

	// Determine amount of accleration.
	accelspeed = accel * pmove->frametime * wishspeed * pmove->friction;
	
	
	

	// Cap at addspeed
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	// Adjust velocity.
	for (i=0 ; i<3 ; i++)
	{
		pmove->velocity[i] += accelspeed * wishdir[i];	
	}


}


//MODDD - NEW.  Latter portion of PM_WalkMove here for re-use.
// Checks for inclines/stairs upward and downward (NEW).
void PM_InclineCheck(void) {

	int i;
	int clip;


	vec3_t dest, start;
	vec3_t original, originalvel;
	vec3_t down, downvel;
	float downdist, updist;

	//MODDD - original trace "trace" given a... better name.
	//        The original was made to see if we run into an incline going upwards (and counts for bumping against something way too tall/steep like a wall instead)
	pmtrace_t traceInclineDetection_Forward;
	//MODDD 
	pmtrace_t traceInclineDetection_Down;
	pmtrace_t traceInclineDetection_ForwardDown;

	float oldZ;


	// Try sliding forward both on ground and up 16 pixels
	//  take the move that goes farthest
	VectorCopy_f(pmove->origin, original);       // Save out original pos &
	VectorCopy_f(pmove->velocity, originalvel);  //  velocity.


	// Slide move, simple version
	// ...maybe?
	//clip = PM_FlyMoveTest();
	clip = PM_FlyMove();









	// Copy the results out
	VectorCopy_f(pmove->origin, down);
	VectorCopy_f(pmove->velocity, downvel);



	//MODDD - new block!   Can we detect a floor below a little further out?
	// This is the main support for downward stairs / inclines.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//if (!skipInclineChecks) {
		// old logic
		////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////
		/*
		float flatVel = sqrt(pmove->velocity[0] * pmove->velocity[0] + pmove->velocity[1] * pmove->velocity[1]);

		vec3_t destDown;
		//vec3_t tempVec = { 0, 0, -(flatVel * pmove->frametime*50 * 0.3) };
		vec3_t tempVec = { 0, 0, -pmove->movevars->stepsize };

		VectorAdd_f(dest, tempVec, destDown);


		pmtrace_t traceInclineDetection_ForwardDown = pmove->PM_PlayerTrace(dest, destDown, PM_NORMAL, -1);

		if (traceInclineDetection_ForwardDown.fraction == -1) {
			// oh piffle.
		}
		else {
			float normalZ = traceInclineDetection_ForwardDown.plane.normal[2];

			if (normalZ < 0.99) {
				int x = 4; //breakpoint
			}


			if (
				!traceInclineDetection_ForwardDown.startsolid &&
				!traceInclineDetection_ForwardDown.allsolid &&
				normalZ >= 0.8 //&& normalZ < 0.99
				) {


				//got a hit?  ok.
				VectorCopy_f(traceInclineDetection_ForwardDown.endpos, pmove->origin);
				return;
			}

		}
		*/
		/*
		// Start out up one stair height
		VectorCopy_f(pmove->origin, dest);
		dest[2] -= pmove->movevars->stepsize;

		traceInclineDetection_Forward = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);
		// If we started okay and made it part of the way at least,
		//  copy the results to the movement start position and then
		//  run another move try.
		if (!traceInclineDetection_Forward.startsolid && !traceInclineDetection_Forward.allsolid)
		{
			VectorCopy_f(traceInclineDetection_Forward.endpos, pmove->origin);
		}
		*/
		////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////


		/*
		VectorCopy_f(pmove->origin, original);       // Save out original pos &
		VectorCopy_f(pmove->velocity, originalvel);  //  velocity.

		// slide move the rest of the way.
		clip = PM_FlyMove();

		// Now try going back <up> <no down..?> from the end point
		//  press down the stepheight
		*/

		//pmove->basevelocity[2] > 0 || (pmove->flags & FL_BASEVELOCITY)
		// nevermind basevelocity checks, velocity is fine.
		if(pmove->velocity[2] > 0 ){
			// If being pushed up, cancel any downward incline/stair checks.
		}else{

			VectorCopy_f(pmove->origin, dest);
			dest[2] -= pmove->movevars->stepsize;

			traceInclineDetection_ForwardDown = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);


			/*
			VectorCopy_f(traceInclineDetection_Forward.endpos, dest);
			dest[2] -= pmove->movevars->stepsize;
			traceInclineDetection_ForwardDown = pmove->PM_PlayerTrace(traceInclineDetection_Forward.endpos, dest, PM_NORMAL, -1);
			*/




			// If we are not on the ground any more then
			//  use the original movement attempt

			// ????????????????????????????????????????????????????????????????
			// 'normal[2] < 0.7' tends to mean that's a floor by the way.
			//if (traceInclineDetection_Forward.plane.normal[2] < 0.7)
			//	goto usedown;

			//MODDD NOTICE - "goto usedown" above is getting called if the plane we hit can't be walked up, like an incline way to steep
			// or flat-out a wall in the way.  So skipping to below instead of trying to skip to the top of it (below up to the downdist > updist comparison)
			// That is for forcing the origin to the top of an incline I look to be trying to go up.



			// If the trace ended up in empty space, copy the end
			//  over to the origin.
			//MODDD NOTE - this might be what places us exactly at the ramp.
			// And if the fraction is way too small, that's a sign we're either going along flat-ground or an upward incline/stairs.
			// Strange it isn't an issue more often than it is, but it can happen in the stairs near the start of retail t0a0.bsp
			// (Not the stairs up to the suit,  the stairs down.  Try going back up after going down those with a _ForwardDown fraction
			// of > 0 allowed).  0.01 seems to fix it though, mighteven allow smaller.
			if (
				//clip == 0 &&
				pmove->origin[2] <= original[2] &&
				traceInclineDetection_ForwardDown.fraction > 0.01 && traceInclineDetection_ForwardDown.fraction < 1.0 &&
				!traceInclineDetection_ForwardDown.startsolid && !traceInclineDetection_ForwardDown.allsolid
				//traceInclineDetection_ForwardDown.endpos[2] < pmove->origin[2]
				)
			{
				// Is a downward incline or stairs?  Snap to the ground then!  No further checks needed.
				// Already even have 'downvel' here for pmove->velocity.  Did not touch the velocity so it stays as it is.

				// TEST!
				if (1) {
					//!!! SETS g_interp_z OF view.cpp
					pmove->vuser4[0] = traceInclineDetection_ForwardDown.endpos[2] - pmove->origin[2];

					VectorCopy_f(traceInclineDetection_ForwardDown.endpos, pmove->origin);
					return;
				}
			}
			else {
				// Reset original values.  (no, done below already)
				/*
				VectorCopy_f(original, pmove->origin);
				VectorCopy_f(originalvel, pmove->velocity);
				*/
				// The rest of the method proceeds with the rest of the upward incline/stairs check, which may
				// find that or deem it a blocking wall.
			}

		}//END OF no upward velocity check

	//}//skipInclineChecks
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////




	// Reset original values.
	VectorCopy_f(original, pmove->origin);
	VectorCopy_f(originalvel, pmove->velocity);

	//MODDD - good point to record this?
	oldZ = pmove->origin[2];

	// Start out up one stair height
	VectorCopy_f(pmove->origin, dest);
	dest[2] += pmove->movevars->stepsize;

	traceInclineDetection_Forward = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);
	// If we started okay and made it part of the way at least,
	//  copy the results to the movement start position and then
	//  run another move try.
	if (!traceInclineDetection_Forward.startsolid && !traceInclineDetection_Forward.allsolid)
	{
		VectorCopy_f(traceInclineDetection_Forward.endpos, pmove->origin);
	}

	// slide move the rest of the way.
	clip = PM_FlyMove();

	// Now try going back down from the end point
	//  press down the stepheight
	VectorCopy_f(pmove->origin, dest);
	dest[2] -= pmove->movevars->stepsize;

	traceInclineDetection_Forward = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);

	//easyForcePrintLine("congratulations Z:%.2f fr:%.2f ::: ss:%d as:%d io:%d", traceInclineDetection_Forward.endpos[2], traceInclineDetection_Forward.fraction, traceInclineDetection_Forward.startsolid, traceInclineDetection_Forward.allsolid, traceInclineDetection_Forward.inopen);

	// If we are not on the ground any more then
	//  use the original movement attempt
	if (traceInclineDetection_Forward.plane.normal[2] < 0.7){
		goto usedown;
	}

	//MODDD NOTICE - "goto usedown" above is getting called if the plane we hit can't be walked up, like an incline way to steep
	// or flat-out a wall in the way.  So skipping to below instead of trying to skip to the top of it (below up to the downdist > updist comparison)
	// That is for forcing the origin to the top of an incline I look to be trying to go up.


	// If the trace ended up in empty space, copy the end
	//  over to the origin.
	//MODDD NOTE - this might be what places us exactly at the ramp.
	if (!traceInclineDetection_Forward.startsolid && !traceInclineDetection_Forward.allsolid)
	{
		VectorCopy_f(traceInclineDetection_Forward.endpos, pmove->origin);
	}
	// Copy this origion to up.
	//MODDD NOTE - pmove->up being set below might be to help move up the incline more next frame? unsure.
	//             Could be just to affect the updist calculation below too.
	VectorCopy_f(pmove->origin, pmove->up);

	// decide which one went farther
	downdist = (down[0] - original[0]) * (down[0] - original[0])
		+ (down[1] - original[1]) * (down[1] - original[1]);
	updist = (pmove->up[0] - original[0]) * (pmove->up[0] - original[0])
		+ (pmove->up[1] - original[1]) * (pmove->up[1] - original[1]);



	if (downdist > updist)
	{
	usedown:
		//MODDD NOTE - I get picked for running into walls.  Or by the "usedown" skip above.
		VectorCopy_f(down, pmove->origin);
		VectorCopy_f(downvel, pmove->velocity);
	}
	else { // copy z value from slide move
	 //MODDD NOTE - I get picked for moving into ramps to slide up them instead.
		pmove->velocity[2] = downvel[2];
	}

	// TEST!
	//!!! SETS g_interp_z OF view.cpp
	// wait.  why not pmove->origin?
	//pmove->vuser4[0] = traceInclineDetection_Forward.endpos[2] - oldZ;
	pmove->vuser4[0] = pmove->origin[2] - oldZ;

	//easyForcePrintLine("I AM THE unhappy fellow %.2f - %.2f", traceInclineDetection_Forward.endpos[2], oldZ);


	/*
	usedown:
	VectorCopy_f (down   , pmove->origin);
	VectorCopy_f (downvel, pmove->velocity);

	pmove->velocity[2] = downvel[2];
	*/


}//PM_InclineCheck



void PM_InclineAirCheck(vec3_t vecOldVel) {
	int i;
	int		clip;

	vec3_t dest, start;
	vec3_t original, originalvel;
	vec3_t down, downvel;
	float downdist, updist;

	//MODDD - original trace "trace" given a... better name.
	//        The original was made to see if we run into an incline going upwards (and counts for bumping against something way too tall/steep like a wall instead)
	pmtrace_t traceInclineDetection_Forward;
	//MODDD 
	pmtrace_t traceInclineDetection_Down;
	float oldZ;


	/*
	// UNUSED PARAMETER, this no longer happens.
	if (recentClip == 0) {
		// oh wait, nothing in the way?  What're we doing here then.
		return;
	}
	*/



	// Try sliding forward both on ground and up 16 pixels
	//  take the move that goes farthest
    VectorCopy_f(pmove->origin, original);       // Save out original pos &
	VectorCopy_f(pmove->velocity, originalvel);  //  velocity.


	// Slide move, simple version
	// ...maybe?
	//clip = PM_FlyMoveTest();
	clip = PM_FlyMove();


	if (clip == 0) {
		// aha,  NOW we're done.
		return;
	}


	// Copy the results out
	VectorCopy_f(pmove->origin, down);
	VectorCopy_f(pmove->velocity, downvel);


	// .............




	// Reset original values.
	VectorCopy_f(original, pmove->origin);
	VectorCopy_f(originalvel, pmove->velocity);

	//MODDD - good point to record this?
	oldZ = pmove->origin[2];

	// Start out up one stair height
	VectorCopy_f(pmove->origin, dest);
	dest[2] += pmove->movevars->stepsize;

	traceInclineDetection_Forward = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);
	// If we started okay and made it part of the way at least,
	//  copy the results to the movement start position and then
	//  run another move try.
	if (!traceInclineDetection_Forward.startsolid && !traceInclineDetection_Forward.allsolid)
	{
		VectorCopy_f(traceInclineDetection_Forward.endpos, pmove->origin);
	}

	// slide move the rest of the way.
	clip = PM_FlyMove();

	// Now try going back down from the end point
	//  press down the stepheight
	VectorCopy_f(pmove->origin, dest);
	dest[2] -= pmove->movevars->stepsize;

	traceInclineDetection_Forward = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);

	// If we are not on the ground any more then
	//  use the original movement attempt
	//MODDD - DISABLED!   or not?
	if (traceInclineDetection_Forward.plane.normal[2] < 0.7)
		goto usedown;

	//MODDD NOTICE - "goto usedown" above is getting called if the plane we hit can't be walked up, like an incline way to steep
	// or flat-out a wall in the way.  So skipping to below instead of trying to skip to the top of it (below up to the downdist > updist comparison)
	// That is for forcing the origin to the top of an incline I look to be trying to go up.


	// If the trace ended up in empty space, copy the end
	//  over to the origin.
	//MODDD NOTE - this might be what places us exactly at the ramp.
	if (!traceInclineDetection_Forward.startsolid && !traceInclineDetection_Forward.allsolid)
	{

		VectorCopy_f(traceInclineDetection_Forward.endpos, pmove->origin);
		if (pmove->velocity[2] > 0) {
			//easyForcePrintLine("B2");
			VectorCopy_f(vecOldVel, pmove->velocity);
		}
	}



	// Copy this origion to up.
	//MODDD NOTE - pmove->up being set below might be to help move up the incline more next frame? unsure.
	//             Could be just to affect the updist calculation below too.
	VectorCopy_f(pmove->origin, pmove->up);

	// decide which one went farther
	downdist = (down[0] - original[0]) * (down[0] - original[0])
		+ (down[1] - original[1]) * (down[1] - original[1]);
	updist = (pmove->up[0] - original[0]) * (pmove->up[0] - original[0])
		+ (pmove->up[1] - original[1]) * (pmove->up[1] - original[1]);




	if (downdist > updist)
	{
	usedown:
		//easyForcePrintLine("A");
		//MODDD NOTE - I get picked for running into walls.  Or by the "usedown" skip above.
		VectorCopy_f(down, pmove->origin);
		VectorCopy_f(downvel, pmove->velocity);

		//pmove->onground = traceInclineDetection_Forward.ent;
		//pmove->velocity[2] = 0;


		//MODDD - restore instead?
		//VectorCopy_f(original, pmove->origin);
		//VectorCopy_f(originalvel, pmove->velocity);

	}
	else { // copy z value from slide move

		//easyForcePrintLine("B");
	 //MODDD NOTE - I get picked for moving into ramps to slide up them instead.
		if (pmove->velocity[2] <= 0) {
			pmove->velocity[2] = downvel[2];
		}

		//MODDD - restore instead?
		//VectorCopy_f(originalvel, pmove->velocity);
	}

	// TEST!... ehhhh.  not this time.
	//pmove->vuser4[0] = traceInclineDetection_Forward.endpos[2] - oldZ;




	//now moov.
	//PM_FlyMove();


	/*
	usedown:
	VectorCopy_f (down   , pmove->origin);
	VectorCopy_f (downvel, pmove->velocity);

	pmove->velocity[2] = downvel[2];
	*/
}//PM_InclineAirCheck









/*
=====================
PM_WalkMove

Only used by players.  Moves along the ground when player is a MOVETYPE_WALK.
======================
*/
void PM_WalkMove ()
{
	int i;
	vec3_t		wishvel;
	float       spd;
	float	fmove, smove;
	vec3_t		wishdir;
	float	wishspeed;
	vec3_t dest, start;
	float normalSpeedMult;
	int		oldonground;
	BOOL skipInclineChecks = FALSE;
	pmtrace_t traceInclineDetection_Forward;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;
	
	
	//return;
	// Zero out z components of movement vectors
	pmove->forward[2] = 0;
	pmove->right[2]   = 0;
	
	VectorNormalize (pmove->forward);  // Normalize remainder of vectors.
	VectorNormalize (pmove->right);    // 

	//MODDD - allow influence from a CVar.
	normalSpeedMult = getSafeSqureRoot(atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "nsm" ) ));
	


	/*
	// more printout tests
#ifdef CLIENT_DLL
	pmove->Con_Printf( "CL: Con_Printf : %i %d %g %s end\n", 12, 12, 12.5f, "test");
	pmove->Con_DPrintf("CL: Con_DPrintf: %i %d %g %s end\n", 12, 12, 12.5f, "test");
#else
	pmove->Con_Printf( "SV: Con_Printf : %i %d %g %s end\n", 12, 12, 12.5f, "test");
	pmove->Con_DPrintf("SV: Con_DPrintf: %i %d %g %s end\n", 12, 12, 12.5f, "test");
#endif
	*/



	for (i=0 ; i<2 ; i++)       // Determine x and y parts of velocity
		wishvel[i] = pmove->forward[i]*fmove*normalSpeedMult + pmove->right[i]*smove*normalSpeedMult;
	

	
	/*
		if(pmove->server){
			pmove->Con_Printf("Stes1 %f\n", wishvel[0]);
			pmove->Con_Printf("Stes2 %f\n", wishvel[1]);

		}else{
			pmove->Con_Printf("Ctes1 %f\n", wishvel[0]);
			pmove->Con_Printf("Ctes2 %f\n", wishvel[1]);
		}
		*/


	wishvel[2] = 0;             // Zero out z part of velocity

	VectorCopy_f (wishvel, wishdir);   // Determine maginitude of speed of move
	wishspeed = VectorNormalize(wishdir);

//
// Clamp to server defined max speed
//
	//NOTE: only bother to cap the speed if the "normalSpeedMult" is 1.
	//If it is other than 1, the player does not care about this.
	if (normalSpeedMult == 1 && wishspeed > pmove->maxspeed)
	{
		VectorScale (wishvel, pmove->maxspeed/wishspeed, wishvel);
		wishspeed = pmove->maxspeed;
		//
	}


	

	// Set pmove velocity
	pmove->velocity[2] = 0;
	PM_Accelerate (wishdir, wishspeed, pmove->movevars->accelerate);
	pmove->velocity[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd_f (pmove->velocity, pmove->basevelocity, pmove->velocity );

	spd = Length( pmove->velocity );

	if (spd < 1.0f)
	{
		VectorClear_f( pmove->velocity );
		return;
	}

	// If we are not moving, do nothing
	//if (!pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2])
	//	return;

	//return;

	oldonground = pmove->onground;



	//MODDD - 'few checks placed further above'
	// placed here so that whether to check for going up inclines or going down inclines can
	// be skipped if either of these checks says to.
	if (oldonground == -1 &&   // Don't walk up stairs if not on ground.
		pmove->waterlevel == 0) {
		skipInclineChecks = TRUE;
	}

	if (pmove->waterjumptime) {         // If we are jumping out of water, don't do anything more.
		skipInclineChecks = TRUE;
	}






	//MODDD - few checks placed further above instead.
	// replaced with this now.
	if (skipInclineChecks) {

		//MODDD - section moved here, was above the 'oldonground == -1' check above here.
		// No point in setting dest to be ignored if skipInclineChecks is FALSE.
		// first try just moving to the destination	
		dest[0] = pmove->origin[0] + pmove->velocity[0] * pmove->frametime;
		dest[1] = pmove->origin[1] + pmove->velocity[1] * pmove->frametime;
		dest[2] = pmove->origin[2];


		//MODDD  block moved inside this 'skipIncludeChecks' area,
		// as with downward incline/stairs detection it just gets in the way
		// if we plan on doing those checks later.  If not, this way works fine.
		/////////////////////////////////////////////////////////////////////////////////////////////
		// first try moving directly to the next spot
		VectorCopy_f(dest, start);
		traceInclineDetection_Forward = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);
		// If we made it all the way, then copy trace end
		//  as new player position.
		//MODDD NOTE - fraction of 1 says nothing is in the way. So we're trusting we can move forwards that much.
		//             If this soon turns out to no longer be on the ground, we'll start falling by some other _MOVE method in here instead.
		if (traceInclineDetection_Forward.fraction == 1)
		{

			VectorCopy_f(traceInclineDetection_Forward.endpos, pmove->origin);
			return;
		}
		/////////////////////////////////////////////////////////////////////////////////////////////


		// don't proceed then, below is only for checking for warping to the top of stairs in a frame
		// (smooth movement against an upward incline).
		return;
	}//skipInclineChecks

	PM_InclineCheck();
}

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
void PM_Friction (void)
{
	float *vel;
	float speed, newspeed, control;
	float friction;
	float drop;
	vec3_t newvel;
	
	// If we are in water jump cycle, don't apply friction
	if (pmove->waterjumptime)
		return;

	// Get velocity
	vel = pmove->velocity;
	
	// Calculate speed
	speed = sqrt(vel[0]*vel[0] +vel[1]*vel[1] + vel[2]*vel[2]);
	
	// If too slow, return
	if (speed < 0.1f)
	{
		return;
	}

	drop = 0;

// apply ground friction
	if (pmove->onground != -1)  // On an entity that is the ground
	{
		vec3_t start, stop;
		pmtrace_t trace;

		start[0] = stop[0] = pmove->origin[0] + vel[0]/speed*16;
		start[1] = stop[1] = pmove->origin[1] + vel[1]/speed*16;
		start[2] = pmove->origin[2] + pmove->player_mins[pmove->usehull][2];
		stop[2] = start[2] - 34;

		trace = pmove->PM_PlayerTrace (start, stop, PM_NORMAL, -1 );

		if (trace.fraction == 1.0)
			friction = pmove->movevars->friction*pmove->movevars->edgefriction;
		else
			friction = pmove->movevars->friction;
		
		// Grab friction value.
		//friction = pmove->movevars->friction;      

		friction *= pmove->friction;  // player friction?

		// Bleed off some speed, but if we have less than the bleed
		//  threshhold, bleed the theshold amount.
		control = (speed < pmove->movevars->stopspeed) ?
			pmove->movevars->stopspeed : speed;
		// Add the amount to t'he drop amount.
		drop += control*friction*pmove->frametime;
	}

// apply water friction
//	if (pmove->waterlevel)
//		drop += speed * pmove->movevars->waterfriction * waterlevel * pmove->frametime;

// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;

	// Determine proportion of old speed we are using.
	newspeed /= speed;

	// Adjust velocity according to proportion.
	newvel[0] = vel[0] * newspeed;
	newvel[1] = vel[1] * newspeed;
	newvel[2] = vel[2] * newspeed;

	VectorCopy_f( newvel, pmove->velocity );
}

void PM_AirAccelerate (vec3_t wishdir, float wishspeed, float accel)
{
	int i;
	float addspeed, accelspeed, currentspeed, wishspd = wishspeed;
	
	int sv_player_midair_accel_val = 1;
	sv_player_midair_accel_val = atoi(pmove->PM_Info_ValueForKey(pmove->physinfo, "maa"));


	if (pmove->dead)
		return;
	if (pmove->waterjumptime)
		return;

	// Cap speed     - this was found commented out as-is
	//wishspd = VectorNormalize (pmove->wishveloc);
	


	// Determine veer amount
	currentspeed = DotProduct_f (pmove->velocity, wishdir);


	if(sv_player_midair_accel_val != 1){
		// retail cap
		if (wishspd > 30){
			wishspd = 30;
		}
		
	}else{
		for(i = 0; i < 2; i++){
			// how about some air resistance?  (apply even if not holding down any move keys)
			// Also only for moving faster than a typical jump allows (full speed + jump).
			// That way normal platforming can't be made at all harder by this.
			if(pmove->gravity > 0 && currentspeed > pmove->maxspeed){
				// less gravity = less air resistance (let's assume that means space or something).
				// (effect here is, half gravity = half air resistance.
				// Stopping faster while having more slowly descending jumps is kinda odd to notice.)
				pmove->velocity[i] = pmove->velocity[i] * 0.994 * pmove->gravity;
			}else{
				// no gravity, no effect.
				// "what?  Air resistance in space?"  Well there's non-negligible amounts of gravity in xen
				// to begin with so there.
			}
		}

		// greater cap, unrealistic to reach in the opposite direction in typical jumps though
		if (wishspd > 160){
			wishspd = 160;
		}

	}



	// See how much to add
	addspeed = wishspd - currentspeed;
	// If not adding any, done.
	if (addspeed <= 0)
		return;
	// Determine acceleration speed after acceleration


	if(sv_player_midair_accel_val != 1){
		// retail way
		accelspeed = accel * wishspeed * pmove->frametime * pmove->friction;
	}else{
		// less impact per frame
		// Could make depend on gravity, buuuut at 0 gravity that would mean, no influence from the player.
		// Which kinda makes sense?   ehhhh keep the same influence always.
		accelspeed = accel * wishspeed * pmove->frametime * pmove->friction * 0.28;
	}

	// Cap it
	if (accelspeed > addspeed){
		accelspeed = addspeed;
	}
	
	// Adjust pmove vel.
	for (i=0 ; i<3 ; i++)
	{
		pmove->velocity[i] += accelspeed*wishdir[i];	
	}


	


	/*
	// Nevermind this too, still some oddities.

	float maxInfluence;
	float wishVelChange[2];

	float wishVelTarget[2];

	if (pmove->dead)
		return;
	if (pmove->waterjumptime)
		return;


	if (wishspd > 90)
		wishspd = 90;



	for(i = 0; i < 2; i++){
		// how about some air resistance?  (apply even if not holding down any move keys)
		pmove->velocity[i] = pmove->velocity[i] * 0.97;
	}


	// Determine veer amount
	currentspeed = DotProduct_f (pmove->velocity, wishdir);
	// See how much to add
	addspeed = wishspd - currentspeed;
	// If not adding any, done.
	if (addspeed <= 0)return;


	for(i = 0; i < 2; i++){
		// What speed am I suggesting I want?
		wishVelTarget[i] = wishdir[i] * wishspeed;
		// How much can I change towards that in this frame?
		wishVelChange[i] = wishdir[i] * wishspeed * accel * pmove->frametime * pmove->friction;

		// And, am I already going in that direction in this coord?  If so, no change.
		// Otherwise change up to that coord in that direction, but don't exceed the VelTarget.
		// (holding forward while moving faster than the target speed won't decelerate or snap to the then-slower target)
		if(wishVelChange[i] > 0){
			if(pmove->velocity[i] >= wishVelTarget[i]){
				// don't do anything, already greatr in this coord
			}else{
				// How much can be changed?
				float changeTest = pmove->velocity[i] + wishVelChange[i];
				if (changeTest < wishVelTarget[i]) {
					// Haven't reached the target yet? accept
					pmove->velocity[i] = changeTest;
				}else{
					// snap to it
					pmove->velocity[i] = wishVelChange[i];
				}
			}
		}else if(wishVelChange[i] < 0){
			if(pmove->velocity[i] <= wishVelTarget[i]){
				// don't do anything, already greatr in this coord
			}else{
				// How much can be changed?
				float changeTest = pmove->velocity[i] + wishVelChange[i];
				if (changeTest > wishVelTarget[i]) {
					// Haven't reached the target yet? accept
					pmove->velocity[i] = changeTest;
				}else{
					// snap to it
					pmove->velocity[i] = wishVelChange[i];
				}
			}
		}
	}//for(i...)
	*/





	// Nevermind this.
	/*
	// if going faster than this in the direciton I want to go, no influence.
	maxInfluence = 150;

	wishVel[0] = wishdir[0] * wishspeed * accel * pmove->frametime * pmove->friction;
	wishVel[1] = wishdir[1] * wishspeed * accel * pmove->frametime * pmove->friction;

	//wishVel[0] = accelspeed*wishdir[0];
	//wishVel[1] = accelspeed*wishdir[1];
	


	for (i=0 ; i<2 ; i++){
		if(wishVel[i] > 0){
			if(pmove->velocity[i] + wishVel[i] < maxInfluence){
				// safe
			}else{
				wishVel[i] = maxInfluence - pmove->velocity[i];
				if(wishVel[i] < 0){
					// don't slow down from being over the max, don't do anything
					wishVel[i] = 0;
				}
			}
		}else if(wishVel[i] < 0){
			if(pmove->velocity[i] + wishVel[i] > maxInfluence){
				// safe
			}else{
				wishVel[i] = -maxInfluence - pmove->velocity[i];
				if(wishVel[i] > 0){
					// don't slow down from being over the max, don't do anything
					wishVel[i] = 0;
				}
			}
		}
	}

	// Adjust pmove vel.
	for (i=0 ; i<2 ; i++)
	{
		//pmove->velocity[i] += accelspeed*wishdir[i];	
		pmove->velocity[i] += wishVel[i];	
	}
	*/
	
}

/*
===================
PM_WaterMove

===================
*/
void PM_WaterMove (void)
{
	int	i;
	vec3_t	wishvel;
	float wishspeed;
	vec3_t	wishdir;
	vec3_t	start, dest;
	vec3_t  temp;
	pmtrace_t	trace;

	float speed, newspeed, addspeed, accelspeed;
	float normalSpeedMult = 1;
//
// user intentions
//

	//MODDD
	normalSpeedMult = getSafeSqureRoot(atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "nsm" ) ));
	
	//MODDD
	for (i=0 ; i<3 ; i++)
		//MODDD
		wishvel[i] = pmove->forward[i]*pmove->cmd.forwardmove*normalSpeedMult + pmove->right[i]*pmove->cmd.sidemove*normalSpeedMult;

	// Sinking after no other movement occurs
	if (!pmove->cmd.forwardmove && !pmove->cmd.sidemove && !pmove->cmd.upmove)
		wishvel[2] -= 60;		// drift towards bottom
	else  // Go straight up by upmove amount.
		wishvel[2] += pmove->cmd.upmove;

	// Copy it over and determine speed
	VectorCopy_f (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	// Cap speed.
	//MODDD
	if (normalSpeedMult == 1 && wishspeed > pmove->maxspeed)
	{
		VectorScale (wishvel, pmove->maxspeed/wishspeed, wishvel);
		wishspeed = pmove->maxspeed;
	}
	// Slow us down a bit.
	wishspeed *= 0.8;

	VectorAdd_f (pmove->velocity, pmove->basevelocity, pmove->velocity);
// Water friction
	VectorCopy_f(pmove->velocity, temp);
	speed = VectorNormalize(temp);
	if (speed)
	{
		newspeed = speed - pmove->frametime * speed * pmove->movevars->friction * pmove->friction;

		if (newspeed < 0)
			newspeed = 0;
		VectorScale (pmove->velocity, newspeed/speed, pmove->velocity);
	}
	else
		newspeed = 0;

//
// water acceleration
//
	if ( wishspeed < 0.1f )
	{
		return;
	}

	addspeed = wishspeed - newspeed;
	if (addspeed > 0)
	{

		VectorNormalize(wishvel);
		accelspeed = pmove->movevars->accelerate * wishspeed * pmove->frametime * pmove->friction;
		if (accelspeed > addspeed)
			accelspeed = addspeed;

		for (i = 0; i < 3; i++)
			pmove->velocity[i] += accelspeed * wishvel[i];
	}

// Now move
// assume it is a stair or a slope, so press down from stepheight above
	VectorMA (pmove->origin, pmove->frametime, pmove->velocity, dest);
	VectorCopy_f (dest, start);
	start[2] += pmove->movevars->stepsize + 1;
	trace = pmove->PM_PlayerTrace (start, dest, PM_NORMAL, -1 );
	if (!trace.startsolid && !trace.allsolid)	// FIXME: check steep slope?
	{	// walked up the step, so just keep result and exit
		VectorCopy_f (trace.endpos, pmove->origin);
		return;
	}
	
	// Try moving straight along out normal path.
	PM_FlyMove ();
}


/*
===================
PM_AirMove

===================
*/
void PM_AirMove (void)
{
	int		i;
	vec3_t		wishvel;
	float	fmove, smove;
	vec3_t		wishdir;
	float	wishspeed;
	float normalSpeedMult = 1;
	//int clip; //MODDD - new
	vec3_t vecOldVel;
	int sv_player_midair_fix_val;

	//MODDD - reset before proceeding
	ary_iMidAirMoveBlocked[pmove->player_index] = FALSE;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;
	
	// Zero out z components of movement vectors
	pmove->forward[2] = 0;
	pmove->right[2]   = 0;
	// Renormalize
	VectorNormalize (pmove->forward);
	VectorNormalize (pmove->right);


	//MODDD
	normalSpeedMult = getSafeSqureRoot(atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "nsm" ) ));
	

	// Determine x and y parts of velocity
	for (i=0 ; i<2 ; i++)       
	{
		//MODDD
		wishvel[i] = pmove->forward[i]*fmove*normalSpeedMult + pmove->right[i]*smove*normalSpeedMult;
	}
	// Zero out z part of velocity
	wishvel[2] = 0;             

	 // Determine maginitude of speed of move
	VectorCopy_f (wishvel, wishdir);  
	wishspeed = VectorNormalize(wishdir);

	// Clamp to server defined max speed
	//MODDD - if playing with cheats (normalSpeedMult other than 1), ignore the max.
	if (normalSpeedMult == 1 && wishspeed > pmove->maxspeed)
	{
		VectorScale (wishvel, pmove->maxspeed/wishspeed, wishvel);
		wishspeed = pmove->maxspeed;
	}
	
	//MODDD - NOTE.
	// ok.  Don't fully understand how air acceleration works, so no mods for it, at least not yet.
	// It would be a CVar 'sv_player_midair_accelerate_mode', something like that.
	// 0 for retail, 1 for more mid-air control (disable the '30' cap in there).
	// There is a little oddity with acceleration that get epsecially noticeable with the cap removed though,
	// but it's easier to prove with the cap on in a large open space without gravity.
	// Example:
	//   console,    map crossfire
	//               sv_gravity 0
	//               setmyorigin 870 -1080 -1520
	// Now, without touching any walls, hold forward the hole time and rotate the camera in the same direction somewhat slowly,
	// just not too fast.  The speed will pick up over time and exceed the usual '30' cap, and even exceed the player's
	// own walk movement speed (possible to go over the longways portion in a second afte doing this a while).
	// It's as though this 'sideways' velocity isn't getting limited like it's supposed to.
	// Again, small, but this flaw really shows up if the midair cap were completley lifted.
	PM_AirAccelerate (wishdir, wishspeed, pmove->movevars->airaccelerate);

	// Add in any base velocity to the current velocity.
	VectorAdd_f (pmove->velocity, pmove->basevelocity, pmove->velocity );



	sv_player_midair_fix_val = atoi(pmove->PM_Info_ValueForKey(pmove->physinfo, "maf"));

	if (sv_player_midair_fix_val == 2) {
		VectorCopy_f(pmove->velocity, vecOldVel);
		//MODDD - Strangely, the PM_InclineAirCheck can completely replace the as-is plain PM_FlyMove check now.
		//clip = PM_FlyMove ();
		// mod
		PM_InclineAirCheck(vecOldVel);
	}
	else {
		// retail
		//MODDD - save the result of this to ary_iMidAirMoveBlocked.  Wasn't saved before.
		ary_iMidAirMoveBlocked[pmove->player_index] = (PM_FlyMove() == 2);
	}

}

qboolean PM_InWater( void )
{
	return ( pmove->waterlevel > 1 );
}

/*
=============
PM_CheckWater

Sets pmove->waterlevel and pmove->watertype values.
=============
*/

qboolean PM_CheckWater ()
{
	vec3_t	point;
	int	cont;
	int	truecont;
	float     height;
	float	heightover2;


	// Pick a spot just above the players feet.
	point[0] = pmove->origin[0] + (pmove->player_mins[pmove->usehull][0] + pmove->player_maxs[pmove->usehull][0]) * 0.5;
	point[1] = pmove->origin[1] + (pmove->player_mins[pmove->usehull][1] + pmove->player_maxs[pmove->usehull][1]) * 0.5;
	point[2] = pmove->origin[2] + pmove->player_mins[pmove->usehull][2] + 1;
	
	// Assume that we are not in water at all.
	pmove->waterlevel = 0;
	pmove->watertype = CONTENTS_EMPTY;

	// Grab point contents.
	cont = pmove->PM_PointContents (point, &truecont );
	// Are we under water? (not solid and not empty?)
	if (cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT )
	{
		// Set water type
		pmove->watertype = cont;

		// We are at least at level one
		pmove->waterlevel = 1;

		height = (pmove->player_mins[pmove->usehull][2] + pmove->player_maxs[pmove->usehull][2]);
		heightover2 = height * 0.5;
		
		// Now check a point that is at the player hull midpoint.
		point[2] = pmove->origin[2] + heightover2;
		cont = pmove->PM_PointContents (point, NULL );
		// If that point is also under water...
		if (cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT )
		{
			// Set a higher water level.
			pmove->waterlevel = 2;

			// Now check the eye position.  (view_ofs is relative to the origin)
			point[2] = pmove->origin[2] + pmove->view_ofs[2];

			cont = pmove->PM_PointContents (point, NULL );

			//MODDD
			//PLAN: for the player, have a way of telling that the water level is "3" without making the water level 3.
			//This would stop the default water-fog from showing up (blocking any attempts at editing fog color beyond that forced default).
			//We could re-create the default water fog, or even allow the water level to be "3" when that is fine.  Depends on which is easier.
			
			if (cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT ) {
				pmove->waterlevel = 3;  // In over our eyes
			}


		}
		
		// Adjust velocity based on water current, if any.
		if ( ( truecont <= CONTENTS_CURRENT_0 ) &&
			 ( truecont >= CONTENTS_CURRENT_DOWN ) )
		{
			// The deeper we are, the stronger the current.
			static vec3_t current_table[] =
			{
				{1, 0, 0}, {0, 1, 0}, {-1, 0, 0},
				{0, -1, 0}, {0, 0, 1}, {0, 0, -1}
			};

			VectorMA (pmove->basevelocity, 50.0*pmove->waterlevel, current_table[CONTENTS_CURRENT_0 - truecont], pmove->basevelocity);
		}
	}

	return pmove->waterlevel > 1;
}

/*
=============
PM_CatagorizePosition
=============
*/

//MODDD - now accepts "physent_t *pLadder".

/*
void PM_CatagorizePosition (physent_t *pLadder);

void PM_CatagorizePosition(){
	PM_CatagorizePosition(NULL);
}
*/



//prototype, so that the ...BASIC version can see it even though it comes before (above).
void PM_CatagorizePosition (physent_t *pLadder);

//Apparaently, the C language does NOT support overloading.  
//So, this renamed version will handle cases of method calls that don't have the 
//ladder available (assume we don't have one, doesn't matter, probably)
void PM_CatagorizePositionBASIC(){
	PM_CatagorizePosition(NULL);
}


//NOTE:::it is possible to imply whether on a ladder or not by this check too:
//fLadder = ( pmove->movetype == MOVETYPE_FLY );// IsOnLadder();
//Use this if it is necessary to check for a ladder at all times, unless "MOVETYPE_FLY" is ever used for
//something other than latters.

//void PM_CatagorizePosition (void)
void PM_CatagorizePosition (physent_t *pLadder)
{
	vec3_t point;
	pmtrace_t tr;

// if the player hull point one unit down is solid, the player
// is on ground

// see if standing on something solid	

	// Doing this before we move may introduce a potential latency in water detection, but
	// doing it after can get us stuck on the bottom in water if the amount we move up
	// is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
	// this several times per frame, so we really need to avoid sticking to the bottom of
	// water on each call, and the converse case will correct itself if called twice.
	PM_CheckWater();

	point[0] = pmove->origin[0];
	point[1] = pmove->origin[1];
	point[2] = pmove->origin[2] - 2;

	if (pmove->velocity[2] > 180)   // Shooting up really fast.  Definitely not on ground.
	{
		pmove->onground = -1;
	}
	else
	{

		// Try and move down.
		tr = pmove->PM_PlayerTrace (pmove->origin, point, PM_NORMAL, -1 );
		// If we hit a steep plane, we are not on ground
		if ( tr.plane.normal[2] < 0.7){
			//MODDD - NOTE.  Beware!  It is possible to land on a sloped surface that cannot be jumped out of and remain stuck,
			// like spots in the rails in a2a1.
			// Idea:  Mark it here, and if the Z velocity remains 0 or below 0 but not by much and stays that way for 0.2 seconds,
			// allow a jump out.
			// OTHER IDEA: is there a place that bounces the player off an incline on contact?  If so, noticing no 'bounce' movement
			// (something in the way) means jumping to get out should be allowed.
			///////////////////////////////////////////////////////////////////////////////////////////////////
			if(tr.fraction < 1.0){
				// actually hit something?  ok.
				//if(ary_flUnstuckJumpTimer[pmove->player_index] == 0){
				//	ary_flUnstuckJumpTimer[pmove->player_index] = pmove->Sys_FloatTime() + 0.3;
				//}

				ary_iTallSlopeBelow[pmove->player_index] = TRUE;
			}


			///////////////////////////////////////////////////////////////////////////////////////////////////

			pmove->onground = -1;	// too steep
		}else{
			pmove->onground = tr.ent;  // Otherwise, point to index of ent under us.
		}

		// If we are on something...
		if (pmove->onground != -1)
		{
			// Then we are not in water jump sequence
			pmove->waterjumptime = 0;
			
			// If we could make the move, drop us down that 1 pixel
			//NOTE!!!!! THis is the line that is screwing up the ladder at speeds slower than 130.
			//Solution?  Check to see if the player is on a ladder.  If so, don't do this!
			//if (pmove->waterlevel < 2 && !tr.startsolid && !tr.allsolid)
			if (pmove->waterlevel < 2 && !tr.startsolid && !tr.allsolid && pLadder != NULL)
				VectorCopy_f (tr.endpos, pmove->origin);
		}
		
		// Standing on an entity other than the world
		if (tr.ent > 0)   // So signal that we are touching something.
		{
			PM_AddToTouched(tr, pmove->velocity);
		}
	}
}

/*
=================
PM_GetRandomStuckOffsets

When a player is stuck, it's costly to try and unstick them
Grab a test offset for the player based on a passed in index
=================
*/
int PM_GetRandomStuckOffsets(int nIndex, int server, vec3_t offset)
{
 // Last time we did a full
	int idx;
	idx = rgStuckLast[nIndex][server]++;

	VectorCopy_f(rgv3tStuckTable[idx % 54], offset);

	return (idx % 54);
}

void PM_ResetStuckOffsets(int nIndex, int server)
{
	rgStuckLast[nIndex][server] = 0;
}

/*
=================
NudgePosition

If pmove->origin is in a solid position,
try nudging slightly on all axis to
allow for the cut precision of the net coordinates
=================
*/

int PM_CheckStuck (void)
{
	vec3_t	base;
	vec3_t  offset;
	vec3_t  test;
	int     hitent;
	int	idx;
	float fTime;
	int i;
	pmtrace_t traceresult;

	static float rgStuckCheckTime[MAX_CLIENTS][2]; // Last time we did a full

	// If position is okay, exit
	hitent = pmove->PM_TestPlayerPosition (pmove->origin, &traceresult );
	if (hitent == -1 )
	{
		PM_ResetStuckOffsets( pmove->player_index, pmove->server );
		return 0;
	}

	VectorCopy_f (pmove->origin, base);

	// 
	// Deal with precision error in network.
	// 
	if (!pmove->server)
	{
		// World or BSP model
		if ( ( hitent == 0 ) ||
			 ( pmove->physents[hitent].model != NULL ) )
		{
			int nReps = 0;
			PM_ResetStuckOffsets( pmove->player_index, pmove->server );
			do 
			{
				i = PM_GetRandomStuckOffsets(pmove->player_index, pmove->server, offset);

				VectorAdd_f(base, offset, test);
				if (pmove->PM_TestPlayerPosition (test, &traceresult ) == -1)
				{
					PM_ResetStuckOffsets( pmove->player_index, pmove->server );
		
					VectorCopy_f ( test, pmove->origin );
					return 0;
				}
				nReps++;
			} while (nReps < 54);
		}
	}

	// Only an issue on the client.

	if (pmove->server)
		idx = 0;
	else
		idx = 1;

	fTime = pmove->Sys_FloatTime();
	// Too soon?
	if (rgStuckCheckTime[pmove->player_index][idx] >= 
		( fTime - PM_CHECKSTUCK_MINTIME ) )
	{
		return 1;
	}
	rgStuckCheckTime[pmove->player_index][idx] = fTime;

	pmove->PM_StuckTouch( hitent, &traceresult );

	i = PM_GetRandomStuckOffsets(pmove->player_index, pmove->server, offset);

	VectorAdd_f(base, offset, test);
	if ( ( hitent = pmove->PM_TestPlayerPosition ( test, NULL ) ) == -1 )
	{
		//Con_DPrintf("Nudged\n");

		PM_ResetStuckOffsets( pmove->player_index, pmove->server );

		if (i >= 27)
			VectorCopy_f ( test, pmove->origin );

		return 0;
	}

	// If player is flailing while stuck in another player ( should never happen ), then see
	//  if we can't "unstick" them forceably.
	if ( pmove->cmd.buttons & ( IN_JUMP | IN_DUCK | IN_ATTACK ) && ( pmove->physents[ hitent ].player != 0 ) )
	{
		float x, y, z;
		float xystep = 8.0;
		float zstep = 18.0;
		float xyminmax = xystep;
		float zminmax = 4 * zstep;
		
		for ( z = 0; z <= zminmax; z += zstep )
		{
			for ( x = -xyminmax; x <= xyminmax; x += xystep )
			{
				for ( y = -xyminmax; y <= xyminmax; y += xystep )
				{
					VectorCopy_f( base, test );
					test[0] += x;
					test[1] += y;
					test[2] += z;

					if ( pmove->PM_TestPlayerPosition ( test, NULL ) == -1 )
					{
						VectorCopy_f( test, pmove->origin );
						return 0;
					}
				}
			}
		}
	}

	//VectorCopy_f (base, pmove->origin);

	return 1;
}

/*
===============
PM_SpectatorMove
===============
*/
void PM_SpectatorMove (void)
{
	float speed, drop, friction, control, newspeed;
	//float   accel;
	float currentspeed, addspeed, accelspeed;
	int		i;
	vec3_t		wishvel;
	float	fmove, smove;
	vec3_t		wishdir;
	float	wishspeed;
	// this routine keeps track of the spectators psoition
	// there a two different main move types : track player or moce freely (OBS_ROAMING)
	// doesn't need excate track position, only to generate PVS, so just copy
	// targets position and real view position is calculated on client (saves server CPU)
	
	if ( pmove->iuser1 == OBS_ROAMING)
	{

#ifdef CLIENT_DLL
		// jump only in roaming mode
		if ( iJumpSpectator )
		{
			VectorCopy_f( vJumpOrigin, pmove->origin );
			VectorCopy_f( vJumpAngles, pmove->angles );
			VectorCopy_f( vec3_origin, pmove->velocity );
			iJumpSpectator	= 0;
			return;
		}
		#endif
		// Move around in normal spectator method

		speed = Length (pmove->velocity);
		if (speed < 1)
		{
			//MODDD - missing a semicolon?  Evidence of using the '_f' version.
			VectorCopy_f(vec3_origin, pmove->velocity);
		}
		else
		{
			drop = 0;

			friction = pmove->movevars->friction*1.5;	// extra friction
			control = speed < pmove->movevars->stopspeed ? pmove->movevars->stopspeed : speed;
			drop += control*friction*pmove->frametime;

			// scale the velocity
			newspeed = speed - drop;
			if (newspeed < 0)
				newspeed = 0;
			newspeed /= speed;

			VectorScale (pmove->velocity, newspeed, pmove->velocity);
		}

		// accelerate
		fmove = pmove->cmd.forwardmove;
		smove = pmove->cmd.sidemove;
		
		VectorNormalize (pmove->forward);
		VectorNormalize (pmove->right);

		for (i=0 ; i<3 ; i++)
		{
			wishvel[i] = pmove->forward[i]*fmove + pmove->right[i]*smove;
		}
		wishvel[2] += pmove->cmd.upmove;

		VectorCopy_f (wishvel, wishdir);
		wishspeed = VectorNormalize(wishdir);

		//
		// clamp to server defined max speed
		//
		if (wishspeed > pmove->movevars->spectatormaxspeed)
		{
			VectorScale (wishvel, pmove->movevars->spectatormaxspeed/wishspeed, wishvel);
			wishspeed = pmove->movevars->spectatormaxspeed;
		}

		currentspeed = DotProduct_f(pmove->velocity, wishdir);
		addspeed = wishspeed - currentspeed;
		if (addspeed <= 0)
			return;

		accelspeed = pmove->movevars->accelerate*pmove->frametime*wishspeed;
		if (accelspeed > addspeed)
			accelspeed = addspeed;
		
		for (i=0 ; i<3 ; i++)
			pmove->velocity[i] += accelspeed*wishdir[i];	

		// move
		VectorMA (pmove->origin, pmove->frametime, pmove->velocity, pmove->origin);
	}
	else
	{
		// all other modes just track some kind of target, so spectator PVS = target PVS

		int target;

		// no valid target ?
		if ( pmove->iuser2 <= 0)
			return;

		// Find the client this player's targeting
		for (target = 0; target < pmove->numphysent; target++)
		{
			if ( pmove->physents[target].info == pmove->iuser2 )
				break;
		}

		if (target == pmove->numphysent)
			return;

		// use targets position as own origin for PVS
		VectorCopy_f( pmove->physents[target].angles, pmove->angles );
		VectorCopy_f( pmove->physents[target].origin, pmove->origin );

		// no velocity
		VectorCopy_f( vec3_origin, pmove->velocity );
	}
}

/*
==================
PM_SplineFraction

Use for ease-in, ease-out style interpolation (accel/decel)
Used by ducking code.
==================
*/
float PM_SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void PM_FixPlayerCrouchStuck( int direction )
{
	int     hitent;
	int i;
	vec3_t test;

	hitent = pmove->PM_TestPlayerPosition ( pmove->origin, NULL );
	if (hitent == -1 )
		return;
	
	VectorCopy_f( pmove->origin, test );	
	for ( i = 0; i < 36; i++ )
	{
		pmove->origin[2] += direction;
		hitent = pmove->PM_TestPlayerPosition ( pmove->origin, NULL );
		if (hitent == -1 )
			return;
	}

	VectorCopy_f( test, pmove->origin ); // Failed
}

void PM_UnDuck( void )
{
	int i;
	pmtrace_t trace;
	vec3_t newOrigin;

	VectorCopy_f( pmove->origin, newOrigin );




	//MODDD - crouch interp, modified section
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	if ( pmove->onground != -1 )
	{
		//MODDD
		// That is, only do this origin adjustment if the hull was actually changed to the duck-one (usehull == 1).
		// If not, it just brings the player upward for no reason.  No need for factoring in the view-offset Z either
		// (it gets smoothly adjusted here going from standing to ducking, but doesn't affect the bounds until it finishes).
		if (pmove->usehull == 1) {
			for (i = 0; i < 3; i++)
			{
				newOrigin[i] += (pmove->player_mins[1][i] - pmove->player_mins[0][i]);
			}
		}
	}
	else {
		//int x = 4;  //breakpoint
	}
	
	trace = pmove->PM_PlayerTrace( newOrigin, newOrigin, PM_NORMAL, -1 );

	if ( !trace.startsolid )
	{
		pmove->usehull = 0;

		// Oh, no, changing hulls stuck us into something, try unsticking downward first.
		trace = pmove->PM_PlayerTrace( newOrigin, newOrigin, PM_NORMAL, -1  );
		if ( trace.startsolid )
		{
			// See if we are stuck?  If so, stay ducked with the duck hull until we have a clear spot
			//Con_Printf( "unstick got stuck\n" );
			pmove->usehull = 1;
			return;
		}

		pmove->flags &= ~FL_DUCKING;
		pmove->bInDuck  = false;
		pmove->view_ofs[2] = VEC_VIEW_Z;
		pmove->flDuckTime = 0;
		
		VectorCopy_f( newOrigin, pmove->origin );

		// Recatagorize position since ducking can change origin
		PM_CatagorizePositionBASIC();
	}
}

void PM_Duck( void )
{
	int i;
	float time;
	float duckFraction;

	//MODDD - nice, but... why isn't this something done early on in this file and 
	// made available to all other methods then?
	int buttonsChanged	= ( pmove->oldbuttons ^ pmove->cmd.buttons );	// These buttons have changed this frame
	int nButtonPressed	=  buttonsChanged & pmove->cmd.buttons;		// The changed ones still down are "pressed"

	//MODDD - nice, but was never referred to.  Whoops.
	//int duckchange		= buttonsChanged & IN_DUCK ? 1 : 0;
	//int duckpressed		= nButtonPressed & IN_DUCK ? 1 : 0;

	if ( pmove->cmd.buttons & IN_DUCK )
	{
		pmove->oldbuttons |= IN_DUCK;
	}
	else
	{
		pmove->oldbuttons &= ~IN_DUCK;
	}

	// Prevent ducking if the iuser3 variable is set
	if ( pmove->iuser3 || pmove->dead )
	{
		// Try to unduck
		if ( pmove->flags & FL_DUCKING )
		{
			PM_UnDuck();
		}
		return;
	}

	if ( pmove->flags & FL_DUCKING )
	{
		//MODDD - how about reducing the move speed penalties a little.
		pmove->cmd.forwardmove *= DUCK_SPEED_MULTI;
		pmove->cmd.sidemove    *= DUCK_SPEED_MULTI;
		pmove->cmd.upmove      *= DUCK_SPEED_MULTI;
	}

	if ( ( pmove->cmd.buttons & IN_DUCK ) || ( pmove->bInDuck ) || ( pmove->flags & FL_DUCKING ) )
	{
		if ( pmove->cmd.buttons & IN_DUCK )
		{
			if ( (nButtonPressed & IN_DUCK ) && !( pmove->flags & FL_DUCKING ) )
			{
				// Use 1 second so super long jump will work
				pmove->flDuckTime = 1000;
				pmove->bInDuck    = true;
			}

			time = max( 0.0, ( 1.0 - (float)pmove->flDuckTime / 1000.0 ) );
			
			if ( pmove->bInDuck )
			{
				// Finish ducking immediately if duck time is over or not on ground
				if ( ( (float)pmove->flDuckTime / 1000.0 <= ( 1.0 - TIME_TO_DUCK ) ) ||
					 ( pmove->onground == -1 ) )
				{
					pmove->usehull = 1;
					pmove->view_ofs[2] = VEC_DUCK_VIEW_Z;
					pmove->flags |= FL_DUCKING;
					pmove->bInDuck = false;

					// HACKHACK - Fudge for collision bug - no time to fix this properly
					if ( pmove->onground != -1 )
					{
						for ( i = 0; i < 3; i++ )
						{
							pmove->origin[i] -= ( pmove->player_mins[1][i] - pmove->player_mins[0][i] );
						}
						// See if we are stuck?
						PM_FixPlayerCrouchStuck( STUCK_MOVEUP );

						// Recatagorize position since ducking can change origin
						PM_CatagorizePositionBASIC();
					}
				}
				else
				{
					float fMore = (VEC_DUCK_HULL_MIN_Z - VEC_HULL_MIN_Z);

					// Calc parametric time
					duckFraction = PM_SplineFraction( time, (1.0/TIME_TO_DUCK) );
					pmove->view_ofs[2] = ((VEC_DUCK_VIEW_Z - fMore ) * duckFraction) + (VEC_VIEW_Z * (1-duckFraction));
				}
			}
		}
		else
		{
			// Try to unduck
			PM_UnDuck();
		}
	}
}



void PM_LadderMove( physent_t *pLadder )
{
	vec3_t		ladderCenter;
	trace_t		trace;
	qboolean	onFloor;
	vec3_t		floor;
	vec3_t		modelmins, modelmaxs;

	//MODDD - new
	float varGen;
	float varApply;
	float varRaw;
	int playerLadderMovement = 0;
	float ladderSpeed;
	int filterediuser4;
	float normalSpeedMult = 1;
	float ladderCycleMulti;
	float ladderSpeedMulti;
	float ladderCycleActual;
	BOOL cyclePassed = FALSE;


	//MODDD - control.  Might want to act on jump being tapped (pressed for the first frame), not continuously.
	//////////////////////////////////////////////////////////////////////////////
	//static iJumpOffLadderFrames = 0;
	int iJumpPressed = 0;


	//MODDD - moved earlier.  What's the point of anything else if this will end it?
	if ( pmove->movetype == MOVETYPE_NOCLIP )
		return;


	//MODDD
	// sometimes jumping off a ladder only on the tap (first) frame isn't enough.
	// Allow a few more frames of holding space to jump while touching a ladder.
	// CHANGE, done earlier by first contact with the ladder instead.
	//if ((pmove->cmd.buttons & IN_JUMP) && !(pmove->oldbuttons & IN_JUMP)) {
	//	ary_iJumpOffDenyLadderFrames[pmove->player_index] = 3;
	//}

	// NOTICE - was > 0 in old logic
	// ALSO, now allowing only a clean jump-press to jump off, not holding jump down
	if ((pmove->cmd.buttons & IN_JUMP) && !(pmove->oldbuttons & IN_JUMP)) {
		if (ary_iJumpOffDenyLadderFrames[pmove->player_index] <= 0) {
			// If jump wasn't held down the previous frame (this is the first in a while),
			// or we've already jumped off recently,  count this as a button press.

			if (pmove->cmd.buttons & IN_JUMP) {
				iJumpPressed = 1;
			}
		}
	}


	//static int iJumpPressed = 0;
	/*
	if (pmove->cmd.buttons & IN_JUMP) {
		if (iJumpPressed == 0) {
			// released to tapped
			iJumpPressed = 1;
		}else if (iJumpPressed == 1) {
			// tapped to pressed
			iJumpPressed = 2;
		}else {
			// stay pressed.
		}
	}else {
		// no longer held? released
		iJumpPressed = 0;
	}
	*/
	//////////////////////////////////////////////////////////////////////////////


	

	pmove->PM_GetModelBounds( pLadder->model, modelmins, modelmaxs );

	VectorAdd_f( modelmins, modelmaxs, ladderCenter );
	VectorScale( ladderCenter, 0.5, ladderCenter );

	pmove->movetype = MOVETYPE_FLY;

	// On ladder, convert movement to be relative to the ladder
	
	VectorCopy_f( pmove->origin, floor );
	floor[2] += pmove->player_mins[pmove->usehull][2] - 1;


	if ( pmove->PM_PointContents( floor, NULL ) == CONTENTS_SOLID )
		onFloor = true;
	else
		onFloor = false;

	pmove->gravity = 0;
	pmove->PM_TraceModel( pLadder, pmove->origin, ladderCenter, &trace );
	if ( trace.fraction != 1.0 )
	{
		float forward = 0, right = 0;
		//moddd - new
		float forwardRaw = 0, rightRaw = 0;
		vec3_t vpn, v_right;

		AngleVectors( pmove->angles, vpn, v_right, NULL );
		
		//pmove->iuser1 += (int) (Length(pmove->velocity) * 100);

		/*
		if(pmove->server){
			pmove->Con_Printf("S3 %d\n", pmove->iuser4);
		}else{
			pmove->Con_Printf("C3 %d\n", pmove->iuser4);
		}
		*/

		playerLadderMovement = atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "plm" ) );

		normalSpeedMult = getSafeSqureRoot(atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "nsm" ) ));
		
		ladderCycleMulti = atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "lcm" ) );
		ladderSpeedMulti = getSafeSqureRoot(atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "lsm" ) ));

		//pmove->punchangle[2] = 20; //!!!  TEST

		if(playerLadderMovement == 0){
			ladderSpeed = MAX_CLIMB_SPEED_RETAIL * 1.28; //1.414;
		}else{
			ladderSpeed = MAX_CLIMB_SPEED_ALPHA * 1.28;
		}
		

		//the counter we are concerned with.
		filterediuser4 = pmove->iuser4 & ~(FLAG_JUMPED | FLAG_RESET_RECEIVED | FLAG_CYCLE_PASSED);
		

		ladderCycleActual = LADDER_CYCLE_BASE*ladderCycleMulti;

		if(filterediuser4 >= ladderCycleActual){
			cyclePassed = TRUE;  // send this to iuser4 later
			filterediuser4-= ladderCycleActual;
		}
		

		if(playerLadderMovement == 0){
			//if 0, retail's ladder movement is constant (no sine slow-down to immitate steps).
			varGen = 1;
		}else{
			varGen = sin( (float)( (float)filterediuser4 / (ladderCycleActual ) * M_PI) );
		}

		//can't be negative.
		if(varGen < 0){
			varGen *= -1;
		}

		/*
		if(pmove->server){
			pmove->Con_Printf("Swat2 %f\n", pmove->fuser4);

		}else{
			pmove->Con_Printf("Cwat2 %f\n", pmove->fuser4);
		}
		*/
		
		//could "pmove->frametime" be used to be consistent b/w framerates other than 60?   But the devs didn't use it for ladder-move logic.

		varApply = ladderSpeed * varGen * ladderSpeedMulti;
		varRaw = ladderSpeed;
		

		//MODDD - each has received the appropriate actions with "varRaw" too.
		if ( pmove->cmd.buttons & IN_BACK ){
			forward -= varApply;
			forwardRaw -= varRaw;
		}
		if ( pmove->cmd.buttons & IN_FORWARD ){
			forward += varApply;
			forwardRaw += varRaw;
		}
		if ( pmove->cmd.buttons & IN_MOVELEFT ){
			right -= varApply;
			rightRaw -= varRaw;
		}
		if ( pmove->cmd.buttons & IN_MOVERIGHT ){
			right += varApply;
			rightRaw += varRaw;
		}



		//MODDD - only a solid jump tap allowed, not holding it down.  This should stop trying to jump from a ladder the moment
		// it is hit from having space held down ever so slightly too long.
		if ( iJumpPressed == 1 )
		{
			filterediuser4 = 0;
			pmove->iuser4 |= FLAG_JUMPED;  //why?  so that, on jumping down, the "drop" tilt isn't triggered.

			pmove->movetype = MOVETYPE_WALK;
			//MODDD - lower ladder jump distance, no need to be obnoxious.
			// Was 270, see constant.
			VectorScale( trace.plane.normal, 190, pmove->velocity );

			// Finally, deny going back to the ladder for a few frames.
			ary_iDenyLadderFrames[pmove->player_index] = 12;
		}
		else
		{
			//MODDD - refering to "raws" instead.  If the speed ends up 0 from the low point of a step, we still want to know we tried to climb to advance "pmove->iuser4".
			if ( forwardRaw != 0 || rightRaw != 0 )
			{
				vec3_t velocity, perp, cross, lateral, tmp;
				float normal;
				float lateralLength;
				float ladderTravelLength;
				float velLength;

				//ALERT(at_console, "pev %.2f %.2f %.2f - ",
				//	pev->velocity.x, pev->velocity.y, pev->velocity.z);
				// Calculate player's intended velocity
				//Vector velocity = (forward * gpGlobals->v_forward) + (right * gpGlobals->v_right);
				VectorScale( vpn, forward, velocity );
				VectorMA( velocity, right, v_right, velocity );

				//easyForcePrintLine("ang:%.2f %.2f %.2f for:%.2f %.2f %.2f", pmove->angles[0], pmove->angles[1], pmove->angles[2], vpn[0], vpn[1], vpn[2]);
				//easyForcePrintLine("earlyvel:%.2f %.2f %.2f", velocity[0], velocity[1], velocity[2]);

				
				// Perpendicular in the ladder plane
	//					Vector perp = CrossProduct( Vector(0,0,1), trace.vecPlaneNormal );
	//					perp = perp.Normalize();
				VectorClear_f( tmp );
				tmp[2] = 1;
				CrossProduct( tmp, trace.plane.normal, perp );
				VectorNormalize( perp );
				

				// decompose velocity into ladder plane
				normal = DotProduct_f( velocity, trace.plane.normal );
				// This is the velocity into the face of the ladder
				VectorScale( trace.plane.normal, normal, cross );


				// This is the player's additional velocity
				VectorSubtract_f( velocity, cross, lateral );


				// This turns the velocity into the face of the ladder into velocity that
				// is roughly vertically perpendicular to the face of the ladder.
				// NOTE: It IS possible to face up and move down or face down and move up
				// because the velocity is a sum of the directional velocity and the converted
				// velocity through the face of the ladder -- by design.
				CrossProduct( trace.plane.normal, perp, tmp );


				/*
				lateralLength = Length(lateral);
				ladderTravelLength = getSafeSqureRoot(lateralLength * lateralLength + normal * normal);

				if (ladderTravelLength > ladderSpeed) {
					float lengthModMulti = ladderSpeed / ladderTravelLength;
					// scale them down
					lateralLength *= lengthModMulti;
					normal *= lengthModMulti;
					VectorScale(lateral, lengthModMulti, lateral);
				}

				ladderTravelLength = getSafeSqureRoot(lateralLength * lateralLength + normal * normal);
				*/


				//MODDD - little issue. If the player looks at the ladder moreso diagonally (pitch-wise, like a 45 degree angle above straight at the ladder),
				// the player will go up faster instead of facing straight up.  Kinda odd.
				// I think it's kind of like how going in two directions at the same speed (4 right, 4 up in a 2D setup) has a net higher speed than moving
				// only 4 right or only 4 up.  Similarly, the lateral and -normal * tmp below make a pmove->Velocity over the planned MAX_CLIMB_SPEED.
				// So, idea.  Scale them if necessary.






				//easyForcePrintLine("testah prevel:%.2f %.2f %.2f  L:%.2f %.2f %.2f  tmp:%.2f %.2f %.2f", velocity[0], velocity[1], velocity[2], lateral[0], lateral[1], lateral[2], tmp[0], tmp[1], tmp[2]);
				

				//////////////////////////////////////////////////////////////////////////////////////////////////////
				//MODDD - change, original way.
				//!!!  VITAL APPLY
				//VectorMA( lateral, -normal, tmp, pmove->velocity );




				// 2nd way
				/*
				lateral[0] = lateral[0];
				lateral[1] = lateral[1];
				lateral[2] = lateral[2];

				vec3_t normalVec;
				normalVec[0] = (-normal * tmp[0]);
				normalVec[1] = (-normal * tmp[1]);
				normalVec[2] = (-normal * tmp[2]);

				vec3_t finalSum;
				finalSum[0] = sqrt(lateral[0] * lateral[0] + normalVec[0] * normalVec[0]);
				finalSum[1] = sqrt(lateral[1] * lateral[1] + normalVec[1] * normalVec[1]);
				finalSum[2] = sqrt(lateral[2] * lateral[2] + normalVec[2] * normalVec[2]);

				vec3_t plainSum;
				plainSum[0] = lateral[0] + normalVec[0];
				plainSum[1] = lateral[1] + normalVec[1];
				plainSum[2] = lateral[2] + normalVec[2];

				if (plainSum[0] < 0) finalSum[0] *= -1;
				if (plainSum[1] < 0) finalSum[1] *= -1;
				if (plainSum[2] < 0) finalSum[2] *= -1;

				VectorCopy_f(finalSum, pmove->velocity);
				*/


				// 3rd way.
				//!!!  VITAL APPLY
				//VectorCopy_f( lateral, pmove->velocity );
				


				
				// Seems even in the original,  -normal * tmp is biased towards moving up.
				// Why is this?  Is it normal, however that's calculated, or tmp's fault?
				// does tmp even change depending on whether the player is looking mostly up or down?
				// 4th way
				VectorMA( lateral, -normal, tmp, pmove->velocity );

				velLength = Length(pmove->velocity);
				if (velLength > ladderSpeed) {
					float lengthModMulti = ladderSpeed / velLength;
					VectorScale(pmove->velocity, lengthModMulti, pmove->velocity);
				}
				
				//////////////////////////////////////////////////////////////////////////////////////////////////////



				//easyForcePrintLine("finalvel: %.2f %.2f %.2f", pmove->velocity[0], pmove->velocity[1], pmove->velocity[2]);


				//pmove->iuser4 += 3.0f;

				//How to print in here!
				//pmove->Con_DPrintf("wat %.2f", varGen);
				/*
				if(pmove->server){
					pmove->Con_DPrintf("Swat1 %d\n", pmove->iuser4);
					pmove->Con_Printf("Swat2 %d\n", pmove->iuser4);

				}else{
					pmove->Con_DPrintf("Cwat1 %d\n", pmove->iuser4);
					pmove->Con_Printf("Cwat2 %d\n", pmove->iuser4);

				}
				*/


				/*
				VectorScale( vpn, forward, velocity );
				VectorMA( velocity, right, v_right, velocity );
				VectorClear_f( tmp );
				tmp[2] = 1;
				CrossProduct( tmp, trace.plane.normal, perp );
				VectorNormalize( perp );
				normal = DotProduct_f( velocity, trace.plane.normal );
				VectorScale( trace.plane.normal, normal, cross );
				VectorSubtract_f( velocity, cross, lateral );
				CrossProduct( trace.plane.normal, perp, tmp );
				VectorMA( lateral, -normal, tmp, pmove->velocity );
				*/


				//0.017 ??
				//pmove->iuser1 += (int) (Length(pmove->velocity) * 100);


				//MODDD - don't multiply by normalSpeedMulti now! The ladder speed is affected by its own
				//CVar now
				filterediuser4 += 1 * (int)(pmove->frametime * 1 * 10000); // * normalSpeedMult;
				//this var gets bumped by 1 for progression (when to play a noise, reset value per sin function's period)

				/*
				if( filterediuser4  >= ladderCycle){
					filterediuser4 -= ladderCycle;
				}
				*/
				//if I used to have the flag, bring it back.
				

				

				if ( onFloor && normal > 0 )	// On ground moving away from the ladder
				{
					filterediuser4 = 0;  //reset.  A jump will add the "FLAG_JUMPED" (avoid the drop-tilt on small drops) later if necessary.
					VectorMA( pmove->velocity, JUMP_OFF_FORCE, trace.plane.normal, pmove->velocity );
				}
				

				//NOTE:::this was found commented out!
				//pev->velocity = lateral - (CrossProduct( trace.vecPlaneNormal, perp ) * normal);
			}
			else
			{
				VectorClear_f( pmove->velocity );
			}
		}


		//HOW ABOUT NO
		/*
		if(pmove->server){
			pmove->Con_DPrintf("S1 iuser4::raw:%d filtered:%d jmpflg:%d \n", pmove->iuser4, filterediuser4, (pmove->iuser4 & FLAG_JUMPED)!=0 );
			pmove->Con_Printf("S2 iuser4::raw:%d filtered:%d jmpflg:%d \n", pmove->iuser4, filterediuser4, (pmove->iuser4 & FLAG_JUMPED)!=0 );

		}else{
			pmove->Con_DPrintf("C1 iuser4::raw:%d filtered:%d jmpflg:%d \n", pmove->iuser4, filterediuser4, (pmove->iuser4 & FLAG_JUMPED)!=0  );
			pmove->Con_Printf("C2 iuser4::raw:%d filtered:%d jmpflg:%d \n", pmove->iuser4, filterediuser4, (pmove->iuser4 & FLAG_JUMPED)!=0  );

		}
		*/

		if(pmove->iuser4 & FLAG_JUMPED){
			filterediuser4 |= FLAG_JUMPED;
		}
		if (cyclePassed) {
			filterediuser4 |= FLAG_CYCLE_PASSED;
		}
		pmove->iuser4 = filterediuser4;


	}//END OF (trace check)
}

physent_t *PM_Ladder( void )
{
	int		i;
	physent_t	*pe;
	hull_t		*hull;
	int		num;
	vec3_t		test;

	for ( i = 0; i < pmove->nummoveent; i++ )
	{
		pe = &pmove->moveents[i];
		
		if ( pe->model && (modtype_t)pmove->PM_GetModelType( pe->model ) == mod_brush && pe->skin == CONTENTS_LADDER )
		{

			hull = (hull_t *)pmove->PM_HullForBsp( pe, test );
			num = hull->firstclipnode;

			// Offset the test point appropriately for this hull.
			VectorSubtract_f ( pmove->origin, test, test);

			// Test the player's hull for intersection with this model
			if ( pmove->PM_HullPointContents (hull, num, test) == CONTENTS_EMPTY)
				continue;
			
			return pe;
		}
	}

	return NULL;
}



void PM_WaterJump (void)
{
	if ( pmove->waterjumptime > 10000 )
	{
		pmove->waterjumptime = 10000;
	}

	if ( !pmove->waterjumptime )
		return;

	pmove->waterjumptime -= pmove->cmd.msec;
	if ( pmove->waterjumptime < 0 ||
		 !pmove->waterlevel )
	{
		pmove->waterjumptime = 0;
		pmove->flags &= ~FL_WATERJUMP;
	}

	pmove->velocity[0] = pmove->movedir[0];
	pmove->velocity[1] = pmove->movedir[1];
}



/*
============
PM_PushEntity

Does not change the entities velocity at all
============
*/
pmtrace_t PM_PushEntity (vec3_t push)
{
	pmtrace_t	trace;
	vec3_t	end;
		
	VectorAdd_f (pmove->origin, push, end);

	trace = pmove->PM_PlayerTrace (pmove->origin, end, PM_NORMAL, -1 );
	
	VectorCopy_f (trace.endpos, pmove->origin);

	// So we can run impact function afterwards.
	if (trace.fraction < 1.0 &&
		!trace.allsolid)
	{
		PM_AddToTouched(trace, pmove->velocity);
	}

	return trace;
}	

/*
============
PM_Physics_Toss()

Dead player flying through air., e.g.
============
*/
void PM_Physics_Toss()
{
	pmtrace_t trace;
	vec3_t	move;
	float backoff;

	PM_CheckWater();

	if (pmove->velocity[2] > 0)
		pmove->onground = -1;

	// If on ground and not moving, return.
	if ( pmove->onground != -1 )
	{
		if (VectorCompare(pmove->basevelocity, vec3_origin) &&
		    VectorCompare(pmove->velocity, vec3_origin))
			return;
	}

	PM_CheckVelocity ();

// add gravity
	if ( pmove->movetype != MOVETYPE_FLY &&
		 pmove->movetype != MOVETYPE_BOUNCEMISSILE &&
		 pmove->movetype != MOVETYPE_FLYMISSILE )
		PM_AddGravity ();

// move origin
	// Base velocity is not properly accounted for since this entity will move again after the bounce without
	// taking it into account
	VectorAdd_f (pmove->velocity, pmove->basevelocity, pmove->velocity);
	
	PM_CheckVelocity();
	VectorScale (pmove->velocity, pmove->frametime, move);
	VectorSubtract_f (pmove->velocity, pmove->basevelocity, pmove->velocity);

	trace = PM_PushEntity (move);	// Should this clear basevelocity

	PM_CheckVelocity();

	if (trace.allsolid)
	{	
		// entity is trapped in another solid
		pmove->onground = trace.ent;
		VectorCopy_f (vec3_origin, pmove->velocity);
		return;
	}
	
	if (trace.fraction == 1)
	{
		PM_CheckWater();
		return;
	}


	if (pmove->movetype == MOVETYPE_BOUNCE)
		backoff = 2.0 - pmove->friction;
	else if (pmove->movetype == MOVETYPE_BOUNCEMISSILE)
		backoff = 2.0;
	else
		backoff = 1;

	PM_ClipVelocity (pmove->velocity, trace.plane.normal, pmove->velocity, backoff);

	// stop if on ground
	if (trace.plane.normal[2] > 0.7)
	{		
		float vel;
		vec3_t base;

		VectorClear_f( base );
		if (pmove->velocity[2] < pmove->movevars->gravity * pmove->frametime)
		{
			// we're rolling on the ground, add static friction.
			pmove->onground = trace.ent;
			pmove->velocity[2] = 0;
		}

		vel = DotProduct_f( pmove->velocity, pmove->velocity );

		// Con_DPrintf("%f %f: %.0f %.0f %.0f\n", vel, trace.fraction, ent->velocity[0], ent->velocity[1], ent->velocity[2] );

		if (vel < (30 * 30) || (pmove->movetype != MOVETYPE_BOUNCE && pmove->movetype != MOVETYPE_BOUNCEMISSILE))
		{
			pmove->onground = trace.ent;
			VectorCopy_f (vec3_origin, pmove->velocity);
		}
		else
		{
			VectorScale (pmove->velocity, (1.0 - trace.fraction) * pmove->frametime * 0.9, move);
			trace = PM_PushEntity (move);
		}
		//MODDD - missing a semicolon?  Evidence of using the '_f' version.
		VectorSubtract_f(pmove->velocity, base, pmove->velocity);
	}
	
// check for in water
	PM_CheckWater();
}

/*
====================
PM_NoClip

====================
*/
void PM_NoClip()
{
	int		i;
	vec3_t		wishvel;
	float	fmove, smove;
//	float	currentspeed, addspeed, accelspeed;


	//qboolean superNoclipSpeed;
	float noclipSpeedMult = 1;

	// Copy movement amounts

	//MODDD - check for noclip speed hack
	//superNoclipSpeed = atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "ncm" ) ) == 1 ? true : false;
	
	noclipSpeedMult = getSafeSqureRoot(atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "ncm" ) ));
	

	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;
	
	VectorNormalize ( pmove->forward ); 
	VectorNormalize ( pmove->right );


	for (i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
	{
		wishvel[i] = pmove->forward[i]*fmove*noclipSpeedMult + pmove->right[i]*smove*noclipSpeedMult;
	}
	wishvel[2] += pmove->cmd.upmove*noclipSpeedMult;

	/*
	if(!superNoclipSpeed){
		for (i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
		{
			wishvel[i] = pmove->forward[i]*fmove + pmove->right[i]*smove;
		}
		wishvel[2] += pmove->cmd.upmove;
	}else{
		for (i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
		{
			wishvel[i] = pmove->forward[i]*fmove*3 + pmove->right[i]*smove*3;
		}
		wishvel[2] += pmove->cmd.upmove*3;
	}
	*/


	VectorMA (pmove->origin, pmove->frametime, wishvel, pmove->origin);
	
	// Zero out the velocity so that we don't accumulate a huge downward velocity from
	//  gravity, etc.
	VectorClear_f( pmove->velocity );

}

//-----------------------------------------------------------------------------
// Purpose: Corrects bunny jumping ( where player initiates a bunny jump before other
//  movement logic runs, thus making onground == -1 thus making PM_Friction get skipped and
//  running PM_AirMove, which doesn't crop velocity to maxspeed like the ground / other
//  movement logic does.
//-----------------------------------------------------------------------------
void PM_PreventMegaBunnyJumping( void )
{

	// Current player speed
	float spd;
	// If we have to crop, apply this cropping fraction to velocity
	float fraction;
	// Speed at which bunny jumping is limited
	float maxscaledspeed;

	maxscaledspeed = BUNNYJUMP_MAX_SPEED_FACTOR * pmove->maxspeed;

	// Don't divide by zero
	if ( maxscaledspeed <= 0.0f )
		return;

	spd = Length( pmove->velocity );

	if ( spd <= maxscaledspeed )
		return;

	fraction = ( maxscaledspeed / spd ) * 0.65; //Returns the modifier for the velocity
	
	VectorScale( pmove->velocity, fraction, pmove->velocity ); //Crop it down!.
}

/*
=============
PM_Jump
=============
*/
void PM_Jump (void)
{
	int i;
	//MODDD
	float jumpForceMulti = 1;
	qboolean cansuperjump = false;
	int sv_player_midair_fix_val;

	//MODDD - new temp var.
	long random;

	if (pmove->dead)
	{
		pmove->oldbuttons |= IN_JUMP ;	// don't jump again until released
		return;
	}

	//MODDD - tfc check removed

	// See if we are waterjumping.  If so, decrement count and return.
	if ( pmove->waterjumptime )
	{
		pmove->waterjumptime -= pmove->cmd.msec;
		if (pmove->waterjumptime < 0)
		{
			pmove->waterjumptime = 0;
		}
		return;
	}

	// If we are in the water most of the way...
	if (pmove->waterlevel >= 2)
	{	// swimming, not jumping
		pmove->onground = -1;

		if (pmove->watertype == CONTENTS_WATER)    // We move up a certain amount
			pmove->velocity[2] = 100;
		else if (pmove->watertype == CONTENTS_SLIME)
			pmove->velocity[2] = 80;
		else  // LAVA
			pmove->velocity[2] = 50;

		// play swiming sound
		if ( pmove->flSwimTime <= 0 )
		{
			// Don't play sound again for 1 second
			pmove->flSwimTime = 1000;
			switch ( pmove->RandomLong( 0, 3 ) )
			{ 
			case 0:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			case 1:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			case 2:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade3.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			case 3:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade4.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			}
		}

		return;
	}





	//MODDD - exception granted?
	//if(ary_flUnstuckJumpTimer[pmove->player_index] > 0 && pmove->Sys_FloatTime() >= ary_flUnstuckJumpTimer[pmove->player_index]){



	sv_player_midair_fix_val = atoi(pmove->PM_Info_ValueForKey(pmove->physinfo, "maf"));

	if(sv_player_midair_fix_val == 1 && pmove->velocity[2] < 0 && pmove->velocity[2] > -60 && ary_iMidAirMoveBlocked[pmove->player_index] == TRUE && ary_iTallSlopeBelow[pmove->player_index] == TRUE){
		// allow the jump anyway to get unstuck!
	}else{

		// No more effect
		if ( pmove->onground == -1 )
		{
			// Flag that we jumped.
			// HACK HACK HACK
			// Remove this when the game .dll no longer does physics code!!!!
			pmove->oldbuttons |= IN_JUMP;	// don't jump again until released
			return;		// in air, so no effect
		}

	}






	if ( pmove->oldbuttons & IN_JUMP )
		return;		// don't pogo stick

	// In the air now.
    pmove->onground = -1;

	//MODDD - does removing this alone re-allow bunny hopping?
	//PM_PreventMegaBunnyJumping();

	//MODDD - tfc check removed.
	
	PM_PlayStepSound( PM_MapTextureTypeStepType( pmove->chtexturetype ), 1.0 );

	//MODDDREMOVE
	random = pmove->RandomLong(0, 1);
	if(random == 0){
		pmove->PM_PlaySound( CHAN_BODY, "player/pl_jump1.wav", 0.8, ATTN_NORM, 0, PITCH_NORM );
	}else if(random == 1){
		pmove->PM_PlaySound( CHAN_BODY, "player/pl_jump2.wav", 0.8, ATTN_NORM, 0, PITCH_NORM );
	}


	//MODDD TODO - would it  be safer to let pm_shared here change physics key "slj" to a value of 0 after this is performed?
	//Right now the player assumes that one frame is needed for this to pick up on the jump and run it, which seems to work anyways.

	// See if user can super long jump?
	cansuperjump = atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "slj" ) ) == 1 ? true : false;
	

	/*
	if(pmove->server){
		pmove->Con_DPrintf("S infostring?! %s\n",  pmove->physinfo   );
	}else{
		pmove->Con_DPrintf("C infostring?! %s\n",  pmove->physinfo   );
	}
	*/




	jumpForceMulti = getSafeSqureRoot(atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "jfm" ) ));

	// Acclerate upward
	// If we are ducking...
	if ( ( pmove->bInDuck ) || ( pmove->flags & FL_DUCKING ) )
	{
		// Adjust for super long jump module
		// UNDONE -- note this should be based on forward angles, not current velocity.
		

		//MODDD - condition altered:

		//if ( cansuperjump  )
		/*
		if ( cansuperjump &&
			( pmove->cmd.buttons & IN_DUCK ) &&
			( pmove->flDuckTime > 0 ) &&
			Length( pmove->velocity ) > 50 )
			*/

		/*
		if ( cansuperjump){
			pmove->Con_Printf("pm Length( pmove->velocity ) %.3f \n", Length( pmove->velocity ) );
		}
		*/

		if ( cansuperjump &&
			( pmove->cmd.buttons & IN_DUCK ) &&
			//( !(pmove->cmd.buttons & IN_JUMP) ) &&
			( pmove->flDuckTime >= 0 ) &&
			Length( pmove->velocity ) > 7
		)
			//velocity is ~6 when standing still?  Odd?
		{
			pmove->punchangle[0] = -5;

			for (i =0; i < 2; i++)
			{
				pmove->velocity[i] = pmove->forward[i] * PLAYER_LONGJUMP_SPEED * 1.6*jumpForceMulti;
			}
		
			pmove->velocity[2] = sqrt(2 * 800 * 56.0)*jumpForceMulti;
		}
		else
		{
			pmove->velocity[2] = sqrt(2 * 800 * 45.0)*jumpForceMulti;
		}
	}
	else
	{
		pmove->velocity[2] = sqrt(2 * 800 * 45.0)*jumpForceMulti;
	}

	// Decay it for simulation
	PM_FixupGravityVelocity();

	// Flag that we jumped.
	pmove->oldbuttons |= IN_JUMP;	// don't jump again until released

	//MODDD - flag that we jumped, stored until fall (not cleared before, unlike IN_JUMP)
	//pmove->oldbuttons |= FALLFROMJUMP;

	/*
	if(pmove->server){
		pmove->Con_DPrintf("S WE JUmPED!! %d\n", pmove->oldbuttons & FALLFROMJUMP);
	}else{
		pmove->Con_DPrintf("C WE JUmPED!! %d\n", pmove->oldbuttons & FALLFROMJUMP);
	}
	*/

	pmove->iuser4 |= FLAG_JUMPED;


}

/*
=============
PM_CheckWaterJump
=============
*/

void PM_CheckWaterJump (void)
{
	vec3_t	vecStart, vecEnd;
	vec3_t	flatforward;
	vec3_t	flatvelocity;
	float curspeed;
	pmtrace_t tr;
	int	savehull;
	//MODDD
	float jumpForceMulti;

	

	// Already water jumping.
	if ( pmove->waterjumptime )
		return;

	// Don't hop out if we just jumped in
	if ( pmove->velocity[2] < -180 )
		return; // only hop out if we are moving up


	jumpForceMulti = getSafeSqureRoot(atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "jfm" ) ));




	// See if we are backing up
	flatvelocity[0] = pmove->velocity[0];
	flatvelocity[1] = pmove->velocity[1];
	flatvelocity[2] = 0;

	// Must be moving
	curspeed = VectorNormalize( flatvelocity );
	
	// see if near an edge
	flatforward[0] = pmove->forward[0];
	flatforward[1] = pmove->forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	// Are we backing into water from steps or something?  If so, don't pop forward
	if ( curspeed != 0.0 && ( DotProduct_f( flatvelocity, flatforward ) < 0.0 ) )
		return;

	VectorCopy_f( pmove->origin, vecStart );
	vecStart[2] += WJ_HEIGHT;

	VectorMA ( vecStart, 24, flatforward, vecEnd );
	
	// Trace, this trace should use the point sized collision hull
	savehull = pmove->usehull;
	pmove->usehull = 2;
	tr = pmove->PM_PlayerTrace( vecStart, vecEnd, PM_NORMAL, -1 );
	if ( tr.fraction < 1.0 && fabs( tr.plane.normal[2] ) < 0.1f )  // Facing a near vertical wall?
	{
		vecStart[2] += pmove->player_maxs[ savehull ][2] - WJ_HEIGHT;
		VectorMA( vecStart, 24, flatforward, vecEnd );
		VectorMA( vec3_origin, -50, tr.plane.normal, pmove->movedir );

		//if jumpForceMulti is 1, use the game's normal check for water jumping.
		if(jumpForceMulti == 1){
			tr = pmove->PM_PlayerTrace( vecStart, vecEnd, PM_NORMAL, -1 );
			if ( tr.fraction == 1.0 )
			{
			
				pmove->waterjumptime = 2000;
				//MODDD - involve jumpForceMulti.
				pmove->velocity[2] = 225 * jumpForceMulti;
				pmove->oldbuttons |= IN_JUMP;
				pmove->flags |= FL_WATERJUMP;
			}

		}else{
			//otherwise, skip it.
			pmove->waterjumptime = 2000;
			//MODDD - involve jumpForceMulti.
			pmove->velocity[2] = 225 * jumpForceMulti;
			pmove->oldbuttons |= IN_JUMP;
			pmove->flags |= FL_WATERJUMP;
		}



	}

	// Reset the collision hull
	pmove->usehull = savehull;
}



void PM_CheckFalling( void )
{

	if ( pmove->onground != -1 &&
		 !pmove->dead &&
		 pmove->flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD )
	{
		float fvol = 0.5;

#if DEBUG_PRINTFALL == 1
		if(pmove->server){
			pmove->Con_Printf("FallVelocity SERVER: %.2f %d\n",  pmove->flFallVelocity, pmove->iuser4 & FLAG_JUMPED);
		}else{
			pmove->Con_Printf("FallVelocity CLIENT: %.2f %d\n",  pmove->flFallVelocity, pmove->iuser4 & FLAG_JUMPED);
		}
#endif
		
		

		if ( pmove->waterlevel > 0 )
		{
		}
		else if ( pmove->flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED )
		{
#if DEBUG_PRINTFALL == 1
			pmove->Con_Printf("FALL BRACKET 4 %.2f\n", pmove->flFallVelocity);
#endif
			// NOTE:  In the original game dll , there were no breaks after these cases, causing the first one to 
			// cascade into the second
			//switch ( RandomLong(0,1) )
			//{
			//case 0:
				//pmove->PM_PlaySound( CHAN_VOICE, "player/pl_fallpain2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				//break;
			//case 1:
				
				pmove->PM_PlaySound( CHAN_VOICE, "player/pl_fallpain3.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				
				//MODDDREMOVE
				//pmove->punchangle[ 2 ] = pmove->flFallVelocity * 0.013;	// punch z axis
				pmove->punchangle[ 0 ] = pmove->flFallVelocity * 0.013;	// punch z axis
				//??? PAY ATTENTION THERE...


			//	break;
			//}
			fvol = 1.0;
		}
		else if ( pmove->flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED / 2 )
		{
#if DEBUG_PRINTFALL == 1
			pmove->Con_Printf("FALL BRACKET 3 %.2f\n", pmove->flFallVelocity);
#endif

			//MODDD - glock disabled, re-declare 'tfc' to see again.   Although just playing pl_fallpain3?
			// Why did this require being tf?
			/*
			tfc = atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "tfc" ) ) == 1 ? true : false;

			if ( tfc )
			{
				pmove->PM_PlaySound( CHAN_VOICE, "player/pl_fallpain3.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			}
			*/

			fvol = 0.85;
		}
		else if(pmove->flFallVelocity > PLAYER_FALL_PUNCH_THRESHHOLD){
			//leave "fvol" at 0.5.  This would normally mean none of these if-thens get called.
#if DEBUG_PRINTFALL == 1
			pmove->Con_Printf("FALL BRACKET 2 %.2f\n", pmove->flFallVelocity);
#endif
			//FALLFROMJUMP ?
			if( !(pmove->iuser4 & FLAG_JUMPED)){
					//UNSURE.  is this okay?
					pmove->punchangle[ 0 ] = pmove->flFallVelocity * 0.009;	// punch z axis  ... no, that is the x axis.  leftover comment?
					//fvol = 0;
			}
		}
		//NOTE: eh, what?
		else if ( pmove->flFallVelocity < PLAYER_MIN_BOUNCE_SPEED )
		{
#if DEBUG_PRINTFALL == 1
			pmove->Con_Printf("FALL BRACKET 1 %.2f\n", pmove->flFallVelocity);
#endif
			fvol = 0;
		}
		

		if ( fvol > 0.0 )
		{
			// Play landing step right away
			pmove->flTimeStepSound = 0;
			
			PM_UpdateStepSound();
			
			// play step sound for current texture
			PM_PlayStepSound( PM_MapTextureTypeStepType( pmove->chtexturetype ), fvol );

			// Knock the screen around a little bit, temporary effect
			
			//MODDDREMOVE - the line below (2nd) used to be this (1st):
			//pmove->punchangle[ 2 ] = pmove->flFallVelocity * 0.013;	// punch z axis
			//pmove->punchangle[ 0 ] = pmove->flFallVelocity * 0.013;   //...and yet this was still found commented out too?

			if ( pmove->punchangle[ 0 ] > 8 )
			{
				pmove->punchangle[ 0 ] = 8;
			}
		}

		//MODDDREMOVE - ?  this if statement and its contents are completely new.
		//Hey, this is kinda sloppily added.  It doesn't meld into the above, so this may coincide with other punches above. change that?
		if ( pmove->flFallVelocity >= 350 )
		{
			pmove->punchangle[ 0 ] = pmove->flFallVelocity * 0.013;	// punch z axis  ... no, that is the x axis.  leftover comment?
			pmove->PM_PlaySound( CHAN_BODY, "player/pl_jumpland2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
		}

		//MODDD
		//if(pmove->flFallVelocity != 0){
			//pmove->oldbuttons &= ~FALLFROMJUMP;
		//}
		pmove->iuser4 &= (~FLAG_JUMPED);
	}

	if ( pmove->onground != -1 ) 
	{
		pmove->flFallVelocity = 0;
	}

}

/*
=================
PM_PlayWaterSounds

=================
*/
void PM_PlayWaterSounds( void )
{
	// Did we enter or leave water?
	if  ( ( pmove->oldwaterlevel == 0 && pmove->waterlevel != 0 ) ||
		  ( pmove->oldwaterlevel != 0 && pmove->waterlevel == 0 ) )
	{
		switch ( pmove->RandomLong(0,3) )
		{
		case 0:
			pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			break;
		case 1:
			pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			break;
		case 2:
			pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade3.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			break;
		case 3:
			pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade4.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			break;
		}
	}
}

/*
===============
PM_CalcRoll

===============
*/
float PM_CalcRoll (vec3_t angles, vec3_t velocity, float rollangle, float rollspeed )
{
    float   sign;
    float   side;
    float   value;
	vec3_t  forward, right, up;
    
	AngleVectors (angles, forward, right, up);
    
	side = DotProduct_f (velocity, right);
    
	sign = side < 0 ? -1 : 1;
    
	side = fabs(side);
    
	value = rollangle;
    
	if (side < rollspeed)
	{
		side = side * value / rollspeed;
	}
    else
	{
		side = value;
	}
  
	return side * sign;
}

/*
=============
PM_DropPunchAngle

=============
*/
void PM_DropPunchAngle ( vec3_t punchangle )
{
	float len;
	
	len = VectorNormalize ( punchangle );
	len -= (10.0 + len * 0.5) * pmove->frametime;
	len = max( len, 0.0 );
	VectorScale ( punchangle, len, punchangle);
}

/*
==============
PM_CheckParamters

==============
*/

//MODDD - TODO:::inspect for the continuous punch - add issue?  Nitpick, but oh well.
void PM_CheckParamters( void )
{
	float spd;
	float maxspeed;
	vec3_t	v_angle;

	spd = ( pmove->cmd.forwardmove * pmove->cmd.forwardmove ) +
		  ( pmove->cmd.sidemove * pmove->cmd.sidemove ) +
		  ( pmove->cmd.upmove * pmove->cmd.upmove );
	spd = sqrt( spd );

	maxspeed = pmove->clientmaxspeed; //atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "maxspd" ) );
	if ( maxspeed != 0.0 )
	{
		pmove->maxspeed = min( maxspeed, pmove->maxspeed );
	}

	if ( ( spd != 0.0 ) &&
		 ( spd > pmove->maxspeed ) )
	{
		float fRatio = pmove->maxspeed / spd;
		pmove->cmd.forwardmove *= fRatio;
		pmove->cmd.sidemove    *= fRatio;
		pmove->cmd.upmove      *= fRatio;
	}

	if ( pmove->flags & FL_FROZEN ||
		 pmove->flags & FL_ONTRAIN || 
		 pmove->dead )
	{
		pmove->cmd.forwardmove = 0;
		pmove->cmd.sidemove    = 0;
		pmove->cmd.upmove      = 0;
	}


	PM_DropPunchAngle( pmove->punchangle );

	// Take angles from command.
	if ( !pmove->dead )
	{
		VectorCopy_f ( pmove->cmd.viewangles, v_angle );  

		//MODDD - REMOVED.  Deemed unnecessary.  view.cpp already does this just fine.  COME HERE IF THERE ARE ANY BUGS THAT RESULT FROM THIS.
		//VectorAdd_f( v_angle, pmove->punchangle, v_angle );

		/*
		if(pmove->server){
			pmove->Con_DPrintf("S444 %d\n", 4);
		}else{
			pmove->Con_DPrintf("C444 %d\n", 4);
		}
		*/

		// Set up view angles.
		pmove->angles[ROLL]	=	PM_CalcRoll ( v_angle, pmove->velocity, pmove->movevars->rollangle, pmove->movevars->rollspeed )*4;
		pmove->angles[PITCH] =	v_angle[PITCH];
		pmove->angles[YAW]   =	v_angle[YAW];
	}
	else
	{
		VectorCopy_f( pmove->oldangles, pmove->angles );
	}

	// Set dead player view_offset
	if ( pmove->dead )
	{
		//MODDD - DEAD CAMERA SETTING
		pmove->view_ofs[2] = PM_DEAD_VIEWHEIGHT;
	}

	// Adjust client view angles to match values used on server.
	if (pmove->angles[YAW] > 180.0f)
	{
		pmove->angles[YAW] -= 360.0f;
	}

}

void PM_ReduceTimers( void )
{
	if ( pmove->flTimeStepSound > 0 )
	{
		pmove->flTimeStepSound -= pmove->cmd.msec;
		if ( pmove->flTimeStepSound < 0 )
		{
			pmove->flTimeStepSound = 0;
		}
	}
	if ( pmove->flDuckTime > 0 )
	{
		pmove->flDuckTime -= pmove->cmd.msec;
		if ( pmove->flDuckTime < 0 )
		{
			pmove->flDuckTime = 0;
		}
	}
	if ( pmove->flSwimTime > 0 )
	{
		pmove->flSwimTime -= pmove->cmd.msec;
		if ( pmove->flSwimTime < 0 )
		{
			pmove->flSwimTime = 0;
		}
	}
}

/*
=============
PlayerMove

Returns with origin, angles, and velocity modified in place.

Numtouch and touchindex[] will be set if any of the physents
were contacted during the move.
=============
*/

//NOTICE: throughout this method, "pLadder" has been given to calls to "PM_CatagorizePosition".
//Knowing whether the player is on the ladder or not can fix a bug with failing to break the bond with the ground.
void PM_PlayerMove ( qboolean server )
{
	physent_t *pLadder = NULL;
	float pushSpeedMult; //MODDD - new temp var

	// Are we running server code?
	pmove->server = server;                

	
	// Adjust speeds etc.
	PM_CheckParamters();

	// Assume we don't touch anything
	pmove->numtouch = 0;                    

	// # of msec to apply movement
	pmove->frametime = pmove->cmd.msec * 0.001;    

	PM_ReduceTimers();

	// Convert view angles to vectors
	AngleVectors (pmove->angles, pmove->forward, pmove->right, pmove->up);

	// PM_ShowClipBox();

	// Special handling for spectator and observers. (iuser1 is set if the player's in observer mode)
	if ( pmove->spectator || pmove->iuser1 > 0 )
	{
		PM_SpectatorMove();
		PM_CatagorizePosition(pLadder);
		return;
	}

	// Always try and unstick us unless we are in NOCLIP mode
	if ( pmove->movetype != MOVETYPE_NOCLIP && pmove->movetype != MOVETYPE_NONE )
	{
		if ( PM_CheckStuck() )
		{
			return;  // Can't move, we're stuck
		}
	}

	// Now that we are "unstuck", see where we are ( waterlevel and type, pmove->onground ).
	PM_CatagorizePosition(pLadder);

	// Store off the starting water level
	pmove->oldwaterlevel = pmove->waterlevel;

	// If we are not on ground, store off how fast we are moving down
	if ( pmove->onground == -1 )
	{
		pmove->flFallVelocity = -pmove->velocity[2];
	}

	g_onladder = 0;
	// Don't run ladder code if dead or on a train
	if ( !pmove->dead && !(pmove->flags & FL_ONTRAIN) )
	{
		pLadder = PM_Ladder();
		if ( pLadder )
		{
			g_onladder = 1;
		}
	}

	//MODDD - why wasn't UpdateStepSound after LadderMove below?
	// Was here.

	PM_Duck();
	
	// Don't run ladder code if dead or on a train
	if ( !pmove->dead && !(pmove->flags & FL_ONTRAIN) )
	{
		//MODDD - don't stick to a ladder if recently jumped off!
		if ( pLadder && ary_iDenyLadderFrames[pmove->player_index] <= 0)
		{

			if (pmove->movetype != MOVETYPE_FLY) {
				//MODDD - reset this then, don't allow jumping off a ladder too soon since
				// getting on
				ary_iJumpOffDenyLadderFrames[pmove->player_index] = 9;
			}

			PM_LadderMove( pLadder );
		}
		//MODDD - little interjection...
		else if ( pmove->movetype != MOVETYPE_WALK &&
			      pmove->movetype != MOVETYPE_NOCLIP )
		{
			// Clear ladder stuff unless player is noclipping
			//  it will be set immediately again next frame if necessary
			pmove->movetype = MOVETYPE_WALK;
		}
	}

	PM_UpdateStepSound();


	
	// Slow down, I'm pulling it! (a box maybe) but only when I'm standing on ground
	//MODDD - behavior changed. Instead, hold down use does NOT cause a base slowdown of always 30% like this (effectively friction, more effort
	//        into movement gets sucked away to look slower).
	//        But not all crates should slow you down equally, right?
	//        In the new system, a crate's weight (friction) affects how much it slows the player pushing or use'ing on it.
	//        The box otherwise moves consistently with the player's velocity, but forces the player to slow down while use'ing on it,
	//        or forces the player to slow down if pushing the box.
	//        This can be done by setting a physics flag on the player when a box makes contact with the player (physical touch-push) or is use'd by
	//        the player.  So a heavier box will slow the player down much more on being use'd or physically touch-pushed, but a lighter one will
	//        not affect the player much while being moved.
	/*
	if ( ( pmove->onground != -1 ) && ( pmove->cmd.buttons & IN_USE) )
	{
		VectorScale( pmove->velocity, 0.3, pmove->velocity );
	}
	*/

	//MODDD - NEW WAY. Is the push speed modifier physics flag set?
	//pushSpeedMult = getSafeSqureRoot(atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "psm" ) ));
	pushSpeedMult = atof(pmove->PM_Info_ValueForKey( pmove->physinfo, "psm" ));
	if(pushSpeedMult != 1){
		//no, apply to the X and Y coordinates only.
		//VectorScale( pmove->velocity, pushSpeedMult, pmove->velocity  );

		pmove->velocity[0] = pmove->velocity[0]*pushSpeedMult;
		pmove->velocity[1] = pmove->velocity[1]*pushSpeedMult;
	}
	//pmove->ConD//pushSpeedMult

	/*
	if(pmove->server){
		// Con_Printf or Con_DPrintf ???
		pmove->Con_Printf("S psm:%.2f\n",  pushSpeedMult   );
	}else{
		pmove->Con_Printf("C psm:%.2f\n",  pushSpeedMult   );
	}
	*/
	

	// Handle movement
	switch ( pmove->movetype )
	{
	default:
		pmove->Con_DPrintf("Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", pmove->movetype, pmove->server);
		break;

	case MOVETYPE_NONE:
		break;

	case MOVETYPE_NOCLIP:
		PM_NoClip();
		break;

	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
		PM_Physics_Toss();
		break;

	case MOVETYPE_FLY:
	
		PM_CheckWater();

		// Was jump button pressed?
		// If so, set velocity to 270 away from ladder.  This is currently wrong.
		// Also, set MOVE_TYPE to walk, too.
		if ( pmove->cmd.buttons & IN_JUMP )
		{
			if ( !pLadder )
			{
				PM_Jump ();
			}
		}
		else
		{
			pmove->oldbuttons &= ~IN_JUMP;
		}
		
		// Perform the move accounting for any base velocity.
		VectorAdd_f (pmove->velocity, pmove->basevelocity, pmove->velocity);
		PM_FlyMove ();
		VectorSubtract_f (pmove->velocity, pmove->basevelocity, pmove->velocity);
		break;

	case MOVETYPE_WALK:
		if ( !PM_InWater() )
		{
			PM_AddCorrectGravity();
		}

		// If we are leaping out of the water, just update the counters.
		if ( pmove->waterjumptime )
		{
			PM_WaterJump();
			PM_FlyMove();

			// Make sure waterlevel is set correctly
			PM_CheckWater();
			return;
		}

		// If we are swimming in the water, see if we are nudging against a place we can jump up out
		//  of, and, if so, start out jump.  Otherwise, if we are not moving up, then reset jump timer to 0
		if ( pmove->waterlevel >= 2 ) 
		{
			if ( pmove->waterlevel == 2 )
			{
				PM_CheckWaterJump();
			}

			// If we are falling again, then we must not trying to jump out of water any more.
			if ( pmove->velocity[2] < 0 && pmove->waterjumptime )
			{
				pmove->waterjumptime = 0;
			}

			// Was jump button pressed?
			if (pmove->cmd.buttons & IN_JUMP)
			{
				PM_Jump ();
			}
			else
			{
				pmove->oldbuttons &= ~IN_JUMP;
			}

			// Perform regular water movement
			PM_WaterMove();
			
			VectorSubtract_f (pmove->velocity, pmove->basevelocity, pmove->velocity);

			// Get a final position
			PM_CatagorizePosition(pLadder);
		}
		else

		// Not underwater
		{
			// Was jump button pressed?
			if ( pmove->cmd.buttons & IN_JUMP )
			{
				if ( !pLadder )
				{
					PM_Jump ();
				}
			}
			else
			{
				pmove->oldbuttons &= ~IN_JUMP;
			}

			// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor, 
			//  we don't slow when standing still, relative to the conveyor.
			if ( pmove->onground != -1 )
			{
				pmove->velocity[2] = 0.0;
				PM_Friction();
			}

			// Make sure velocity is valid.
			PM_CheckVelocity();

			// Are we on ground now
			if ( pmove->onground != -1 )
			{
				PM_WalkMove();
			}
			else
			{
				PM_AirMove();  // Take into account movement when in air.
			}

			// Set final flags.
			PM_CatagorizePosition(pLadder);

			// Now pull the base velocity back out.
			// Base velocity is set if you are on a moving object, like
			//  a conveyor (or maybe another monster?)
			VectorSubtract_f (pmove->velocity, pmove->basevelocity, pmove->velocity );
				
			// Make sure velocity is valid.
			PM_CheckVelocity();

			// Add any remaining gravitational component.
			if ( !PM_InWater() )
			{
				PM_FixupGravityVelocity();
			}

			// If we are on ground, no downward velocity.
			if ( pmove->onground != -1 )
			{
				pmove->velocity[2] = 0;
			}

			// See if we landed on the ground with enough force to play
			//  a landing sound.
			PM_CheckFalling();
		}

		// Did we enter or leave the water?
		PM_PlayWaterSounds();
		break;
	}
}

void PM_CreateStuckTable( void )
{
	float x, y, z;
	int idx;
	int i;
	float zi[3];

	memset(rgv3tStuckTable, 0, 54 * sizeof(vec3_t));

	idx = 0;
	// Little Moves.
	x = y = 0;
	// Z moves
	for (z = -0.125 ; z <= 0.125 ; z += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	x = z = 0;
	// Y moves
	for (y = -0.125 ; y <= 0.125 ; y += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for (x = -0.125 ; x <= 0.125 ; x += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for ( x = - 0.125; x <= 0.125; x += 0.250 )
	{
		for ( y = - 0.125; y <= 0.125; y += 0.250 )
		{
			for ( z = - 0.125; z <= 0.125; z += 0.250 )
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}

	// Big Moves.
	x = y = 0;
	zi[0] = 0.0f;
	zi[1] = 1.0f;
	zi[2] = 6.0f;

	for (i = 0; i < 3; i++)
	{
		// Z moves
		z = zi[i];
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	x = z = 0;

	// Y moves
	for (y = -2.0f ; y <= 2.0f ; y += 2.0)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for (x = -2.0f ; x <= 2.0f ; x += 2.0f)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for (i = 0 ; i < 3; i++)
	{
		z = zi[i];
		
		for (x = -2.0f ; x <= 2.0f ; x += 2.0f)
		{
			for (y = -2.0f ; y <= 2.0f ; y += 2.0)
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}
}



/*
This modume implements the shared player physics code between any particular game and 
the engine.  The same PM_Move routine is built into the game .dll and the client .dll and is
invoked by each side as appropriate.  There should be no distinction, internally, between server
and client.  This will ensure that prediction behaves appropriately.
*/


/*
C WE JUmPED!! \hl\1\slj\0\ncm\5.000000\nsm\3.000000\jfm\3.000000\plm\1
S WE JUmPED!! \hl\1\slj\0\ncm\5.000000\nsm\3.000000\jfm\3.000000\plm\1
C WE JUmPED!! \hl\1\ncm\5.000000\nsm\3.000000\jfm\3.000000\plm\1\slj\1
S WE JUmPED!! \hl\1\ncm\5.000000\nsm\3.000000\jfm\3.000000\plm\1\slj\1
*/

void CUSTOM_setPhysicsKey(const char* physString, const char* keyToChange, const char* newValue){

	
	const char* attemptLoc = strstr(physString, keyToChange);
	int attemptLocIndex;
	int slashStart;
	char charAttt;


	if(attemptLoc == NULL){
		//key not found? Fail. Nothing can be done.
		return;
	}

	attemptLocIndex = attemptLoc - physString;

	
	slashStart = attemptLocIndex + strlen(keyToChange);


	if(pmove->server){
		pmove->Con_DPrintf("S infostring?! %s\n",  pmove->physinfo   );
	}else{
		pmove->Con_DPrintf("C infostring?! %s\n",  pmove->physinfo   );
	}


	charAttt = physString[slashStart];

	if(physString[slashStart] == '\\'){
		//should land right on the slash already.
		
		int physLength = strlen(physString);
		int slashEnd = physLength;  //in case we can't find the end slash, there isn't a slash after the last key value.
		//find it.
		int i;
		for(i = slashStart+1; i < physLength; i++){
			if(physString[i] == '\\'){
				//found it!
				slashEnd = i;
				break;
			}else{
				continue; //look more.
			}
		}

		if(slashEnd != -1){
			//have a slashStart and slashEnd. write between them.
			//Going to assume the length does not need changing for simplicity.
			int writeStart = slashStart + 1;
			int writeEnd = slashEnd - 1;  //inclusive limit.

			strncpy( (char*)&physString[writeStart], newValue, strlen(newValue) );
			
			if(pmove->server){
				pmove->Con_DPrintf("S NEWI?! %s\n",  pmove->physinfo   );
			}else{
				pmove->Con_DPrintf("C NEWI?! %s\n",  pmove->physinfo   );
			}
		}

	}
	


}//END OF CUSTOM_setPhyiscsKey



void PM_Move ( struct playermove_s *ppmove, int server )
{

	/*
	if(1 == 1){
		return;
	}
	*/
	assert( pm_shared_initialized );

	pmove = ppmove;


	// keep up to date?
	ary_flGravityModMulti[pmove->player_index] = atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "gmm" ) );


	// must be set TRUE throughout the frame to count
	//ary_iMidAirMoveBlocked[pmove->player_index] = FALSE;
	ary_iTallSlopeBelow[pmove->player_index] = FALSE;
	/*
	if(ary_flUnstuckJumpTimer[pmove->player_index] > 0){
		if(pmove->velocity[2] > 0 || pmove->velocity[2] < -20){
			// significant change in Z vel?  Stop the timer
			ary_flUnstuckJumpTimer[pmove->player_index] = 0;
		}
	}
	*/






	//MODDD - do a check. If the "res" physics flag is on ("1"), reset fall velocity and set that flag back to "0".

	//pmove->PM_Info_ValueForKey( pmove->physinfo, "slj" )

	
	if( !strcmp(pmove->PM_Info_ValueForKey( pmove->physinfo, "res" ), "1")){
		pmove->flFallVelocity = 0;

		/*
		if(pmove->server){
			pmove->Con_DPrintf("S HA... yAYYYYY!\n");
		}else{
			pmove->Con_DPrintf("C HA... yAYYYYY!\n");
		}
		*/

		//CUSTOM_setPhysicsKey( pmove->physinfo, "res", "0");
		//This stuff isn't working. Try a fuser flag.
		
		pmove->iuser4 |= FLAG_RESET_RECEIVED;

	}



	//MODDD - count this down if needed
	if (ary_iJumpOffDenyLadderFrames[pmove->player_index] > 0) {
		ary_iJumpOffDenyLadderFrames[pmove->player_index] -= 1;
	}
	if (ary_iDenyLadderFrames[pmove->player_index] > 0) {
		ary_iDenyLadderFrames[pmove->player_index] -= 1;
	}
	
	PM_PlayerMove( ( server != 0 ) ? true : false );

	if ( pmove->onground != -1 )
	{
		pmove->flags |= FL_ONGROUND;
	}
	else
	{
		pmove->flags &= ~FL_ONGROUND;
	}

	// In single player, reset friction after each movement to FrictionModifier Triggers work still.
	if ( !pmove->multiplayer && ( pmove->movetype == MOVETYPE_WALK  ) )
	{
		pmove->friction = 1.0f;
	}
}

int PM_GetVisEntInfo( int ent )
{
	if ( ent >= 0 && ent <= pmove->numvisent )
	{
		return pmove->visents[ ent ].info;
	}
	return -1;
}

int PM_GetPhysEntInfo( int ent )
{
	if ( ent >= 0 && ent <= pmove->numphysent)
	{
		return pmove->physents[ ent ].info;
	}
	return -1;
}

void PM_Init( struct playermove_s *ppmove )
{
	assert( !pm_shared_initialized );

	pmove = ppmove;

	PM_CreateStuckTable();
	PM_InitTextureTypes();

	pm_shared_initialized = 1;
}
