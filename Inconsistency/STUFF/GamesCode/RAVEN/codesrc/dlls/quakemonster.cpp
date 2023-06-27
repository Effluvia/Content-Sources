/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Squadmonster  functions
//=========================================================
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "monsters.h"
#include "animation.h"
#include "saverestore.h"
#include "quakemonster.h"
#include "plane.h"
#include "studio.h"

//=========================================================
// Precache
//
//=========================================================
void CQuakeMonster :: Precache( void )
{
	SetAliasData();

	// Call base class for rest
	CBaseMonster::Precache();
}

//=========================================================
// SetAliasData
//
//=========================================================
void CQuakeMonster :: SetAliasData( void )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	if(!pmodel)
		return;

	if(m_modelExtraData.numsequences)
		return;

	char szgamedir[256];
	GET_GAME_DIR(szgamedir);

	Alias_GetSequenceInfo((aliashdr_t*)pmodel, &m_modelExtraData);
	Alias_LoadSequenceInfo(STRING(pev->model), szgamedir, &m_modelExtraData);
}

//=========================================================
// StudioFrameAdvance
//
//=========================================================
float CQuakeMonster::StudioFrameAdvance( float flInterval )
{
	if (flInterval == 0.0)
	{
		flInterval = (gpGlobals->time - pev->animtime);
		if (flInterval <= 0.001)
		{
			pev->animtime = gpGlobals->time;
			return 0.0;
		}
	}

	if (!pev->animtime)
		flInterval = 0.0;
	
	pev->frame += flInterval * m_flFrameRate * pev->framerate;
	pev->animtime = gpGlobals->time;

	if (pev->frame < 0.0 || pev->frame >= 256.0) 
	{
		if (m_fSequenceLoops)
			pev->frame -= (int)(pev->frame / 256.0) * 256.0;
		else
			pev->frame = (pev->frame < 0.0) ? 0 : 255;

		m_fSequenceFinished = TRUE;	// just in case it wasn't caught in GetEvents
	}

	return flInterval;
}

//=========================================================
// LookupActivity
//
//=========================================================
int CQuakeMonster :: LookupActivity ( int activity )
{
	int sequence = ACTIVITY_NOT_AVAILABLE;
	for(int i = 0; i < m_modelExtraData.numsequences; i++)
	{
		if(m_modelExtraData.sequences[i].activity == activity)
		{
			if(sequence == ACTIVITY_NOT_AVAILABLE || RANDOM_LONG(0, 1))
				sequence = i;
		}
	}

	return sequence;
}

//=========================================================
// LookupActivityHeaviest
//
//=========================================================
int CQuakeMonster :: LookupActivityHeaviest ( int activity )
{
	return LookupActivity( activity );
}

//=========================================================
// LookupSequence
//
//=========================================================
int CQuakeMonster :: LookupSequence ( const char *label )
{
	for(int i = 0; i < m_modelExtraData.numsequences; i++)
	{
		if(!strcmp(label, m_modelExtraData.sequences[i].name))
			return i;
	}

	return -1;
}


//=========================================================
// ResetSequenceInfo
//
//=========================================================
void CQuakeMonster :: ResetSequenceInfo ( )
{
	if (pev->sequence >= m_modelExtraData.numsequences)
	{
		m_flFrameRate = 0.0;
		m_flGroundSpeed = 0.0;
		return;
	}

	alias_sequence_t* psequence = &m_modelExtraData.sequences[pev->sequence];
	int numframes = psequence->looped ? (psequence->numframes+1) : psequence->numframes;

	if (psequence->numframes > 1)
	{
		m_flFrameRate = 256 * psequence->fps / (numframes - 1);
		m_flGroundSpeed = psequence->groundspeed * psequence->fps / (psequence->numframes - 1);
	}
	else
	{
		m_flFrameRate = 256.0;
		m_flGroundSpeed = 0.0;
	}

	m_fSequenceLoops = ((GetSequenceFlags() & STUDIO_LOOPING) != 0);

	pev->animtime = gpGlobals->time;
	pev->framerate = 1.0;

	m_fSequenceFinished = FALSE;
	m_flLastEventCheck = gpGlobals->time;
}



//=========================================================
// GetSequenceFlags
//
//=========================================================
BOOL CQuakeMonster :: GetSequenceFlags( )
{
	if (pev->sequence >= m_modelExtraData.numsequences)
		return 0;

	alias_sequence_t* psequence = &m_modelExtraData.sequences[pev->sequence];
	return psequence->looped ? STUDIO_LOOPING : 0;
}

//=========================================================
// DispatchAnimEvents
//
//=========================================================
int CQuakeMonster :: GetAnimationEvent( MonsterEvent_t *pMonsterEvent, float flStart, float flEnd, int index )
{
	if (!pMonsterEvent)
		return 0;

	if (pev->sequence >= m_modelExtraData.numsequences)
		return 0;

	alias_sequence_t* psequence = &m_modelExtraData.sequences[pev->sequence];
	if (psequence->numevents == 0 || index > psequence->numevents)
		return 0;

	if (psequence->numframes > 1)
	{
		flStart *= (psequence->numframes - 1) / 256.0;
		flEnd *= (psequence->numframes - 1) / 256.0;
	}
	else
	{
		flStart = 0;
		flEnd = 1.0;
	}

	alias_event_t* pevent = psequence->events;
	for (; index < psequence->numevents; index++)
	{
		// Don't send client-side events to the server AI
		if ( pevent[index].event >= EVENT_CLIENT )
			continue;

		if ( (pevent[index].frame >= flStart && pevent[index].frame < flEnd) || 
			(psequence->looped && flEnd >= (psequence->numframes - 1) && pevent[index].frame < flEnd - psequence->numframes + 1) )
		{
			pMonsterEvent->event = pevent[index].event;
			pMonsterEvent->options = pevent[index].options;
			return index + 1;
		}
	}
	return 0;
}

//=========================================================
// DispatchAnimEvents
//
//=========================================================
void CQuakeMonster :: DispatchAnimEvents ( float flInterval )
{
	// FIXME: I have to do this or some events get missed, and this is probably causing the problem below
	flInterval = 0.1;

	// FIX: this still sometimes hits events twice
	float flStart = pev->frame + (m_flLastEventCheck - pev->animtime) * m_flFrameRate * pev->framerate;
	float flEnd = pev->frame + flInterval * m_flFrameRate * pev->framerate;
	m_flLastEventCheck = pev->animtime + flInterval;

	m_fSequenceFinished = FALSE;
	if (flEnd >= 256 || flEnd <= 0.0) 
		m_fSequenceFinished = TRUE;

	int index = 0;
	MonsterEvent_t event;
	while ( (index = GetAnimationEvent( &event, flStart, flEnd, index ) ) != 0 )
		HandleAnimEvent( &event );
}

//=========================================================
// ExtractBbox
//
//=========================================================
int CQuakeMonster :: ExtractBbox( int sequence, float *mins, float *maxs )
{
	return 0;
}

//=========================================================
// SetSequenceBox
//
//=========================================================
void CQuakeMonster :: SetSequenceBox( void )
{
	Vector mins, maxs;

	// Get sequence bbox
	if ( ExtractBbox( pev->sequence, mins, maxs ) )
	{
		// expand box for rotation
		// find min / max for rotations
		float yaw = pev->angles.y * (M_PI / 180.0);
		
		Vector xvector, yvector;
		xvector.x = cos(yaw);
		xvector.y = sin(yaw);
		yvector.x = -sin(yaw);
		yvector.y = cos(yaw);
		Vector bounds[2];

		bounds[0] = mins;
		bounds[1] = maxs;
		
		Vector rmin( 9999, 9999, 9999 );
		Vector rmax( -9999, -9999, -9999 );
		Vector base, transformed;

		for (int i = 0; i <= 1; i++ )
		{
			base.x = bounds[i].x;
			for ( int j = 0; j <= 1; j++ )
			{
				base.y = bounds[j].y;
				for ( int k = 0; k <= 1; k++ )
				{
					base.z = bounds[k].z;
					
				// transform the point
					transformed.x = xvector.x*base.x + yvector.x*base.y;
					transformed.y = xvector.y*base.x + yvector.y*base.y;
					transformed.z = base.z;
					
					if (transformed.x < rmin.x)
						rmin.x = transformed.x;
					if (transformed.x > rmax.x)
						rmax.x = transformed.x;
					if (transformed.y < rmin.y)
						rmin.y = transformed.y;
					if (transformed.y > rmax.y)
						rmax.y = transformed.y;
					if (transformed.z < rmin.z)
						rmin.z = transformed.z;
					if (transformed.z > rmax.z)
						rmax.z = transformed.z;
				}
			}
		}
		rmin.z = 0;
		rmax.z = rmin.z + 1;
		UTIL_SetSize( pev, rmin, rmax );
	}
}

//=========================================================
// TraceAttack 
//=========================================================
void CQuakeMonster::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	CBaseEntity *pAttacker = CBaseEntity::Instance(pevAttacker);
	CBaseMonster *pMonster = pAttacker->MyMonsterPointer();

	if(pMonster && pMonster->IsQuakeMonster())
		PushEnemy(pAttacker, pevAttacker->origin);

	CBaseMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

//=========================================================
// SetEyePosition
//
// queries the monster's model for $eyeposition and copies
// that vector to the monster's view_ofs
//
//=========================================================
void CQuakeMonster :: SetEyePosition ( void )
{
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	if(!pmodel)
		return;

	aliashdr_t* phdr = (aliashdr_t*)pmodel;

	float alias_scale = CVAR_GET_FLOAT("r_aliasscale");
	float alias_offs = CVAR_GET_FLOAT("r_aliasoffset");

	pev->view_ofs = phdr->eyeposition * alias_scale * -1;
	pev->view_ofs = pev->view_ofs + Vector(0, 0, alias_offs);

	if ( pev->view_ofs == g_vecZero )
	{
		ALERT ( at_aiconsole, "%s has no view_ofs!\n", STRING ( pev->classname ) );
	}
}