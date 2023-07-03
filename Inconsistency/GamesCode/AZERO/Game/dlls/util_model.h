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

//MODDD - NOTE.
// This file contains utility methods mainly for CBaseAnimating, although any entity 
// should be able to use most of these methods.
// NOW.  Renamed from 'animation.h' to 'util_model.h', same for the .cpp file, to ease
// confusion with 'animating.h/.cpp' (home of CBaseAnimating.
// As this file doesn't define a class at all, only a bunch of floaty global methods.
// Like a util_ file.

#ifndef UTIL_MODEL_H
#define UTIL_MODEL_H

#define ACTIVITY_NOT_AVAILABLE -1

//MODDD - Is this ok?
#include "studio.h"

#include "monsterevent.h"

// From /engine/studio.h
//oh, including studio.h here now.  I think this already included too then?
//#define STUDIO_LOOPING		0x0001


extern int IsSoundEvent( int eventNumber );

int LookupActivity( void *pmodel, entvars_t *pev, int activity );
int LookupActivityHeaviest( void *pmodel, entvars_t *pev, int activity );
int LookupSequence( void *pmodel, const char *label );
void GetSequenceInfo( void *pmodel, entvars_t *pev, float *pflFrameRate, float *pflGroundSpeed );

//MODDD
void GetSequenceInfoSafe( void *pModel, float *pflFrameRate, entvars_t *pev);

int GetSequenceFlags( void *pmodel, entvars_t *pev );
int LookupAnimationEvents( void *pmodel, entvars_t *pev, float flStart, float flEnd );

//MODDD - new
void showHitboxInfoAll(void *pmodel, entvars_t *pev);
void showHitboxInfoOfBone(void *pmodel, entvars_t *pev, int argBone);
void showHitboxInfoOfGroup(void *pmodel, entvars_t *pev, int argGroup);
void showHitboxInfoNumber(void *pmodel, entvars_t *pev, int argID);


void getHitboxInfoAll(void *pmodel, entvars_t *pev, mstudiobbox_t* argHitboxBuffer, int& argHitboxCount);
void getHitboxInfoOfBone(void *pmodel, entvars_t *pev, mstudiobbox_t* argHitboxBuffer, int& argHitboxCount, int argBone);
void getHitboxInfoOfGroup(void *pmodel, entvars_t *pev, mstudiobbox_t* argHitboxBuffer, int& argHitboxCount, int argGroup);
void getHitboxInfoNumber(void *pmodel, entvars_t *pev, mstudiobbox_t*& argHitbox, int argID);
void getHitboxCount(void *pmodel, entvars_t *pev, int& argCount);


float SetController( void *pmodel, entvars_t *pev, int iController, float flValue );

//MODDD
float SetControllerUnsafe( void *pmodel, entvars_t *pev, int iController, float flValue );
int getNumberOfBodyParts(void *pmodel, entvars_t *pev);
int getNumberOfSkins(void *pmodel, entvars_t *pev);

float SetBlending( void *pmodel, entvars_t *pev, int iBlender, float flValue );
void GetEyePosition( void *pmodel, float *vecEyePosition );


void Sequence_PrintSound_All(void* pmodel);
void Sequence_PrintSound(void* pmodel, int index);

void SequencePrecache(void* pmodel, int index);
void SequencePrecache( void *pmodel, const char *pSequenceName );

int FindTransition( void *pmodel, int iEndingAnim, int iGoalAnim, int *piDir );
void SetBodygroup( void *pmodel, entvars_t *pev, int iGroup, int iValue );
int GetBodygroup( void *pmodel, entvars_t *pev, int iGroup );

int GetAnimationEvent( void *pmodel, entvars_t *pev, MonsterEvent_t *pMonsterEvent, float flStart, float flEnd, int index );
int GetAnimationEvent( void *pmodel, entvars_t *pev, MonsterEvent_t *pMonsterEvent, float flStart, float flEnd, int index, int argLoops );
int ExtractBbox( void *pmodel, int sequence, float *mins, float *maxs );


#endif	//UTIL_MODEL_H
