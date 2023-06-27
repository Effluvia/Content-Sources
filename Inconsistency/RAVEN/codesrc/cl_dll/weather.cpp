/***+
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
// Ammo.cpp
//
// implementation of CHudAmmo class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

#include <string.h>
#include <stdio.h>
#include "elightlist.h"

#include "pmtrace.h"
#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"
#include "pm_defs.h"
#include "weather.h"

#include "studio_util.h"
#include "r_studioint.h"
#include "triangleapi.h"

#include "StudioModelRenderer.h"

#include "blackhole_shared.h"
#include "blackhole.h"

#define IMPACT_LIFE			1.0f
#define IMPACT_FADE_TIME	0.5f

#define SYSTEM_SIZE			1024.0f
#define MAX_HEIGHT			768.0f

#define PARTICLE_FADEIN		1.0f

extern int RecursiveLightPoint( model_t* pmodel, mnode_t *node, const vec3_t &start, const vec3_t &end, vec3_t &color );

// Class declaration
CWeather gWeather;

int __MsgFunc_Weather( const char *pszName, int iSize, void *pbuf )
{
	return gWeather.MsgFunc_Weather(pszName, iSize, pbuf );
}

/*
====================
Init

====================
*/
void CWeather::Init( void )
{
	glActiveTexture	= (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");

	HOOK_MESSAGE( Weather );
}

/*
====================
VidInit

====================
*/
void CWeather::VidInit( void )
{
	// Clear previous particles
	memset(m_particlesArray, 0, sizeof(m_particlesArray));

	// Link up the particles
	for(int i = 1; i < MAX_WEATHER_PARTICLES; i++)
	{
		m_particlesArray[i-1].pnext = &m_particlesArray[i];
		m_particlesArray[i].pprev = &m_particlesArray[i-1];
	}

	// Set free particles pointer
	m_pFreeParticles = m_particlesArray;
	m_pParticleChain = NULL;
	m_pImpactChain = NULL;

	// Reset these
	m_isActive = false;
	m_weatherType = WEATHER_OFF;

	m_particleFrequency = 0;
	m_numParticles = 0;
	m_numSpawned = 0;

	m_pParticleSprite = NULL;
	m_pImpactSprite = NULL;
}

/*
====================
Think

====================
*/
void CWeather::Think( void )
{
	IEngineStudio.GetTimes( &m_frameCount, &m_time, &m_oldTime );

	double life = m_time - m_spawnTime;
	if(life < 0)
		life = 0;

	double freq = 1/(float)m_particleFrequency;
	unsigned long itimesspawn = life/freq;
	m_frametime =  m_time - m_oldTime;

	// Spawn any particles if needed
	if(itimesspawn > m_numSpawned)
	{
		int inumspawn = itimesspawn - m_numSpawned;

		// Create new particles
		for(int j = 0; j < inumspawn; j++)
			CreateParticle();

		// Add to counter
		m_numSpawned += inumspawn;
	}

	if(!m_pImpactChain && !m_pParticleChain)
		return;

	// Update standard particles
	w_particle_t *pparticle = m_pParticleChain;
	while(pparticle)
	{
		if(!UpdateParticle(pparticle))
		{
			w_particle_t *pfree = pparticle;
			pparticle = pfree->pnext;

			FreeParticle(&m_pParticleChain, pfree);
			continue;
		}

		w_particle_t *pnext = pparticle->pnext;
		pparticle = pnext;
	}

	// Update impact particles
	pparticle = m_pImpactChain;
	while(pparticle)
	{
		if(!UpdateParticle(pparticle))
		{
			w_particle_t *pfree = pparticle;
			pparticle = pfree->pnext;

			FreeParticle(&m_pImpactChain, pfree);
			continue;
		}

		w_particle_t *pnext = pparticle->pnext;
		pparticle = pnext;
	}
}

/*
====================
CreateParticle

====================
*/
void CWeather::CreateParticle( void )
{
	vec3_t testOrigin;
	vec3_t randomOffset;

	cl_entity_t *pplayer = gEngfuncs.GetLocalPlayer();
	randomOffset[0] = gEngfuncs.pfnRandomLong(-SYSTEM_SIZE, SYSTEM_SIZE);
	randomOffset[1] = gEngfuncs.pfnRandomLong(-SYSTEM_SIZE, SYSTEM_SIZE);
	randomOffset[2] = min(m_spawnHeight, pplayer->origin[2] + MAX_HEIGHT);

	// Test if we hit sky from the spawn origin
	testOrigin[0] = pplayer->origin[0]+randomOffset[0];
	testOrigin[1] = pplayer->origin[1]+randomOffset[1];
	testOrigin[2] = m_spawnHeight+16;

	// Spawn in a parabole
	float fraction = max(0.75, min(1, (randomOffset.Length2D()/SYSTEM_SIZE)));
	randomOffset[2] = (1-(fraction))*(randomOffset[2]-pplayer->origin[2]);

	static pmtrace_t tr;
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(pplayer->origin+randomOffset, testOrigin, PM_WORLD_ONLY, -1, &tr);

	if(tr.fraction != 1.0 && gEngfuncs.PM_PointContents(tr.endpos, NULL) != CONTENTS_SKY)
		return;

	// Create the particle
	w_particle_t* pparticle = AllocParticle(&m_pParticleChain);
	if(!pparticle)
		return;

	VectorAdd(pplayer->origin, randomOffset, pparticle->origin);
	pparticle->gravity = SNOW_GRAVITY;
	pparticle->alpha = 1.0;
	pparticle->wind = true;
	pparticle->scale = gEngfuncs.pfnRandomFloat(0.008, 0.08);
	pparticle->life = -1;
	pparticle->fadein = PARTICLE_FADEIN;
	pparticle->spawntime = m_time;

	vec3_t lightEnd;
	lightEnd = pparticle->origin;
	lightEnd.z -= MAX_HEIGHT*2;

	model_t* pmodel = IEngineStudio.GetModelByIndex( 1 );
	RecursiveLightPoint( pmodel, pmodel->nodes, pparticle->origin, lightEnd, pparticle->color );

	VectorCopy(pparticle->origin, pparticle->last_check);
}

/*
====================
CreateImpactParticle

====================
*/
void CWeather::CreateImpactParticle( vec3_t& origin, vec3_t& normal, float scale )
{
	w_particle_t* pparticle = AllocParticle(&m_pImpactChain);
	if(!pparticle)
		return;

	VectorCopy(origin, pparticle->origin);
	VectorCopy(normal, pparticle->normal);
	pparticle->gravity = 0;
	pparticle->alpha = 1.0;
	pparticle->wind = false;
	pparticle->scale = scale;
	pparticle->fadeout = IMPACT_FADE_TIME;
	pparticle->life = IMPACT_LIFE;
	pparticle->spawntime = m_time;

	vec3_t lightEnd;
	lightEnd = pparticle->origin;
	lightEnd.z -= MAX_HEIGHT*2;

	model_t* pmodel = IEngineStudio.GetModelByIndex( 1 );
	RecursiveLightPoint( pmodel, pmodel->nodes, origin, lightEnd, pparticle->color );

	VectorCopy(origin, pparticle->last_check);
}

/*
====================
UpdateParticle

====================
*/
bool CWeather::UpdateParticle( w_particle_t* particle )
{
	// Add in any black holes
	gBlackHole.AffectParticle( particle );

	// Kill the particle if the time is exceeded
	if( particle->life != -1 && particle->spawntime+particle->life < m_time )
		return false;

	// Skip collision testing if possible
	if( particle->gravity )
	{
		// Add in gravity first
		static vec3_t testVelocity;
		VectorMA( particle->velocity, m_frametime*m_gravity, Vector(0, 0, -800), testVelocity );

		// Now add in any wind
		if(particle->wind)
		{
			if(m_windX)
			{
				float windFactor = max(0, abs(sin(m_time * 0.3)) - 0.25);
				testVelocity.x += m_windX * windFactor * m_frametime * particle->scale/0.08;
			}

			if(m_windY)
			{
				float windFactor = max(0, abs(sin(m_time * 0.2)) - 0.25);
				testVelocity.y += m_windY * windFactor * m_frametime * particle->scale/0.08;
			}
		}

		// Calculate the test origin
		static vec3_t testOrigin;
		VectorMA( particle->origin, m_frametime, testVelocity, testOrigin );

		// Do collision testing
		int testContents = gEngfuncs.PM_PointContents( testOrigin, NULL );
		if( testContents != CONTENTS_EMPTY )
		{
			// Kill immediately if we hit sky
			if( testContents == CONTENTS_SKY )
				return false;
			
			static pmtrace_t tr;
			gEngfuncs.pEventAPI->EV_PlayerTrace( particle->origin, testOrigin, PM_WORLD_ONLY, -1, &tr );
			if(tr.fraction != 1.0)
			{
				CreateImpactParticle( tr.endpos, tr.plane.normal, particle->scale );
				return false;
			}
		}

		// Just update the origin and the velocity
		VectorCopy( testOrigin, particle->origin );
		VectorCopy( testVelocity, particle->velocity );
	}

	// Fade it out if needed
	if( particle->fadeout && particle->life != -1 )
	{
		double timeLeft = (particle->spawntime + particle->life) - m_time;
		double flfraction = timeLeft / particle->fadeout;
		flfraction = max(flfraction, 0);
		flfraction = min(flfraction, 1);

		particle->alpha = flfraction;
	}
	
	// Fade in if needed
	if( particle->fadein )
	{
		double lifeTime = m_time - particle->spawntime;
		double flfraction = lifeTime / particle->fadein;
		flfraction = max(flfraction, 0);
		flfraction = min(flfraction, 1);

		particle->alpha = flfraction;
		if(particle->alpha == 1.0)
			particle->fadein = 0;
	}

	// Get light values if needed
	if( (particle->last_check - particle->origin).Length2D() > PARTICLE_LIGHTCHECK_DIST )
	{
		vec3_t lightEnd;
		lightEnd = particle->origin;
		lightEnd.z -= MAX_HEIGHT*2;

		model_t* pmodel = IEngineStudio.GetModelByIndex( 1 );
		RecursiveLightPoint( pmodel, pmodel->nodes, particle->origin, lightEnd, particle->color );

		VectorCopy( particle->origin, particle->last_check );
	}

	return true;
}

/*
====================
AllocParticle

====================
*/
w_particle_t* CWeather::AllocParticle( w_particle_t** pchain )
{
	if(!m_pFreeParticles)
		return NULL;

	w_particle_t *pparticle = m_pFreeParticles;
	m_pFreeParticles = pparticle->pnext;
	memset(pparticle, 0, sizeof(w_particle_t));

	// Add system into pointer array
	if((*pchain))
	{
		(*pchain)->pprev = pparticle;
		pparticle->pnext = (*pchain);
	}

	(*pchain) = pparticle;
	return pparticle;
}

/*
====================
FreeParticle

====================
*/
void CWeather::FreeParticle( w_particle_t** pchain, w_particle_t* pparticle )
{
	if(pparticle->pprev) 
		pparticle->pprev->pnext = pparticle->pnext;
	else 
		(*pchain) = pparticle->pnext;

	if(pparticle->pnext) 
		pparticle->pnext->pprev = pparticle->pprev;

	pparticle->pnext = m_pFreeParticles;
	m_pFreeParticles = pparticle;
}

/*
====================
DrawParticles

====================
*/
void CWeather::DrawParticles( void )
{
	if(!m_isActive)
		return;

	if(!m_pImpactChain && !m_pParticleChain)
		return;

	// Push texture state
	glPushAttrib(GL_TEXTURE_BIT);

	// Disable textures on all units except first
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE3);
	glDisable(GL_TEXTURE_2D);

	// Set the active texture unit
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D); // todo

	// Set up texture state
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	// Get view info
	IEngineStudio.GetViewInfo( m_renderOrigin, m_viewUp, m_viewRight, m_viewForward );

	// Draw normal particles
	if( m_pParticleChain )
	{
		mspriteframe_t* pframe = GetSpriteFrame( m_pParticleSprite, 0 );
		glBindTexture( GL_TEXTURE_2D, pframe->gl_texturenum );

		w_particle_t *pparticle = m_pParticleChain;
		while(pparticle)
		{
			DrawParticle(pparticle, pframe);
			pparticle = pparticle->pnext;
		}
	}

	// Draw impact particles
	if( m_pImpactChain )
	{
		mspriteframe_t* pframe = GetSpriteFrame( m_pImpactSprite, 0 );
		glBindTexture( GL_TEXTURE_2D, pframe->gl_texturenum );

		w_particle_t *pparticle = m_pImpactChain;
		while(pparticle)
		{
			DrawParticle(pparticle, pframe);
			pparticle = pparticle->pnext;
		}
	}

	// Restore states
	glDisable(GL_BLEND);
	glPopAttrib();
}

/*
====================
GetSpriteFrame

====================
*/
mspriteframe_t* CWeather::GetSpriteFrame ( model_t *sprite, int frame )
{
	mspriteframe_t *pspriteframe;
	msprite_t *psprite = (msprite_t*)sprite->cache.data;

	if ((frame >= psprite->numframes) || (frame < 0))
	{
		gEngfuncs.Con_Printf ("R_GetSpriteFrame: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		mspritegroup_t *pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		float *pintervals = pspritegroup->intervals;
		float fullinterval = pintervals[pspritegroup->numframes-1];

		// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
		// are positive, so we don't have to worry about division by 0
		float targettime = m_time - ((int)(m_time / fullinterval)) * fullinterval;

		int i = 0;
		for (; i < (pspritegroup->numframes-1); i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}

/*
====================
DrawParticles

====================
*/
void CWeather::DrawParticle( w_particle_t* pparticle, mspriteframe_t* pframe )
{
	// Set color
	glColor4f(pparticle->color.x, pparticle->color.y, pparticle->color.z, pparticle->alpha);

	glBegin( GL_QUADS );
	vec3_t vertexPos = pparticle->origin - m_viewForward * 0.01 + m_viewUp * pframe->up * pparticle->scale;
	vertexPos = vertexPos + m_viewRight * pframe->left * pparticle->scale;
	glTexCoord2f( 0, 0 );
	glVertex3fv( vertexPos );

	vertexPos = pparticle->origin - m_viewForward * 0.01 + m_viewUp * pframe->up * pparticle->scale;
	vertexPos = vertexPos + m_viewRight * pframe->right * pparticle->scale;
	glTexCoord2f( 1, 0 );
	glVertex3fv( vertexPos );

	vertexPos = pparticle->origin - m_viewForward * 0.01 + m_viewUp * pframe->down * pparticle->scale;
	vertexPos = vertexPos + m_viewRight * pframe->right * pparticle->scale;
	glTexCoord2f( 1, 1 );
	glVertex3fv( vertexPos );

	vertexPos = pparticle->origin - m_viewForward * 0.01 + m_viewUp * pframe->down * pparticle->scale;
	vertexPos = vertexPos + m_viewRight * pframe->left * pparticle->scale;
	glTexCoord2f( 0, 1 );
	glVertex3fv( vertexPos );
	glEnd();
}

/*
====================
DrawParticles

====================
*/
int CWeather::MsgFunc_Weather( const char *pszName, int iSize, void *pBuf )
{
	BEGIN_READ( pBuf, iSize );

	int iActive = READ_BYTE();
	m_isActive = iActive ? true : false;
	if(!m_isActive)
	{
		m_particleFrequency = 0;
		return 1;
	}

	int iType = READ_BYTE();
	m_weatherType = !iType ? WEATHER_SNOW : WEATHER_RAIN;
	m_particleFrequency = READ_COORD()*10;
	m_spawnHeight = READ_COORD();
	m_windX = READ_COORD();
	m_windY = READ_COORD();
	
	int particleIndex = READ_SHORT();
	m_pParticleSprite = IEngineStudio.GetModelByIndex( particleIndex );
	if(!m_pParticleSprite || m_pParticleSprite->type != mod_sprite)
	{
		gEngfuncs.Con_Printf( "Invalid or missing particle sprite specified for weather entity.\n" );
		m_isActive = false;
		return 1;
	}

	int impactIndex = READ_SHORT();
	m_pImpactSprite = IEngineStudio.GetModelByIndex( impactIndex );
	if(!m_pImpactSprite || m_pImpactSprite->type != mod_sprite)
	{
		gEngfuncs.Con_Printf( "Invalid or missing impact sprite specified for weather entity.\n" );
		m_isActive = false;
		return 1;
	}

	if( m_weatherType == WEATHER_SNOW )
		m_gravity = SNOW_GRAVITY;
	else
		m_gravity = RAIN_GRAVITY;

	IEngineStudio.GetTimes( &m_frameCount, &m_spawnTime, &m_oldTime );
	m_numSpawned = 0;

	return 1;
}

/*
==================
RecursiveLightPoint

==================
*/
int RecursiveLightPoint( model_t* pmodel, mnode_t *node, const vec3_t &start, const vec3_t &end, vec3_t &color )
{
	float		front, back, frac;
	int			side;
	mplane_t	*plane;
	vec3_t		mid;
	msurface_t	*surf;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	*tex;
	color24		*lightmap;

	if (node->contents < 0)
		return FALSE;		// didn't hit anything
	
	plane = node->plane;
	front = DotProduct (start, plane->normal) - plane->dist;
	back = DotProduct (end, plane->normal) - plane->dist;
	side = front < 0;
	
	if ( (back < 0) == side )
		return RecursiveLightPoint (pmodel, node->children[side], start, end, color);
	
	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;
	
// go down front side	
	int r = RecursiveLightPoint (pmodel, node->children[side], start, mid, color);

	if (r) 
		return TRUE;
		
	if ((back < 0) == side)
		return FALSE;

	surf = pmodel->surfaces + node->firstsurface;
	for (i = 0; i < node->numsurfaces; i++, surf++)
	{
		if (surf->flags & (SURF_DRAWTILED | SURF_DRAWSKY))
			continue;	// no lightmaps

		int index = node->firstsurface+i;
		tex = surf->texinfo;
		s = DotProduct(mid, tex->vecs[0])+tex->vecs[0][3];
		t = DotProduct(mid, tex->vecs[1])+tex->vecs[1][3];

		if (s < surf->texturemins[0] ||
		t < surf->texturemins[1])
			continue;
		
		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];
		
		if ( ds > surf->extents[0] || dt > surf->extents[1] )
			continue;

		if (!surf->samples)
			continue;

		ds >>= 4;
		dt >>= 4;

		lightmap = surf->samples;

		if (lightmap)
		{
			int surfindex = node->firstsurface+i;
			int size = ((surf->extents[1]>>4)+1)*((surf->extents[0]>>4)+1);
			lightmap += dt * ((surf->extents[0]>>4)+1) + ds;

			float flIntensity = (lightmap->r + lightmap->g + lightmap->b)/3;
			float flScale = flIntensity/50;

			if(flScale > 1.0) 
				flScale = 1.0;

			color[0] = (float)(lightmap->r * flScale)/255.0f;
			color[1] = (float)(lightmap->g * flScale)/255.0f;
			color[2] = (float)(lightmap->b * flScale)/255.0f;
		}	
		else
		{
			color[0] = color[1] = color[2] = 0.5;
		}
		return TRUE;
	}

// go down back side
	return RecursiveLightPoint (pmodel, node->children[!side], mid, end, color);
}