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

#ifndef ALIAS_SHARED_H
#define ALIAS_SHARED_H

#define MAX_ALIAS_SEQUENCES		256
#define MAX_ALIAS_MODELS		512
#define MAX_ALIAS_SEQ_EVENTS	16

struct alias_event_t
{
	int frame;
	int event;

	char options[32];
};

struct alias_sequence_t
{
	char name[64];

	int startframe;
	int numframes;

	int activity;

	float groundspeed;
	float fps;

	bool looped;

	alias_event_t events[MAX_ALIAS_SEQ_EVENTS];
	int numevents;
};

struct alias_extradata_t
{
	// Name in model_t->name
	char name[256];

	// Sequence data
	alias_sequence_t sequences[MAX_ALIAS_SEQUENCES];
	int numsequences;

	// Pointer to model
	model_t* pmodel;
};

void Alias_GetSequenceInfo( const aliashdr_t* paliashdr, alias_extradata_t* pextradata );
void Alias_LoadSequenceInfo( const char* pszmodelname, const char* pszgamedir, alias_extradata_t* pextradata );
#endif //ALIAS_SHARED_H