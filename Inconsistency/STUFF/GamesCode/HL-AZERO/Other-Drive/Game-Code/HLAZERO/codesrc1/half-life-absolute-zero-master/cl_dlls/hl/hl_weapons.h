// NEW FILE.  For getting the globals from hl_weapons.cpp easily


#ifndef HL_WEAPONS_H
#define HL_WEAPONS_H

#ifdef _WIN32
#pragma once
#endif


#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "hud_iface.h"

// better know what this is at least
class CBasePlayer;






// !!! From com_weapons.h
////////////////////////////////////////////////////////////////////////
extern "C"
{
	void DLLEXPORT HUD_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double arg_time, unsigned int random_seed );
}

void		COM_Log( char *pszFile, char *fmt, ...);
int			CL_IsDead( void );

float		UTIL_SharedRandomFloat( unsigned int seed, float low, float high );
int			UTIL_SharedRandomLong( unsigned int seed, int low, int high );

int			HUD_GetWeaponAnim( void );
void		HUD_SendWeaponAnim( int iAnim, int body, int force );
void		HUD_PlaySound( char *sound, float volume );
void		HUD_PlaybackEvent( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
void		HUD_SetMaxSpeed( const struct edict_s *ed, float speed );
int			stub_PrecacheModel( char* s );
int			stub_PrecacheSound( char* s );
unsigned short	stub_PrecacheEvent( int type, const char *s );
const char		*stub_NameForFunction	( unsigned long function );
void		stub_SetModel			( struct edict_s *e, const char *m );


extern cvar_t *cl_lw;

extern int g_runfuncs;
extern vec3_t v_origin;
extern vec3_t v_angles;
extern float g_lastFOV;
extern struct local_state_s *g_finalstate;

////////////////////////////////////////////////////////////////////////







extern CBasePlayer localPlayer;

extern float g_flApplyVel;
extern BOOL g_irunninggausspred;

extern vec3_t previousorigin;



//clientside only variable.
extern BOOL reloadBlocker;

//extern float flNextAttackChangeMem;


extern BOOL blockUntilModelChange;
extern int oldModel;
extern int queuedBlockedModelAnim;

extern float forgetBlockUntilModelChangeTime;
extern float resistTime;
extern float seqPlayDelay;
extern int seqPlay;
extern BOOL queuecall_lastinv;
extern int g_currentanim;

#endif //HL_WEAPONS_H
