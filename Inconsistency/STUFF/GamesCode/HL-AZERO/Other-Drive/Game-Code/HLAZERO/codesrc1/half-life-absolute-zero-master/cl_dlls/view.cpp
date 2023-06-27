//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// view/refresh setup functions

#include "hud.h"
#include "util_shared.h"
#include "cl_util.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"

#include "entity_state.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "pm_movevars.h"
#include "pm_shared.h"
#include "pm_defs.h"
#include "event_api.h"
#include "pmtrace.h"
#include "screenfade.h"
#include "shake.h"
#include "hltv.h"

#include "r_studioint.h"
#include "com_model.h"

//MODDD - important
//EASY_CVAR_EXTERN_CLIENT_MASS

EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(myCameraSucks)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosFixedX)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosFixedY)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosFixedZ)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotFixedX)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotFixedY)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotFixedZ)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosOffX)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosOffY)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraPosOffZ)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotOffX)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotOffY)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(cameraRotOffZ)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset)
EASY_CVAR_EXTERN(cl_viewpunch)
EASY_CVAR_EXTERN(cl_viewroll)
EASY_CVAR_EXTERN(cl_interp_view_extra)
EASY_CVAR_EXTERN(cl_interp_view_standard)
EASY_CVAR_EXTERN_CLIENTONLY(cl_viewpunch_mod)



#define CAM_MODE_RELAX 1
#define CAM_MODE_FOCUS 2

#define ORIGIN_BACKUP 64
#define ORIGIN_MASK ( ORIGIN_BACKUP - 1 )


//MODDD - important point before.
//extern globalvars_t* gpGlobals;

//extern BOOL g_recentDuckVal;

extern float global2PSEUDO_IGNOREcameraMode;
extern float global2PSEUDO_grabbedByBarancle;


extern void command_updateCameraPerspectiveF(void);
extern void command_updateCameraPerspectiveT(void);





typedef struct
{
	float Origins[ORIGIN_BACKUP][3];
	float OriginTime[ORIGIN_BACKUP];

	float Angles[ORIGIN_BACKUP][3];
	float AngleTime[ORIGIN_BACKUP];

	int CurrentOrigin;
	int CurrentAngle;
} viewinterp_t;



// Spectator Mode
extern "C"
{
	float vecNewViewAngles[3];
	int iHasNewViewAngles;
	float vecNewViewOrigin[3];
	int iHasNewViewOrigin;
	int iIsSpectator;
}

extern "C"
{
	int CL_IsThirdPerson(void);
	void CL_CameraOffset(float* ofs);

	void DLLEXPORT V_CalcRefdef(struct ref_params_s* pparams);

	void PM_ParticleLine(float* start, float* end, int pcolor, float life, float vert);
	int PM_GetVisEntInfo(int ent);
	int PM_GetPhysEntInfo(int ent);


	//MODDD - no need to prototype these, handled by common/vector.h for sanity.
	/*
	void	InterpolateAngles(float* start, float* end, float* output, float frac);
	void	NormalizeAngles(float* angles);
	float	Distance(const float* v1, const float* v2);
	float	AngleBetweenVectors(const float* v1, const float* v2);
	*/

	float vJumpOrigin[3];
	float vJumpAngles[3];
}


extern engine_studio_api_t IEngineStudio;

extern cvar_t* cl_viewrollangle;
extern cvar_t* cl_viewrollspeed;
extern cvar_t* cl_forwardspeed;
extern cvar_t* chase_active;
extern cvar_t* scr_ofsx, * scr_ofsy, * scr_ofsz;
extern cvar_t* cl_vsmoothing;

extern void RenderFog(void);
void V_DropPunchAngle(float frametime, float* arg_ev_punchangle);
void VectorAngles(const float* forward, float* angles);


/*
The view is allowed to move slightly from it's true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.
*/


vec3_t		v_origin, v_angles, v_cl_angles, v_sim_org, v_lastAngles;
float		v_frametime, v_lastDistance;
float		v_cameraRelaxAngle = 5.0f;
float		v_cameraFocusAngle = 35.0f;
int			v_cameraMode = CAM_MODE_FOCUS;
qboolean	v_resetCamera = 1;

vec3_t ev_punchangle;

cvar_t* scr_ofsx;
cvar_t* scr_ofsy;
cvar_t* scr_ofsz;

cvar_t* v_centermove;
cvar_t* v_centerspeed;

cvar_t* cl_bobcycle;
cvar_t* cl_bob;
cvar_t* cl_bobup;
cvar_t* cl_waterdist;
cvar_t* cl_chasedist;

// These cvars are not registered (so users can't cheat), so set the ->value field directly
// Register these cvars in V_Init() if needed for easy tweaking
cvar_t	v_iyaw_cycle = { "v_iyaw_cycle", "2", 0, 2 };
cvar_t	v_iroll_cycle = { "v_iroll_cycle", "0.5", 0, 0.5 };
cvar_t	v_ipitch_cycle = { "v_ipitch_cycle", "1", 0, 1 };
cvar_t	v_iyaw_level = { "v_iyaw_level", "0.3", 0, 0.3 };
cvar_t	v_iroll_level = { "v_iroll_level", "0.1", 0, 0.1 };
cvar_t	v_ipitch_level = { "v_ipitch_level", "0.3", 0, 0.3 };

float	v_idlescale;  // used by TFC for concussion grenade effect

//=============================================================================
/*
void V_NormalizeAngles( float *angles )
{
	int i;
	// Normalize angles
	for ( i = 0; i < 3; i++ )
	{
		if ( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if ( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}

/*
===================
V_InterpolateAngles

Interpolate Euler angles.
FIXME:  Use Quaternions to avoid discontinuities
Frac is 0.0 to 1.0 ( i.e., should probably be clamped, but doesn't have to be )
===================

void V_InterpolateAngles( float *start, float *end, float *output, float frac )
{
	int i;
	float ang1, ang2;
	float d;

	V_NormalizeAngles( start );
	V_NormalizeAngles( end );

	for ( i = 0 ; i < 3 ; i++ )
	{
		ang1 = start[i];
		ang2 = end[i];

		d = ang2 - ang1;
		if ( d > 180 )
		{
			d -= 360;
		}
		else if ( d < -180 )
		{
			d += 360;
		}

		output[i] = ang1 + d * frac;
	}

	V_NormalizeAngles( output );
} */

// Quakeworld bob code, this fixes jitters in the mutliplayer since the clock (pparams->time) isn't quite linear
float V_CalcBob(struct ref_params_s* pparams)
{
	static double bobtime;
	static float bob;
	float cycle;
	static float lasttime;
	vec3_t vel;


	if (pparams->onground == -1 ||
		pparams->time == lasttime)
	{
		// just use old value
		return bob;
	}

	lasttime = pparams->time;

	bobtime += pparams->frametime;
	cycle = bobtime - (int)(bobtime / cl_bobcycle->value) * cl_bobcycle->value;
	cycle /= cl_bobcycle->value;

	if (cycle < cl_bobup->value)
	{
		cycle = M_PI * cycle / cl_bobup->value;
	}
	else
	{
		cycle = M_PI + M_PI * (cycle - cl_bobup->value) / (1.0f - cl_bobup->value);
	}

	// bob is proportional to simulated velocity in the xy plane
	// (don't count Z, or jumping messes it up)
	VectorCopy_f(pparams->simvel, vel);
	vel[2] = 0;


	bob = sqrt(vel[0] * vel[0] + vel[1] * vel[1]) * cl_bob->value;
	bob = bob * 0.3 + bob * 0.7 * sin(cycle);
	bob = min(bob, 4);
	bob = max(bob, -7);
	return bob;

}

/*
===============
V_CalcRoll
Used by view and sv_user
===============
*/
float V_CalcRoll(vec3_t angles, vec3_t velocity, float rollangle, float rollspeed)
{
	float sign;
	float side;
	float value;
	vec3_t forward;
	vec3_t right;
	vec3_t up;

	AngleVectors(angles, forward, right, up);

	side = DotProduct(velocity, right);
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

typedef struct pitchdrift_s
{
	float		pitchvel;
	int			nodrift;
	float		driftmove;
	double		laststop;
} pitchdrift_t;

static pitchdrift_t pd;

void V_StartPitchDrift(void)
{
	if (pd.laststop == gEngfuncs.GetClientTime())
	{
		return;		// something else is keeping it from drifting
	}

	if (pd.nodrift || !pd.pitchvel)
	{
		pd.pitchvel = v_centerspeed->value;
		pd.nodrift = 0;
		pd.driftmove = 0;
	}
}

void V_StopPitchDrift(void)
{
	pd.laststop = gEngfuncs.GetClientTime();
	pd.nodrift = 1;
	pd.pitchvel = 0;
}

/*
===============
V_DriftPitch

Moves the client pitch angle towards idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.
===============
*/
void V_DriftPitch(struct ref_params_s* pparams)
{
	float delta;
	float move;

	if (gEngfuncs.IsNoClipping() || !pparams->onground || pparams->demoplayback || pparams->spectator)
	{
		pd.driftmove = 0;
		pd.pitchvel = 0;
		return;
	}

	// don't count small mouse motion
	if (pd.nodrift)
	{
		if (fabs(pparams->cmd->forwardmove) < cl_forwardspeed->value)
			pd.driftmove = 0;
		else
			pd.driftmove += pparams->frametime;

		if (pd.driftmove > v_centermove->value)
		{
			V_StartPitchDrift();
		}
		return;
	}

	delta = pparams->idealpitch - pparams->cl_viewangles[PITCH];

	if (!delta)
	{
		pd.pitchvel = 0;
		return;
	}

	move = pparams->frametime * pd.pitchvel;
	pd.pitchvel += pparams->frametime * v_centerspeed->value;

	if (delta > 0)
	{
		if (move > delta)
		{
			pd.pitchvel = 0;
			move = delta;
		}
		pparams->cl_viewangles[PITCH] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			pd.pitchvel = 0;
			move = -delta;
		}
		pparams->cl_viewangles[PITCH] -= move;
	}
}

/*
==============================================================================
						VIEW RENDERING
==============================================================================
*/

/*
==================
V_CalcGunAngle
==================
*/
void V_CalcGunAngle(struct ref_params_s* pparams)
{
	cl_entity_t* viewent;

	viewent = gEngfuncs.GetViewModel();
	if (!viewent)
		return;

	viewent->angles[YAW] = pparams->viewangles[YAW] + pparams->crosshairangle[YAW];
	viewent->angles[PITCH] = -pparams->viewangles[PITCH] + pparams->crosshairangle[PITCH] * 0.25;
	viewent->angles[ROLL] -= v_idlescale * sin(pparams->time * v_iroll_cycle.value) * v_iroll_level.value;

	// don't apply all of the v_ipitch to prevent normally unseen parts of viewmodel from coming into view.
	viewent->angles[PITCH] -= v_idlescale * sin(pparams->time * v_ipitch_cycle.value) * (v_ipitch_level.value * 0.5);
	viewent->angles[YAW] -= v_idlescale * sin(pparams->time * v_iyaw_cycle.value) * v_iyaw_level.value;

	VectorCopy_f(viewent->angles, viewent->curstate.angles);
	VectorCopy_f(viewent->angles, viewent->latched.prevangles);
}

/*
==============
V_AddIdle

Idle swaying
==============
*/
void V_AddIdle(struct ref_params_s* pparams)
{
	pparams->viewangles[ROLL] += v_idlescale * sin(pparams->time * v_iroll_cycle.value) * v_iroll_level.value;
	pparams->viewangles[PITCH] += v_idlescale * sin(pparams->time * v_ipitch_cycle.value) * v_ipitch_level.value;
	pparams->viewangles[YAW] += v_idlescale * sin(pparams->time * v_iyaw_cycle.value) * v_iyaw_level.value;
}

/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void V_CalcViewRoll(struct ref_params_s* pparams)
{
	//	float		side;
	cl_entity_t* viewentity;

	viewentity = gEngfuncs.GetEntityByIndex(pparams->viewentity);
	if (!viewentity)
		return;

	if(EASY_CVAR_GET(cl_viewroll) != 0){
		pparams->viewangles[ROLL] = V_CalcRoll(pparams->viewangles, pparams->simvel, cl_viewrollangle->value, cl_viewrollspeed->value) * 4;
	}

	//side = V_CalcRoll ( viewentity->angles, pparams->simvel, pparams->movevars->rollangle, pparams->movevars->rollspeed ); [REMOVED]

	if (pparams->health <= 0 && (pparams->viewheight[2] != 0))
	{
		// only roll the view if the player is dead and the viewheight[2] is nonzero 
		// this is so deadcam in multiplayer will work.
		//MODDD - DEAD CAMERA SETTING
		pparams->viewangles[ROLL] = 80;	// dead view angle
		return;
	}
}


/*
==================
V_CalcIntermissionRefdef

==================
*/
void V_CalcIntermissionRefdef(struct ref_params_s* pparams)
{
	cl_entity_t* ent;
	cl_entity_t *view;
	float		old;

	// ent is the player model ( visible when out of body )
	ent = gEngfuncs.GetLocalPlayer();

	// view is the weapon model (only visible from inside body )
	view = gEngfuncs.GetViewModel();

	VectorCopy_f(pparams->simorg, pparams->vieworg);
	VectorCopy_f(pparams->cl_viewangles, pparams->viewangles);

	view->model = NULL;

	// allways idle in intermission
	old = v_idlescale;
	v_idlescale = 1;

	V_AddIdle(pparams);

	if (gEngfuncs.IsSpectateOnly())
	{
		// in HLTV we must go to 'intermission' position by ourself
		VectorCopy_f(gHUD.m_Spectator.m_cameraOrigin, pparams->vieworg);
		VectorCopy_f(gHUD.m_Spectator.m_cameraAngles, pparams->viewangles);
	}

	v_idlescale = old;

	v_cl_angles = pparams->cl_viewangles;
	v_origin = pparams->vieworg;
	v_angles = pparams->viewangles;
}

/*
==================
V_CalcRefdef

==================
*/


//extern BOOL resetNormalRefDefVars;

//MODD - these used to be inside method 'V_CalcNormalRefdef', but moved outside for loading a game to force them instantly.
// ...or just change them with a variable set by elsewhere in-method instead, makes more sense that way.
BOOL resetNormalRefDefVars = FALSE;
static float oldz = 0;
static float oldRawz = 0;
//MODDD - NEW for cl_interp_view_extra
static float oldViewHeight = 0;
static int oldHull = 0;
static float prevOriginZ = 0;
static float deltaOriginZ_cumula = 0;
float g_interp_z = 0; // Set by an outside source!

static int g_recentCrouchChangeFrames;



void V_CalcNormalRefdef(struct ref_params_s* pparams)
{
	//MODDD -
	//Spirit of half life 1.9 calls "RenderFog" in here, but this causes fog not to show up when paused.
	//When moved to "UpdateClientData" of hud_update.cpp, it works.
	//RenderFog();

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(myCameraSucks) == 1) {
		return;
	}
	cl_entity_t* ent;
	cl_entity_t* view;
	int i;
	vec3_t		angles;
	float		bob;
	float		waterOffset;
	static viewinterp_t ViewInterp;

	static float lasttime;


	vec3_t camAngles;
	vec3_t camForward;
	vec3_t camRight;
	vec3_t camUp;

	cl_entity_t* pwater;

	V_DriftPitch(pparams);

	if (gEngfuncs.IsSpectateOnly())
	{
		ent = gEngfuncs.GetEntityByIndex(g_iUser2);
	}
	else
	{
		// ent is the player model ( visible when out of body )
		ent = gEngfuncs.GetLocalPlayer();
	}

	// view is the weapon model (only visible from inside body )
	view = gEngfuncs.GetViewModel();

	// transform the view offset by the model's matrix to get the offset from
	// model origin for the view
	bob = V_CalcBob(pparams);

	// refresh position
	VectorCopy_f(pparams->simorg, pparams->vieworg);
	pparams->vieworg[2] += (bob);

	// MODDD - this line moved further below ...or not
	VectorAdd_f(pparams->vieworg, pparams->viewheight, pparams->vieworg);

	VectorCopy_f(pparams->cl_viewangles, pparams->viewangles);

	gEngfuncs.V_CalcShake();
	gEngfuncs.V_ApplyShake(pparams->vieworg, pparams->viewangles, 1.0);

	// never let view origin sit exactly on a node line, because a water plane can
	// dissapear when viewed with the eye exactly on it.
	// FIXME, we send origin at 1/128 now, change this?
	// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis

	pparams->vieworg[0] += 1.0 / 32;
	pparams->vieworg[1] += 1.0 / 32;
	pparams->vieworg[2] += 1.0 / 32;

	// Check for problems around water, move the viewer artificially if necessary 
	// -- this prevents drawing errors in GL due to waves

	//VectorAdd_f( pparams->vieworg, pparams->viewheight, pparams->vieworg );


	waterOffset = 0;
	if (pparams->waterlevel >= 2)
	{
		int contents, waterDist, waterEntity;
		vec3_t	point;
		waterDist = cl_waterdist->value;

		if (pparams->hardware)
		{
			waterEntity = gEngfuncs.PM_WaterEntity(pparams->simorg);
			if (waterEntity >= 0 && waterEntity < pparams->max_entities)
			{
				pwater = gEngfuncs.GetEntityByIndex(waterEntity);
				if (pwater && (pwater->model != NULL))
				{
					waterDist += (pwater->curstate.scale * 16);	// Add in wave height
				}
			}
		}
		else
		{
			waterEntity = 0;	// Don't need this in software
		}

		VectorCopy_f(pparams->vieworg, point);

		// Eyes are above water, make sure we're above the waves
		if (pparams->waterlevel == 2)
		{
			point[2] -= waterDist;
			for (i = 0; i < waterDist; i++)
			{
				contents = gEngfuncs.PM_PointContents(point, NULL);
				if (contents > CONTENTS_WATER)
					break;
				point[2] += 1;
			}
			waterOffset = (point[2] + waterDist) - pparams->vieworg[2];
		}
		else
		{
			// eyes are under water.  Make sure we're far enough under
			point[2] += waterDist;

			for (i = 0; i < waterDist; i++)
			{
				contents = gEngfuncs.PM_PointContents(point, NULL);
				if (contents <= CONTENTS_WATER)
					break;
				point[2] -= 1;
			}
			waterOffset = (point[2] - waterDist) - pparams->vieworg[2];
		}
	}

	pparams->vieworg[2] += waterOffset;

	V_CalcViewRoll(pparams);

	V_AddIdle(pparams);

	// offsets
	VectorCopy_f(pparams->cl_viewangles, angles);

	AngleVectors(angles, pparams->forward, pparams->right, pparams->up);

	// don't allow cheats in multiplayer
	if (pparams->maxclients <= 1)
	{
		for (i = 0; i < 3; i++)
		{
			pparams->vieworg[i] += scr_ofsx->value * pparams->forward[i] + scr_ofsy->value * pparams->right[i] + scr_ofsz->value * pparams->up[i];
		}
	}

	// Treating cam_ofs[2] as the distance
	if (CL_IsThirdPerson())
	{
		vec3_t ofs;

		ofs[0] = ofs[1] = ofs[2] = 0.0;

		CL_CameraOffset((float*)&ofs);

		VectorCopy_f(ofs, camAngles);
		camAngles[ROLL] = 0;

		AngleVectors(camAngles, camForward, camRight, camUp);

		for (i = 0; i < 3; i++)
		{
			pparams->vieworg[i] += -ofs[2] * camForward[i];
		}
	}

	// Give gun our viewangles
	VectorCopy_f(pparams->cl_viewangles, view->angles);

	//MODDDREMOVE - does nothing as this is not depended on / referred to?
	//Vector lastAngles = view->angles; // save oldangles

	// set up gun position
	V_CalcGunAngle(pparams);

	// Use predicted origin as view origin.
	VectorCopy_f(pparams->simorg, view->origin);
	view->origin[2] += (waterOffset);


	// MODDD - this line moved further below ...or not.
	VectorAdd_f(view->origin, pparams->viewheight, view->origin);

	// Let the viewmodel shake at about 10% of the amplitude
	gEngfuncs.V_ApplyShake(view->origin, view->angles, 0.9);


	for (i = 0; i < 3; i++)
	{
		view->origin[i] += bob * 0.4 * pparams->forward[i];
	}
	view->origin[2] += bob;

	// throw in a little tilt.
	view->angles[YAW] -= bob * 0.5;
	view->angles[ROLL] -= bob * 1;
	view->angles[PITCH] -= bob * 0.3;


	//MODDDREMOVE: ??? why was this here?
	VectorCopy_f(view->angles, view->curstate.angles);
	//OKAY.  Ask around, is that weapon-bob thing fixed by un-commenting this out?


	// pushing the view origin down off of the same X/Z plane as the ent's origin will give the
	// gun a very nice 'shifting' effect when the player looks up/down. If there is a problem
	// with view model distortion, this may be a cause. (SJB). 
	view->origin[2] -= 1;

	// fudge position around to keep amount of weapon visible
	// roughly equal with different FOV
	if (pparams->viewsize == 110)
	{
		view->origin[2] += 1;
	}
	else if (pparams->viewsize == 100)
	{
		view->origin[2] += 2;
	}
	else if (pparams->viewsize == 90)
	{
		view->origin[2] += 1;
	}
	else if (pparams->viewsize == 80)
	{
		view->origin[2] += 0.5;
	}


	//MODDD - ladder bob?

	//OH THAT ROTATION.  motion sickness anyone?
	/*
	float extraFactor = sin(gpGlobals->time / 250);
	Vector vecTemp = Vector(0.0f, 0.0f, extraFactor * 333);
	VectorAdd_f ( pparams->viewangles, vecTemp, pparams->viewangles );
	*/

	//float extraFactor = sin(gpGlobals->time / 250);
	//Vector vecTemp = Vector(0.0f, 0.0f, extraFactor * 333);


	// Add in the punchangle, if any
	// MODDD NOTE - is "pparams->punchangle" always 0's?  what is the point?  OHH.  It comes from raw settings from "pev->viewpunch" changes
	//              done in melee attacks serverside, and that just got sent over here.  That's fine.
	// Looks like "ev_punchangle" below comes from say, a weapon's lasting effect.

	if (EASY_CVAR_GET(cl_viewpunch) > 0) {
		Vector vPunchFinal;
		if (EASY_CVAR_GET(cl_viewpunch) == 1) {
			VectorCopy_f(pparams->punchangle, vPunchFinal);
		}
		else {
			VectorScale(pparams->punchangle, EASY_CVAR_GET(cl_viewpunch), vPunchFinal);
		}

		VectorAdd_f(pparams->viewangles, vPunchFinal, pparams->viewangles);
		if (pparams->punchangle[0] != 0 && pparams->punchangle[1] != 0 && pparams->punchangle[2] != 0) {
			int x = 45;
		}
		//VectorAdd_f ( pparams->viewangles, vecTemp, pparams->viewangles );
	}


	// Include client side punch, too

	//MODDD - only use the punch if the "minimumfiredelay" cheat is off.  This way, rapid fire doesn't
	//distort the real damage-point.
	// Also we have another CVar just for stopping viewpunch.  Sometimes we don't need that distraction.

	// ALSO, ev_punchangle seems to come from 'events' in ev_hldm (weapon recoil most often, if not always).
	// See V_PunchAxis which sets it).   pparms->punchangle  above looks to come from serverside entities
	// inflicting it on the player by setting 'pev->punchangle', usually melee attacks throwing the camera off.
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) != 1 && EASY_CVAR_GET(cl_viewpunch) > 0) {
		VectorAdd_f(pparams->viewangles, (float*)&ev_punchangle, pparams->viewangles);
	}
	else {

	}


	//MODDD - this seems to reduce the angle punch a little, given how much time has passed since last frame for how much reduction to apply.
	//        This lets a view punch linearly lose its influence until it's gone pretty fast.
	//        In particular, it reduces the total amount of ev_punchangle, makes it approach 0.
	//        ev_punchangle is freshly added to our actual view-angles each frame (pparams->viewangles).
	//        So looking in the same direction while a view punch happens and expires, leaves you looking in the exact same direction as before the punch.
	V_DropPunchAngle(pparams->frametime, (float*)&ev_punchangle);



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// smooth out stair step ups
	//MODDD - support for going down stairs too to look smooth!
	//MODDD - NOTE.  These seem to be looking at a hardcoded stepsize of "18", even though
	// sv_stepsize is a CVar that can be changed.
	// Perhaps it should be broadcasted to clients, then use that client-cached version here?

	//MODDD - crouch interp
	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	// OLD WAY.
	if (!pparams->smoothing && pparams->onground && pparams->simorg[2] - oldz > 0)
	{
		float steptime;

		steptime = pparams->time - lasttime;
		if (steptime < 0)
			//FIXME		I_Error ("steptime < 0");
			steptime = 0;

		oldz += steptime * 150;
		if (oldz > pparams->simorg[2])
			oldz = pparams->simorg[2];
		if (pparams->simorg[2] - oldz > 18)
			oldz = pparams->simorg[2] - 18;
		pparams->vieworg[2] += oldz - pparams->simorg[2];
		view->origin[2] += oldz - pparams->simorg[2];
	}
	else
	{
		oldz = pparams->simorg[2];
	}
	*/


	// retail is 150, tested with 140.
#define STEPPER 150
#define STEPPERFAST 190

	float safeSimZ = pparams->simorg[2];   // - pparams->viewheight[2]
	
	//MODDD - between games/maps, forget any differences, interp from that is just silly.
	if (resetNormalRefDefVars) {
		resetNormalRefDefVars = FALSE;
		oldz = safeSimZ;
		oldViewHeight = pparams->viewheight[2];
		oldHull = ent->curstate.usehull;
		prevOriginZ = ent->curstate.origin.z;
		deltaOriginZ_cumula = 0;
		g_recentCrouchChangeFrames = 0;
	}


	if (g_recentCrouchChangeFrames > 0) {
		g_recentCrouchChangeFrames--;
	}

	//if (!pparams->smoothing && pparams->onground && safeSimZ - oldz > 0)
	if (!pparams->smoothing)
	{
		// allow extra interp for changes in viewheight (most likely ducking/unducking) never, on the ground, or always (+midair)
		// depending on cl_interp_view_extra.
		
		//easyForcePrintLine("the what H:%d %.2f %.2f::%.2f   %.2f %.2f::%.2f", ent->curstate.usehull, pparams->viewheight[2], oldViewHeight, pparams->viewheight[2] - oldViewHeight + (safeSimZ - oldz), safeSimZ, oldz, (safeSimZ - oldz));


		float deltaOriginZ = ent->curstate.origin.z - prevOriginZ;
		prevOriginZ = ent->curstate.origin.z;
		// same thing
		//float deltaSimZ = safeSimZ - oldRawz;
		//oldRawz = safeSimZ;



		// On the hull changing, correct the observed change in Z.
		// It doesn't need to play a role in view changes, this is already handled elsewhere and would
		// cause glitchiness if it were.
		if (ent->curstate.usehull != oldHull) {

			if (ent->curstate.usehull == 0) {
				// Going to 0 (ducking -> standing)?
				if (pparams->onground) {
					// Why is this extra offset on oldZ needed to avoid an upward
					// jolt at the start of crouching?
					// Something about hull-sizes between crouch/standing maybe.
					oldViewHeight = pparams->viewheight[2] - 0;
					oldz -= 16;
				}
				// Don't count the change in hull-size in deltaOriginZ either
				deltaOriginZ -= 18;
			}
			else if (ent->curstate.usehull == 1) {
				// Going to 1 (standing -> ducking)?
				if (pparams->onground) {
					// Ending a duck on the ground?  No further interp logic needed,
					// pm_shared already makes this go smoothly.  Just call it finished.
					oldViewHeight = pparams->viewheight[2];
					oldz = safeSimZ;
				}
				else {
					g_recentCrouchChangeFrames = 4;
				}
				// Don't count the change in hull-size in deltaOriginZ either
				deltaOriginZ += 18;
			}
			else {
				// ????
			}

			oldHull = ent->curstate.usehull;
		}//END OF hull change check


		deltaOriginZ_cumula += deltaOriginZ;
		//easyForcePrintLine("here comes honey %.2f : %.2f", deltaOriginZ_cumula, deltaOriginZ);
		//easyForcePrintLine("here comes honey %.2f : %.2f", deltaOriginZ);


		// can we count falling velocity?  Don't think we need it though, if that's what this is?
		//    pparams->cmd->upmove

		//MODDD - TODO?  maybe?  low priority.
		// As great of an idea as ignoring changes in Z origin outside of hull-changes is,
		// it means stairs are no longer affected by interp (really jumpy-looking going up/down).
		// Sadly I don't see a way to tell whether a change in Z is from going up a surface
		// (when pm_shared does that) or being taken higher/lower by going up/down stairs.
		// If there were some way for pm_shared to give the difference in height that is allowed
		// to be interpolated, that would be nice.
		// GOT IT!  g_interp_z is set by pm_shared to give the amount of change explained by interp-fix.
		// Sadly this is inadequate for multiplayer, anything but singleplayer or the player running a 
		// non-dedicated server will get some awkward lag from this.
		// For this to be better, we'd need some synched variable accessible from cl_dlls/entity.cpp that's
		// better handled over multiplayer latency, or tied better to clientside really.
		// Could it be some global that both C (pm_shared.c) and C++ (here) can read?  Would that even
		// make sense?  The devs never did anything like that.  So, no clue.
		// There is still an improvement compared to the way it was before, dropping this.
		if (!IsMultiplayer()) {
			//if (g_interp_z != 0) {
			//	easyForcePrintLine("no %.2f", g_interp_z);
			//}
			oldz += deltaOriginZ - g_interp_z;
		}



		// This 'g_recentCrouchChangeFrames' thing isn't so great.
		// MODDD TODO - how about pm_shared tells us if a change comes from midair hidding a ledge, and if so,
		// make that interp smooth ?

		BOOL doExtraInterp = FALSE;
		BOOL doStandardInterp = FALSE;

		if (EASY_CVAR_GET(cl_interp_view_extra) == 2) {
			// that was easy
			doExtraInterp = TRUE;
		}
		else if (EASY_CVAR_GET(cl_interp_view_extra) == 1){
			// Doing this on g_recentCrouchChangeFrames lets sv_
			//if (pparams->onground || g_recentCrouchChangeFrames > 0) {
			if(pparams->onground){
				doExtraInterp = TRUE;
			}
		}

		if (EASY_CVAR_GET(cl_interp_view_standard) == 2) {
			// that was easy
			doStandardInterp = TRUE;
		}
		else if (EASY_CVAR_GET(cl_interp_view_standard) == 1) {
			// Doing this on g_recentCrouchChangeFrames lets sv_
			//if (pparams->onground || g_recentCrouchChangeFrames > 0) {
			if(pparams->onground){
				doStandardInterp = TRUE;
			}
		}



		
		if (doExtraInterp) {
			// unmodified now, no need for changes.
			const float filteredViewheight = pparams->viewheight[2];

			if (filteredViewheight - oldViewHeight > 0) {
				//MODDD - EXPERIMENTAL.
				float steptime;

				steptime = pparams->time - lasttime;
				if (steptime < 0) {
					//FIXME		I_Error ("steptime < 0");
					steptime = 0;
				}

				if (pparams->onground) {
					oldViewHeight += steptime * STEPPER;
				}
				else {
					oldViewHeight += steptime * STEPPERFAST;
				}
				if (oldViewHeight > filteredViewheight) {
					oldViewHeight = filteredViewheight;
				}
				if (filteredViewheight - oldViewHeight > 34) {
					oldViewHeight = filteredViewheight - 34;
				}

				pparams->vieworg[2] += (oldViewHeight - (filteredViewheight)) * 1;
				view->origin[2] += (oldViewHeight - (filteredViewheight)) * 1;
				//easyForcePrintLine("test %.2f %.2f :::%.2f", oldViewHeight, filteredViewheight, (oldViewHeight - (filteredViewheight)));

			}
			else if (filteredViewheight - oldViewHeight < 0) {
				float steptime;

				steptime = pparams->time - lasttime;
				if (steptime < 0) {
					//FIXME		I_Error ("steptime < 0");
					steptime = 0;
				}

				if (pparams->onground) {
					oldViewHeight -= steptime * STEPPER;
				}else {
					oldViewHeight -= steptime * STEPPERFAST;
				}
				if (oldViewHeight < filteredViewheight) {
					oldViewHeight = filteredViewheight;
				}
				if (-filteredViewheight + oldViewHeight > 34) {
					oldViewHeight = filteredViewheight + 34;
				}

				pparams->vieworg[2] -= (-oldViewHeight + (filteredViewheight)) * 1;
				view->origin[2] -= (-oldViewHeight + (filteredViewheight)) * 1;
				////////////easyForcePrintLine("test %.2f", (oldViewHeight - (filteredViewheight)));

			}
			else {
				oldViewHeight = pparams->viewheight[2];
			}

		}//END OF cl_interp_view_extra checks
		else {
			oldViewHeight = pparams->viewheight[2];
		}


		// ONLY interp changes in Z if standing on the ground.
		// Works for the change from a complete-duck to standing and going up/down stairs & inclines.
		if (doStandardInterp)
		{
			// Now, changes in veiw height alone (like canceling a duck early) can be recorded.

			// A sudden change in hull-size throws off the viewheight, add in that difference so it has no effect
			// on what we're working with


			// And back to the normal Z comparisons.  Retail only handled interp for going up stairs to look more smooth
			// and unducking after a full duck (this changes the hull size, so the change showed up for this).
			if (safeSimZ - oldz > 0) {
				float steptime;

				steptime = pparams->time - lasttime;
				if (steptime < 0) {
					//FIXME		I_Error ("steptime < 0");
					steptime = 0;
				}

				oldz += steptime * STEPPER;
				if (oldz > safeSimZ) {
					oldz = safeSimZ;
				}


				

				deltaOriginZ_cumula -= steptime * STEPPER;
				if (deltaOriginZ_cumula < 0) deltaOriginZ_cumula = 0;

				//MODDD - tolerance changed from 18 to allow for the difference
				// between the duck and standing hulls.  I think.
				
				//if (ent->curstate.usehull == 1) {
					// going up?  don't allow as much push-down from duck lest we look through the floor of a rising platform.
					// Unfortunately there isn't enough information to tell whether a change in camera origin is from
					// going from standing to crouch or from going up/down a platform.
					// If so, the '34' below could shift to stop the player from looking below a platform that's
					// ascending fast enough by going from stand to crouch while ascending.  Oh well.
					// ent->curstate.maxs, ent->curstate.mins to help make that judgement?  do they change during
					// stand/crouch changes?  Doubt it, stand to crouch changes the hull at the very end, crouch to stand
					// at the very beginning.
				    // CHANGE SINCE!!!
				    // Can judge the change in origin (minus one from a change in hull-size that can be offset
				    // in detecting a change in ducking/standing) to affect the amount of interp allowed.
				    // Moving high enough, no interp from below to up is allowed (forced exact), and vice versa.
				    // Can give some tolerance too (the 'max' below's 2nd parameter, from 8 to 18-ish).
				//	if (safeSimZ - oldz > 18) {
				//		oldz = safeSimZ - 18;
				//	}
				//}
				//else {
				float stepTester = (max(34 - deltaOriginZ_cumula, 0));
					if (safeSimZ - oldz > stepTester) {
						oldz = safeSimZ - stepTester;
					}
				//}
				
				//if (safeSimZ - oldz > 34) {
				//    oldz = safeSimZ - 34;
				//}


				pparams->vieworg[2] += oldz - (safeSimZ);
				view->origin[2] += oldz - (safeSimZ);

			}
			else if (safeSimZ - oldz < 0) {

				// FOR the new downward stairs/incline checks, don't do interpolation
				// while the user's viewheight has changed since last frame (ducking and unducking typically).
				// This wasn't expected so it looks wonky when it happens.  Interp from the other way around 
				// is needed though, go figure.
				
				//MODDD - camera Z interp logic for going down stairs.
				float steptime;

				steptime = pparams->time - lasttime;
				if (steptime < 0) {
					//FIXME		I_Error ("steptime < 0");
					steptime = 0;
				}

				oldz -= steptime * STEPPER;
				if (oldz < safeSimZ) {
					oldz = safeSimZ;
				}


				
				deltaOriginZ_cumula += steptime * STEPPER;
				if (deltaOriginZ_cumula > 0) deltaOriginZ_cumula = 0;

				//if (ent->curstate.usehull == 1) {
				//	if (-safeSimZ + oldz > 18) {
				//		oldz = safeSimZ + 18;
				//	}
				//}
				//else {
				float stepTester = (max(34 + deltaOriginZ_cumula, 0));
					if (-safeSimZ + oldz > stepTester) {
						oldz = safeSimZ + stepTester;
					}
				//}
				
				//if (-safeSimZ + oldz > 34) {
				//	oldz = safeSimZ + 34;
				//}


				pparams->vieworg[2] -= -oldz + (safeSimZ);
				view->origin[2] -= -oldz + (safeSimZ);
				
			}
			else{
				// oh
				oldz = safeSimZ;
				deltaOriginZ_cumula = 0;
			}

			//if (safeSimZ - oldz != 0) {
			//	oldViewHeight = pparams->viewheight[2];
			//}

		}// END OF onground check
		else{
			oldz = safeSimZ;
			deltaOriginZ_cumula = 0;
		}
	}// END OF smoothing check
	else{
		oldz = safeSimZ;
		deltaOriginZ_cumula = 0;
	}


	// no.  This is unholy.
//skipper:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//MODDD - lines from above moved here.  ...or not
	//VectorAdd_f(view->origin, pparams->viewheight, view->origin);
	//VectorAdd_f(pparams->vieworg, pparams->viewheight, pparams->vieworg);





	{
		static float lastorg[3];
		vec3_t delta;

		VectorSubtract_f(pparams->simorg, lastorg, delta);

		if (Length(delta) != 0.0)
		{
			VectorCopy_f(pparams->simorg, ViewInterp.Origins[ViewInterp.CurrentOrigin & ORIGIN_MASK]);
			ViewInterp.OriginTime[ViewInterp.CurrentOrigin & ORIGIN_MASK] = pparams->time;
			ViewInterp.CurrentOrigin++;

			VectorCopy_f(pparams->simorg, lastorg);
		}
	}

	// Smooth out whole view in multiplayer when on trains, lifts
	if (cl_vsmoothing && cl_vsmoothing->value &&
		(pparams->smoothing && (pparams->maxclients > 1)))
	{
		int foundidx;
		float t;

		if (cl_vsmoothing->value < 0.0)
		{
			gEngfuncs.Cvar_SetValue("cl_vsmoothing", 0.0);
		}

		t = pparams->time - cl_vsmoothing->value;

		for (i = 1; i < ORIGIN_MASK; i++)
		{
			foundidx = ViewInterp.CurrentOrigin - 1 - i;
			if (ViewInterp.OriginTime[foundidx & ORIGIN_MASK] <= t)
				break;
		}

		if (i < ORIGIN_MASK && ViewInterp.OriginTime[foundidx & ORIGIN_MASK] != 0.0)
		{
			// Interpolate
			vec3_t delta;
			double frac;
			double dt;
			vec3_t neworg;

			dt = ViewInterp.OriginTime[(foundidx + 1) & ORIGIN_MASK] - ViewInterp.OriginTime[foundidx & ORIGIN_MASK];
			if (dt > 0.0)
			{
				frac = (t - ViewInterp.OriginTime[foundidx & ORIGIN_MASK]) / dt;
				frac = min(1.0, frac);
				VectorSubtract_f(ViewInterp.Origins[(foundidx + 1) & ORIGIN_MASK], ViewInterp.Origins[foundidx & ORIGIN_MASK], delta);
				VectorMA(ViewInterp.Origins[foundidx & ORIGIN_MASK], frac, delta, neworg);

				// Dont interpolate large changes
				if (Length(delta) < 64)
				{
					VectorSubtract_f(neworg, pparams->simorg, delta);

					VectorAdd_f(pparams->simorg, delta, pparams->simorg);
					VectorAdd_f(pparams->vieworg, delta, pparams->vieworg);
					VectorAdd_f(view->origin, delta, view->origin);

				}
			}
		}
	}

	// Store off v_angles before munging for third person
	v_angles = pparams->viewangles;
	v_lastAngles = pparams->viewangles;
	//	v_cl_angles = pparams->cl_viewangles;	// keep old user mouse angles !
	if (CL_IsThirdPerson())
	{
		VectorCopy_f(camAngles, pparams->viewangles);
		float pitch = camAngles[0];

		// Normalize angles
		if (pitch > 180)
			pitch -= 360.0;
		else if (pitch < -180)
			pitch += 360;

		// Player pitch is inverted
		pitch /= -3.0;

		// Slam local player's pitch value
		ent->angles[0] = pitch;
		ent->curstate.angles[0] = pitch;
		ent->prevstate.angles[0] = pitch;
		ent->latched.prevangles[0] = pitch;
	}


	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(myCameraSucks) == 2 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(myCameraSucks) == 4) {
		//let's rely on the CVars for the camera's position.

		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraPosFixedX) == -1) {
			pparams->vieworg[0] = ent->origin[0] + EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraPosOffX);
		}
		else {
			pparams->vieworg[0] = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraPosFixedX);
		}
		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraPosFixedY) == -1) {
			pparams->vieworg[1] = ent->origin[1] + EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraPosOffY);
		}
		else {
			pparams->vieworg[1] = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraPosFixedY);
		}
		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraPosFixedZ) == -1) {
			pparams->vieworg[2] = ent->origin[2] + EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraPosOffZ);
		}
		else {
			pparams->vieworg[2] = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraPosFixedZ);
		}
		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraRotFixedX) == -1) {
			pparams->viewangles[0] = ent->angles[0] + EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraRotOffX);
		}
		else {
			pparams->viewangles[0] = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraRotFixedX);
		}
		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraRotFixedY) == -1) {
			pparams->viewangles[1] = ent->angles[1] + EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraRotOffY);
		}
		else {
			pparams->viewangles[1] = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraRotFixedY);
		}
		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraRotFixedZ) == -1) {
			pparams->viewangles[2] = ent->angles[2] + EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraRotOffZ);
		}
		else {
			pparams->viewangles[2] = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(cameraRotFixedZ);
		}
	}

	/*
	if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(myCameraSucks) == 2){
		//Top-down time!
		//up / down?
		pparams->viewangles[0] = 90;
		//left / right?
		pparams->viewangles[1] = 0;
		//roll?  unsure
		pparams->viewangles[2] = 0;
		pparams->vieworg[2] = ent->origin[2] + 260;
	}
	if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(myCameraSucks) == 3){
		//Top-down time, but rotate so that the top of the screen is where the player is facing.
		//up / down?
		pparams->viewangles[0] = 90;
		//left / right?
		pparams->viewangles[1] = ent->angles[1];
		//roll?  unsure
		pparams->viewangles[2] = 0;
		//easyPrintLine("PRINTOUT SON! %.2f %.2f %.2f", pparams->viewangles[0], pparams->viewangles[1], pparams->viewangles[2]);
		pparams->vieworg[2] = ent->origin[2] + 260;
	}
	if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(myCameraSucks) == 4){
		//Top-down time, but rotate so that the top of the screen is where the player is facing.
		//up / down?
		pparams->viewangles[0] = 90;
		//left / right?
		pparams->viewangles[1] = ent->angles[1];
		//roll?  unsure
		pparams->viewangles[2] = 0;
		//easyPrintLine("PRINTOUT SON! %.2f %.2f %.2f", pparams->viewangles[0], pparams->viewangles[1], pparams->viewangles[2]);
		pparams->vieworg[2] = ent->origin[2] + 260;
	}
	*/

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(myCameraSucks) == 3 || EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(myCameraSucks) == 4) {
		easyPrintLine("PRINTOUT SON! %.2f %.2f %.2f", pparams->viewangles[0], pparams->viewangles[1], pparams->viewangles[2]);
	}


	float degreesToRadsMulti = M_PI / 180.0f;

	if (global2PSEUDO_grabbedByBarancle) {
		//CAMERA LOCATION
		pparams->vieworg[0] += EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset) * cos(pparams->viewangles[1] * degreesToRadsMulti);
		pparams->vieworg[1] += EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset) * sin(pparams->viewangles[1] * degreesToRadsMulti);

		//FIRST PERSON MODEL LOCATION
		view->origin[0] += EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset) * cos(pparams->viewangles[1] * degreesToRadsMulti);
		view->origin[1] += EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(playerBarnacleVictimViewOffset) * sin(pparams->viewangles[1] * degreesToRadsMulti);
	}



	// override all previous settings if the viewent isn't the client
	if (pparams->viewentity > pparams->maxclients)
	{
		cl_entity_t* viewentity;
		viewentity = gEngfuncs.GetEntityByIndex(pparams->viewentity);
		if (viewentity)
		{
			VectorCopy_f(viewentity->origin, pparams->vieworg);
			VectorCopy_f(viewentity->angles, pparams->viewangles);

			// Store off overridden viewangles
			v_angles = pparams->viewangles;
		}
	}

	lasttime = pparams->time;

	v_origin = pparams->vieworg;

}

void V_SmoothInterpolateAngles(float* startAngle, float* endAngle, float* finalAngle, float degreesPerSec)
{
	float absd;
	float frac;
	float d;
	float threshhold;

	NormalizeAngles(startAngle);
	NormalizeAngles(endAngle);

	for (int i = 0; i < 3; i++)
	{
		d = endAngle[i] - startAngle[i];

		if (d > 180.0f)
		{
			d -= 360.0f;
		}
		else if (d < -180.0f)
		{
			d += 360.0f;
		}

		absd = fabs(d);

		if (absd > 0.01f)
		{
			frac = degreesPerSec * v_frametime;

			threshhold = degreesPerSec / 4;

			if (absd < threshhold)
			{
				float h = absd / threshhold;
				h *= h;
				frac *= h;  // slow down last degrees
			}

			if (frac > absd)
			{
				finalAngle[i] = endAngle[i];
			}
			else
			{
				if (d > 0)
					finalAngle[i] = startAngle[i] + frac;
				else
					finalAngle[i] = startAngle[i] - frac;
			}
		}
		else
		{
			finalAngle[i] = endAngle[i];
		}

	}

	NormalizeAngles(finalAngle);
}

// Get the origin of the Observer based around the target's position and angles
void V_GetChaseOrigin(float* angles, float* origin, float distance, float* returnvec)
{
	vec3_t vecEnd;
	vec3_t forward;
	vec3_t vecStart;
	pmtrace_t* trace;
	int maxLoops = 8;

	int ignoreent = -1;	// first, ignore no entity

	cl_entity_t* ent = NULL;

	// Trace back from the target using the player's view angles
	AngleVectors(angles, forward, NULL, NULL);

	VectorScale(forward, -1, forward);

	VectorCopy_f(origin, vecStart);

	VectorMA(vecStart, distance, forward, vecEnd);

	while (maxLoops > 0)
	{
		trace = gEngfuncs.PM_TraceLine(vecStart, vecEnd, PM_TRACELINE_PHYSENTSONLY, 2, ignoreent);

		// WARNING! trace->ent is is the number in physent list not the normal entity number

		if (trace->ent <= 0)
			break;	// we hit the world or nothing, stop trace

		ent = gEngfuncs.GetEntityByIndex(PM_GetPhysEntInfo(trace->ent));

		if (ent == NULL)
			break;

		// hit non-player solid BSP , stop here
		if (ent->curstate.solid == SOLID_BSP && !ent->player)
			break;

		// if close enought to end pos, stop, otherwise continue trace
		if (Distance(trace->endpos, vecEnd) < 1.0f)
		{
			break;
		}
		else
		{
			ignoreent = trace->ent;	// ignore last hit entity
			VectorCopy_f(trace->endpos, vecStart);
		}

		maxLoops--;
	}

	/*	if ( ent )
		{
			gEngfuncs.Con_Printf("Trace loops %i , entity %i, model %s, solid %i\n",(8-maxLoops),ent->curstate.number, ent->model->name , ent->curstate.solid );
		} */

	VectorMA(trace->endpos, 4, trace->plane.normal, returnvec);

	v_lastDistance = Distance(trace->endpos, origin);	// real distance without offset
}

/*void V_GetDeathCam(cl_entity_t * ent1, cl_entity_t * ent2, float * angle, float * origin)
{
	float newAngle[3]; float newOrigin[3];

	float distance = 168.0f;

	v_lastDistance+= v_frametime * 96.0f;	// move unit per seconds back

	if ( v_resetCamera )
		v_lastDistance = 64.0f;

	if ( distance > v_lastDistance )
		distance = v_lastDistance;

	VectorCopy_f(ent1->origin, newOrigin);

	if ( ent1->player )
		newOrigin[2]+= 17; // head level of living player

	// get new angle towards second target
	if ( ent2 )
	{
		VectorSubtract_f( ent2->origin, ent1->origin, newAngle );
		VectorAngles( newAngle, newAngle );
		newAngle[0] = -newAngle[0];
	}
	else
	{
		// if no second target is given, look down to dead player
		newAngle[0] = 90.0f;
		newAngle[1] = 0.0f;
		newAngle[2] = 0;
	}

	// and smooth view
	V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 120.0f );

	V_GetChaseOrigin( angle, newOrigin, distance, origin );

	VectorCopy_f(angle, v_lastAngles);
}*/

void V_GetSingleTargetCam(cl_entity_t* ent1, float* angle, float* origin)
{
	float newAngle[3]; float newOrigin[3];

	int flags = gHUD.m_Spectator.m_iObserverFlags;

	// see is target is a dead player
	qboolean deadPlayer = ent1->player && (ent1->curstate.solid == SOLID_NOT);

	float dfactor = (flags & DRC_FLAG_DRAMATIC) ? -1.0f : 1.0f;

	float distance = 112.0f + (16.0f * dfactor); // get close if dramatic;

	// go away in final scenes or if player just died
	if (flags & DRC_FLAG_FINAL)
		distance *= 2.0f;
	else if (deadPlayer)
		distance *= 1.5f;

	// let v_lastDistance float smoothly away
	v_lastDistance += v_frametime * 32.0f;	// move unit per seconds back

	if (distance > v_lastDistance)
		distance = v_lastDistance;

	VectorCopy_f(ent1->origin, newOrigin);

	if (ent1->player)
	{
		if (deadPlayer)
			newOrigin[2] += 2;	//laying on ground
		else
			newOrigin[2] += 17; // head level of living player

	}
	else
		newOrigin[2] += 8;	// object, tricky, must be above bomb in CS

	// we have no second target, choose view direction based on
	// show front of primary target
	VectorCopy_f(ent1->angles, newAngle);

	// show dead players from front, normal players back
	if (flags & DRC_FLAG_FACEPLAYER)
		newAngle[1] += 180.0f;


	newAngle[0] += 12.5f * dfactor; // lower angle if dramatic

	// if final scene (bomb), show from real high pos
	if (flags & DRC_FLAG_FINAL)
		newAngle[0] = 22.5f;

	// choose side of object/player			
	if (flags & DRC_FLAG_SIDE)
		newAngle[1] += 22.5f;
	else
		newAngle[1] -= 22.5f;

	V_SmoothInterpolateAngles(v_lastAngles, newAngle, angle, 120.0f);

	// HACK, if player is dead don't clip against his dead body, can't check this
	V_GetChaseOrigin(angle, newOrigin, distance, origin);
}

float MaxAngleBetweenAngles(float* a1, float* a2)
{
	float d, maxd = 0.0f;

	NormalizeAngles(a1);
	NormalizeAngles(a2);

	for (int i = 0; i < 3; i++)
	{
		d = a2[i] - a1[i];
		if (d > 180)
		{
			d -= 360;
		}
		else if (d < -180)
		{
			d += 360;
		}

		d = fabs(d);

		if (d > maxd)
			maxd = d;
	}

	return maxd;
}

void V_GetDoubleTargetsCam(cl_entity_t* ent1, cl_entity_t* ent2, float* angle, float* origin)
{
	float newAngle[3]; float newOrigin[3]; float tempVec[3];

	int flags = gHUD.m_Spectator.m_iObserverFlags;

	float dfactor = (flags & DRC_FLAG_DRAMATIC) ? -1.0f : 1.0f;

	float distance = 112.0f + (16.0f * dfactor); // get close if dramatic;

	// go away in final scenes or if player just died
	if (flags & DRC_FLAG_FINAL)
		distance *= 2.0f;

	// let v_lastDistance float smoothly away
	v_lastDistance += v_frametime * 32.0f;	// move unit per seconds back

	if (distance > v_lastDistance)
		distance = v_lastDistance;

	VectorCopy_f(ent1->origin, newOrigin);

	if (ent1->player)
		newOrigin[2] += 17; // head level of living player
	else
		newOrigin[2] += 8;	// object, tricky, must be above bomb in CS

	// get new angle towards second target
	VectorSubtract_f(ent2->origin, ent1->origin, newAngle);

	VectorAngles(newAngle, newAngle);
	newAngle[0] = -newAngle[0];

	// set angle diffrent in Dramtaic scenes
	newAngle[0] += 12.5f * dfactor; // lower angle if dramatic

	if (flags & DRC_FLAG_SIDE)
		newAngle[1] += 22.5f;
	else
		newAngle[1] -= 22.5f;

	float d = MaxAngleBetweenAngles(v_lastAngles, newAngle);

	if ((d < v_cameraFocusAngle) && (v_cameraMode == CAM_MODE_RELAX))
	{
		// difference is to small and we are in relax camera mode, keep viewangles
		VectorCopy_f(v_lastAngles, newAngle);
	}
	else if ((d < v_cameraRelaxAngle) && (v_cameraMode == CAM_MODE_FOCUS))
	{
		// we catched up with our target, relax again
		v_cameraMode = CAM_MODE_RELAX;
	}
	else
	{
		// target move too far away, focus camera again
		v_cameraMode = CAM_MODE_FOCUS;
	}

	// and smooth view, if not a scene cut
	if (v_resetCamera || (v_cameraMode == CAM_MODE_RELAX))
	{
		VectorCopy_f(newAngle, angle);
	}
	else
	{
		V_SmoothInterpolateAngles(v_lastAngles, newAngle, angle, 180.0f);
	}

	V_GetChaseOrigin(newAngle, newOrigin, distance, origin);

	// move position up, if very close at target
	if (v_lastDistance < 64.0f)
		origin[2] += 16.0f * (1.0f - (v_lastDistance / 64.0f));

	// calculate angle to second target
	VectorSubtract_f(ent2->origin, origin, tempVec);
	VectorAngles(tempVec, tempVec);
	tempVec[0] = -tempVec[0];

	/* take middle between two viewangles
	InterpolateAngles( newAngle, tempVec, newAngle, 0.5f); */
}

void V_GetDirectedChasePosition(cl_entity_t* ent1, cl_entity_t* ent2, float* angle, float* origin)
{

	if (v_resetCamera)
	{
		v_lastDistance = 4096.0f;
		// v_cameraMode = CAM_MODE_FOCUS;
	}

	if ((ent2 == (cl_entity_t*)0xFFFFFFFF) || (ent1->player && (ent1->curstate.solid == SOLID_NOT)))
	{
		// we have no second target or player just died
		V_GetSingleTargetCam(ent1, angle, origin);
	}
	else if (ent2)
	{
		// keep both target in view
		V_GetDoubleTargetsCam(ent1, ent2, angle, origin);
	}
	else
	{
		// second target disappeard somehow (dead)

		// keep last good viewangle
		float newOrigin[3];

		int flags = gHUD.m_Spectator.m_iObserverFlags;

		float dfactor = (flags & DRC_FLAG_DRAMATIC) ? -1.0f : 1.0f;

		float distance = 112.0f + (16.0f * dfactor); // get close if dramatic;

		// go away in final scenes or if player just died
		if (flags & DRC_FLAG_FINAL)
			distance *= 2.0f;

		// let v_lastDistance float smoothly away
		v_lastDistance += v_frametime * 32.0f;	// move unit per seconds back

		if (distance > v_lastDistance)
			distance = v_lastDistance;

		VectorCopy_f(ent1->origin, newOrigin);

		if (ent1->player)
			newOrigin[2] += 17; // head level of living player
		else
			newOrigin[2] += 8;	// object, tricky, must be above bomb in CS

		V_GetChaseOrigin(angle, newOrigin, distance, origin);
	}

	VectorCopy_f(angle, v_lastAngles);
}

void V_GetChasePos(int target, float* cl_angles, float* origin, float* angles)
{
	cl_entity_t* ent = NULL;

	if (target)
	{
		ent = gEngfuncs.GetEntityByIndex(target);
	};

	if (!ent)
	{
		// just copy a save in-map position
		VectorCopy_f(vJumpAngles, angles);
		VectorCopy_f(vJumpOrigin, origin);
		return;
	}

	if (gHUD.m_Spectator.m_autoDirector->value)
	{
		if (g_iUser3)
			V_GetDirectedChasePosition(ent, gEngfuncs.GetEntityByIndex(g_iUser3),
				angles, origin);
		else
			V_GetDirectedChasePosition(ent, (cl_entity_t*)0xFFFFFFFF,
				angles, origin);
	}
	else
	{
		if (cl_angles == NULL)	// no mouse angles given, use entity angles ( locked mode )
		{
			VectorCopy_f(ent->angles, angles);
			angles[0] *= -1;
		}
		else
			VectorCopy_f(cl_angles, angles);


		VectorCopy_f(ent->origin, origin);

		//MODDD - actual constant used, why not.
		origin[2] += DEFAULT_VIEWHEIGHT; // DEFAULT_VIEWHEIGHT - some offset

		V_GetChaseOrigin(angles, origin, cl_chasedist->value, origin);
	}

	v_resetCamera = false;
}

void V_ResetChaseCam()
{
	v_resetCamera = true;
}


void V_GetInEyePos(int target, float* origin, float* angles)
{
	if (!target)
	{
		// just copy a save in-map position
		VectorCopy_f(vJumpAngles, angles);
		VectorCopy_f(vJumpOrigin, origin);
		return;
	};

	cl_entity_t* ent = gEngfuncs.GetEntityByIndex(target);

	if (!ent)
		return;

	VectorCopy_f(ent->origin, origin);
	VectorCopy_f(ent->angles, angles);

	angles[PITCH] *= -3.0f;	// see CL_ProcessEntityUpdate()

	if (ent->curstate.solid == SOLID_NOT)
	{
		//MODDD - DEAD CAMERA SETTING
		angles[ROLL] = 80;	// dead view angle
		origin[2] += PM_DEAD_VIEWHEIGHT; //MODDD  - just use the constant dangit
	}
	else if (ent->curstate.usehull == 1) {
		origin[2] += VEC_DUCK_VIEW_Z; //MODDD - constant used, why not.
	}
	else {
		// exacty eye position can't be caluculated since it depends on
		// client values like cl_bobcycle, this offset matches the default values
		origin[2] += DEFAULT_VIEWHEIGHT; //MODDD - constant used, why not.
	}
}

void V_GetMapFreePosition(float* cl_angles, float* origin, float* angles)
{
	vec3_t forward;
	vec3_t zScaledTarget;

	VectorCopy_f(cl_angles, angles);

	// modify angles since we don't wanna see map's bottom
	angles[0] = 51.25f + 38.75f * (angles[0] / 90.0f);

	zScaledTarget[0] = gHUD.m_Spectator.m_mapOrigin[0];
	zScaledTarget[1] = gHUD.m_Spectator.m_mapOrigin[1];
	zScaledTarget[2] = gHUD.m_Spectator.m_mapOrigin[2] * ((90.0f - angles[0]) / 90.0f);


	AngleVectors(angles, forward, NULL, NULL);

	VectorNormalize(forward);

	VectorMA(zScaledTarget, -(4096.0f / gHUD.m_Spectator.m_mapZoom), forward, origin);
}

void V_GetMapChasePosition(int target, float* cl_angles, float* origin, float* angles)
{
	vec3_t forward;

	if (target)
	{
		cl_entity_t* ent = gEngfuncs.GetEntityByIndex(target);

		if (gHUD.m_Spectator.m_autoDirector->value)
		{
			// this is done to get the angles made by director mode
			V_GetChasePos(target, cl_angles, origin, angles);
			VectorCopy_f(ent->origin, origin);

			// keep fix chase angle horizontal
			angles[0] = 45.0f;
		}
		else
		{
			VectorCopy_f(cl_angles, angles);
			VectorCopy_f(ent->origin, origin);

			// modify angles since we don't wanna see map's bottom
			angles[0] = 51.25f + 38.75f * (angles[0] / 90.0f);
		}
	}
	else
	{
		// keep out roaming position, but modify angles
		VectorCopy_f(cl_angles, angles);
		angles[0] = 51.25f + 38.75f * (angles[0] / 90.0f);
	}

	origin[2] *= ((90.0f - angles[0]) / 90.0f);
	angles[2] = 0.0f;	// don't roll angle (if chased player is dead)

	AngleVectors(angles, forward, NULL, NULL);

	VectorNormalize(forward);

	VectorMA(origin, -1536, forward, origin);
}

int V_FindViewModelByWeaponModel(int weaponindex)
{
	static char* modelmap[][2] = {
		{ "models/p_crossbow.mdl", "models/v_crossbow.mdl" },
		{ "models/p_crowbar.mdl", "models/v_crowbar.mdl" },
		{ "models/p_egon.mdl", "models/v_egon.mdl" },
		{ "models/p_gauss.mdl", "models/v_gauss.mdl" },
		{ "models/p_9mmhandgun.mdl", "models/v_9mmhandgun.mdl" },
		{ "models/p_grenade.mdl", "models/v_grenade.mdl" },
		{ "models/p_hgun.mdl", "models/v_hgun.mdl" },
		{ "models/p_9mmAR.mdl", "models/v_9mmAR.mdl" },
		{ "models/p_357.mdl", "models/v_357.mdl" },
		{ "models/p_rpg.mdl", "models/v_rpg.mdl" },
		{ "models/p_shotgun.mdl", "models/v_shotgun.mdl" },
		{ "models/p_squeak.mdl", "models/v_squeak.mdl" },
		{ "models/p_tripmine.mdl", "models/v_tripmine.mdl" },
		{ "models/p_satchel_radio.mdl", "models/v_satchel_radio.mdl" },
		{ "models/p_satchel.mdl", "models/v_satchel.mdl" },
		//MODDD - new.  Note that the chumtoad does not yet have a "p_..." model.
		{ "models/chumtoad.mdl", "models/v_chub.mdl" },
		{ NULL, NULL }
	};

	struct model_s* weaponModel = IEngineStudio.GetModelByIndex(weaponindex);

	if (weaponModel)
	{
		int len = strlen(weaponModel->name);
		int i = 0;

		while (modelmap[i] != NULL)
		{
			if (!strnicmp(weaponModel->name, modelmap[i][0], len))
			{
				return gEngfuncs.pEventAPI->EV_FindModelIndex(modelmap[i][1]);
			}
			i++;
		}

		return 0;
	}
	else
		return 0;

}


/*
==================
V_CalcSpectatorRefdef

==================
*/
void V_CalcSpectatorRefdef(struct ref_params_s* pparams)
{
	static vec3_t			velocity(0.0f, 0.0f, 0.0f);

	static int lastWeaponModelIndex = 0;
	static int lastViewModelIndex = 0;

	cl_entity_t* ent = gEngfuncs.GetEntityByIndex(g_iUser2);

	pparams->onlyClientDraw = false;

	// refresh position
	VectorCopy_f(pparams->simorg, v_sim_org);

	// get old values
	VectorCopy_f(pparams->cl_viewangles, v_cl_angles);
	VectorCopy_f(pparams->viewangles, v_angles);
	VectorCopy_f(pparams->vieworg, v_origin);

	if ((g_iUser1 == OBS_IN_EYE || gHUD.m_Spectator.m_pip->value == INSET_IN_EYE) && ent)
	{
		// calculate player velocity
		float timeDiff = ent->curstate.msg_time - ent->prevstate.msg_time;

		if (timeDiff > 0)
		{
			vec3_t distance;
			VectorSubtract_f(ent->prevstate.origin, ent->curstate.origin, distance);
			VectorScale(distance, 1 / timeDiff, distance);

			velocity[0] = velocity[0] * 0.9f + distance[0] * 0.1f;
			velocity[1] = velocity[1] * 0.9f + distance[1] * 0.1f;
			velocity[2] = velocity[2] * 0.9f + distance[2] * 0.1f;

			VectorCopy_f(velocity, pparams->simvel);
		}

		// predict missing client data and set weapon model ( in HLTV mode or inset in eye mode )
		if (gEngfuncs.IsSpectateOnly())
		{
			V_GetInEyePos(g_iUser2, pparams->simorg, pparams->cl_viewangles);

			pparams->health = 1;

			cl_entity_t* gunModel = gEngfuncs.GetViewModel();

			if (lastWeaponModelIndex != ent->curstate.weaponmodel)
			{
				// weapon model changed

				lastWeaponModelIndex = ent->curstate.weaponmodel;
				lastViewModelIndex = V_FindViewModelByWeaponModel(lastWeaponModelIndex);
				if (lastViewModelIndex)
				{
					gEngfuncs.pfnWeaponAnim(0, 0);	// reset weapon animation
				}
				else
				{
					// model not found
					gunModel->model = NULL;	// disable weapon model
					lastWeaponModelIndex = lastViewModelIndex = 0;
				}
			}

			if (lastViewModelIndex)
			{
				gunModel->model = IEngineStudio.GetModelByIndex(lastViewModelIndex);
				gunModel->curstate.modelindex = lastViewModelIndex;
				gunModel->curstate.frame = 0;
				gunModel->curstate.colormap = 0;
				gunModel->index = g_iUser2;
			}
			else
			{
				gunModel->model = NULL;	// disable weaopn model
			}
		}
		else
		{
			// only get viewangles from entity
			VectorCopy_f(ent->angles, pparams->cl_viewangles);
			pparams->cl_viewangles[PITCH] *= -3.0f;	// see CL_ProcessEntityUpdate()
		}
	}

	v_frametime = pparams->frametime;

	if (pparams->nextView == 0)
	{
		// first renderer cycle, full screen

		switch (g_iUser1)
		{
		case OBS_CHASE_LOCKED:	V_GetChasePos(g_iUser2, NULL, v_origin, v_angles);
			break;

		case OBS_CHASE_FREE:	V_GetChasePos(g_iUser2, v_cl_angles, v_origin, v_angles);
			break;

		case OBS_ROAMING:	VectorCopy_f(v_cl_angles, v_angles);
			VectorCopy_f(v_sim_org, v_origin);
			break;

		case OBS_IN_EYE:   V_CalcNormalRefdef(pparams);
			break;

		case OBS_MAP_FREE:	pparams->onlyClientDraw = true;
			V_GetMapFreePosition(v_cl_angles, v_origin, v_angles);
			break;

		case OBS_MAP_CHASE:	pparams->onlyClientDraw = true;
			V_GetMapChasePosition(g_iUser2, v_cl_angles, v_origin, v_angles);
			break;
		}

		if (gHUD.m_Spectator.m_pip->value)
			pparams->nextView = 1;	// force a second renderer view

		gHUD.m_Spectator.m_iDrawCycle = 0;

	}
	else
	{
		// second renderer cycle, inset window

		// set inset parameters
		pparams->viewport[0] = XRES(gHUD.m_Spectator.m_OverviewData.insetWindowX);	// change viewport to inset window
		pparams->viewport[1] = YRES(gHUD.m_Spectator.m_OverviewData.insetWindowY);
		pparams->viewport[2] = XRES(gHUD.m_Spectator.m_OverviewData.insetWindowWidth);
		pparams->viewport[3] = YRES(gHUD.m_Spectator.m_OverviewData.insetWindowHeight);
		pparams->nextView = 0;	// on further view

		// override some settings in certain modes
		switch ((int)gHUD.m_Spectator.m_pip->value)
		{
		case INSET_CHASE_FREE: V_GetChasePos(g_iUser2, v_cl_angles, v_origin, v_angles);
			break;

		case INSET_IN_EYE:	V_CalcNormalRefdef(pparams);
			break;

		case INSET_MAP_FREE:	pparams->onlyClientDraw = true;
			V_GetMapFreePosition(v_cl_angles, v_origin, v_angles);
			break;

		case INSET_MAP_CHASE:	pparams->onlyClientDraw = true;

			if (g_iUser1 == OBS_ROAMING)
				V_GetMapChasePosition(0, v_cl_angles, v_origin, v_angles);
			else
				V_GetMapChasePosition(g_iUser2, v_cl_angles, v_origin, v_angles);

			break;
		}

		gHUD.m_Spectator.m_iDrawCycle = 1;
	}

	// write back new values into pparams
	VectorCopy_f(v_cl_angles, pparams->cl_viewangles);
	//MODDD - missing a semicolon?  Evidence of using the '_f' version.
	VectorCopy_f(v_angles, pparams->viewangles);
	VectorCopy_f(v_origin, pparams->vieworg);
}



void DLLEXPORT V_CalcRefdef(struct ref_params_s* pparams)
{
	// intermission / finale rendering
	if (pparams->intermission)
	{
		V_CalcIntermissionRefdef(pparams);
	}
	else if (pparams->spectator || g_iUser1)	// g_iUser true if in spectator mode
	{
		V_CalcSpectatorRefdef(pparams);
	}
	else if (!pparams->paused)
	{
		V_CalcNormalRefdef(pparams);
	}

	/*
	// Example of how to overlay the whole screen with red at 50 % alpha
	#define SF_TEST
	#if defined SF_TEST
		{
			screenfade_t sf;
			gEngfuncs.pfnGetScreenFade( &sf );

			sf.fader = 255;
			sf.fadeg = 0;
			sf.fadeb = 0;
			sf.fadealpha = 128;
			sf.fadeFlags = FFADE_STAYOUT | FFADE_OUT;

			gEngfuncs.pfnSetScreenFade( &sf );
		}
	#endif
	*/
}

/*
=============
V_DropPunchAngle

=============
*/
void V_DropPunchAngle(float frametime, float* arg_ev_punchangle)
{
	float	len;

	len = VectorNormalize(arg_ev_punchangle);

	//MODDD - little touchup
	// UNDONE, let's be careful about changes, even small ones.
	// Increased kickbacks since, go ahead.
	// Now a CVar.

	if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) != 1){
		// retail
		len -= (10.0 + len * 0.5) * frametime;
		// alternate similar way?
		//len -= (11.0 + len * 0.67) * frametime;
	}else{
		// modded way.  Decays much faster with distance, lower minimum rate.
		// Expected to work with higher-kickbacks in general.
		len -= (3.7 + len * 2.85) * frametime;
	}

	len = max(len, 0.0);
	VectorScale(arg_ev_punchangle, len, arg_ev_punchangle);
}

/*
=============
V_PunchAxis

Client side punch effect
=============
*/
void V_PunchAxis(int axis, float punch)
{
	if (EASY_CVAR_GET(cl_viewpunch) > 0) {
		ev_punchangle[axis] = punch * EASY_CVAR_GET(cl_viewpunch);
	}
}



//MODDDMIRROR - both methods below.
void CMD_ThirdPerson(void)
{
	if (!gHUD.viewFlags) {
		gHUD.m_iLastCameraMode = gHUD.m_iCameraMode = 1;

	}
	else {
		gHUD.m_iLastCameraMode = 1;
	}//set new view after release camera


	gHUD.PESUDO_cameraModeMem = 1;
	global2PSEUDO_IGNOREcameraMode = 1;
	command_updateCameraPerspectiveT();

}

void CMD_FirstPerson(void)
{
	if (!gHUD.viewFlags) {
		gHUD.m_iLastCameraMode = gHUD.m_iCameraMode = 0;
	}
	else {
		gHUD.m_iLastCameraMode = 0;
	}//set new view after release camera


	gHUD.PESUDO_cameraModeMem = 0;
	global2PSEUDO_IGNOREcameraMode = 0;
	command_updateCameraPerspectiveF();

}



/*
=============
V_Init
=============
*/
void V_Init(void)
{
	gEngfuncs.pfnAddCommand("centerview", V_StartPitchDrift);

	scr_ofsx = gEngfuncs.pfnRegisterVariable("scr_ofsx", "0", 0);
	scr_ofsy = gEngfuncs.pfnRegisterVariable("scr_ofsy", "0", 0);
	scr_ofsz = gEngfuncs.pfnRegisterVariable("scr_ofsz", "0", 0);

	v_centermove = gEngfuncs.pfnRegisterVariable("v_centermove", "0.15", 0);
	v_centerspeed = gEngfuncs.pfnRegisterVariable("v_centerspeed", "500", 0);

	cl_bobcycle = gEngfuncs.pfnRegisterVariable("cl_bobcycle", "0.8", 0);// best default for my experimental gun wag (sjb)
	cl_bob = gEngfuncs.pfnRegisterVariable("cl_bob", "0.01", 0);// best default for my experimental gun wag (sjb)
	cl_bobup = gEngfuncs.pfnRegisterVariable("cl_bobup", "0.5", 0);
	cl_waterdist = gEngfuncs.pfnRegisterVariable("cl_waterdist", "4", 0);
	cl_chasedist = gEngfuncs.pfnRegisterVariable("cl_chasedist", "112", 0);

	//MODDDMIRROR
	//nah, see in_camera.cpp where it is already done (methods seem ignored in here?)
	//gEngfuncs.pfnAddCommand		( "thirdperson", CMD_ThirdPerson );	//G-Cont
	//gEngfuncs.pfnAddCommand		( "firstperson", CMD_FirstPerson );	//G-Cont

}


//#define TRACE_TEST
#if defined( TRACE_TEST )

extern float in_fov;
/*
====================
CalcFov
====================
*/
float CalcFov(float fov_x, float width, float height)
{
	float	a;
	float	x;

	if (fov_x < 1 || fov_x > 179)
		fov_x = 90;	// error, set to 90

	x = width / tan(fov_x / 360.0f * M_PI);

	a = atan(height / x);

	a = a * 360.0f / M_PI;

	return a;
}

int hitent = -1;

void V_Move(int mx, int my)
{
	float fov;
	float fx, fy;
	float dx, dy;
	float c_x, c_y;
	float dX, dY;
	vec3_t forward, up, right;
	vec3_t newangles;

	vec3_t farpoint;
	pmtrace_t tr;

	fov = CalcFov(in_fov, (float)ScreenWidth, (float)ScreenHeight);

	c_x = (float)ScreenWidth / 2.0;
	c_y = (float)ScreenHeight / 2.0;

	dx = (float)mx - c_x;
	dy = (float)my - c_y;

	// Proportion we moved in each direction
	fx = dx / c_x;
	fy = dy / c_y;

	dX = fx * in_fov / 2.0;
	dY = fy * fov / 2.0;

	newangles = v_angles;

	newangles[YAW] -= dX;
	newangles[PITCH] += dY;

	// Now rotate v_forward around that point
	AngleVectors(newangles, forward, right, up);

	farpoint = v_origin + 8192 * forward;

	// Trace
	tr = *(gEngfuncs.PM_TraceLine((float*)&v_origin, (float*)&farpoint, PM_TRACELINE_PHYSENTSONLY, 2 /*point sized hull*/, -1));

	if (tr.fraction != 1.0 && tr.ent != 0)
	{
		hitent = PM_GetPhysEntInfo(tr.ent);
		PM_ParticleLine((float*)&v_origin, (float*)&tr.endpos, 5, 1.0, 0.0);
	}
	else
	{
		hitent = -1;
	}
}

#endif
