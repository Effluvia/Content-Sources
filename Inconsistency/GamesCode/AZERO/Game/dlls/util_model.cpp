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

#include "external_lib_include.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>

#include <string.h>

//MODDD - why??  All this file did was include unistd.h if this wasn't windows.
// File removed.  That's handled by external_lib_include.h anyway.
//#include "../common/nowin.h"


// hack into header files that we can ship
//MODDD - NOTE.     what... on earth was that comment supposed to mean.

#include "const.h"

//MODDD - what?  But we include pm_shared/pm_math, who's header is common/mathlib.h,
// utils/common/mathlib.h is the header for utils/common/mathlib.c, which we don't include.
// AT LEAST THAT'S NOT CONFUSING.
// Changing to mathlib.h (implies common/mathlib.h) like everything else.
//#include "../utils/common/mathlib.h"
//#include "mathlib.h"
// Nevermind!  Being included by const.h now, just go everwhere, jeez.


#include "util_shared.h"
#include "progdefs.h"
#include "edict.h"
#include "eiface.h"

#include "studio.h"

#include "util_printout.h"

#include "activity.h"
#include "activitymap.h"
#include "util_model.h"
#include "scriptevent.h"
#include "enginecallback.h"



//Prototyped. Inner method for this file (animation.cpp) only.
void printHitboxInfo(mstudiobbox_t* thisHitbox, int id);




char* UTIL_VarArgsANIMATION( char *format, ... )
{
	va_list		argptr;
	static char arychr_buffer[1024];
	
	va_start (argptr, format);
	vsprintf (arychr_buffer, format,argptr);
	va_end (argptr);

	return arychr_buffer;	
}
//This is essentially "UTIL_VarArgs" that accepts a "va_list" argument instead.  How it is applied involes the sender (wherever the call to this
//method is made) having responsibility for having "va_start" before calling this method, and "va_end" afterwards.
char* UTIL_VarArgsVAANIMATION( const char *format, va_list argptr )
{
	//va_list		argptr;
	static char arychr_buffer[1024];
	
	//va_start (argptr, format);
	vsprintf (arychr_buffer, format,argptr);
	//va_end (argptr);

	return arychr_buffer;	
}

inline void easyPrintLineANIMATION(const char *format, ...){
	va_list argptr;
	va_start(argptr, format);
	g_engfuncs.pfnServerPrint( UTIL_VarArgsANIMATION( "%s\n", UTIL_VarArgsVAANIMATION(format, argptr ) )  );
	va_end(argptr);

	//MODDD - TODO - would  this make more sense?
	/*
	va_list argptr;
	va_start(argptr, format);
	g_engfuncs.pfnServerPrint( UTIL_VarArgsVA(format, argptr ) );
	g_engfuncs.pfnServerPrint("\n");
	va_end(argptr);
	*/
}


void printHitboxInfo(mstudiobbox_t* thisHitbox, int id){
	easyForcePrintLine("Hitbox #%d min:(%.2f, %.2f, %.2f) max:(%.2f, %.2f, %.2f) bone:%d group:%d", 
		id,
		thisHitbox->bbmin[0],
		thisHitbox->bbmin[1],
		thisHitbox->bbmin[2],
		thisHitbox->bbmax[0],
		thisHitbox->bbmax[1],
		thisHitbox->bbmax[2],
		thisHitbox->bone,
		thisHitbox->group
	);
}






int ExtractBbox( void *pmodel, int sequence, float *mins, float *maxs )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return 0;

	mstudioseqdesc_t	*pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);
	
	mins[0] = pseqdesc[ sequence ].bbmin[0];
	mins[1] = pseqdesc[ sequence ].bbmin[1];
	mins[2] = pseqdesc[ sequence ].bbmin[2];

	maxs[0] = pseqdesc[ sequence ].bbmax[0];
	maxs[1] = pseqdesc[ sequence ].bbmax[1];
	maxs[2] = pseqdesc[ sequence ].bbmax[2];

	return 1;
}


int LookupActivity( void *pmodel, entvars_t *pev, int activity )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return 0;

	mstudioseqdesc_t	*pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);

	int weighttotal = 0;
	int seq = ACTIVITY_NOT_AVAILABLE;
	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (pseqdesc[i].activity == activity)
		{
			weighttotal += pseqdesc[i].actweight;
			if (!weighttotal || RANDOM_LONG(0,weighttotal-1) < pseqdesc[i].actweight)
				seq = i;
		}
	}

	if (seq == -1 && activity != 23) {
		// Breakpoint?
		int x = 45;
	}

	return seq;
}


int LookupActivityHeaviest( void *pmodel, entvars_t *pev, int activity )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if ( !pstudiohdr )
		return 0;

	mstudioseqdesc_t	*pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);

	int weight = 0;
	int seq = ACTIVITY_NOT_AVAILABLE;
	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (pseqdesc[i].activity == activity)
		{
			if ( pseqdesc[i].actweight > weight )
			{
				weight = pseqdesc[i].actweight;
				seq = i;
			}
		}
	}
	return seq;
}

void GetEyePosition ( void *pmodel, float *vecEyePosition )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;

	if ( !pstudiohdr )
	{
		ALERT ( at_console, "GetEyePosition() Can't get pstudiohdr ptr!\n" );
		return;
	}

	VectorCopy_f ( pstudiohdr->eyeposition, vecEyePosition );
}

int LookupSequence( void *pmodel, const char *label )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return 0;

	mstudioseqdesc_t	*pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);

	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (stricmp( pseqdesc[i].label, label ) == 0)
			return i;
	}

	return -1;
}


int IsSoundEvent( int eventNumber )
{
	if ( eventNumber == SCRIPT_EVENT_SOUND || eventNumber == SCRIPT_EVENT_SOUND_VOICE )
		return 1;
	return 0;
}





//MODDD - print out the sounds in all sequences belonging to a given model.
void Sequence_PrintSound_All(void* pmodel) {
	studiohdr_t* pstudiohdr;

	pstudiohdr = (studiohdr_t*)pmodel;

	for (int index = 0; index < pstudiohdr->numseq; index++) {

		Sequence_PrintSound(pmodel, index);
	}
}

//MODDD - version of SequencePrecache that accepts sequence index here, and prints out any sounds mentioned by the sequence.
void Sequence_PrintSound(void* pmodel, int index) {
	studiohdr_t* pstudiohdr;

	pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr || index >= pstudiohdr->numseq)
		return;

	mstudioseqdesc_t* pseqdesc;
	mstudioevent_t* pevent;

	pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex) + index;
	pevent = (mstudioevent_t*)((byte*)pstudiohdr + pseqdesc->eventindex);

	for (int i = 0; i < pseqdesc->numevents; i++)
	{
		// Don't send client-side events to the server AI
		if (pevent[i].event >= EVENT_CLIENT) {
			continue;
		}

		// UNDONE: Add a callback to check to see if a sound is precached yet and don't allocate a copy
		// of it's name if it is.
		if (IsSoundEvent(pevent[i].event))
		{
			if (!strlen(pevent[i].options))
			{
				ALERT(at_error, "Bad sound event %d in sequence %s::seqIndex:%d, label:%s (sound is \"%s\")\n", pevent[i].event, pstudiohdr->name, index, pseqdesc->label, pevent[i].options);
			}
			else {
				//MODDD - let me know.
				ALERT(at_console, "Sound event found %d in sequence %s::seqIndex:%d, label:%s (sound is \"%s\")\n", pevent[i].event, pstudiohdr->name, index, pseqdesc->label, pevent[i].options);
			}
			// don't do anything else, the printout is all
		}
	}
}

//MODDD - version of SequencePrecache that accepts sequence index here too.
void SequencePrecache(void* pmodel, int index) {
	studiohdr_t* pstudiohdr;

	pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr || index >= pstudiohdr->numseq)
		return;

	mstudioseqdesc_t* pseqdesc;
	mstudioevent_t* pevent;

	pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex) + index;
	pevent = (mstudioevent_t*)((byte*)pstudiohdr + pseqdesc->eventindex);

	for (int i = 0; i < pseqdesc->numevents; i++)
	{
		// Don't send client-side events to the server AI
		if (pevent[i].event >= EVENT_CLIENT) {
			continue;
		}

		// UNDONE: Add a callback to check to see if a sound is precached yet and don't allocate a copy
		// of it's name if it is.
		if (IsSoundEvent(pevent[i].event))
		{
			if (!strlen(pevent[i].options))
			{
				//MODDD - does ->label even work?
				ALERT(at_error, "Bad sound event %d in sequence %s::seqIndex:%d (sound is \"%s\")\n", pevent[i].event, pstudiohdr->name, index, pevent[i].options);
			}

			PRECACHE_SOUND((char*)(gpGlobals->pStringBase + ALLOC_STRING(pevent[i].options)));
		}
	}
}

void SequencePrecache( void *pmodel, const char *pSequenceName ){
	int index = LookupSequence( pmodel, pSequenceName );
	if ( index >= 0 )
	{
		studiohdr_t *pstudiohdr;
	
		pstudiohdr = (studiohdr_t *)pmodel;
		if ( !pstudiohdr || index >= pstudiohdr->numseq )
			return;

		mstudioseqdesc_t	*pseqdesc;
		mstudioevent_t		*pevent;

		pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + index;
		pevent = (mstudioevent_t *)((byte *)pstudiohdr + pseqdesc->eventindex);

		for (int i = 0; i < pseqdesc->numevents; i++)
		{
			// Don't send client-side events to the server AI
			if (pevent[i].event >= EVENT_CLIENT) {
				continue;
			}

			// UNDONE: Add a callback to check to see if a sound is precached yet and don't allocate a copy
			// of it's name if it is.
			if ( IsSoundEvent( pevent[i].event ) )
			{
				if ( !strlen(pevent[i].options) )
				{
					ALERT( at_error, "Bad sound event %d in sequence %s::%s (sound is \"%s\")\n", pevent[i].event, pstudiohdr->name, pSequenceName, pevent[i].options );
				}

				PRECACHE_SOUND( (char *)(gpGlobals->pStringBase + ALLOC_STRING(pevent[i].options) ) );
			}
		}
	}
}


void GetSequenceInfo( void *pmodel, entvars_t *pev, float *pflFrameRate, float *pflGroundSpeed )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return;

	mstudioseqdesc_t	*pseqdesc;

	if (pev->sequence >= pstudiohdr->numseq)
	{
		*pflFrameRate = 0.0;
		*pflGroundSpeed = 0.0;
		return;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		*pflGroundSpeed = sqrt( pseqdesc->linearmovement[0]*pseqdesc->linearmovement[0]+ pseqdesc->linearmovement[1]*pseqdesc->linearmovement[1]+ pseqdesc->linearmovement[2]*pseqdesc->linearmovement[2] );
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);

		// -0.00 109.08 0.00
		//0.08 0.00 0.0

		//MODDD - Hacky!
		//pflGroundSpeed = 0;


		//easyPrintLineANIMATION("YOU MANGY LITTLE animation  %.2f %.2f %.2f", pseqdesc->linearmovement[0], pseqdesc->linearmovement[1], pseqdesc->linearmovement[2]); 
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}


void GetSequenceInfoSafe( void *pmodel, float *pflFrameRate, entvars_t *pev)
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return;

	mstudioseqdesc_t	*pseqdesc;

	if (pev->sequence >= pstudiohdr->numseq)
	{
		return;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		//!!!not allowed to edit ground speed.
		//*pflGroundSpeed = sqrt( pseqdesc->linearmovement[0]*pseqdesc->linearmovement[0]+ pseqdesc->linearmovement[1]*pseqdesc->linearmovement[1]+ pseqdesc->linearmovement[2]*pseqdesc->linearmovement[2] );
		//*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 256.0;
		//*pflGroundSpeed = 0.0;
	}

}



int GetSequenceFlags( void *pmodel, entvars_t *pev )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if ( !pstudiohdr || pev->sequence >= pstudiohdr->numseq )
		return 0;

	mstudioseqdesc_t	*pseqdesc;
	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	return pseqdesc->flags;
}



int GetAnimationEvent( void *pmodel, entvars_t *pev, MonsterEvent_t *pMonsterEvent, float flStart, float flEnd, int index ){
	//no argLoops value provied? Will determine from the sequence on the model.
	return GetAnimationEvent(pmodel, pev, pMonsterEvent, flStart, flEnd, index, -1 );
}

int GetAnimationEvent( void *pmodel, entvars_t *pev, MonsterEvent_t *pMonsterEvent, float flStart, float flEnd, int index, int argLoops )
{
	studiohdr_t *pstudiohdr;
	BOOL loopPass;
	BOOL ordinaryPass;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if ( !pstudiohdr || pev->sequence >= pstudiohdr->numseq || !pMonsterEvent )
		return 0;


	int events = 0;

	mstudioseqdesc_t	*pseqdesc;
	mstudioevent_t		*pevent;

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;
	pevent = (mstudioevent_t *)((byte *)pstudiohdr + pseqdesc->eventindex);

	if (pseqdesc->numevents == 0 || index > pseqdesc->numevents )
		return 0;

	if (pseqdesc->numframes > 1)
	{
		//MODDD - it's all floats right?
		flStart *= (pseqdesc->numframes - 1) / 256.0f;
		flEnd *= (pseqdesc->numframes - 1) / 256.0f;
	}
	else
	{
		flStart = 0;
		flEnd = 1.0;
	}


	if(argLoops == -1){
		//that means up to what this model's sequence defaults to.
		argLoops = ((pseqdesc->flags & STUDIO_LOOPING)!=0) ;
	}



	//until proven otherwise, this didn't loop around. And only looping animations (argLoops) can even do that.
	//otherwise, an anim sits frozen at the end (frame 256, out of 256).
	loopPass = FALSE;



	if(argLoops){
		//ex: anim is 9 frames long.
		//we're at frame 8 (start).
		//ev: 2
		//rs: 8
		//re: 11
		//SV: I AM A good fellow: ind:0 evFrame:2 rs:8.00 re:11.00 numf-1:8 diff:3.00

		//end - len

		//2 < 11 - 9 + 1
		//2 < 3

		//ex: BACKWARDS.
		//anim  is 9 frames long.
		//ev: 8
		// rs: 1
		// re: -2
		// numf-1: 8
		//diff:  (-2 +9 - 1)
		//diff:  (6)

		//end + len
		//8 > -2 + 9
		//8 > 7
		//

			
		if(pev->framerate >= 0){
			//loopPass = (flEnd >= pseqdesc->numframes - 1 && pevent[index].frame < flEnd - pseqdesc->numframes + 1) ;
			if(flEnd >= pseqdesc->numframes - 1){
				loopPass = TRUE;
				//too high? We think the loop happened.
				flEnd -= (pseqdesc->numframes - 1);
				//nope, let an event pass if it went above the leftover flStart, since we skipped those end frames!
				//flStart = 0;
			}
		}else{
			//loopPass = (flEnd <= 0                       && pevent[index].frame > flEnd + pseqdesc->numframes - 1) ;

			if(flEnd < 0){
				loopPass = TRUE;
				//too low?
				flEnd += (pseqdesc->numframes - 1);
				//nope, let an event pass if it went below the leftover flStart
				//flStart = (pseqdesc->numframes - 1 );
			}
		}
	}

	for (; index < pseqdesc->numevents; index++)
	{
		// Don't send client-side events to the server AI
		if ( pevent[index].event >= EVENT_CLIENT )
			continue;

		//?????

		/*
		if(pevent[index].event == 10 || pevent[index].event == 11){
			int x = pevent[index].event;
			int te1 = pevent[index].frame;
			int te2 = pevent[index].type;
			char what = pevent[index].options[0];
			int breakme = 666;
		}
		*/


		//by default. Loop pass will be TRUE For looping anims that would have had an event
		//between the frames skipped by the (assumed?) jump from the very end back to this point.
		//loopPass = FALSE;
		//...but in the middle of checking events? why?

		//Need to let backwards animations trigger a wrap-around too. The one now is just for positive framerates.
		//argLoops = TRUE;

		
		/*
		if(index == 0){
			easyForcePrintLine("ARE YOU intoxicated in:%d rs:%.2f, re:%.2f ev:%d PASS1:%d PASS2:%d", index, flStart, flEnd, pevent[index].frame, (pevent[index].frame >= flStart), (pevent[index].frame < flEnd) );
		}
		*/


		if(pev->framerate >= 0){
			float relativeFrame = pevent[index].frame;

			if(!loopPass){
				//nothing special.
				ordinaryPass =  (relativeFrame >= flStart && relativeFrame < flEnd);
			}else{
				//If our event was skipped from the leftover flStart to the last frame possible,
				//or between the first frame possible (0) and flEnd's new place, count it.
				ordinaryPass = (relativeFrame >= flStart || relativeFrame < flEnd);
			}

		}else{
			//MODDD - hopefully this has no side-effects.  Lets the 'thud' sounds on bodies hitting the floor
			// in reversed death anims be played in the right place.
			//float relativeFrame =  ( ( pseqdesc->numframes - 1) - pevent[index].frame);
			float relativeFrame = pevent[index].frame;

			//easyForcePrintLine("absfr:%.2f relfr%.2f res:%.2f ree:%.2f", pevent[index].frame, relativeFrame, flStart, flEnd);
			if(!loopPass){
				//nothing special.
				ordinaryPass =  ( relativeFrame >= flEnd && relativeFrame < flStart);
			}else{
				//If our event was skipped from the leftover flStart to the last frame possible,
				//or between the first frame possible (0) and flEnd's new place, count it.
				ordinaryPass = (relativeFrame >= flEnd || relativeFrame < flStart);
			}

		}




		//if ( (pevent[index].frame >= flStart && pevent[index].frame < flEnd) || 
		//	((pseqdesc->flags & STUDIO_LOOPING) && flEnd >= pseqdesc->numframes - 1 && pevent[index].frame < flEnd - pseqdesc->numframes + 1) )
		
		

		//if ( (pevent[index].frame >= flStart && pevent[index].frame < flEnd) || 
		//	loopPass)
		
		if(ordinaryPass)
		{
			//easyForcePrintLine("I AM A good fellow: ind:%d evFrame:%d rs:%.2f re:%.2f numf-1:%d diff:%.2f", index, pevent[index].frame, flStart, flEnd, (pseqdesc->numframes - 1),  flEnd - pseqdesc->numframes + 1   );

			pMonsterEvent->event = pevent[index].event;
			pMonsterEvent->options = pevent[index].options;
			return index + 1;
		}
	}
	return 0;
}



void showHitboxInfoAll(void *pmodel, entvars_t *pev){
	studiohdr_t *pstudiohdr;
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)return;
	mstudiobbox_t	*pThishitbox = (mstudiobbox_t *)((byte *)pstudiohdr + pstudiohdr->hitboxindex);
	int i;
	for ( i = 0; i < pstudiohdr->numhitboxes; i++, pThishitbox++)
	{
		printHitboxInfo(pThishitbox, i);
	}
}

void showHitboxInfoOfBone(void *pmodel, entvars_t *pev, int argBone){
	BOOL everPrintedOut = FALSE;
	studiohdr_t *pstudiohdr;	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)return;
	mstudiobbox_t	*pThishitbox = (mstudiobbox_t *)((byte *)pstudiohdr + pstudiohdr->hitboxindex);
	int i;
	for ( i = 0; i < pstudiohdr->numhitboxes; i++, pThishitbox++)
	{
		if(pThishitbox->bone == argBone){
			printHitboxInfo(pThishitbox, i);
			everPrintedOut = TRUE;
		}
	}
	if(!everPrintedOut){
		//never printed out? Tell the user.
		easyForcePrintLine("No hitboxes of bone %d found.", argBone);
	}
}
void showHitboxInfoOfGroup(void *pmodel, entvars_t *pev, int argGroup){
	BOOL everPrintedOut = FALSE;
	studiohdr_t *pstudiohdr;	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)return;
	mstudiobbox_t	*pThishitbox = (mstudiobbox_t *)((byte *)pstudiohdr + pstudiohdr->hitboxindex);
	int i;
	for ( i = 0; i < pstudiohdr->numhitboxes; i++, pThishitbox++)
	{
		if(pThishitbox->group == argGroup){
			printHitboxInfo(pThishitbox, i);
			everPrintedOut = TRUE;
		}
	}
	if(!everPrintedOut){
		//never printed out? Tell the user.
		easyForcePrintLine("No hitboxes of group %d found.", argGroup);
	}
}
void showHitboxInfoNumber(void *pmodel, entvars_t *pev, int argID){
	studiohdr_t *pstudiohdr;	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)return;
	mstudiobbox_t	*pThishitbox = (mstudiobbox_t *)((byte *)pstudiohdr + pstudiohdr->hitboxindex);
	int i;
	for ( i = 0; i < pstudiohdr->numhitboxes; i++, pThishitbox++)
	{
		if(i == argID){
			printHitboxInfo(pThishitbox, i);
			return;
		}
	}
	//made it here? not found.
	easyForcePrintLine("Bone #%d not found. Bone ID\'s from 0 to less than max %d.", argID, pstudiohdr->numhitboxes);
}





void getHitboxInfoAll(void *pmodel, entvars_t *pev, mstudiobbox_t* argHitboxBuffer, int& argHitboxCount){
	argHitboxCount = 0;
	studiohdr_t *pstudiohdr;
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)return;
	mstudiobbox_t	*pThishitbox = (mstudiobbox_t *)((byte *)pstudiohdr + pstudiohdr->hitboxindex);
	int i;
	for ( i = 0; i < pstudiohdr->numhitboxes; i++, pThishitbox++)
	{
		argHitboxBuffer[argHitboxCount] = *pThishitbox;
		argHitboxCount++;
	}
}

void getHitboxInfoOfBone(void *pmodel, entvars_t *pev, mstudiobbox_t* argHitboxBuffer, int& argHitboxCount, int argBone){
	argHitboxCount = 0;
	studiohdr_t *pstudiohdr;	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)return;
	mstudiobbox_t	*pThishitbox = (mstudiobbox_t *)((byte *)pstudiohdr + pstudiohdr->hitboxindex);
	int i;
	for ( i = 0; i < pstudiohdr->numhitboxes; i++, pThishitbox++)
	{
		if(pThishitbox->bone == argBone){
			argHitboxBuffer[argHitboxCount] = *pThishitbox;
			argHitboxCount++;
		}
	}
	if(argHitboxCount==0){
		//never printed out? Tell the user.
		easyForcePrintLine("No hitboxes of bone %d found.", argBone);
	}
}
void getHitboxInfoOfGroup(void *pmodel, entvars_t *pev, mstudiobbox_t* argHitboxBuffer, int& argHitboxCount, int argGroup){
	argHitboxCount = 0;
	studiohdr_t *pstudiohdr;	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)return;
	mstudiobbox_t	*pThishitbox = (mstudiobbox_t *)((byte *)pstudiohdr + pstudiohdr->hitboxindex);
	int i;
	for ( i = 0; i < pstudiohdr->numhitboxes; i++, pThishitbox++)
	{
		if(pThishitbox->group == argGroup){
			argHitboxBuffer[argHitboxCount] = *pThishitbox;
			argHitboxCount++;
		}
	}
	if(argHitboxCount==0){
		//never printed out? Tell the user.
		easyForcePrintLine("No hitboxes of group %d found.", argGroup);
	}
}
void getHitboxInfoNumber(void *pmodel, entvars_t *pev, mstudiobbox_t*& argHitbox, int argID){
	argHitbox = NULL;
	studiohdr_t *pstudiohdr;	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)return;
	mstudiobbox_t	*pThishitbox = (mstudiobbox_t *)((byte *)pstudiohdr + pstudiohdr->hitboxindex);
	int i;
	for ( i = 0; i < pstudiohdr->numhitboxes; i++, pThishitbox++)
	{
		if(i == argID){
			argHitbox = pThishitbox;
			return;
		}
	}
	//made it here? not found.
	easyForcePrintLine("Bone #%d not found. Bone ID\'s from 0 to less than max %d.", argID, pstudiohdr->numhitboxes);
}

void getHitboxCount(void *pmodel, entvars_t *pev, int& argCount){
	argCount = 0;
	studiohdr_t *pstudiohdr;	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)return;
	//just want this.
	argCount = pstudiohdr->numhitboxes;
}





float SetController( void *pmodel, entvars_t *pev, int iController, float flValue )
{
	studiohdr_t *pstudiohdr;
	int iControllerActual;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return flValue;

	mstudiobonecontroller_t	*pbonecontroller = (mstudiobonecontroller_t *)((byte *)pstudiohdr + pstudiohdr->bonecontrollerindex);


	// find first controller that matches the index
	int i;

	for (i = 0; i <= pstudiohdr->numbonecontrollers; i++, pbonecontroller++)
	{
		if (pbonecontroller->index == iController) {
			break;
		}
	}
	if (i >= pstudiohdr->numbonecontrollers) {
		return flValue;
	}

	//MODDD - NEW SECTION
	// If the pbonecontroller picked has an index over numbonecontrollers, we likely
	// picked the special controller number (4), which means to change the mouth
	// controller instead (which is never actually at 4).
	// Instead, just use this same 'i' index to determine that.
	if (iController < pstudiohdr->numbonecontrollers) {
		// no problem with the default way?
		iControllerActual = i;
	}else {
		// stop at "i" instead, controller #4 does not exist!
		iControllerActual = i;
	}
	////////////////////////////////////////


	// wrap 0..360 if it's a rotational controller
	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pbonecontroller->end < pbonecontroller->start)
			flValue = -flValue;

		// does the controller not wrap?
		if (pbonecontroller->start + 359.0 >= pbonecontroller->end)
		{
			if (flValue > ((pbonecontroller->start + pbonecontroller->end) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pbonecontroller->start + pbonecontroller->end) / 2.0) - 180)
				flValue = flValue + 360;
		}
		else
		{
			if (flValue > 360)
				flValue = flValue - (int)(flValue / 360.0) * 360.0;
			else if (flValue < 0)
				flValue = flValue + (int)((flValue / -360.0) + 1) * 360.0;
		}
	}

	int setting = 255 * (flValue - pbonecontroller->start) / (pbonecontroller->end - pbonecontroller->start);

	if (setting < 0) setting = 0;
	if (setting > 255) setting = 255;

	pev->controller[iControllerActual] = setting;
	

	return setting * (1.0 / 255.0) * (pbonecontroller->end - pbonecontroller->start) + pbonecontroller->start;
}


//MODDD - for those who feel risky.
float SetControllerUnsafe( void *pmodel, entvars_t *pev, int iController, float flValue )
{
	studiohdr_t *pstudiohdr;
	int iControllerActual;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return flValue;

	mstudiobonecontroller_t	*pbonecontroller = (mstudiobonecontroller_t *)((byte *)pstudiohdr + pstudiohdr->bonecontrollerindex);

	// find first controller that matches the index
	int i;
	for ( i = 0; i < pstudiohdr->numbonecontrollers; i++, pbonecontroller++)
	{
		if (pbonecontroller->index == iController)
			break;
	}
	if (i >= pstudiohdr->numbonecontrollers)
		return flValue;


	//MODDD - NEW SECTION
	// If the pbonecontroller picked has an index over numbonecontrollers, we likely
	// picked the special controller number (4), which means to change the mouth
	// controller instead (which is never actually at 4).
	// Instead, just use this same 'i' index to determine that.
	if (iController < pstudiohdr->numbonecontrollers) {
		// no problem with the default way?
		iControllerActual = iController;
	}
	else {
		// stop at "i" instead, controller #4 does not actually exist!
		iControllerActual = i;
	}
	////////////////////////////////////////


	// wrap 0..360 if it's a rotational controller

	
	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		/*
		if (pbonecontroller->end < pbonecontroller->start)
			flValue = -flValue;
		*/

		/*
		// does the controller not wrap?
		if (pbonecontroller->start + 359.0 >= pbonecontroller->end)
		{
			if (flValue > ((pbonecontroller->start + pbonecontroller->end) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pbonecontroller->start + pbonecontroller->end) / 2.0) - 180)
				flValue = flValue + 360;
		}
		else
		{
			if (flValue > 360)
				flValue = flValue - (int)(flValue / 360.0) * 360.0;
			else if (flValue < 0)
				flValue = flValue + (int)((flValue / -360.0) + 1) * 360.0;
		}
		*/
	}

	/*
	int setting = 255 * (flValue - pbonecontroller->start) / (pbonecontroller->end - pbonecontroller->start);

	if (setting < 0) setting = 0;
	if (setting > 255) setting = 255;
	*/
	//pev->controller[iController] = setting;
	pev->controller[iControllerActual] = (byte)flValue;

	//return setting * (1.0 / 255.0) * (pbonecontroller->end - pbonecontroller->start) + pbonecontroller->start;
	//??? why return something?
	return 0;
}


float SetBlending( void *pmodel, entvars_t *pev, int iBlender, float flValue )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return flValue;

	mstudioseqdesc_t	*pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	//easyPrintLineANIMATION("AW %d", pseqdesc->blendtype[iBlender]);

	//MODDD - NOTE.  Apparently sequences themselves can allow/disallow blend adjustments
	// like torso pitch?  Just seems odd the barney unholstering can't aim pitch-wise,
	// but while firing can, so there's a near-instant moment where the model changes
	// to looking what he's firing at.  Weird.
	// Best not to force this, it won't do anything.  That's up to the model then.
	if (pseqdesc->blendtype[iBlender] == 0)
		return flValue;

	if (pseqdesc->blendtype[iBlender] & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pseqdesc->blendend[iBlender] < pseqdesc->blendstart[iBlender])
			flValue = -flValue;

		// does the controller not wrap?
		if (pseqdesc->blendstart[iBlender] + 359.0 >= pseqdesc->blendend[iBlender])
		{
			if (flValue > ((pseqdesc->blendstart[iBlender] + pseqdesc->blendend[iBlender]) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pseqdesc->blendstart[iBlender] + pseqdesc->blendend[iBlender]) / 2.0) - 180)
				flValue = flValue + 360;
		}
	}

	int setting = 255 * (flValue - pseqdesc->blendstart[iBlender]) / (pseqdesc->blendend[iBlender] - pseqdesc->blendstart[iBlender]);

	if (setting < 0) setting = 0;
	if (setting > 255) setting = 255;

	pev->blending[iBlender] = setting;

	//easyPrintLineANIMATION("BLENDAH::::%d", setting);

	float temp = setting * (1.0 / 255.0) * (pseqdesc->blendend[iBlender] - pseqdesc->blendstart[iBlender]) + pseqdesc->blendstart[iBlender];
	
	//easyForcePrintLine("YO YO YO %.2f", temp);
	return temp;
}


int FindTransition( void *pmodel, int iEndingAnim, int iGoalAnim, int *piDir )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return iGoalAnim;

	mstudioseqdesc_t	*pseqdesc;
	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);

	// bail if we're going to or from a node 0
	if (pseqdesc[iEndingAnim].entrynode == 0 || pseqdesc[iGoalAnim].entrynode == 0)
	{
		return iGoalAnim;
	}

	int iEndNode;

	// ALERT( at_console, "from %d to %d: ", pEndNode->iEndNode, pGoalNode->iStartNode );

	if (*piDir > 0)
	{
		iEndNode = pseqdesc[iEndingAnim].exitnode;
	}
	else
	{
		iEndNode = pseqdesc[iEndingAnim].entrynode;
	}

	if (iEndNode == pseqdesc[iGoalAnim].entrynode)
	{
		*piDir = 1;
		return iGoalAnim;
	}

	byte *pTransition = ((byte *)pstudiohdr + pstudiohdr->transitionindex);

	int iInternNode = pTransition[(iEndNode-1)*pstudiohdr->numtransitions + (pseqdesc[iGoalAnim].entrynode-1)];

	if (iInternNode == 0)
		return iGoalAnim;

	int i;

	// look for someone going
	for (i = 0; i < pstudiohdr->numseq; i++)
	{
		if (pseqdesc[i].entrynode == iEndNode && pseqdesc[i].exitnode == iInternNode)
		{
			*piDir = 1;
			return i;
		}
		if (pseqdesc[i].nodeflags)
		{
			if (pseqdesc[i].exitnode == iEndNode && pseqdesc[i].entrynode == iInternNode)
			{
				*piDir = -1;
				return i;
			}
		}
	}
	ALERT( at_console, "error in transition graph" );
	return iGoalAnim;
}


//MODDD
int getNumberOfBodyParts(void *pmodel, entvars_t *pev){
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;

	if (! pstudiohdr)
		return 0;

	//if (iGroup > pstudiohdr->numbodyparts)
	//	return;
	//easyPrintLine("HOWWWWW %d", pstudiohdr->numbodyparts);
	return pstudiohdr->numbodyparts;
}


//MODDD
int getNumberOfSkins(void *pmodel, entvars_t *pev){
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;

	if (! pstudiohdr)
		return 0;

	//if (iGroup > pstudiohdr->numbodyparts)
	//	return;
	//easyPrintLine("HOWWWWW %d", pstudiohdr->numbodyparts);
	
	return pstudiohdr->numskinfamilies;
}


void SetBodygroup( void *pmodel, entvars_t *pev, int iGroup, int iValue )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return;

	if (iGroup > pstudiohdr->numbodyparts)
		return;

	mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((byte *)pstudiohdr + pstudiohdr->bodypartindex) + iGroup;

	if (iValue >= pbodypart->nummodels)
		return;

	int iCurrent = (pev->body / pbodypart->base) % pbodypart->nummodels;

	pev->body = (pev->body - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));
}


int GetBodygroup( void *pmodel, entvars_t *pev, int iGroup )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return 0;

	if (iGroup > pstudiohdr->numbodyparts)
		return 0;

	mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((byte *)pstudiohdr + pstudiohdr->bodypartindex) + iGroup;

	if (pbodypart->nummodels <= 1)
		return 0;

	int iCurrent = (pev->body / pbodypart->base) % pbodypart->nummodels;

	return iCurrent;
}