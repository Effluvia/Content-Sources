//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( WEATHER_H )
#define WEATHER_H
#if defined( _WIN32 )
#pragma once
#endif

#include "windows.h"
#include "gl/gl.h"
#include "glext.H"
#include "com_model.h"

#define MAX_WEATHER_PARTICLES		65536

#define SURF_DRAWTILED				0x20
#define	SURF_DRAWSKY				4

#define SPLASH_FRAMES_NUM			14
#define SPLASH_FRAME_X				64
#define SPLASH_FRAME_Y				64

#define SNOW_GRAVITY		0.025f
#define RAIN_GRAVITY		0.3f

#define PARTICLE_LIGHTCHECK_DIST	8

enum weather_e
{
	WEATHER_OFF = 0,
	WEATHER_SNOW,
	WEATHER_RAIN
};

struct w_particle_t
{
	double life;
	double spawntime;

	float gravity;
	float alpha;
	float fadeout;
	float fadein;
	float scale;
	
	bool wind;

	vec3_t velocity;
	vec3_t origin;
	vec3_t normal;
	vec3_t color;
	vec3_t last_check;

	int frame;

	w_particle_t *pnext;
	w_particle_t *pprev;
};

extern void Sys_Error ( char *fmt, ... );

/*
====================
CWeather

====================
*/
class CWeather
{
public:
	void Init( void );
	void VidInit( void );
	void Clear( void );
	
	void Think( void );
	void DrawParticles( void );

	int MsgFunc_Weather( const char *pszName, int iSize, void *pBuf );

private:
	void CreateParticle( void );
	void CreateImpactParticle( vec3_t& origin, vec3_t& normal, float scale );
	bool UpdateParticle( w_particle_t* particle );

	w_particle_t* AllocParticle( w_particle_t** pchain );
	void FreeParticle( w_particle_t** pchain, w_particle_t* pparticle );

	void DrawParticle( w_particle_t* pparticle, mspriteframe_t *pframe );
	mspriteframe_t* GetSpriteFrame ( model_t *sprite, int frame );

private:
	w_particle_t* m_pParticleChain;
	w_particle_t* m_pImpactChain;

	w_particle_t* m_pFreeParticles;
	w_particle_t m_particlesArray[MAX_WEATHER_PARTICLES];

private:
	bool m_isActive;

	float m_windX;
	float m_windY;
	float m_gravity;
	float m_spawnHeight;
	float m_particleFrequency;

	int m_weatherType;
	int m_frameCount;

	double m_spawnTime;
	double m_time;
	double m_oldTime;
	double m_frametime;

	vec3_t m_renderOrigin;
	vec3_t m_viewUp;
	vec3_t m_viewRight;
	vec3_t m_viewForward;

	unsigned long m_numSpawned;
	unsigned long m_numParticles;

private:
	model_t* m_pParticleSprite;
	model_t* m_pImpactSprite;

	PFNGLACTIVETEXTUREPROC glActiveTexture;
};

extern CWeather gWeather;
#endif // WEATHER_H