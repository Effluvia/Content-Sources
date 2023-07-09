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
#if !defined ( EVENT_APIH )
#define EVENT_APIH
#ifdef _WIN32
#pragma once
#endif

#define EVENT_API_VERSION 1

extern cvar_t *g_fps_max;
extern cvar_t *g_sys_timescale;
inline bool isSlowmotionEnabled() {
	float sys_timescale = CVAR_GET_FLOAT( "sys_timescale" );
	bool using_sys_timescale = sys_timescale != 0.0f; // dirty way
	
	if ( using_sys_timescale ) {
		return sys_timescale < 1.0f;
	} else {
		float host_framerate = CVAR_GET_FLOAT( "host_framerate" );
		float base = ( 1000.0f / g_fps_max->value ) / 1000.0f;

		return host_framerate > 0.0f && host_framerate < base;
	}	
}

typedef struct event_api_s
{
	int		version;
	void	( *EV_PlaySoundCustom ) ( int ent, float *origin, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch );
	void	EV_PlaySound ( int ent, float *origin, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch, bool ignoreSlowmotion = false ) {
		if ( !ignoreSlowmotion ) {
			if ( isSlowmotionEnabled() ) {
				pitch *= 0.55;
			}
		}
		EV_PlaySoundCustom( ent, origin, channel, sample, volume, attenuation, fFlags, pitch );
	}
	void	( *EV_StopSound ) ( int ent, int channel, const char *sample );
	int		( *EV_FindModelIndex )( const char *pmodel );
	int		( *EV_IsLocal ) ( int playernum );
	int		( *EV_LocalPlayerDucking ) ( void );
	void	( *EV_LocalPlayerViewheight ) ( float * );
	void	( *EV_LocalPlayerBounds ) ( int hull, float *mins, float *maxs );
	int		( *EV_IndexFromTrace) ( struct pmtrace_s *pTrace );
	struct physent_s *( *EV_GetPhysent ) ( int idx );
	void	( *EV_SetUpPlayerPrediction ) ( int dopred, int bIncludeLocalClient );
	void	( *EV_PushPMStates ) ( void );
	void	( *EV_PopPMStates ) ( void );
	void	( *EV_SetSolidPlayers ) (int playernum);
	void	( *EV_SetTraceHull ) ( int hull );
	void	( *EV_PlayerTrace ) ( float *start, float *end, int traceFlags, int ignore_pe, struct pmtrace_s *tr );
	void	( *EV_WeaponAnimation ) ( int sequence, int body );
	unsigned short ( *EV_PrecacheEvent ) ( int type, const char* psz );
	void	( *EV_PlaybackEvent ) ( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
	const char *( *EV_TraceTexture ) ( int ground, float *vstart, float *vend );
	void	( *EV_StopAllSounds ) ( int entnum, int entchannel );
	void    ( *EV_KillEvents ) ( int entnum, const char *eventname );
} event_api_t;

extern event_api_t eventapi;

#endif
