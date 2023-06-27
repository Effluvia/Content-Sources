//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// eventscripts.h
#if !defined ( EVENTSCRIPTSH )
#define EVENTSCRIPTSH


//MODDD - DEFAULT_VIEWHEIGHT and VEC_DUCK_VIEW defines merged better with
// util_shared.h, from some defined elsewhere.  VEC_DUCK_VIEW here is just a single number,
// unlike that of another place (whole vector), so the single-number one  was renamed to
// VEC_DUCK_VIEW_Z.

//MODDD - why was FTENT_FADEOUT defined here when it already is in common/r_efx.h?
// oh.. don't need FTENT_FADEOUT present here at all.  ok then.

//MODDD - identical copy of DMG_ macros from health.h removed.
// Include em' from somewhere common, don't copy and paste that much stuff
// around three times ya hacks.


// Some of these are HL/TFC specific?
void EV_EjectBrass( float *origin, float *velocity, float rotation, int model, int soundtype );
void EV_GetGunPosition( struct event_args_s *args, float *pos, float *origin );
void EV_GetDefaultShellInfo( struct event_args_s *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale );
qboolean EV_IsLocal( int idx );
qboolean EV_IsPlayer( int idx );
void EV_CreateTracer( float *start, float *end );

struct cl_entity_s *GetEntity( int idx );
struct cl_entity_s *GetViewEntity( void );
void EV_MuzzleFlash( void );

#endif // EVENTSCRIPTSH
