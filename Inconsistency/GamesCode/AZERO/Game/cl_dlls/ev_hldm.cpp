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

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"
#include "eventscripts.h"
#include "ev_hldm.h"
#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "r_studioint.h"
#include "com_model.h"
#include "hl/hl_weapons.h"

//MODDD - new include.
// See the note at the top of ev_hldm.h about several removals in here (now in dlls/<specific weapon.h files>
// to avoid redundancy).
#include "dlls/weapon/chumtoadweapon.h"
#include "dlls/weapon/crossbow.h"
#include "dlls/weapon/crowbar.h"
#include "dlls/weapon/egon.h"
#include "dlls/weapon/gauss.h"
#include "dlls/weapon/glock.h"
#include "dlls/weapon/handgrenade.h"
#include "dlls/weapon/hornetgun.h"
#include "dlls/weapon/mp5.h"
#include "dlls/weapon/python.h"
#include "dlls/weapon/rpg.h"
#include "dlls/weapon/satchel.h"
#include "dlls/weapon/shotgun.h"
#include "dlls/weapon/squeak.h"
#include "dlls/weapon/tripmine.h"


//MODDD - new include.  THESE WERE NEVER ON, deemed unnecessary.
//#include "hud_iface.h"
//#include "extdll.h"
//#include "util.h"  <-- includes  """ #include "enginecallback.h" """

//ALSO UNNECESSARY. just use "gEngfuncs".
//#include "enginecallback.h"
//should be acceptable.

EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeDurationMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeDurationMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeRadiusMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeRadiusMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(strobeMultiColor)

EASY_CVAR_EXTERN(ravelaserlength)

EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax)

EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserDurationMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserDurationMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserMultiColor)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserEnabled)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserLength)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(gauss_mode)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(playerBulletHitEffectForceServer)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire)
EASY_CVAR_EXTERN_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(playerWeaponTracerMode)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts)
EASY_CVAR_EXTERN(cl_gaussfollowattachment)
EASY_CVAR_EXTERN_DEBUGONLY(sparksPlayerCrossbowMulti)
EASY_CVAR_EXTERN(cl_mp5_evil_skip)

EASY_CVAR_EXTERN(cl_mp5_viewpunch_mod)
EASY_CVAR_EXTERN_CLIENTONLY(cl_viewpunch_mod)
EASY_CVAR_EXTERN_CLIENTONLY(cl_gauss_viewpunch_mod)



//MODDD - from in_camera.cpp 
extern "C"
{
	void DLLEXPORT CAM_Think(void);
	int DLLEXPORT CL_IsThirdPerson(void);
	void DLLEXPORT CL_CameraOffset(float* ofs);
}




extern engine_studio_api_t IEngineStudio;
extern float g_flApplyVel;
extern cvar_t* cl_lw;

extern "C" {
	extern char PM_FindTextureType(char* name);
	extern int PM_GetPhysEntInfo(int ent);
}



//MODDD - is it safe to assume every integer of this tracerCount array is going to start at 0?  And not garbage memory?
//        I assume it has one index per type of weapon to store the number of bullets
//        fired yet per weapon for figuring out how often to draw a tracer effect.
//        ...if what index is used comes from the "entity" index of the caller, we sure do hope it's between 0 and 31 
//        inclusive then.
static int tracerCount[32];


void V_PunchAxis(int axis, float punch);


//MODDD - event method EV_... prototypes / externs moved to ev_hldm.h
//VECTOR_CONE_... macros moved to util_shared.h

// extern the client CVars here.   ...or not
//EASY_CVAR_EXTERN_CLIENT_MASS

/*
EASY_CVAR_DECLARATION_CLIENT_MASS

//similar to how serverside's "updateCVarRefs" of combat.cpp, but this is just for the rave lights.
void updateCVarRefsClient(){

	EASY_CVAR_UPDATE_CLIENT_MASS

}
*/





//MODDD - few replacements with macros for convenience.
// Only for the terms in ev_hldm.cpp.
///////////////////////////////////////////////////////////////////////////////////////////
// The FILLIN_TRACEFLAGS_... choices let any existing reference to PM_whatever
// be replaced specifically, like all that used to be PM_STUDIO_BOX's turn into
// PM_NORMAL's.

// retail way
/*
#define FILLIN_TRACE_HULL HULL_TYPE::large_hull
#define FILLIN_TRACEFLAGS_STUDIO_BOX PM_STUDIO_BOX
#define FILLIN_TRACEFLAGS_NORMAL PM_NORMAL
*/

// new way, better accuracy at times
#define FILLIN_TRACE_HULL large_hull
#define FILLIN_TRACEFLAGS_STUDIO_BOX PM_NORMAL
#define FILLIN_TRACEFLAGS_NORMAL PM_NORMAL
///////////////////////////////////////////////////////////////////////////////////////////




extern float g_cl_mp5_NextAnimTime;








// play a strike sound based on the texture that was hit by the attack traceline.  VecSrc/VecEnd are the
// original traceline endpoints used by the attacker, iBulletType is the type of bullet that hit the texture.
// returns volume of strike instrument (crowbar) to play
//MODDD - IMPORTANT. This will find a chTextureType of '\0' (null character; numeric 0) if the hit surface is an entity at all.
//        Or simply not part of the map geometry, probably that.
float EV_HLDM_PlayTextureSound(int idx, pmtrace_t* ptr, float* vecSrc, float* vecEnd, int iBulletType)
{
	// hit the world, try to play sound based on texture material type
	char chTextureType;   //what... was the point of the CHAR_TEX_CONCRETE default to be replaced with 0 shortly below
	float fvol;
	float fvolbar;
	char* rgsz[4];
	int cnt;
	float fattn = ATTN_NORM;
	int entityIndex;
	char* pTextureName;
	char texname[64];
	char szbuffer[64];

	// FIXME check if playtexture sounds movevar is set
	//

	//MODDD - the above was not written by me, appeared as-is.  Dunno what a "movevar" is. pm_shared.c thing? huh?
	//Anyhow, mutes these sounds if "muteBulletHitSounds" is on, or the forceAllowServersideTextureSounds choice of 2 (only serverside allowed).
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteBulletHitSounds) == 1 || EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(forceAllowServersideTextureSounds) == 2) {
		return 0;
	}


	chTextureType = 0;


	BOOL isEntityWorld;
	entityIndex = gEngfuncs.pEventAPI->EV_IndexFromTrace(ptr);

	cl_entity_t* cEntRefAlt = gEngfuncs.GetEntityByIndex(entityIndex);

	// a 'null' check for a number where 0 is valid doesn't make sense.
	//if (entity == NULL) {
	//	isEntityWorld = TRUE;  // just go ahead and say 'yes'.
	//}
	//else {
		// more to work with.
		//isEntityWorld = (pEntity->IsWorld() || pEntity->Classify() == CLASS_NONE);

		//TODO - for now, assume anything without the ISNPC flag is world-affiliated, or whatever.
		// Later go in ad ad ISWORLDAFFILIATED to all stuff with say Class == CLASS_NONE or indeed, that IsWorldAffiliated() as TRUE.
		// No idea why serverside didn't check for that too.
		// There is also a check for pEntity->solid == SOLID_BSP that can be done (curstate.renderfx, or get a physent if possible
		// and do pe->solid?  If it's the same result, just checking).  Would any non-map entities be SOLID_BSP?
		//isEntityWorld = (entityIndex == 0 || cEntRefAlt->curstate.renderfx & ISWORLDAFFILIATED);
		isEntityWorld = (entityIndex == 0 || !(cEntRefAlt->curstate.renderfx & ISNPC) );

	//}




	
	//cl_entity_t* testent = gEngfuncs.GetEntityByIndex(entityIndex);
	//testent->curstate.playerclass;
	//testent->curstate.number;

	if (!isEntityWorld) {
		// can be a little more specific.


		if ((entityIndex >= 1 && entityIndex <= gEngfuncs.GetMaxClients()))
		{
			// hit body, players count as flesh.   Unless there's some weird german censorship thing to force them into robots I don't know of.
			chTextureType = CHAR_TEX_FLESH;
		}
		else {
			// Not the world, not a player.  Is it organic?

			//physent_t* pe = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);
			//cl_entity_t* cEntRef = gEngfuncs.GetEntityByIndex(PM_GetPhysEntInfo(tr.ent));
			
			
			// !!!
			// possible to check for ptr->hitgroup?  Doubt it's necesary.

			if(cEntRefAlt != NULL && ((cEntRefAlt->curstate.renderfx & ISMETALNPC) == ISMETALNPC) ){
				// metal.
				// Come to think of it, odd that retail behavior on serverside would have tried to trace the texture of a non-organic entity (also non-world) anyway,
				// but clientside never did, required that strict 'entityIntex == 0' check to do that.
				
					// CHANGE, custom sound behavior.  Use all the metal sounds, some are used by func_breakable vents, seems fitting for hitting
				// a sentry.
				//chTextureType = CHAR_TEX_METAL;

				// NOTE - is sending the entityIndex instead of the world index (0) ok?  No idea.
				// organic sounds will still use it from relying on normal texture logic below though.
				const int randomSound = gEngfuncs.pfnRandomLong(0, 5);
				const float metalVol = 0.57;
				const float metalAttn = ATTN_NORM - 0.02;
				switch (randomSound) {
				case 0:gEngfuncs.pEventAPI->EV_PlaySound(entityIndex, ptr->endpos, CHAN_STATIC, "debris/metal1.wav", metalVol - 0.15, metalAttn, 0, 92 + gEngfuncs.pfnRandomLong(0, 5));
				case 1:gEngfuncs.pEventAPI->EV_PlaySound(entityIndex, ptr->endpos, CHAN_STATIC, "debris/metal2.wav", metalVol, metalAttn, 0, 94 + gEngfuncs.pfnRandomLong(0, 5));
				case 2:gEngfuncs.pEventAPI->EV_PlaySound(entityIndex, ptr->endpos, CHAN_STATIC, "debris/metal3.wav", metalVol, metalAttn, 0, 94 + gEngfuncs.pfnRandomLong(0, 5));
				case 3:gEngfuncs.pEventAPI->EV_PlaySound(entityIndex, ptr->endpos, CHAN_STATIC, "debris/metal4.wav", metalVol - 0.1, metalAttn, 0, 90 + gEngfuncs.pfnRandomLong(0, 5));
				case 4:gEngfuncs.pEventAPI->EV_PlaySound(entityIndex, ptr->endpos, CHAN_STATIC, "debris/metal5.wav", metalVol - 0.08, metalAttn, 0, 92 + gEngfuncs.pfnRandomLong(0, 5));
				case 5:gEngfuncs.pEventAPI->EV_PlaySound(entityIndex, ptr->endpos, CHAN_STATIC, "debris/metal6.wav", metalVol, metalAttn, 0, 94 + gEngfuncs.pfnRandomLong(0, 5));
				}//switch

				fvolbar = 0.45;
				return fvolbar;  //and done.
			}else {
				// hit body
				chTextureType = CHAR_TEX_FLESH;
			}

		}


	}
	else
	{
		// It is the world?

		/*
		// OLD WAY! replaced with below
		pTextureName = (char *)gEngfuncs.pEventAPI->EV_TraceTexture( ptr->ent, vecSrc, vecEnd );
		*/

		//MODDD - new chunk, should fix a little obscure issue with certain texture-hit sounds being wrong.
		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		pmtrace_t tr;
		// do we even need this again..?
		//gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

		gEngfuncs.pEventAPI->EV_SetTraceHull(FILLIN_TRACE_HULL);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, FILLIN_TRACEFLAGS_STUDIO_BOX, -1, &tr);

		vec3_t vecSrc_trace;
		vec3_t vecEnd_trace;

		if (tr.fraction == 1.0) {
			//WTF.
			return 0.0;
		}
		else {
			vec3_t thaDirr;

			VectorSubtract_f(vecEnd, vecSrc, thaDirr);
			float theLen = VectorNormalize(thaDirr);

			//vecSrc_trace = tr.endpos - thaDirr * 3;
			//vecEnd_trace = tr.endpos + thaDirr * 3;

			vec3_t negaDirr;
			vec3_t posaDirr;
			VectorScale(thaDirr, -3, negaDirr);
			VectorScale(thaDirr, 3, posaDirr);

			VectorAdd_f(tr.endpos, negaDirr, vecSrc_trace);
			VectorAdd_f(tr.endpos, posaDirr, vecEnd_trace);

		}

		// get texture from entity or world (world is ent(0))
		pTextureName = (char*)gEngfuncs.pEventAPI->EV_TraceTexture(ptr->ent, vecSrc_trace, vecEnd_trace);
		////////////////////////////////////////////////////////////////////////////////////////////////////////////


		if (pTextureName)
		{
			strcpy(texname, pTextureName);
			pTextureName = texname;

			// strip leading '-0' or '+0~' or '{' or '!'
			if (*pTextureName == '-' || *pTextureName == '+'){
				pTextureName += 2;
			}

			if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' '){
				pTextureName++;
			}

			// '}}'
			strcpy(szbuffer, pTextureName);
			szbuffer[CBTEXTURENAMEMAX - 1] = 0;

			// get texture type
			chTextureType = PM_FindTextureType(szbuffer);
		}
	}//END OF world check



	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(textureHitSoundPrintouts) == 1)easyPrintLine("EV PLAY TEXTURE SOUND: %d, %c", (int)chTextureType, chTextureType);

	//MODDD - healthy default, just in case nothing was picked?
	cnt = 0;

	switch (chTextureType)
	{
		//...I mean, clever.
		//No, let's not do that.
		//default:
	case CHAR_TEX_CONCRETE: fvol = 0.9;	fvolbar = 0.6;
		rgsz[0] = "player/pl_step1.wav";
		rgsz[1] = "player/pl_step2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_METAL: fvol = 0.9; fvolbar = 0.3;
		rgsz[0] = "player/pl_metal1.wav";
		rgsz[1] = "player/pl_metal2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_DIRT:	fvol = 0.9; fvolbar = 0.1;
		rgsz[0] = "player/pl_dirt1.wav";
		rgsz[1] = "player/pl_dirt2.wav";
		rgsz[2] = "player/pl_dirt3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_VENT:	fvol = 0.5; fvolbar = 0.3;
		rgsz[0] = "player/pl_duct1.wav";
		rgsz[1] = "player/pl_duct1.wav";
		cnt = 2;
		break;
	case CHAR_TEX_GRATE: fvol = 0.9; fvolbar = 0.5;
		rgsz[0] = "player/pl_grate1.wav";
		rgsz[1] = "player/pl_grate4.wav";
		cnt = 2;
		break;
	case CHAR_TEX_TILE:	fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "player/pl_tile1.wav";
		rgsz[1] = "player/pl_tile3.wav";
		rgsz[2] = "player/pl_tile2.wav";
		rgsz[3] = "player/pl_tile4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_SLOSH: fvol = 0.9; fvolbar = 0.0;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_WOOD: fvol = 0.9; fvolbar = 0.2;
		rgsz[0] = "debris/wood1.wav";
		rgsz[1] = "debris/wood2.wav";
		rgsz[2] = "debris/wood3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_GLASS:
	case CHAR_TEX_COMPUTER:
		fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "debris/glass1.wav";
		rgsz[1] = "debris/glass2.wav";
		rgsz[2] = "debris/glass3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_FLESH:

		if (iBulletType == BULLET_PLAYER_CROWBAR)
			return 0.0; // crowbar already makes this sound

		fvol = 1.0;	fvolbar = 0.2;
		rgsz[0] = "weapons/bullet_hit1.wav";
		rgsz[1] = "weapons/bullet_hit2.wav";
		fattn = 1.0;
		cnt = 2;
		break;
	}

	// play material hit sound
	if (cnt > 0) {
		gEngfuncs.pEventAPI->EV_PlaySound(0, ptr->endpos, CHAN_STATIC, rgsz[gEngfuncs.pfnRandomLong(0, cnt - 1)], fvol, fattn, 0, 96 + gEngfuncs.pfnRandomLong(0, 0xf));
	}
	return fvolbar;
}



char* EV_HLDM_DamageDecal(physent_t* pe)
{
	static char decalname[32];
	int idx;

	// what does 'classnumber == 1' mean
	if (pe->classnumber == 1)
	{
		idx = gEngfuncs.pfnRandomLong(0, 2);
		sprintf(decalname, "{break%i", idx + 1);
	}
	else if (pe->rendermode != kRenderNormal)
	{
		sprintf(decalname, "{bproof1");
	}
	else
	{
		idx = gEngfuncs.pfnRandomLong(0, 4);
		sprintf(decalname, "{shot%i", idx + 1);
	}
	return decalname;
}

void EV_HLDM_GunshotDecalTrace(pmtrace_t* pTrace, char* decalName)
{
	int iRand;
	physent_t* pe;


	//MODDD - Bullet effects disabled!
	//gEngfuncs.pEfxAPI->R_BulletImpactParticles( pTrace->endpos );

	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound) < 1) {

		// redundant with TE_GUNSHOT now used instead. It automatically plays a ricochet sound.
		/*
		iRand = gEngfuncs.pfnRandomLong(0,0x7FFF);
		if ( iRand < (0x7fff/2) )// not every bullet makes a sound.
		{
			switch( iRand % 5)
			{
			case 0:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
			case 1:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric2.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
			case 2:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric3.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
			case 3:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric4.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
			case 4:	gEngfuncs.pEventAPI->EV_PlaySound( -1, pTrace->endpos, 0, "weapons/ric5.wav", 1.0, ATTN_NORM, 0, PITCH_NORM ); break;
			}
		}
		*/
	}

	pe = gEngfuncs.pEventAPI->EV_GetPhysent(pTrace->ent);
	//return;
	// Only decal brush models such as the world etc.
	//easyPrintLine("EV_HLDM - DECAL NAME: %s", decalName);
	if (decalName && decalName[0] && pe && (pe->solid == SOLID_BSP || pe->movetype == MOVETYPE_PUSHSTEP))
	{
		if (CVAR_GET_FLOAT("r_decals"))
		{
			// ALSO - even if the real filename is capitalized as stored in decals.wad, 
			// still must be given lowercase here.  Good luck figuring that one out.
			
			/*
			int theIndex1 = gEngfuncs.pEfxAPI->Draw_DecalIndexFromName("{blood2");   // ok.
			int theIndex2 = gEngfuncs.pEfxAPI->Draw_DecalIndexFromName("{yblood2");  // ok.
			int theIndex3 = gEngfuncs.pEfxAPI->Draw_DecalIndexFromName("{gblood2");  // ok.
			int theIndex4 = gEngfuncs.pEfxAPI->Draw_DecalIndexFromName("{GBLOOD2"); // gives 0.
			int theIndex5 = gEngfuncs.pEfxAPI->Draw_DecalIndexFromName("{doesnotexist"); // same 0?
			*/
			int theIndex = gEngfuncs.pEfxAPI->Draw_DecalIndexFromName(decalName);  // ok.

			gEngfuncs.pEfxAPI->R_DecalShoot(
				gEngfuncs.pEfxAPI->Draw_DecalIndex(theIndex),
				gEngfuncs.pEventAPI->EV_IndexFromTrace(pTrace), 0, pTrace->endpos, 0);
		}
	}
}

void EV_HLDM_DecalGunshot(pmtrace_t* pTrace, int iBulletType)
{
	physent_t* pe;

	pe = gEngfuncs.pEventAPI->EV_GetPhysent(pTrace->ent);

	if (pe && pe->solid == SOLID_BSP)
	{
		switch (iBulletType)
		{
		case BULLET_PLAYER_9MM:
		case BULLET_MONSTER_9MM:
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_PLAYER_BUCKSHOT:
		case BULLET_PLAYER_357:
		default:
			// smoke and decal
			//MODDD NOTE - this call is important.
			EV_HLDM_GunshotDecalTrace(pTrace, EV_HLDM_DamageDecal(pe));
			break;
		}
	}
}

BOOL EV_HLDM_CheckTracer(int idx, float* vecSrc, float* end, float* forward, float* right, int iBulletType, int iTracerFreq, int* tracerCountChoice)
{
	//MODDD - renamed the genericly named "tracer" to "disableBulletHitDecal".  It's up to whoever calls this method to act on it by not drawing a decal...
	//        for some odd reason.
	int disableBulletHitDecal = FALSE;
	int i;
	qboolean player = idx >= 1 && idx <= gEngfuncs.GetMaxClients() ? TRUE : FALSE;

	if (iTracerFreq != 0 && ((*tracerCountChoice)++ % iTracerFreq) == 0)
	{
		vec3_t vecTracerSrc;

		if (player)
		{
			vec3_t offset(0, 0, -4);

			// adjust tracer position for player
			for (i = 0; i < 3; i++)
			{
				vecTracerSrc[i] = vecSrc[i] + offset[i] + right[i] * 2 + forward[i] * 16;
			}
		}
		else
		{
			VectorCopy_f(vecSrc, vecTracerSrc);
		}

		if (iTracerFreq != 1)		// guns that always trace also always decal
			disableBulletHitDecal = TRUE;

		switch (iBulletType)
		{
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_MONSTER_9MM:
		case BULLET_MONSTER_12MM:
		default:
			EV_CreateTracer(vecTracerSrc, end);
			break;
		}
	}

	return disableBulletHitDecal;
}


/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/
void EV_HLDM_FireBullets(int idx, float* forward, float* right, float* up, int cShots, float* vecSrc, float* vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int* tracerCountChoice, float flSpreadX, float flSpreadY)
{
	int i;
	pmtrace_t tr;
	int iShot;

	//MODDD - renamed "tracer" to this, made it a BOOL for clarity.
	BOOL disableBulletHitDecal;

	//MODDD!!! DEBUG.  Stop this later.
	//flDistance = 200;

	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerBulletHitEffectForceServer) == 1) {
		//Stop. The server is doing this instead.
		return;
	}


	for (iShot = 1; iShot <= cShots; iShot++)
	{
		vec3_t vecDir;
		vec3_t vecEnd;

		float x, y, z;
		//We randomize for the Shotgun.
		if (iBulletType == BULLET_PLAYER_BUCKSHOT)
		{
			do {
				x = gEngfuncs.pfnRandomFloat(-0.5, 0.5) + gEngfuncs.pfnRandomFloat(-0.5, 0.5);
				y = gEngfuncs.pfnRandomFloat(-0.5, 0.5) + gEngfuncs.pfnRandomFloat(-0.5, 0.5);
				z = x * x + y * y;
			} while (z > 1);

			for (i = 0; i < 3; i++)
			{
				vecDir[i] = vecDirShooting[i] + x * flSpreadX * right[i] + y * flSpreadY * up[i];
				vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
			}
		}//But other guns already have their spread randomized in the synched spread.
		else
		{

			for (i = 0; i < 3; i++)
			{
				vecDir[i] = vecDirShooting[i] + flSpreadX * right[i] + flSpreadY * up[i];
				vecEnd[i] = vecSrc[i] + flDistance * vecDir[i];
			}
		}

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(FALSE, TRUE);


		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();


		gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
		gEngfuncs.pEventAPI->EV_SetTraceHull(FILLIN_TRACE_HULL);

		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, FILLIN_TRACEFLAGS_STUDIO_BOX, -1, &tr);


		//MODDD - check playerWeaponTracerMode to see if we need to do something special to iTracerFreq here for clientside.
		//        Any clientside event is for the player here
		int daTraca = (int)EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerWeaponTracerMode);
		switch (daTraca) {
		case 0:
			//nothing.
			iTracerFreq = 0;
		break;
		case 1:
			//clientside, per whatever weapons said to do for tracers.  Leave "iTracerFreq" as it was sent.

		break;
		case 2:
			//serverside only (retail).  So nothing here.
			iTracerFreq = 0;
		break;
		case 3:
			//clientside and serverside as-is.  Let it proceed.

		break;
		case 4:
			//clientside, all weapons, all shots. Force it.
			iTracerFreq = 1;
		break;
		case 5:
			//serverside, all weapons, all shots. Not here.
			iTracerFreq = 0;
		break;
		case 6:
			//clientside and serverside, all weapons, all shots. Go.
			iTracerFreq = 1;
		break;
		default:
			//unrecognized setting?  Default to whatever was sent like in retail.

		break;
		}//END OF switch

		disableBulletHitDecal = EV_HLDM_CheckTracer(idx, vecSrc, tr.endpos, forward, right, iBulletType, iTracerFreq, tracerCountChoice);


		// do damage, paint decals
		if (tr.fraction != 1.0)
		{
			switch (iBulletType)
			{
			default:
			case BULLET_PLAYER_9MM:

				EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
				EV_HLDM_DecalGunshot(&tr, iBulletType);

				break;
			case BULLET_PLAYER_MP5:

				//MODDD NOTE - if this was a tracer don't do the usual gunshot and texturesound? what? Does this matter anymore now that the server can call for tracers
				//             and do this regardless of CVar "playerWeaponTracerMode"?
				//             This seems to mirror the same "tracer" variable also local to fireBullets methods serverside (in dlls/combat.cpp).
				//             It has been renamed to "disableBulletHitDecal" here as well.
				if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(decalTracerExclusivity) != 1 || !disableBulletHitDecal)
				{
					EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
					EV_HLDM_DecalGunshot(&tr, iBulletType);
				}
				break;
			case BULLET_PLAYER_BUCKSHOT:

				EV_HLDM_DecalGunshot(&tr, iBulletType);

				break;
			case BULLET_PLAYER_357:

				EV_HLDM_PlayTextureSound(idx, &tr, vecSrc, vecEnd, iBulletType);
				EV_HLDM_DecalGunshot(&tr, iBulletType);

				break;
			}
		}

		gEngfuncs.pEventAPI->EV_PopPMStates();
	}
}



//======================
//	    GLOCK START
//======================
void EV_FireGlock1(event_args_t* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int empty;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, angles);
	VectorCopy_f(args->velocity, velocity);

	int InAttack = args->iparam1;

	empty = args->bparam1;
	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell.mdl");// brass shell

	int silencerOn = 0;
	if (InAttack == 0 || InAttack == 1) {
		silencerOn = 0;
	}
	else if (InAttack == 2) {
		silencerOn = 1;
	}

	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts) == 1)easyForcePrintLine("!!!! EV_FireGlock1: is event local for player ID %d?  %d", idx, EV_IsLocal(idx));

	if (EV_IsLocal(idx))
	{
		//MODDD - silencer has no flash. This is the flash of light, not the sprite which is controlled by the animation itslef.
		//It is blocked seprately by a separate flag for renderfx that makes it from server to clientside, called NOMUZZLEFLASH.
		//By default the sprite is allowed but must specifically be blocked for the silencer instead, actively done in entity.cpp.
		if (!silencerOn) {
			EV_MuzzleFlash();
		}
		else {
			cl_entity_t* ent = GetViewEntity();
			if (ent) {
				ent->curstate.effects &= ~EF_MUZZLEFLASH;
			}
		}

		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(viewModelPrintouts) == 1)easyForcePrintLine("!!!! EV_FireGlock1: EV_WeaponAnimation.");


		//MODDD - use a body of "InAttack", coordinated with whether the silencer is on or not.
		//gEngfuncs.pEventAPI->EV_WeaponAnimation( empty ? GLOCK_SHOOT_EMPTY : GLOCK_SHOOT, 2 );
		gEngfuncs.pEventAPI->EV_WeaponAnimation(empty ? GLOCK_SHOOT_EMPTY : GLOCK_SHOOT, silencerOn);

		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-3.5, -2.8));
		}else{
			V_PunchAxis(0, -2);
		}
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	//MODDD - why were these commented before?
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);


	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		if (!silencerOn) {
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/pl_gun3.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));
		}
		else {

			// The range of "0.6 - 0.8" for volume is what the "hassassin" uses.  Reasonable? reduced slightly more...
			switch (gEngfuncs.pfnRandomLong(0, 1))
			{
			case 0:
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/pl_gun1.wav", gEngfuncs.pfnRandomFloat(0.55, 0.75), ATTN_IDLE, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));
				break;
			case 1:
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/pl_gun2.wav", gEngfuncs.pfnRandomFloat(0.55, 0.75), ATTN_IDLE, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));
				break;
			}

		}
	}

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy_f(forward, vecAiming);

	//MODDD - why was this missing "&tracerCount[idx-1]"?  Most other weapons had this even for tracer frequencies of 0.
	//        Forcing a different frequency (or 1 for every single shot) causes a crash because that was null.
	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], args->fparam1, args->fparam2);

	/*
			gEngfuncs.GetLocalPlayer()->curstate.frame = 126;
		gEngfuncs.GetLocalPlayer()->curstate.framerate = -1;
		gEngfuncs.GetLocalPlayer()->prevstate.frame = 126;
		gEngfuncs.GetLocalPlayer()->prevstate.framerate = -1;
		gEngfuncs.GetLocalPlayer()->baseline.frame = 126;
		gEngfuncs.GetLocalPlayer()->baseline.framerate = -1;
		*/
}

void EV_FireGlock2(event_args_t* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, angles);
	VectorCopy_f(args->velocity, velocity);

	int InAttack = args->iparam1;


	int silencerOn = 0;
	if (InAttack == 0 || InAttack == 1) {
		silencerOn = 0;
	}
	else if (InAttack == 2) {
		silencerOn = 1;
	}


	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell.mdl");// brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		//MODDD - silencer has no flash.
		if (!silencerOn) {
			EV_MuzzleFlash();
		}
		else {
			cl_entity_t* ent = GetViewEntity();
			if (ent) {
				ent->curstate.effects &= ~EF_MUZZLEFLASH;
			}
		}

		//MODDD - use a body of "InAttack", coordinated with whether the silencer is on or not.
		//gEngfuncs.pEventAPI->EV_WeaponAnimation( GLOCK_SHOOT, 2 );
		gEngfuncs.pEventAPI->EV_WeaponAnimation(GLOCK_SHOOT, silencerOn);

		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-3.5, -2.8));
		}else{
			V_PunchAxis(0, -2);
		}
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	//MODDD - why were these commented?
	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		if (!silencerOn) {
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/pl_gun3.wav", gEngfuncs.pfnRandomFloat(0.92, 1.0), ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));
		}
		else {

			switch (gEngfuncs.pfnRandomLong(0, 1))
			{
			case 0:
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/pl_gun1.wav", gEngfuncs.pfnRandomFloat(0.55, 0.75), ATTN_IDLE, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));
				break;
			case 1:
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/pl_gun2.wav", gEngfuncs.pfnRandomFloat(0.55, 0.75), ATTN_IDLE, 0, 98 + gEngfuncs.pfnRandomLong(0, 3));
				break;
			}

		}
	}

	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy_f(forward, vecAiming);

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}
//======================
//	   GLOCK END
//======================

//======================
//	  SHOTGUN START
//======================
void EV_FireShotGunDouble(event_args_t* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	int j;
	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, angles);
	VectorCopy_f(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl");// brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SHOTGUN_FIRE2, 2);

		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-20, -15));
		}else{
			V_PunchAxis(0, -10);
		}
	}

	for (j = 0; j < 2; j++)
	{
		EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6);

		EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);
	}

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/dbarrel1.wav", gEngfuncs.pfnRandomFloat(0.98, 1.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong(0, 0x1f));
	}

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy_f(forward, vecAiming);

	//playerWeaponSpreadMode.
	//0 = no effect (pick based on single or multiplayer as usual)
	//1 = standard circle (single player)
	//2 = wider, less tall (multiplayer)
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode) != 2 && (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode) == 1 || !IsMultiplayer()))
	{
		//easyForcePrintLine("FLAG C-SINGLEPLAYER");
		//single player circle
		EV_HLDM_FireBullets(idx, forward, right, up, 12, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], VECTOR_CONE_10DEGREES.x, VECTOR_CONE_10DEGREES.y);
	}
	else
	{
		//easyForcePrintLine("FLAG C-MULTIPLAYER");
		//multiplayer wide. NOTE: wider (double) spread than single barrel fire, same height as single barrel fire (half single player).
		EV_HLDM_FireBullets(idx, forward, right, up, 8, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], VECTOR_CONE_DM_DOUBLESHOTGUN.x, VECTOR_CONE_DM_DOUBLESHOTGUN.y);
	}
}

void EV_FireShotGunSingle(event_args_t* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t vecSpread;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, angles);
	VectorCopy_f(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shotgunshell.mdl");// brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SHOTGUN_FIRE, 2);

		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-7.4, -6.2));
		}else{
			V_PunchAxis(0, -5);
		}
	}

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 32, -12, 6);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHOTSHELL);

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/sbarrel1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0x1f));
	}

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy_f(forward, vecAiming);

	//MODDD - same, see above for the ShotGunSingle
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode) != 2 && (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(playerWeaponSpreadMode) == 1 || !IsMultiplayer()))
	{
		EV_HLDM_FireBullets(idx, forward, right, up, 6, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], VECTOR_CONE_10DEGREES.x, VECTOR_CONE_10DEGREES.y);
	}
	else
	{
		//multiplayer wide. NOTE: same spread width as single player, less spread height than single player.
		EV_HLDM_FireBullets(idx, forward, right, up, 4, vecSrc, vecAiming, 2048, BULLET_PLAYER_BUCKSHOT, 0, &tracerCount[idx - 1], VECTOR_CONE_DM_SHOTGUN.x, VECTOR_CONE_DM_SHOTGUN.y);
	}
}
//======================
//	   SHOTGUN END
//======================

//======================
//	    MP5 START
//======================
void EV_FireMP5(event_args_t* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t ShellVelocity;
	vec3_t ShellOrigin;
	int shell;
	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, angles);
	VectorCopy_f(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	int fireAnim = args->iparam1;

	shell = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shell.mdl");// brass shell

	if (EV_IsLocal(idx))
	{
		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();

		
		if(EASY_CVAR_GET(cl_mp5_evil_skip) != 1){
			// NORMAL WAY
			//MODDD - the fire anim is now picked by the client/server with a shared float for more control on that end and sent here
			// as that, instead of being randomized here.
			//gEngfuncs.pEventAPI->EV_WeaponAnimation( MP5_FIRE1 + gEngfuncs.pfnRandomLong(0,2), 2 );
			gEngfuncs.pEventAPI->EV_WeaponAnimation(MP5_FIRE1 + fireAnim, 2);

		}else{
			// EVIL SKIP.
			if (g_cl_mp5_NextAnimTime < gpGlobals->time)
			{
				// old evil way of playing the animation updated to use the events system to meld with this codebase better.
				//SendWeaponAnim( MP5_FIRE1 + random thing in 0,2);
				gEngfuncs.pEventAPI->EV_WeaponAnimation(MP5_FIRE1 + fireAnim, 2);

				// ALSO.  Delay changed a little for a better chance of skipping one in two firing frames, instead of two in three.
				//m_flNextAnimTime = gpGlobals->time + 0.2;
				g_cl_mp5_NextAnimTime = gpGlobals->time + 0.17;
			}
		}


		float theViewpunch;
		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			//MODDD - don't allow very low values anymore, kind of odd when the point is recoil to ever be given those.
			// ... on CVar setting that is.
			if (EASY_CVAR_GET(cl_mp5_viewpunch_mod) == 1) {
				int ranDir = gEngfuncs.pfnRandomLong(0, 1);
				if (ranDir == 0) {
					// neg
					theViewpunch = gEngfuncs.pfnRandomFloat(-4.4, -3.9);
				}else {
					// pos
					theViewpunch = gEngfuncs.pfnRandomFloat(3.9, 4.4);
				}
			}else {
				// retail-style
				theViewpunch = gEngfuncs.pfnRandomFloat(-3.2, 3.2);
			}
		}else{
			// cl_viewpunch_mod off.
			if (EASY_CVAR_GET(cl_mp5_viewpunch_mod) == 1) {
				int ranDir = gEngfuncs.pfnRandomLong(0, 1);
				if (ranDir == 0) {
					// neg
					theViewpunch = gEngfuncs.pfnRandomFloat(-2.6, -1.7);
				}else {
					// pos
					theViewpunch = gEngfuncs.pfnRandomFloat(1.7, 2.6);
				}
			}else {
				// retail
				theViewpunch = gEngfuncs.pfnRandomFloat(-2, 2);
			}
		}
		V_PunchAxis(0, theViewpunch);
	}// END OF IsLocal check

	EV_GetDefaultShellInfo(args, origin, velocity, ShellVelocity, ShellOrigin, forward, right, up, 20, -12, 4);

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], shell, TE_BOUNCE_SHELL);

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		switch (gEngfuncs.pfnRandomLong(0, 1))
		{
		case 0:
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
			break;
		}
	}

	EV_GetGunPosition(args, vecSrc, origin);
	VectorCopy_f(forward, vecAiming);

	//MODDD NOTE - in either case, spread is identical because it comes from what the client provided us with (args->fparam1 and args->fparam2 here).
	//             Not going to bother checking the CVar here then, whatever was decided for this bullet already has been by those params.
	//             Intervene in mp5.cpp calling for this event instead.
	/*
	if ( gEngfuncs.GetMaxClients() > 1 )
	{ ...
	*/

	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_MP5, 2, &tracerCount[idx - 1], args->fparam1, args->fparam2);

}



// We only predict the animation and sound
// The grenade is still launched from the server.
void EV_FireMP52(event_args_t* args)
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);

	if (EV_IsLocal(idx))
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation(MP5_LAUNCH, 2);

		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-17.0, -14.5));
		}else{
			V_PunchAxis(0, -10);
		}
	}

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		float fVol;
		float fAttn;
		// whoops, nevermind.  Have a little less attn (carry further) anyway
		fVol = 1.0f;
		fAttn = ATTN_NORM - 0.03;
		
		switch (gEngfuncs.pfnRandomLong(0, 1)){
		case 0:
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/glauncher.wav", fVol, fAttn, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
		case 1:
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/glauncher2.wav", fVol, fAttn, 0, 94 + gEngfuncs.pfnRandomLong(0, 0xf));
		break;
		}
	}//mutePlayerWeaponFire check
	
}
//======================
//		 MP5 END
//======================

//======================
//	   PHYTON START 
//	     ( .357 )
//======================
void EV_FirePython(event_args_t* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	vec3_t vecSrc, vecAiming;
	vec3_t up, right, forward;
	float flSpread = 0.01;


	int pythonModel = args->iparam1;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, angles);
	VectorCopy_f(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	if (EV_IsLocal(idx))
	{
		// Python uses different body in multiplayer versus single player
		//MODDD - this will be handled by the sender, python.cpp, instead of right here.  Rely on "InAttack" as a received parameter.
		//int multiplayer = gEngfuncs.GetMaxClients() == 1 ? 0 : 1;

		// Add muzzle flash to current weapon model
		EV_MuzzleFlash();

		//MODDD - replaced.
		//gEngfuncs.pEventAPI->EV_WeaponAnimation( PYTHON_FIRE1, multiplayer ? 1 : 0 );
		gEngfuncs.pEventAPI->EV_WeaponAnimation(PYTHON_FIRE1, pythonModel);

		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-14.3, -12.7));
		}else{
			V_PunchAxis(0, -10);
		}
	}


	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		switch (gEngfuncs.pfnRandomLong(0, 1))
		{
		case 0:
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/357_shot1.wav", gEngfuncs.pfnRandomFloat(0.8, 0.9), ATTN_NORM, 0, PITCH_NORM);
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/357_shot2.wav", gEngfuncs.pfnRandomFloat(0.8, 0.9), ATTN_NORM, 0, PITCH_NORM);
			break;
		}
	}


	EV_GetGunPosition(args, vecSrc, origin);

	VectorCopy_f(forward, vecAiming);

	//MODDD - why was this missing "&tracerCount[idx-1]"?  Most other weapons had this even for tracer frequencies of 0.
	//        Forcing a different frequency (or 1 for every single shot) causes a crash because that was null.
	EV_HLDM_FireBullets(idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_357, 0, &tracerCount[idx - 1], args->fparam1, args->fparam2);
}
//======================
//	    PHYTON END 
//	     ( .357 )
//======================

//======================
//	   GAUSS START 
//======================
//MODDD - redefine of SND_CHANGE_PITCH removed, SND_'s are now in util_shared.h.

void EV_SpinGauss(event_args_t* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int iSoundState = 0;

	int pitch;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, angles);
	VectorCopy_f(args->velocity, velocity);

	pitch = args->iparam1;

	iSoundState = args->bparam1 ? SND_CHANGE_PITCH : 0;

	//MODDD - NOTE - period sound.  Played continuously.
	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "ambience/pulsemachine.wav", 1.0, ATTN_NORM, iSoundState, pitch);
}

/*
==============================
EV_StopPreviousGauss

==============================
*/
void EV_StopPreviousGauss(int idx)
{
	// Make sure we don't have a gauss spin event in the queue for this guy
	gEngfuncs.pEventAPI->EV_KillEvents(idx, "events/gaussspin.sc");
	gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_WEAPON, "ambience/pulsemachine.wav");
}



// debugging
#define DISABLE_GAUSS_GLOW 0
#define ALLOW_RISKY_BALLS 1
#define ALLOW_RISKIEST_BALLS 0


void EV_FireGauss(event_args_t* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	float flDamage = args->fparam1;
	int primaryfire = args->bparam1;

	int m_fPrimaryFire = args->bparam1;
	int m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;
	vec3_t vecSrc;
	vec3_t vecDest;
	edict_t* pentIgnore;
	pmtrace_t tr, beam_tr;
	float flMaxFrac = 1.0;
	int nTotal = 0;
	BOOL fHasPunched = 0;
	BOOL fFirstBeam = TRUE;
	int nMaxHits = 10;
	physent_t* pEntity;
	int m_iBeam, m_iGlow, m_iBalls;
	vec3_t up, right, forward;
	//MODDD - new vars
	Vector gunOrig;
	Vector gunDir;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, angles);
	VectorCopy_f(args->velocity, velocity);

	if (args->bparam2)
	{
		EV_StopPreviousGauss(idx);
		//easyPrintLine("GAUSS: FireGauss: redirect to StopPrevious");
		return;
	}
	//easyForcePrintLine("GAUSS: FireGauss");

	//	Con_Printf( "Firing gauss with %f\n", flDamage );
	EV_GetGunPosition(args, vecSrc, origin);
	gunOrig = vecSrc;

	m_iBeam = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke.spr");
	m_iBalls = m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow.spr");

	AngleVectors(angles, forward, right, up);
	gunDir = forward;

	VectorMA(vecSrc, 8192, forward, vecDest);

	float dmgFracto;
	float sndFracto;
	

	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(gauss_mode) != 1) {
		// retail-based
		if (m_fPrimaryFire) {
			sndFracto = 0.04; //0.23;
			dmgFracto = 0.33; //min(flDamage, 20.0f) * (1.0f / 20.0f);
		}else {
			sndFracto = min(flDamage, 600.0f) * (1.0f / 600.0f);
			dmgFracto = min(flDamage, 600.0f) * (1.0f / 600.0f);
		}
	}else {
		// pre-release description.
		// Max damage is 600, but can still be plenty bright up to that point.
		if (m_fPrimaryFire) {
			sndFracto = 0.12; //0.28;
			dmgFracto = 0.6;
		}else {
			sndFracto = min(flDamage, 600.0f) * (1.0f / 600.0f);
			dmgFracto = min(flDamage, 600.0f) * (1.0f / 600.0f);
		}
	}



	if (EV_IsLocal(idx))
	{
		//MODDD - was -2
		gEngfuncs.pEventAPI->EV_WeaponAnimation(GAUSS_FIRE2, 2);

		//MODDD - don't fully understand this, but probably not good to get values over retail's usual max.
		// Crude idea: cut it to some percentage for gauss mode 1
		
		float viewPunchAmount;
		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			if(EASY_CVAR_GET_CLIENTONLY(cl_gauss_viewpunch_mod) == 1){
				// gauss_viewpunch_mod?  Go crazy
				if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(gauss_mode) != 1) {
					// retail mode
					if (m_fPrimaryFire) {
						viewPunchAmount = gEngfuncs.pfnRandomFloat(-4.1, -3.6);
					}else{
						viewPunchAmount = gEngfuncs.pfnRandomFloat(-1.0, -0.95) * 45 * dmgFracto;
						if (viewPunchAmount > -3.9) viewPunchAmount = -3.9;
						g_flApplyVel = flDamage;
					}
				}else{
					// pre-release
					if (m_fPrimaryFire) {
						viewPunchAmount = gEngfuncs.pfnRandomFloat(-6.0, -5.1);
					}else{
						viewPunchAmount = gEngfuncs.pfnRandomFloat(-1.0, -0.95) * 28 * dmgFracto;
						if (viewPunchAmount > -5.5) viewPunchAmount = -5.5;
						g_flApplyVel = flDamage * 0.45;
					}
				}
			}else{
				// no gauss viewpunch mod?  Use primary's way always
				if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(gauss_mode) != 1) {
					viewPunchAmount = gEngfuncs.pfnRandomFloat(-4.1, -3.6);
				}else{
					viewPunchAmount = gEngfuncs.pfnRandomFloat(-6.0, -5.1);
				}
			}
		}else{
			// viewpunch mod of 0
			if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(gauss_mode) != 1) {
				// gauss_viewpunch_mod?  Go crazy
				if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(gauss_mode) != 1) {
					// retail mode
					if (m_fPrimaryFire) {
						viewPunchAmount = -2;
					}else{
						viewPunchAmount = -1 * 45 * 0.62 * dmgFracto;
						if (viewPunchAmount > -2.0) viewPunchAmount = -2.0;
						g_flApplyVel = flDamage;
					}
				}else{
					// pre-release
					if (m_fPrimaryFire) {
						viewPunchAmount = -3.4;
					}else{
						viewPunchAmount = -1 * 28 * 0.62 * dmgFracto;
						if (viewPunchAmount > -3.4) viewPunchAmount = -3.4;
						g_flApplyVel = flDamage * 0.45;
					}
				}
			}else{
				// no gauss viewpunch mod?  Use primary's way always
				if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(gauss_mode) != 1) {
					viewPunchAmount = -2;
				}else{
					viewPunchAmount = -3.4;
				}
			}
		}//cl_viewpunch_mod check

		V_PunchAxis(0, viewPunchAmount);
	}//EV_IsLocal


	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		// original line
		//gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/gauss2.wav", 0.5 + flDamage * (1.0 / 400.0), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );
		float flVol;
		float flAttn;
		int iPitch;

		//MODDD - little safety filter for damage, if over 200 cap it's influence on volume.
		// Paranoid but just being safe.
		// Attenuation is less for higher damages too (sound carries further).
		// damage readings
		// primary : 20
		// secondary min: 27.3719196
		// secondary max: 200

		//if (m_fPrimaryFire) {
			//flVol = 0.7 + min(sndFracto * 3, 1) * 0.3;
			//flAttn = ATTN_NORM - (0.05 + 0.1 * sndFracto);
			//// original pitch range was 85 to 116.
			//iPitch = gEngfuncs.pfnRandomLong(90, 112);
		//}else {
			flVol = 0.7 + min(sndFracto * 3, 1) * 0.3;
			flAttn = ATTN_NORM - (0.05 + 0.58 * sndFracto);
			// original pitch range was 85 to 116.
			iPitch = gEngfuncs.pfnRandomLong(92 - (long)(sndFracto * 10), 115 - (long)(sndFracto * 18));
		//}

		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/gauss2.wav", flVol, flAttn, 0, iPitch);
	}//END OF mutePlayerWeaponFirecheck




	//MODDD - reget this each time in case of changes.
	float beamColor_r;
	float beamColor_g;
	float beamColor_b;
	float beamWidth;  //default:  m_fPrimaryFire ? 1.0 : 2.5
	float beamBrightness;  //default: (m_fPrimaryFire ? 128.0 : flDamage) / 255.0f
	float beamLife;


	if (EASY_CVAR_GET(cl_gaussfollowattachment) == 1) {
		// Retail default is 0.10.
		// Start point follows the gauss as it fires and the player moves... odd as that is to say out loud.
		beamLife = 0.08;
	}else {
		// Decay faster to not look too weird on firing while moving.
		beamLife = 0.029;
	}


	//easyForcePrintLine("HERE COMES MAH BEAM");

//NOTE: on any changes, sync me up with gauss.cpp (server-side)'s "Fire" method with a similar loop.
	while (fFirstBeam || flDamage >= 10 && nMaxHits > 0)
	{
		//easyForcePrintLine("beamo...");

		nMaxHits--;

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(FALSE, TRUE);

		// Store off the old count
		gEngfuncs.pEventAPI->EV_PushPMStates();

		// Now add in all of the players.
		gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

		gEngfuncs.pEventAPI->EV_SetTraceHull(FILLIN_TRACE_HULL);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecDest, FILLIN_TRACEFLAGS_STUDIO_BOX, -1, &tr);

		gEngfuncs.pEventAPI->EV_PopPMStates();

		if (tr.allsolid) {
			break;
		}





		//MODDD - stats re-done each time the beam is reflected
		////////////////////////////////////////////////////////////////////////////////////////////
		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(gauss_mode) != 1) {
			// retail-based
			if (m_fPrimaryFire) {
				sndFracto = 0.04; //0.23;
				dmgFracto = 0.33; //min(flDamage, 20.0f) * (1.0f / 20.0f);
			}
			else {
				sndFracto = min(flDamage, 600.0f) * (1.0f / 600.0f);
				dmgFracto = min(flDamage, 600.0f) * (1.0f / 600.0f);
			}
		}
		else {
			// pre-release description.
			// Max damage is 600, but can still be plenty bright up to that point.
			if (m_fPrimaryFire) {
				sndFracto = 0.12; //0.28;
				dmgFracto = 0.6;
			}
			else {
				sndFracto = min(flDamage, 600.0f) * (1.0f / 600.0f);
				dmgFracto = min(flDamage, 600.0f) * (1.0f / 600.0f);
			}
		}
		// beamLife affected by these too?   maybe not.
		if (m_fPrimaryFire) {
			beamColor_r = 1.0f;
			beamColor_g = 0.5f;
			beamColor_b = 0.0f;
			beamWidth = 0.68f + dmgFracto * 0.62f * 3;
			beamBrightness = 0.6f + dmgFracto * 0.17f * 1.8;
		}
		else {
			beamColor_r = 1.0f;
			beamColor_g = 1.0f;
			beamColor_b = 1.0f;
			beamWidth = 1.15f + dmgFracto * 0.90f * 3;
			beamBrightness = 0.65f + min(dmgFracto * 3, 1) * 0.35f;
		}
		////////////////////////////////////////////////////////////////////////////////////////////


		if (fFirstBeam){
			if (EV_IsLocal(idx))
			{
				// Add muzzle flash to current weapon model
				EV_MuzzleFlash();
			}
			fFirstBeam = FALSE;

			//MODDD - beam color touchups due to hastily pasted serverside script, believed to be from early in development
			// when client prediction was implemented (clientside-effects methods often expect color as a float from 0 to 1,
			// not a byte/int from 0 to 255 as found here).
			// Thanks Nikita Butorin / @vasiavasiavasia95 !

			if (EASY_CVAR_GET(cl_gaussFollowAttachment) == 1) {

				cl_entity_t* viewModelRef = GetViewEntity();
				Vector daPos = viewModelRef->attachment[0];
				//easyForcePrintLine("Dapos: idx:%d %.2f %.2f %.2f - %.2f %.2f %.2f, fr:%.2f spr:%d w:%.2f br:%.2f, rgb:%.1f %.1f %.1f", idx, daPos.x, daPos.y, daPos.z, tr.endpos.x, tr.endpos.y, tr.endpos.z, tr.fraction, m_iBeam, beamWidth, beamBrightness, beamColor_r, beamColor_g, beamColor_b);
				//easyForcePrintLine("===============================");

				gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, m_iBeam, 0.1, beamWidth, 0.0, beamBrightness, 0, 0, 0, beamColor_r, beamColor_g, beamColor_b);

			}else{
				// not wise.
				//gEngfuncs.pEfxAPI->R_BeamPoints(gunOrig, tr.endpos, m_iBeam, 0.1, beamWidth, 0.0, beamBrightness, 0, 0, 0, beamColor_r, beamColor_g, beamColor_b);

				//cl_entity_t* viewModelRef = GetViewEntity();
				//if (viewModelRef != NULL) {
					// Use attachment 0 for the muzzle of the weapon, even though it's bound to go out of date in seconds.
					// Attachmenst 1 to 3 are the same ('doom mode'), as though they're defaults, might match the model origin.
					// Idea: See the direction from attachment 0 to the end point and back up into the weapon a bit to show up while it recoils.
					// Are viewmodel angles worth anything?  Doubt it.

					// gunDir?  no, not as accurate to the attachment

					// viewModelRef->attachment[0]

					// Using a hardcoded offset instead, relative to the angles.
					// attachment[0] is usually fine, but it is still out of date when firing quickly.
					// As the anim puts the attachment back and right aways due to the recoil (what is read as the attachment[0] point),
					// the instant it changes back to frame 0 visually the beam is out of place.


					//Vector fireOrigin = vecSrc + EASY_CVAR_GET(ctt1) * forward  + EASY_CVAR_GET(ctt2) * right + EASY_CVAR_GET(ctt3) * up;

					Vector fireOrigin;
					
					if (EV_IsLocal(idx) && !CL_IsThirdPerson() ) {
						// got a viewmodel to match to.
						fireOrigin = vecSrc + 22 * forward + 5.6 * right + -4.8 * up;
					}
					else {
						// rendering a player model of a different player from the local one.
						// wait.  this isn't working?   Getting the player's third-person model, attachment 0?
						// How did R_BeamEntPoint even work then in this case?  Is the hex in 'idx | 0x1000' a reference to 'attachment 0' for the player,
						// or is 'idx | 0x1000' a separate entity completley?

						//cl_entity_t* pl = gEngfuncs.GetEntityByIndex(idx);
						//if (pl != NULL) {
						//	fireOrigin = pl->attachment[0];
						//}else {
						//	// what???
						//	fireOrigin = vecSrc + 22 * forward + 5.6 * right + -4.8 * up;
						//}

						// BETTER: just use the gun origin at this point dangit.
						// Or not even it maps to the barrel.  Oooookay.
						//fireOrigin = gunOrig;
						fireOrigin = vecSrc + 16 * forward + 8 * right + -12 * up;
					}

					//Vector fireOrigin = viewModelRef->attachment[0];
					Vector vecDir = (tr.endpos - fireOrigin).Normalize();

					gEngfuncs.pEfxAPI->R_BeamPoints(fireOrigin - (vecDir * 8), tr.endpos, m_iBeam, beamLife, beamWidth, 0.0, beamBrightness, 0, 0, 0, beamColor_r, beamColor_g, beamColor_b);
				//}//viewModelRef check
			}// cl_gaussFollowAttachment check

		}else{
			gEngfuncs.pEfxAPI->R_BeamPoints(vecSrc, tr.endpos, m_iBeam, beamLife, beamWidth, 0.0, beamBrightness, 0, 0, 0, beamColor_r, beamColor_g, beamColor_b);
		}

		//MODDD
		if (UTIL_PointContents(tr.endpos) == CONTENTS_SKY) {
			// If we hit the sky, HALT!
			// Just end.  No reflecting off of that, no persistent glow, no sparks.  None of that makes sense.
			break;
		}


		// !!! CLIENTSIDE TRACE ENTITY-FROM-INDEX DEBUG
		// Lots of combinations of ways to get a cl_entity_t or a physent_t, but which is correct?
		// Bad ways commented out.
		
		// Conclusion, short-version:
		// * Best way to get the pev->solid of any entity:
		//     physent_t* somePhysEnt = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);
		//     int theSolid = somePhysEnt->solid;
		// * Best way to get the renderfx of any entity:
		//     int hitEntIndex = gEngfuncs.pEventAPI->EV_IndexFromTrace(&tr);
		//     cl_entity_t* cEntRef = gEngfuncs.GetEntityByIndex(hitEntIndex);
		//     int theRenderFX = cEntRef->curstate.renderfx;
		// Also, note that GetEntityByIndex and the cl_entity* type were only used once in as-is
		// (in the egon).
		// Consider checking whether hitEntIndex is 0, that means the cl_entity* given is for the
		// world.  Probably?  The blank curstate.solid for it is still weird (works for physent_t though)
		// tr.ent would similarly be 0 in such a case.  huh.  For other things retrieved as cl_entity_t*, 
		// the 'EV_IndexFromTrace(&tr)' approach is needed, the tr.ent is not appropriate.
		

		/*
		// First two look to work equally well, but may as well use EV_IndexFromTrace, the EV form.
		// It's better for prediction supposedly, and the same otherwise.
		int hitEntIndex = gEngfuncs.pEventAPI->EV_IndexFromTrace(&tr);
		int hitEntIndex2 = PM_GetPhysEntInfo(tr.ent);
		int hitEntIndex3 = tr.ent;

		cl_entity_t* cEntRef1 = gEngfuncs.GetEntityByIndex(hitEntIndex);
		cl_entity_t* cEntRef2 = gEngfuncs.GetEntityByIndex(hitEntIndex2);
		//cl_entity_t* cEntRef3 = gEngfuncs.GetEntityByIndex(hitEntIndex3);

		//physent_t* pe1 = gEngfuncs.pEventAPI->EV_GetPhysent(hitEntIndex);
		//physent_t* pe2 = gEngfuncs.pEventAPI->EV_GetPhysent(hitEntIndex2);
		physent_t* pe3 = gEngfuncs.pEventAPI->EV_GetPhysent(hitEntIndex3);


		// NOTICE - this correctly gets the pev->solid (as seen serverside) of any entity BUT worldspawn (the map).
		// NOOOOoo freakin' clue.   But look at pe3->solid, that consistently works for solid.
		int cEntRef1Solid = cEntRef1->curstate.solid;
		int cEntRef2Solid = cEntRef2->curstate.solid;
		//int cEntRef3Solid = cEntRef3->curstate.solid;
		//int pe1Solid = pe1->solid;
		//int pe2Solid = pe2->solid;
		int pe3Solid = pe3->solid;

		int cEntRef1IsMetalNPC = (cEntRef1->curstate.renderfx & ISMETALNPC) == ISMETALNPC;
		int cEntRef2IsMetalNPC = (cEntRef2->curstate.renderfx & ISMETALNPC) == ISMETALNPC;
		//int cEntRef3IsMetalNPC = (cEntRef3->curstate.renderfx & ISMETALNPC) == ISMETALNPC;
		*/


		pEntity = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);
		if (pEntity == NULL) {
			break;
		}

		if (pEntity->solid == SOLID_BSP)
		{
			float n;

			pentIgnore = NULL;

			n = -DotProduct(tr.plane.normal, forward);


			BOOL reflectCheckPossible = TRUE;
			
			//MODDD - involved "reflectCheckPossible"
			if (reflectCheckPossible && n < 0.5) // 60 degrees	
			{
				// ALERT( at_console, "reflect %f\n", n );
				// reflect
				vec3_t r;

				VectorMA(forward, 2.0 * n, tr.plane.normal, r);

				flMaxFrac = flMaxFrac - tr.fraction;

				VectorCopy_f(r, forward);

				VectorMA(tr.endpos, 8.0, forward, vecSrc);
				VectorMA(vecSrc, 8192.0, forward, vecDest);


//MODDD - smaller glow sprite?  DISABLED, same reason as the other disabled R_TempSprite call further below.
//#if DISABLE_GAUSS_GLOW != 1
//				gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage * n / 255.0, flDamage * n * 0.5 * 0.1, FTENT_FADEOUT);
//#endif
				vec3_t fwd;
				VectorAdd_f(tr.endpos, tr.plane.normal, fwd);



//MODDD - disabled.   Also prone to getting stuck in the object.
#if ALLOW_RISKY_BALLS == 1
				gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat(14, 20) / 100.0, 100, 255, 100);
#endif

				// lose energy
				if (n == 0)
				{
					n = 0.1;
				}

				flDamage = flDamage * (1 - n);

			}
			else
			{
				int itr = 0;

				//MODDD - factors in the max damage of 600 now, if that comes up.
				float glowEffectOpacity;
				float glowEffectScale;
				// used to be this:    flDamage / 255.0
				if (flDamage <= 10) {
					glowEffectOpacity = 0.30;
					glowEffectScale = 0.70;
				}
				else if (flDamage <= 200) {
					// to 1.2
					glowEffectOpacity = 0.30 + (flDamage - 10)/(200.0-10.0) * 0.90;  //0.53
					// to 0.9
					glowEffectScale = 0.70 + (flDamage - 10)/(200.0-10.0) * 0.20;
				}
				else {
					// deminishing returns towards 1.0
					// NOW towards 2.3 instead (going over 1 draws the image the whole-number of times in place, another partial
					// for the decimal)
					glowEffectOpacity = min(0.30 + 1 * 0.90 + (flDamage - 200.0)/(600.0-200.0) * 1.1, 2.3);
					glowEffectScale = min(0.70 + 1 * 0.20 + (flDamage - 200.0)/(600.0-200.0) * 0.30, 1.2);
				}


				// tunnel
				EV_HLDM_DecalGunshot(&tr, BULLET_MONSTER_12MM);


#if DISABLE_GAUSS_GLOW != 1
				while(glowEffectOpacity >= 1){
					// Draw at opacity 1 each whole time that ifts.
					// Unsure why but fudging the positions of overlaying sprites ever so slightly
					// reduces the chances of some of them failing to redner at times.
					gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos - (forward * (itr * 0.06)), vec3_origin, glowEffectScale, m_iGlow, kRenderGlow, kRenderFxNoDissipation, 1.0f, 6.0, FTENT_FADEOUT);
					glowEffectOpacity -= 1;
					itr += 1;
				}
				// and the remainder (at least 0, under 1), if there is any.
				if(glowEffectOpacity > 0){
					gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos - (forward * (itr * 0.06)), vec3_origin, glowEffectScale, m_iGlow, kRenderGlow, kRenderFxNoDissipation, glowEffectOpacity, 6.0, FTENT_FADEOUT);
				}
#endif





				// limit it to one hole punch
				if(fHasPunched)
				{
					break;
				}
				fHasPunched = TRUE;


				//MODDD - see mirrored portion of serverside's gauss.cpp.
				//if ( !m_fPrimaryFire )
				BOOL punchAttempt = (!m_fPrimaryFire);

				// try punching through wall if secondary attack (primary is incapable of breaking through)
				if (punchAttempt)
				{
					vec3_t start;

					VectorMA(tr.endpos, 8.0, forward, start);

					// Store off the old count
					gEngfuncs.pEventAPI->EV_PushPMStates();

					// Now add in all of the players.
					gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

					gEngfuncs.pEventAPI->EV_SetTraceHull(FILLIN_TRACE_HULL);
					gEngfuncs.pEventAPI->EV_PlayerTrace(start, vecDest, FILLIN_TRACEFLAGS_STUDIO_BOX, -1, &beam_tr);



					// MODDD - NOTE - draw these regardless of piercing an object now.
					// And don't draw a static 3.  Let this reflect the damage too.
					// absorption balls
					// MODDD - also, last a little longer (half a second before fading, old life was 0.1)
					{
						//MODDD - factor in the new max of 600, should it come up
						int flyingBalls;
						
						// minimum balls.
						if (flDamage <= 10) {
							flyingBalls = 5;
						}
						else if (flDamage <= 180) {
							// Most additional balls in this range.
							flyingBalls = 5 + (flDamage - 10) * 0.115;
						}
						else {
							// deminishing returns, don't get too spammy.
							flyingBalls = 5 + (180 - 10) * 0.11 + (flDamage - 180) * 0.075;
						}

						//easyForcePrintLine("MY BALLS %d", flyingBalls);

						vec3_t fwd;
						VectorSubtract_f(tr.endpos, forward, fwd);
						gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, flyingBalls, 0.5, gEngfuncs.pfnRandomFloat(14, 20) / 100.0, 100, 255, 100);
					}



					if (!beam_tr.allsolid)
					{
						vec3_t delta;
						//MODDD - renamed from "n" to "m", avoids conflict with outter scope
						float m;

						// trace backwards to find exit point

						gEngfuncs.pEventAPI->EV_PlayerTrace(beam_tr.endpos, tr.endpos, FILLIN_TRACEFLAGS_STUDIO_BOX, -1, &beam_tr);

						VectorSubtract_f(beam_tr.endpos, tr.endpos, delta);

						m = Length(delta);


						if (m < flDamage)
						{
							if (m == 0)
								m = 1;
							flDamage -= m;


							// !!! OLD LOC OF 1st 'absorption balls' area

							//MODDD - oh, an as-is comment.  Anyway that's an AI sound you dolt, not audible. Nothing clientside is concerned with.
							//////////////////////////////////// WHAT TO DO HERE
													// CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );

							EV_HLDM_DecalGunshot(&beam_tr, BULLET_MONSTER_12MM);


							//MODDD - A tiny persistent glow here, same size as the ones that are meant to fly off?
							//        Why?  This seems more like a mistake.  DISABLED.
//#if DISABLE_GAUSS_GLOW != 1
//							gEngfuncs.pEfxAPI->R_TempSprite(beam_tr.endpos, vec3_origin, 0.1, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT);
//#endif

							// balls
							{
								//MODDD - amount reduced since the earlier ball-generation factors in flDamage too (more likely to be more than 3).
								// Old way here was 'flDamage * 0.3'.
								// ACTUALLY, DISABLED.  Seems like this has a higher chance of generating a punch of balls
								// into the object stuck, so there's an unintended annoyingly bright spot where they all overlap at.

#if ALLOW_RISKIEST_BALLS == 1
								vec3_t fwd;
								VectorSubtract_f(beam_tr.endpos, forward, fwd);
								gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, beam_tr.endpos, fwd, m_iBalls, (int)(flDamage * 0.10), 0.1, gEngfuncs.pfnRandomFloat(14, 20) / 100.0, 200, 255, 40);
#endif
							}

							VectorAdd_f(beam_tr.endpos, forward, vecSrc);
						}
						else {


						}
					}
					else
					{
						flDamage = 0;
					}

					gEngfuncs.pEventAPI->EV_PopPMStates();
				}
				else
				{
					//NOTE - punch is already only for secondary, so this point must be reached if not.
					//if (m_fPrimaryFire)
					{
#if DISABLE_GAUSS_GLOW != 1
						// slug doesn't punch through ever with primary 
						// fire, so leave a little glowy bit and make some balls
						gEngfuncs.pEfxAPI->R_TempSprite(tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_FADEOUT);
#endif
						{
							vec3_t fwd;
							VectorAdd_f(tr.endpos, tr.plane.normal, fwd);
							//MODDD - more consistent ball sizes, was a range of 10 to 20 (pfnRandomFloat)
							gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat(14, 20) / 100.0, 100, 255, 200);
						}
					}

					flDamage = 0;
				}
			}
		}
		else
		{
			//MODDD
			BOOL canPierce = TRUE;


			if (canPierce) {
				VectorAdd_f(tr.endpos, forward, vecSrc);
			}else{
				break;
			}

		}
	}//END OF while
}
//======================
//	   GAUSS END 
//======================



//======================
//	   CROWBAR START
//======================

//int g_iSwing = 0;
//#include "GameStudioModelRenderer.h"

//Only predict the miss sounds, hit sounds are still played 
//server side, so players don't get the wrong idea.
void EV_Crowbar(event_args_t* args)
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);

	//Play Swing sound
	//MODDD - if the canPlayHitSound CVar permits.
	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds) != 1 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds) != 3) {
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/cbar_miss1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
	}


	int swingMissChoice = args->iparam1;


	if (EV_IsLocal(idx))
	{
		// why this call??
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK1MISS, 1);

		
		//MODDD - handle by parameter now.
		//switch( (g_iSwing++) % 3 )
		


		//NOTE - this didn't turn out so well.  Would involving sequence info in any other ev_hldm method be helpful?
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/*
		int theAnim = HUD_GetWeaponAnim();
		cl_entity_t* ent = GetViewEntity();
		extern engine_studio_api_t IEngineStudio;
		extern CGameStudioModelRenderer g_StudioRenderer;
		mstudioseqdesc_t* pseqdesc;
		pseqdesc = (mstudioseqdesc_t*)((byte*)g_StudioRenderer.m_pStudioHeader + g_StudioRenderer.m_pStudioHeader->seqindex) + ent->curstate.sequence;
		*/
		
		/*
		float fEstimate = ent->latched.prevframe * ent->curstate.framerate * pseqdesc->fps;
		if (fEstimate >= pseqdesc->numframes - 1.001)
		{
			fEstimate = pseqdesc->numframes - 1.001;
		}
		if (fEstimate < 0.0)
		{
			fEstimate = 0.0;
		}
		*/

		//float portionDone = ent->latched.prevframe / ((float)(pseqdesc->numframes - 1));
		//easyForcePrintLine("SWING please %.2f", portionDone);

		// .frame > 200? no.  Viewmodels don't count curstate.frame at all.
		//

		//if ( (gpGlobals->time - ent->curstate.animtime > 0.25) || !(theAnim == CROWBAR_ATTACK1HIT || theAnim == CROWBAR_ATTACK2HIT || theAnim == CROWBAR_ATTACK3HIT)  ) {
		//if ((portionDone > 0.2) || !(theAnim == CROWBAR_ATTACK1HIT || theAnim == CROWBAR_ATTACK2HIT || theAnim == CROWBAR_ATTACK3HIT)) {
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			switch (swingMissChoice)
			{
			case 0:
				gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK1MISS, 1); break;
			case 1:
				gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK2MISS, 1); break;
			case 2:
				gEngfuncs.pEventAPI->EV_WeaponAnimation(CROWBAR_ATTACK3MISS, 1); break;
			}
		//}
		

	}

}
//======================
//	   CROWBAR END 
//======================



//======================
//	  CROSSBOW START
//======================

//=====================
// EV_BoltCallback
// This function is used to correct the origin and angles 
// of the bolt, so it looks like it's stuck on the wall.
//=====================
void EV_BoltCallback(struct tempent_s* ent, float frametime, float currenttime)
{
	ent->entity.origin = ent->entity.baseline.vuser1;
	ent->entity.angles = ent->entity.baseline.vuser2;
}

// Firing sniper bolts only
void EV_FireCrossbow2(event_args_t* args)
{
	vec3_t vecSrc, vecEnd;
	vec3_t up, right, forward;
	pmtrace_t tr;

	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, angles);

	VectorCopy_f(args->velocity, velocity);

	AngleVectors(angles, forward, right, up);

	EV_GetGunPosition(args, vecSrc, origin);

	VectorMA(vecSrc, 8192, forward, vecEnd);


	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));

		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound) > 0) {
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));
		}
	}

	if (EV_IsLocal(idx))
	{
		if (args->iparam1)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE1, 1);
		else if (args->iparam2)
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE3, 1);

		//MODDD - why was this missing in as-is?  Hard to say if that was intentional or not of as-is.
		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-7.5f, -6.0f));
		}else{
			// Put back in, reduced slightly
			V_PunchAxis(0, -1.6);
		}
	}//EV_IsLocal

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(FILLIN_TRACE_HULL);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, FILLIN_TRACEFLAGS_STUDIO_BOX, -1, &tr);

	// We hit something
	if (tr.fraction < 1.0)
	{
		physent_t* pe = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);
		int hitEntIndex = gEngfuncs.pEventAPI->EV_IndexFromTrace(&tr);
		cl_entity_t* cEntRef = gEngfuncs.GetEntityByIndex(hitEntIndex);

		// ALTERNATE WAY:  Test it out?
		//int hitEntIndex = gEngfuncs.pEventAPI->EV_IndexFromTrace(&tr);
		//cl_entity_t* cEntRef = gEngfuncs.GetEntityByIndex(hitEntIndex);

		/*
		// break point and see all this?
		int testin = cEntRef->index;
		int testin2 = cEntRef->curstate.eflags;
		int xxx = pe->classnumber;
		int xxx2 = pe->info;
		const char* whut = pe->name;
		*/


		// Not the world, let's assume we hit something organic ( dog, cat, uncle joe, etc ).

		//MODDD - NO, don't switch to cEntRef->curstate.solid.  It magically doesn't work if the entity is the world.
		// pe->solid always works.  cEntRef works for getting renderflags though.  Checking for the index being 0 
		// is good enough to tell if this is the world anyway.

		//if (cEntRef->curstate.solid != SOLID_BSP){
		if(pe->solid != SOLID_BSP){
			//if (!(pe->info == 8)) {
			if (cEntRef != NULL && !((cEntRef->curstate.renderfx & ISMETALNPC) == ISMETALNPC) ) {
				switch (gEngfuncs.pfnRandomLong(0, 1))
				{
				case 0:
					gEngfuncs.pEventAPI->EV_PlaySound(idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
				case 1:
					gEngfuncs.pEventAPI->EV_PlaySound(idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM, 0, PITCH_NORM); break;
				}
			}
			else {
				// not organic?  Slightly modified xbow_hit.wav
				gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_BODY, "weapons/xbow_hit1.wav", gEngfuncs.pfnRandomFloat(0.98, 1.0), ATTN_NORM - 0.1, 0, 107 + gEngfuncs.pfnRandomLong(0, 4));
				//if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
				//if(gEngfuncs.PM_PointContents(cEntRef->origin, NULL) != CONTENTS_WATER)
				if (gEngfuncs.PM_PointContents(tr.endpos, NULL) != CONTENTS_WATER)
				{
					UTIL_Sparks(tr.endpos, DEFAULT_SPARK_BALLS, EASY_CVAR_GET_DEBUGONLY(sparksPlayerCrossbowMulti));
				}
				else {
					// A blend of pushed away from the surface collided with, and away from the direction I was moving in.
					// Should push the bubbles away from the wall as to not clip too much and look like it's coming
					// from the bolt itself.
					Vector temp1 = tr.endpos + tr.plane.normal * 2 + -forward * 6 + Vector(-1.7, -1.7, -1.7);
					Vector temp2 = tr.endpos + tr.plane.normal * 2 + -forward * 6 + Vector(1.7, 1.7, 1.7);
					UTIL_Bubbles(temp1, temp2, 8);
				}
			}
		}
		// Stick to world but don't stick to glass, it might break and leave the bolt floating. It can still stick to other non-transparent breakables though.
		//MODDD - it's fine not to want to generate a projectile because the thing that got hit could be glass,
		// but shouldn't the sparks be generated anyway to show a sign of what area was hit?
		//else if ( pe->rendermode == kRenderNormal ) 
		else
		{
			// ALSO, going to assume what is hit isn't organic.  Why not.
			gEngfuncs.pEventAPI->EV_PlaySound(0, tr.endpos, CHAN_BODY, "weapons/xbow_hit1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, PITCH_NORM);

			//Not underwater, do some sparks...
			if (gEngfuncs.PM_PointContents(tr.endpos, NULL) != CONTENTS_WATER) {
				UTIL_Sparks(tr.endpos);
			}
			else {
				Vector temp1 = tr.endpos + tr.plane.normal * 2 + -forward * 6 + Vector(-1.7, -1.7, -1.7);
				Vector temp2 = tr.endpos + tr.plane.normal * 2 + -forward * 6 + Vector(1.7, 1.7, 1.7);
				UTIL_Bubbles(temp1, temp2, 8);
			}

			// Only place an arrow on something on the map we're certain won't disappear (like glass, complex things like trains, etc.)
			// I think this includes trains at least?  Weird.
			// ALSO, this gets some func_tracktrain entities: check for MOVETYPE_PUSH.  Or just require being MOVETYPE_NONE.
			// The world does havea  pe->index of 0, but there may be other map-like entities (func_wall) that
			// a bolt could stick out of just fine too.  I think this is the best we can do.
			// Also, a 'hitEntIndex == 0' check would cover the world, but it is already included by these conditions.
			//if (pe->rendermode == kRenderNormal) {
			// pe->rendermode
			if (cEntRef != NULL && cEntRef->curstate.rendermode == kRenderNormal && cEntRef->curstate.movetype == MOVETYPE_NONE) {
				vec3_t vBoltAngles;
				int iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("models/crossbow_bolt.mdl");

				VectorAngles(forward, vBoltAngles);

				TEMPENTITY* bolt = gEngfuncs.pEfxAPI->R_TempModel(tr.endpos - forward * 10, Vector(0, 0, 0), vBoltAngles, 5, iModelIndex, TE_BOUNCE_NULL);

				if (bolt)
				{
					bolt->flags |= (FTENT_CLIENTCUSTOM); //So it calls the callback function.
					bolt->entity.baseline.vuser1 = tr.endpos - forward * 10; // Pull out a little bit
					bolt->entity.baseline.vuser2 = vBoltAngles; //Look forward!

					//MODDD - be explicit! It's the address OF the method you want!
					//        compiler figures it out anyways I suppose.
					bolt->callback = &EV_BoltCallback; //So we can set the angles and origin back. (Stick the bolt to the wall)
				}
			}//END OF kRenderNormal check
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

//TODO: Fully predict the fliying bolt.
// Firing non-sniper bolts only
void EV_FireCrossbow(event_args_t* args)
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);


	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));

		if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(crossbowFirePlaysReloadSound) > 0) {
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat(0.95, 1.0), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong(0, 0xF));
		}
	}

	//Only play the weapon anims if I shot it. 
	if (EV_IsLocal(idx))
	{
		if (args->iparam1){
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE1, 1);
		}else if (args->iparam2){
			gEngfuncs.pEventAPI->EV_WeaponAnimation(CROSSBOW_FIRE3, 1);
		}

		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-8.5f, -7.0f));
		}else{
			V_PunchAxis(0, -2);
		}
	}
}
//======================
//	   CROSSBOW END 
//======================

//======================
//	    RPG START 
//======================
void EV_FireRpg(event_args_t* args)
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);


	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		
		if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(myRocketsAreBarney) != 1) {
			//ordinary
			//MODDD - little less attenuation for RPG's, it is a danged rocket going off after all
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/rocketfire1.wav", 0.9, ATTN_NORM - 0.14, 0, PITCH_NORM);
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/glauncher.wav", 0.7, ATTN_NORM - 0.04, 0, PITCH_NORM);
		}else {
			//we wanna hear barney!
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "weapons/rocketfire1.wav", 0.10, ATTN_NORM, 0, PITCH_NORM);
			gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_ITEM, "weapons/glauncher.wav", 0.08, ATTN_NORM, 0, PITCH_NORM);
		}
	}

	//Only play the weapon anims if I shot it. 
	if (EV_IsLocal(idx))
	{
		//gEngfuncs.pEventAPI->EV_WeaponAnimation(RPG_FIRE2, 1);
		gEngfuncs.pEventAPI->EV_WeaponAnimation(RPG_FIRE, 1);

		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-15, -12));
		}else{
			V_PunchAxis(0, -5);
		}
	}
}
//======================
//	     RPG END 
//======================

//======================
//	    EGON END 
//======================

int g_fireAnims1[] = { EGON_FIRE1, EGON_FIRE2, EGON_FIRE3, EGON_FIRE4 };
int g_fireAnims2[] = { EGON_ALTFIRECYCLE };


BEAM* pBeam;
BEAM* pBeam2;

BOOL g_cl_egonEffectCreatedYet = FALSE;




void EV_EgonFire(event_args_t* args)
{

	int idx, iFireState, iFireMode;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);

	// what the.. you don't do anything with firestate here.  Assumption that it's on, or this is the first call if it wasn't at the time (set soon after)?
	// 'iStartup' mirrors that the same really.  Initial call gets it to make a different beam sound effect and spawn the beam effect.
	// Also, NEW.   If bparam2 is TRUE, that means coming from a transition.  Pretend iparam1 was on (if it wasn't; spawn the beam) and play the continual sound instead.
	iFireState = args->iparam1;

	iFireMode = args->iparam2;

	int iStartup = args->bparam1;

	if(args->bparam2 == TRUE){
		// special value
		iStartup = 2;
	}


	int hasSpiralBeam;


	if (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode) == 3) {
		// only the narrow beam gets it.
		hasSpiralBeam = (iFireMode == FIRE_NARROW);
		// or... not?  oooookay
		//hasSpiralBeam = TRUE;
	}
	else {
		// otherwise, alsways has it.
		hasSpiralBeam = TRUE;
	}



	//easyForcePrintLine("HERE COMECS HONEY BADASS madeyet:%d startup:%d", g_cl_egonEffectCreatedYet, iStartup);


	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		if (iStartup == 1)
		{
			if(!g_cl_egonEffectCreatedYet){
				// It does not make sense to make this sound if the beam has already been created in a 'startup' call
				if (iFireMode == FIRE_WIDE)
					gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.98, ATTN_NORM, 0, 125);
				else
					gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.9, ATTN_NORM, 0, 100);
			}
		}
		else
		{
			if( (iStartup == 2 && !g_cl_egonEffectCreatedYet) || (iStartup == 0 && g_cl_egonEffectCreatedYet)){
				if (iFireMode == FIRE_WIDE)
					gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.98, ATTN_NORM, 0, 125);
				else
					gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.9, ATTN_NORM, 0, 100);
			}
		}
	}

	//easyPrintLine("EGONEVENT1 %d, %d, %d, %d, %d", iStartup==1, EV_IsLocal( idx ), !pBeam, !pBeam2, (int)cl_lw->value );
	//Only play the weapon anims if I shot it.

	if (EV_IsLocal(idx)) {

		//MODDD - if FIRE_NARROW, assume normal anim.  Otherwise, assume FIRE_WIDE anim instead.
		//MODDD - circumstances for using NARROW and WIDE swapped.
		if (iFireMode == FIRE_NARROW) {
			//gEngfuncs.pEventAPI->EV_WeaponAnimation ( g_fireAnims1[ gEngfuncs.pfnRandomLong( 0, 3 ) ], 1 );
			//MODDD - anims 1 and 2 seem to be defunct.  Removing from random range   [0, 3] --> [2, 3]
			gEngfuncs.pEventAPI->EV_WeaponAnimation(g_fireAnims1[gEngfuncs.pfnRandomLong(2, 3)], 1);
		}
		else {//wide
			gEngfuncs.pEventAPI->EV_WeaponAnimation(g_fireAnims2[0], 1);
		}
	}


	//easyForcePrintLine("EV_EgonFire pid: %d  islocal? %d dathing %d othathing: %.2f", idx, EV_IsLocal(idx), iStartup, cl_lw->value );


	if ( 1 ){
		//gets the spiral.

		//MODDD - SAFETY.  If for whatever reason the effect has not been created yet, ignore iStartup being 0.

		//if ((iStartup == 1 || !g_cl_egonEffectCreatedYet) && EV_IsLocal(idx) && !pBeam && !pBeam2 && cl_lw->value) //Adrian: Added the cl_lw check for those lital people that hate weapon prediction.
		if ((iStartup != 0 || !g_cl_egonEffectCreatedYet) && EV_IsLocal(idx) && cl_lw->value) //Adrian: Added the cl_lw check for those lital people that hate weapon prediction.
		{

			if (pBeam)
			{
				pBeam->die = 0.0;
				pBeam = NULL;
			}
			if (pBeam2)
			{
				pBeam2->die = 0.0;
				pBeam2 = NULL;
			}


			g_cl_egonEffectCreatedYet = TRUE;  // why yes.

			vec3_t vecSrc;
			vec3_t vecEnd;
			//MODDD - what.  why.
			//vec3_t origin;
			vec3_t angles;
			vec3_t forward;
			vec3_t right;
			vec3_t up;

			pmtrace_t tr;
			cl_entity_t* pl = gEngfuncs.GetEntityByIndex(idx);

			//easyPrintLine("EGONEVENT2 %d", pl != 0 );
			if (pl)
			{
				VectorCopy_f(gHUD.m_vecAngles, angles);

				AngleVectors(angles, forward, right, up);

				//MODDD - NOTE.  Interesting not to use the sent 'origin' like most, if not all other wepaons would here.
				// instead of 'pl->origin'
				EV_GetGunPosition(args, vecSrc, pl->origin);

				VectorMA(vecSrc, 2048, forward, vecEnd);

				gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(FALSE, TRUE);

				// Store off the old count
				gEngfuncs.pEventAPI->EV_PushPMStates();

				// Now add in all of the players.
				gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);

				gEngfuncs.pEventAPI->EV_SetTraceHull(FILLIN_TRACE_HULL);
				gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, FILLIN_TRACEFLAGS_STUDIO_BOX, -1, &tr);

				gEngfuncs.pEventAPI->EV_PopPMStates();

				int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex(EGON_BEAM_SPRITE);

				float r = 50.0f;
				float g = 50.0f;
				float b = 125.0f;


				//MODDD NOTE - ........why is this the only place that checks for IsHardware in all of ev_hldm, even though
				// there are other places that deal with colors?
				// And this excludes the 'b' coord?   ...    *Why*
				/*
				if ( IEngineStudio.IsHardware() )
				{
					r /= 100.0f;
					g /= 100.0f;
				}
				*/
				// They'll just be out of 255 instead.
				r = r * (1 / 255.0f);
				g = g * (1 / 255.0f);
				b = b * (1 / 255.0f);


				/*
				if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(egonEffectsMode) == 3 && iFireMode == FIRE_WIDE){
					// less noticeable, I guess?
					if (hasSpiralBeam) {
						//spiral purple beam.
						pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 3.2, 0.12, 0.8, 90, 0, 0, r, g, b);

						//easyPrintLine("EGONEVENT3 %d", pBeam != 0 );
						if (pBeam)
							pBeam->flags |= (FBEAM_SINENOISE);

					}
				}else
				*/
				{
					if (hasSpiralBeam) {
						//spiral purple beam.
						pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 3.5, 0.2, 0.7, 55, 0, 0, r, g, b);

						//easyPrintLine("EGONEVENT3 %d", pBeam != 0 );
						if (pBeam)
							pBeam->flags |= (FBEAM_SINENOISE);

					}
				}


				/*
				const float life = 99999;
				const float width = 5.0;
				const float amplitude = 0.08;
				const float brightness = 0.7;
				const float speed = 25;
				const float startframe = 0;
				const float framerate = 0;
				*/


				// TODO - this needs to be for whatever effects var instead

				if(iFireMode == FIRE_NARROW){

					const float life = 99999;
					const float width = 4.2;   // was 4.5
					const float amplitude = 0.08;
					const float brightness = 0.7;
					const float speed = 25;
					const float startframe = 0;
					const float framerate = 0;
					float t_r = r;
					float t_g = g;
					float t_b = b;

					//straight purple beam.
					pBeam2 = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, life, width, amplitude, brightness, speed, startframe, framerate, t_r, t_g, t_b);
					//pBeam2->freq = 0.2;
					//easyForcePrintLine("WHAT IS FREQ DEF %.2f", pBeam2->freq);

				}else{
					// WIDE

					/*
					const float life = 99999;
					const float width = 4.8;
					const float amplitude = 0.06;
					const float brightness = 0.80;
					const float speed = 40;
					const float startframe = 0;
					const float framerate = 0;
					float t_r = 0.18f;
					float t_g = 0.32f;
					float t_b = 0.74f;

					//straight purple beam.
					pBeam2 = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, life, width, amplitude, brightness, speed, startframe, framerate, t_r, t_g, t_b);
					
					// so this is found counting up with game time.
					// oooookay, not gonna touch that
					//pBeam2->freq = 0.2;
					*/


					// TEST

					// straight purple beam.
					pBeam2 = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 5.3, 0.08, 0.7, 25, 0, 0, r, g, b);

					if(!hasSpiralBeam){
						r = 40.0f;
						g = 90.0f;
						b = 125.0f;
						r = r * (1 / 255.0f) * 1.0;
						g = g * (1 / 255.0f) * 2.0;
						b = b * (1 / 255.0f) * 2.0;
						pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 4.0, 0.105, 0.92, 70, 0, 0, r, g, b);
					}

				}//END OF beam type check
			}
		}
	}//END OF spiral beam check



	// OLD BEAM SCRAPS, remove later

	/*

	float r = 50.0f;
	float g = 50.0f;
	float b = 125.0f;


	//MODDD NOTE - ........why is this the only place that checks for IsHardware in all of ev_hldm, even though
	// there are other places that deal with colors?
	// And this excludes the 'b' coord?   ...    *Why*
	
	//if ( IEngineStudio.IsHardware() )
	//{
	//r /= 100.0f;
	//g /= 100.0f;
	//}
	// They'll just be out of 255 instead.
	r = r * (1 / 255.0f);
	g = g * (1 / 255.0f);
	b = b * (1 / 255.0f);



	if (hasSpiralBeam) {
		//spiral purple beam.
		pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 3.5, 0.2, 0.7, 55, 0, 0, r, g, b);

		//easyPrintLine("EGONEVENT3 %d", pBeam != 0 );
		if (pBeam)
			pBeam->flags |= (FBEAM_SINENOISE);

	}

	//straight purple beam.
	pBeam2 = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 5.0, 0.08, 0.7, 25, 0, 0, r, g, b);





	m_pBeam = CBeam::BeamCreate( EGON_BEAM_SPRITE, 40 );
	m_pBeam->PointEntInit( vecSrc, m_pPlayer->entindex()  );
	//MODDD - commented out.  Causes the beam not to sufficiently shrink when nearing a surface you're firing at, causing the effect to clip backwards into the player & gun.
	//m_pBeam->SetFlags( BEAM_FSINE );
	m_pBeam->SetEndAttachment( 1 );
	m_pBeam->pev->spawnflags |= SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
													//if(testVar == 0 || testVar == 2 ){
													//	m_pBeam->pev->flags |= FL_SKIPLOCALHOST;
													//}
	m_pBeam->pev->owner = m_pPlayer->edict();



	//if ( m_fInAttack == FIRE_WIDE )
	//{
	//	m_pBeam->SetScrollRate( 50 );
	//	m_pBeam->SetNoise( 20 );
	//}
	//else
	//{
	//// want narrow mode's features.
	m_pBeam->SetScrollRate( 110 );
	m_pBeam->SetNoise( 5 );
	//}



	m_pNoise = CBeam::BeamCreate( EGON_BEAM_SPRITE, 55 );
	//new?
	//m_pNoise->SetFlags( BEAM_FSINE );

	m_pNoise->PointEntInit( vecSrc, m_pPlayer->entindex() );
	m_pNoise->SetScrollRate( 25 );
	m_pNoise->SetBrightness( 100 );
	m_pNoise->SetEndAttachment( 1 );
	m_pNoise->pev->spawnflags |= SF_BEAM_TEMPORARY;
	//if(testVar == 0 || testVar == 1){
	//	m_pNoise->pev->flags |= FL_SKIPLOCALHOST;
	//}
	m_pNoise->pev->owner = m_pPlayer->edict();


	//if ( m_fInAttack == FIRE_WIDE )
	//{
	//	m_pNoise->SetColor( 50, 50, 255 );
	//	m_pNoise->SetNoise( 8 );
	//}
	//else
	//{
	//// want narrow's features.
	m_pNoise->SetColor( 80, 120, 255 );
	m_pNoise->SetNoise( 2 );
	//}

	*/
	
}//END OF EV_EgonFire


void EV_EgonStop(event_args_t* args)
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);


	//easyForcePrintLine("EV_EgonStop pid: %d  islocal? %d dathing %d othathing: %.2f", idx, EV_IsLocal(idx), 666, cl_lw->value );


	if(g_cl_egonEffectCreatedYet){
		// no more
		g_cl_egonEffectCreatedYet = FALSE;
	}

	gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, EGON_SOUND_RUN);

	if (args->iparam1)
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, EGON_SOUND_OFF, 0.98, ATTN_NORM, 0, 100);

	if (EV_IsLocal(idx))
	{
		if (pBeam)
		{
			pBeam->die = 0.0;
			pBeam = NULL;
		}

		if (pBeam2)
		{
			pBeam2->die = 0.0;
			pBeam2 = NULL;
		}
	}
}
//======================
//	    EGON END 
//======================

//======================
//	   HORNET START
//======================

void EV_HornetGunFire(event_args_t* args)
{
	int idx, iFireMode;
	vec3_t origin, angles, vecSrc, forward, right, up;

	idx = args->entindex;
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, angles);
	iFireMode = args->iparam1;

	//Only play the weapon anims if I shot it.
	if (EV_IsLocal(idx))
	{
		//MODDD - was from 0 to 2.  Wait, why wasn't it negative like everything else?
		// And a random long?  Why not decimal?

		if(EASY_CVAR_GET_CLIENTONLY(cl_viewpunch_mod) == 1){
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(-4.2, -3.5));
		}else{
			// still using gEngfuncs.pfnRandomFloat instead, still on the positive side like as-is?
			V_PunchAxis(0, gEngfuncs.pfnRandomFloat(0, 2));
		}

		gEngfuncs.pEventAPI->EV_WeaponAnimation(HGUN_SHOOT, 1);
	}

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(mutePlayerWeaponFire) != 1) {
		switch (gEngfuncs.pfnRandomLong(0, 2))
		{
		case 0:	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire1.wav", 1, ATTN_NORM, 0, 100);	break;
		case 1:	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire2.wav", 1, ATTN_NORM, 0, 100);	break;
		case 2:	gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_WEAPON, "agrunt/ag_fire3.wav", 1, ATTN_NORM, 0, 100);	break;
		}
	}

}
//======================
//	   HORNET END
//======================

//======================
//	   TRIPMINE START
//======================


//We only check if it's possible to put a trip mine
//and if it is, then we play the animation. Server still places it.
void EV_TripmineFire(event_args_t* args)
{
	int idx;
	vec3_t vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy_f(args->origin, vecSrc);
	VectorCopy_f(args->angles, angles);

	AngleVectors(angles, forward, NULL, NULL);

	if (!EV_IsLocal(idx))
		return;

	// Grab predicted result for local player
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);

	vecSrc = vecSrc + view_ofs;

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(FILLIN_TRACE_HULL);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + forward * 128, FILLIN_TRACEFLAGS_NORMAL, -1, &tr);

	//Hit something solid

	if (tr.fraction < 1.0) {
		//MODDD - doing this instead. 
	   //gEngfuncs.pEventAPI->EV_WeaponAnimation ( TRIPMINE_DRAW, 0 );
	   //by the way, the "PLACE" animation is named "TRIPMINE_ARM2" here instead.
	   //gEngfuncs.pEventAPI->EV_WeaponAnimation ( TRIPMINE_ARM2, 0 );
	   //No, play it in "tripmine.cpp" instead for more control over doing 2 animations in succession: place, then draw.
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   TRIPMINE END
//======================

//======================
//	   SQUEAK START
//======================

void EV_SnarkFire(event_args_t* args)
{
	int idx;
	vec3_t vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy_f(args->origin, vecSrc);
	VectorCopy_f(args->angles, angles);

	AngleVectors(angles, forward, NULL, NULL);

	if (!EV_IsLocal(idx))
		return;

	//MODDD - duck correction removed.  It just placed things at the same height
	// as standing while ducking, which kinda doesn't make sense.
	//if (args->ducking){
	//	vecSrc = vecSrc - (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);
	//}

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(FILLIN_TRACE_HULL);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc + forward * 20, vecSrc + forward * 64, FILLIN_TRACEFLAGS_NORMAL, -1, &tr);

	// Find space to drop the thing.
	if (tr.allsolid == 0 && tr.startsolid == 0 && tr.fraction > 0.25)
		gEngfuncs.pEventAPI->EV_WeaponAnimation(SQUEAK_THROW, 0);

	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   SQUEAK END
//======================



//MODDD - NEW
//======================
//	   CHUMTOAD START
//=====================
void EV_ChumToadFire(event_args_t* args)
{
	int idx;
	vec3_t vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy_f(args->origin, vecSrc);
	VectorCopy_f(args->angles, angles);

	AngleVectors(angles, forward, NULL, NULL);

	if (!EV_IsLocal(idx))
		return;

	//if (args->ducking){
	//	vecSrc = vecSrc - (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);
	//}
	
	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(FILLIN_TRACE_HULL);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc + forward * 20, vecSrc + forward * 64, FILLIN_TRACEFLAGS_NORMAL, -1, &tr);

	//Find space to drop the thing.
	//if ( tr.allsolid == 0 && tr.startsolid == 0 && tr.fraction > 0.25 )
	gEngfuncs.pEventAPI->EV_WeaponAnimation(CHUMTOADWEAPON_THROW, 0);

	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   CHUMTOAD END
//======================





void EV_TrainPitchAdjust(event_args_t* args)
{
	int idx;
	vec3_t origin;

	unsigned short us_params;
	int noise;
	float m_flVolume;
	int pitch;
	int stop;

	char sz[256];

	idx = args->entindex;

	VectorCopy_f(args->origin, origin);

	us_params = (unsigned short)args->iparam1;
	stop = args->bparam1;

	m_flVolume = (float)(us_params & 0x003f) / 40.0;
	noise = (int)(((us_params) >> 12) & 0x0007);
	pitch = (int)(10.0 * (float)((us_params >> 6) & 0x003f));

	switch (noise)
	{
	case 1: strcpy(sz, "plats/ttrain1.wav"); break;
	case 2: strcpy(sz, "plats/ttrain2.wav"); break;
	case 3: strcpy(sz, "plats/ttrain3.wav"); break;
	case 4: strcpy(sz, "plats/ttrain4.wav"); break;
	case 5: strcpy(sz, "plats/ttrain6.wav"); break;
	case 6: strcpy(sz, "plats/ttrain7.wav"); break;
	default:
		// no sound
		strcpy(sz, "");
		return;
	}

	if (stop)
	{
		gEngfuncs.pEventAPI->EV_StopSound(idx, CHAN_STATIC, sz);
	}
	else
	{
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, CHAN_STATIC, sz, m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, pitch);
	}
}

int EV_TFC_IsAllyTeam(int iTeam1, int iTeam2)
{
	return 0;
}



const float matrix_rot_x[3][3] = {
	1, 0, 0,
	0, 0, -1,
	0, 1, 0
};

const float matrix_rot_y[3][3] = {
	0, 0, 1,
	0, 1, 0,
	-1, 0, 0
};

const float matrix_rot_z[3][3] = {
	0, -1, 0,
	1, 0, 0,
	0, 0, 1
};

void matrixMult(const Vector& m_1, const float m_2[3][3], Vector& v_out) {

	v_out.x = m_1.x * m_2[0][0] + m_1.y * m_2[0][1] + m_1.z * m_2[0][2];
	v_out.y = m_1.x * m_2[1][0] + m_1.y * m_2[1][1] + m_1.z * m_2[1][2];
	v_out.z = m_1.x * m_2[2][0] + m_1.y * m_2[2][1] + m_1.z * m_2[2][2];

}//END OF matrixMult








void createBall(int* sprite, Vector* loc) {



	float randomStrength = 56;

	/*
	float randx = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randy = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randz = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	*/
	float randx = gEngfuncs.pfnRandomFloat(0, randomStrength);
	float randy = gEngfuncs.pfnRandomFloat(0, randomStrength);
	float randz = gEngfuncs.pfnRandomFloat(35, randomStrength + 80);

	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randx *= -1;
	}
	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randy *= -1;
	}

	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randz *= -1;
	}



	vec3_t rot = Vector(randx, randy, randz);


	TEMPENTITY* eh = gEngfuncs.pEfxAPI->R_TempSprite(*loc, rot, 0.12f, *sprite, kRenderGlow, kRenderFxNoDissipation, 250.0 / 255.0, 0.22f, FTENT_GRAVITY | FTENT_COLLIDEWORLD | FTENT_FADEOUT);
	if (eh != NULL) {
		eh->fadeSpeed = 3.3f;
	}
	//easyPrintLine("??? %.2f", eh->bounceFactor);

}



void createBallPowerup(int* sprite, Vector* loc) {



	float randomStrength = 56;

	/*
	float randx = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randy = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randz = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	*/
	float randx = gEngfuncs.pfnRandomFloat(0, randomStrength);
	float randy = gEngfuncs.pfnRandomFloat(0, randomStrength);
	float randz = gEngfuncs.pfnRandomFloat(35, randomStrength + 80);

	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randx *= -1;
	}
	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randy *= -1;
	}

	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randz *= -1;
	}



	vec3_t rot = Vector(randx, randy, randz);


	//MODDD - color it red. how we dod that?
	//life is 2nd from the last with the flags yo.
	TEMPENTITY* eh = gEngfuncs.pEfxAPI->R_TempSprite(*loc, rot, 0.12f, *sprite, kRenderGlow, kRenderFxNoDissipation, 250.0 / 255.0, 1.22f, FTENT_SLOWGRAVITY | FTENT_COLLIDEWORLD | FTENT_FADEOUT);
	if (eh != NULL) {
		eh->fadeSpeed = 0.6f;
		//eh->entity.baseline.gravity
		//eh->entity.curstate.gravity
		//eh->entity.prevstate.gravity
		//???


		//MODDD - remove "FTENT_GRAVITY" from above, yes or no?

		//unnecessary yes?
		//eh->flags |= (FTENT_SLOWGRAVITY);


		//easyPrintLine("??? %.2f", eh->bounceFactor);
	}


}


void createBallVomit(int* sprite, Vector* loc, Vector* dir) {
	float randomStrength = 80;

	/*
	float randx = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randy = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randz = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	*/
	float randx = dir->x * 270 + gEngfuncs.pfnRandomFloat(-20, randomStrength);
	float randy = dir->y * 270 + gEngfuncs.pfnRandomFloat(-20, randomStrength);
	float randz = 50 + gEngfuncs.pfnRandomFloat(-40, randomStrength);

	/*
	if(gEngfuncs.pfnRandomLong(0, 1) == 0){
		randx *= -1;
	}
	if(gEngfuncs.pfnRandomLong(0, 1) == 0){
		randy *= -1;
	}

	if(gEngfuncs.pfnRandomLong(0, 1) == 0){
		randz *= -1;
	}
	*/


	vec3_t rot = Vector(randx, randy, randz);

	TEMPENTITY* eh = gEngfuncs.pEfxAPI->R_TempSprite(*loc, rot, 0.26f, *sprite, kRenderGlow, kRenderFxNoDissipation, 250.0 / 255.0, 0.14f, FTENT_GRAVITY | FTENT_COLLIDEWORLD | FTENT_FADEOUT);
	if (eh) {
		eh->fadeSpeed = 3.4f;
	}
	else {
		easyForcePrintLine("WHY YOU FAIL");
	}
	//easyPrintLine("??? %.2f", eh->bounceFactor);

}














void floaterGasCallback(struct tempent_s* ent, float frametime, float currenttime)
{
	if (currenttime < ent->entity.baseline.fuser1)
		return;

	if (ent->entity.baseline.origin[2] > -3.8) {
		ent->entity.baseline.origin[2] += -0.8;
	}
	else {
		ent->entity.baseline.origin[2] = -3.8;
	}

	//TEST - real slow for now!
	ent->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 0;
}//END OF EV_imitation7_think


void floaterBigGasCallback(struct tempent_s* ent, float frametime, float currenttime)
{
	if (currenttime < ent->entity.baseline.fuser1)
		return;

	if (ent->entity.baseline.origin[2] > -3.4) {
		ent->entity.baseline.origin[2] += -0.5;
	}
	else {
		ent->entity.baseline.origin[2] = -3.4;
	}

	//TEST - real slow for now!
	ent->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 0;
}//END OF EV_imitation7_think



void createBallFloaterGas(int* sprite, const Vector& loc) {

	float randomStrength = 14;

	/*
	float randx = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randy = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randz = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	*/
	float randx = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randy = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randz = 80 + gEngfuncs.pfnRandomFloat(0, 30);

	float randScale = gEngfuncs.pfnRandomFloat(0.88, 1.06);

	/*
	if(gEngfuncs.pfnRandomLong(0, 1) == 0){
		randx *= -1;
	}
	if(gEngfuncs.pfnRandomLong(0, 1) == 0){
		randy *= -1;
	}

	if(gEngfuncs.pfnRandomLong(0, 1) == 0){
		randz *= -1;
	}
	*/


	Vector locWhat = loc;
	vec3_t rot = Vector(randx, randy, randz);

	//FTENT_SLOWGRAVITY  ?
	TEMPENTITY* eh = gEngfuncs.pEfxAPI->R_TempSprite(locWhat, rot, randScale, *sprite, kRenderGlow, kRenderFxNoDissipation, 160.0 / 255.0, 1.8f, FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD | FTENT_FADEOUT);
	if (eh) {
		eh->fadeSpeed = 0.07f;
		eh->bounceFactor = 0;
		eh->callback = &floaterGasCallback;

	}
	else {
		easyForcePrintLine("WHY YOU FAIL");
	}
	//easyPrintLine("??? %.2f", eh->bounceFactor);

}

void createBigBallFloaterGas(int* sprite, const Vector& loc) {

	float randomStrength = 6;

	float randx = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randy = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randz = 30 + gEngfuncs.pfnRandomFloat(0, 30);

	float randScale = gEngfuncs.pfnRandomFloat(2.5, 2.9);

	Vector locWhat = loc;
	vec3_t rot = Vector(randx, randy, randz);

	//FTENT_SLOWGRAVITY  ?
	TEMPENTITY* eh = gEngfuncs.pEfxAPI->R_TempSprite(locWhat, rot, randScale, *sprite, kRenderGlow, kRenderFxNoDissipation, 100.0 / 255.0, 1.0f, FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD | FTENT_FADEOUT);
	if (eh) {
		eh->fadeSpeed = 0.06f;
		eh->bounceFactor = 0;
		eh->callback = &floaterBigGasCallback;

	}
	else {
		easyForcePrintLine("WHY YOU FAIL");
	}

}








void generateFreakyLight(const Vector& arg_origin) {

	//g_engfuncs.pfnCVarSetFloat

	//g_engfuncs.


	//clientside way:
	//gEngfuncs.pfnGetCvarPointer
	//NOTE: equivalent for serverside:
	//g_engfuncs.pfnCVarGetPointer




	/*
	float durationMin = 0.08;
	float durationMax = 0.14;
	float radiusMin = 280;
	float radiusMax = 410;
	float spawnDistHori = 180;
	float spawnDistVertMin = 25;
	float spawnDistVertMax = 75;
	int multiColor = TRUE;
	*/
	float durationMin = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(strobeDurationMin);
	float durationMax = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(strobeDurationMax);
	float radiusMin = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(strobeRadiusMin);
	float radiusMax = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(strobeRadiusMax);
	float spawnDistHori = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(strobeSpawnDistHori);
	float spawnDistVertMin = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMin);
	float spawnDistVertMax = EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(strobeSpawnDistVertMax);


	int multiColor;
	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(strobeMultiColor) == 1) {
		multiColor = TRUE;
	}
	else {
		multiColor = FALSE;
	}



	const Vector& origin = arg_origin;

	float randx;
	float randy;
	float randz;

	dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
	float temp;

	randx = gEngfuncs.pfnRandomFloat(30, spawnDistHori);
	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randx *= -1;
	}
	randy = gEngfuncs.pfnRandomFloat(30, spawnDistHori);
	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randy *= -1;
	}
	randz = gEngfuncs.pfnRandomFloat(spawnDistVertMin, spawnDistVertMax);

	randx += origin.x;
	randy += origin.y;
	randz += origin.z;

	float randRad = gEngfuncs.pfnRandomFloat(radiusMin, radiusMax);
	float randLife = gEngfuncs.pfnRandomFloat(durationMin, durationMax);

	dl->origin.x = randx;
	dl->origin.y = randy;
	dl->origin.z = randz;


	dl->dark = FALSE;
	dl->die = gEngfuncs.GetClientTime() + randLife;


	int colorReceive[3];

	if (multiColor == TRUE) {
		generateColor(colorReceive);
	}
	else {
		//always solid white.
		colorReceive[0] = 255;
		colorReceive[1] = 255;
		colorReceive[2] = 255;
	}



	dl->radius = randRad;
	dl->color.r = colorReceive[0];
	dl->color.g = colorReceive[1];
	dl->color.b = colorReceive[2];

}




void generateFreakyLaser(const Vector& arg_origin) {
	//BEAM		*( *R_BeamPoints )				( float * start, float * end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );
	//BEAM		*( *R_BeamRing )				( int startEnt, int endEnt, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );

	const Vector& origin = arg_origin;

	int m_iLaserSprite = 0;

	//might be wiser to do this in some "init" function clientside-wide instead?  Same for other "EV_FindModelIndex" calls?
	m_iLaserSprite = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");

	float randOrigin1[3];
	float randOrigin2[3];

	//range??
	float mag = randomValue(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserLength), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserLength));

	float fltDeg = randomValue(0, M_2PI);
	if (fltDeg >= M_2PI) {
		fltDeg -= M_2PI;
	}

	float x = cos(fltDeg) * mag;
	float y = sin(fltDeg) * mag;


	fltDeg += M_180_RAD;

	if (fltDeg >= M_2PI) {
		fltDeg -= M_2PI;
	}

	//float fltMag = sqrt( pow(x, 2) + pow(y, 2) + pow(z, 2) );

	float x2 = cos(fltDeg) * mag;
	float y2 = sin(fltDeg) * mag;

	float randomShiftX = randomAbsoluteValue(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax));
	float randomShiftY = randomAbsoluteValue(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMin), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistHoriMax));
	float randomShiftZ = randomAbsoluteValue(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax));
	float randomShiftZ2 = randomAbsoluteValue(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMin), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserSpawnDistVertMax));

	randOrigin1[0] = origin[0] + x + randomShiftX;
	randOrigin1[1] = origin[1] + y + randomShiftY;
	randOrigin1[2] = origin[2] + randomShiftZ;

	randOrigin2[0] = origin[0] + -x + randomShiftX;
	randOrigin2[1] = origin[1] + -y + randomShiftY;
	//test with the same "randomShiftZ" once?
	randOrigin2[2] = origin[2] + randomShiftZ2;


	float randLife = gEngfuncs.pfnRandomFloat(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserDurationMin), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserDurationMax));
	float randWidth = gEngfuncs.pfnRandomFloat(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserThicknessMin), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserThicknessMax));

	float randAmp = gEngfuncs.pfnRandomFloat(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserNoiseMin), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserNoiseMax));

	// and don't multiply brightness by 255!  Not necessary, just has to be divided out in the color to get the same point across
	float randBrightness = gEngfuncs.pfnRandomFloat(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMin), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserBrightnessMax));
	//INTERPRET FROM 0 - 1, like  0.8 - 1.0!
	float frameRate = gEngfuncs.pfnRandomFloat(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMin), EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserFrameRateMax));

	float speed = 1;
	float startFrame = 0;



	int colorReceive[3];

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserMultiColor) != 0) {
		generateColor(colorReceive);
	}
	else {
		//always solid white.
		colorReceive[0] = 255;
		colorReceive[1] = 255;
		colorReceive[2] = 255;
	}




	//(float*)Vector(randx1, randy1, randz1)
	BEAM* someBeam = gEngfuncs.pEfxAPI->R_BeamPoints(randOrigin1, randOrigin2, m_iLaserSprite, randLife, randWidth, randAmp, randBrightness, speed, startFrame, frameRate, 0, 0, 0);


	//short someShort = gEngfuncs.pEfxAPI->R_LookupColor(colorReceive[0], colorReceive[1], colorReceive[2]);
	//easyPrintLine("HOWWWWWWWW %d:::<-- %d %d %d", someShort, colorReceive[0], colorReceive[1], colorReceive[2]);
	//what is this??


	//For unknown reasons, the color arguments, the 3 last args above, aren't working as expected.  Below seems to work though.  
	//Just check to see whether the created beam is null or not (may fail to create on too many requests or... who knows why else).
	//easyPrintLine("IS IT NULLLLL %d", (someBeam == NULL));
	if (someBeam == NULL) {

	}
	else {
		// divided by 65025.0f (this is 256^2).  Why?  Because anything above 1/256^2 distorts colors, making even (255, 12, 0) show up as bright yellow.
		// Anything less than 1/256^2 is treated as getting closer to black, which makes it more transparent up to invisible at 0.
		// NOPE, that was from having a brightness from 0 to 256 instead of 0 to 1.
		// 65025.0f ...  nope.
		someBeam->r = colorReceive[0] / 255.0f;
		someBeam->g = colorReceive[1] / 255.0f;
		someBeam->b = colorReceive[2] / 255.0f;
	}


}





void EV_FreakyLight(event_args_t* args) {
	int i = 0;

	//this is handled in method "UpdateClientData" of file "hud_update.cpp" instead.
	//updateCVarRefsClient();




	generateFreakyLight((float*)&args->origin);

	if (EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserEnabled) == 1) {

		//if there is a decimal in raveLaserSpawnFreq, guarantee spawning of the first whole number lasers (like in 2.4, spawn 2 lasers).
		//Treat the 0.4 decimal as a "40%" chance of producing another laser.  Rounds out nice and evenly when it happens a lot.
		int toGen = (int)EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq);   //truncate.
		//roundoff?
		float roundoff = ((int)(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserSpawnFreq) * 10) % 10) / 10.0f;
		if (roundoff != 0 && gEngfuncs.pfnRandomLong(0, 1) <= roundoff) {
			//extra.
			toGen++;
		}
		for (i = 0; i < toGen; i++) {
			generateFreakyLaser((float*)&args->origin);
		}
	}//END OF if(EASY_CVAR_GET_CLIENTONLY_DEBUGONLY(raveLaserEnabled) == 1)

}






void EV_FriendlyVomit(event_args_t* args) {
	int duckyou = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow.spr");

	int m_iHotglowGreen;
	m_iHotglowGreen = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow_green.spr");


	vec3_t origin;
	vec3_t ang;
	//vec3_origin
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, ang);

	int ballsToSpawn = args->iparam1;

	int balls = ballsToSpawn;
	for (int i = 0; i < balls; i++) {
		createBallVomit(&m_iHotglowGreen, &origin, &ang);
	}

}//END OF EV_FriendlyVomit



//MODDD - TODO. should behavior varry from being destroyed mid-fall or while on the ground?  such as gibbed midair or auto-exploded?
void EV_FloaterExplode(event_args_t* args) {
	int duckyou = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow.spr");
	int m_iHotglowGreen;
	m_iHotglowGreen = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow_green.spr");


	vec3_t origin;
	vec3_t ang;
	//vec3_origin
	VectorCopy_f(args->origin, origin);
	VectorCopy_f(args->angles, ang);

	int ballsToSpawn = 8;

	createBigBallFloaterGas(&m_iHotglowGreen, origin + Vector(0, 0, 20));


	int balls = ballsToSpawn;
	for (int i = 0; i < balls; i++) {
		createBallFloaterGas(&m_iHotglowGreen, origin + Vector(0, 0, 12));
	}

}//END OF EV_FloaterExplode








void shrapnelHitCallback(struct tempent_s* ent, struct pmtrace_s* ptr) {
	dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
	VectorCopy_f(ent->entity.origin, dl->origin);

	dl->dark = FALSE;
	//time of "0.01" or "0.001"?
	//dl->die = gEngfuncs.GetClientTime() + 0.001; //Kill it right away
	dl->die = gEngfuncs.GetClientTime() + 0.03; //Kill it right away

	dl->radius = 80;
	dl->color.r = 255;
	dl->color.g = 120;
	dl->color.b = 0;

}

void createShrapnel(int* sprite, Vector* loc, int testArg, float testArg2, float testArg3) {
	float randomStrength = 190; //121;
	float heightExtra = 140;

	randomStrength = testArg2;
	heightExtra = testArg3;

	/*
	float randx = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randy = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	float randz = gEngfuncs.pfnRandomFloat(-randomStrength, randomStrength);
	*/
	float randx = gEngfuncs.pfnRandomFloat(20, randomStrength);
	float randy = gEngfuncs.pfnRandomFloat(20, randomStrength);
	float randz = gEngfuncs.pfnRandomFloat(45, randomStrength + heightExtra);

	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randx *= -1;
	}
	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randy *= -1;
	}

	if (gEngfuncs.pfnRandomLong(0, 1) == 0) {
		randz *= -1;
	}



	vec3_t rot = Vector(randx, randy, randz);


	float lifeValue = gEngfuncs.pfnRandomFloat(2.25, 3.55);


	int flags = 0;
	if (testArg == 1) {
		//no flash on hit.
		flags = BREAK_METAL | FTENT_SMOKETRAIL | FTENT_COLLIDEWORLD | FTENT_FADEOUT | FTENT_ROTATE;;
	}
	else {
		//flash on hit.
		flags = BREAK_METAL | FTENT_SMOKETRAIL | FTENT_COLLIDEWORLD | FTENT_FADEOUT | FTENT_ROTATE | FTENT_FLASHONHIT;
		//flags = BREAK_METAL | FTENT_SMOKETRAIL | FTENT_COLLIDEWORLD | FTENT_FADEOUT | FTENT_ROTATE | FTENT_FLICKER;
	}

	TEMPENTITY* eh = gEngfuncs.pEfxAPI->R_TempSprite(*loc, rot, 2.5f, *sprite, kRenderNormal, kRenderFxNone, 255.0 / 255.0, lifeValue, flags);
	if (eh != NULL) {
		eh->fadeSpeed = 0.5f;

		//gives this the metal hit sound effects.  I guess it is hardcoded what values of "hitSound" make what sounds, can't find the resulting sound calls for that anywhere.
		//Closest is entity.cpp, where this is referred to but still not used directly.
		//Calls a method that is pre-compiled in this line:
		//    Callback_TempEntPlaySound(pTemp, damp);
		//...assume it is what refers to this.


		//eh->hitSound = 2;    hit sound disabled.  Enabling this gives it the break metal sounds though.
		eh->hitcallback = &shrapnelHitCallback;

		//no "BREAK_METAL " here.
		//eh->flags |= (FTENT_SLOWGRAVITY  | FTENT_SMOKETRAIL );


		eh->entity.baseline.angles[0] = gEngfuncs.pfnRandomFloat(-256, 256);
		eh->entity.baseline.angles[1] = gEngfuncs.pfnRandomFloat(-256, 256);
		eh->entity.baseline.angles[2] = gEngfuncs.pfnRandomFloat(-256, 256);

		//easyPrintLine("??? %.2f", eh->bounceFactor);
		//default bounce factor is 1.0.
	}

}



void EV_QuakeExplosionEffect(event_args_t* args) {
	Vector origin = args->origin;
	int shrapnel3D = gEngfuncs.pEventAPI->EV_FindModelIndex("models/shrapnel.mdl");

	//TEST IN SESSION!

	//R_Explosion

	////void( *R_Explosion )( float *pos, int model, float scale, float framerate, int flags );
	//gEngfuncs.pEfxAPI->R_Explosion( args->origin, m_iBalls, 0.3, 15, TE_EXPLFLAG_NONE );


	//void	( *R_BreakModel )				( float *pos, float *size, float *dir, float random, float life, int count, int modelIndex, char flags );
	//!!!!!!!!!!!!!!!!!!!!!!!!!

	int testArg = args->iparam1;
	float testArg2 = args->fparam1;
	float testArg3 = args->fparam2;

	//int shrapz = 14;
	int shrapz = args->iparam2;

	if (testArg == 0) {
		//"FTENT_FLASHONHIT" doesn't seem to do anything for whatever reason.
		gEngfuncs.pEfxAPI->R_BreakModel(args->origin, Vector(0, 0, 0), Vector(0, 0, 100), 130, 2.5, shrapz, shrapnel3D, BREAK_METAL | FTENT_SMOKETRAIL); //BREAK_METAL | FTENT_GRAVITY | FTENT_COLLIDEWORLD  | FTENT_SMOKETRAIL );
	}
	else {


		dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);

		if (dl != NULL) {
			VectorCopy_f(args->origin, dl->origin);

			dl->radius = 130;
			dl->dark = FALSE;
			dl->die = gEngfuncs.GetClientTime() + 0.03; //Kill it right away

			dl->color.r = 255;
			dl->color.g = 255;
			dl->color.b = 255;
		}


		for (int i = 0; i < shrapz; i++) {
			createShrapnel(&shrapnel3D, &origin, testArg, testArg2, testArg3);
		}
	}


}





//EASY_CVAR_EXTERN_DEBUGONLY???(sparkBallAmmountMulti)
//float global_sparkBallAmmountMulti = 1;
//No, use it to influence the parameter.

void EV_ShowBalls(event_args_t* args)
{

	int ballsToSpawn = args->iparam1;

	/*
	if(global_sparkBallAmmountMulti != 1){
		//multiplying by 1 is useless, so don't if it is.
		ballsToSpawn =  (int) ((float)ballsToSpawn * global_sparkBallAmmountMulti);
	}
	*/
	//...not working out, just trust this CVar affected "args->iparam1".
	//Check for 0 balls, however.

	if (ballsToSpawn == 0) {
		return;  //nothing to do here
	}


	//pmtrace_t tr;
	int m_iBeam, m_iGlow, m_iBalls;
	m_iBalls = m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow.spr");


	vec3_t origin;
	//vec3_origin
	VectorCopy_f(args->origin, origin);


	//vec3_t fwd;
	//VectorAdd_f( tr.endpos, tr.plane.normal, fwd );


	//gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 200 );

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, origin, origin, m_iBalls, 8, 0.1f, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 40 );
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//...( float *pos, float speed, float life, int count, int modelIndex );
	//gEngfuncs.pEfxAPI->R_TempSphereModel(origin, 20, 30, 8, m_iBalls);

	//void	( *R_Sprite_Spray )				( float * pos, float * dir, int modelIndex, int count, int speed, int iRand );


	//try render modes 3 & 5?
	//gEngfuncs.pEfxAPI->R_Spray(origin, rot, m_iBalls, 8, 10, 3, 3);
	//

	int balls = ballsToSpawn;
	for (int i = 0; i < balls; i++) {
		createBall(&m_iBalls, &origin);
	}

	//eh->die = 1;
	//easyPrintLine("??? %.2f", eh->fadeSpeed);

	//eh->entity.dir = 7;
	//eh->entity.curstate.velocity.y = 7;
	//eh->entity.curstate.velocity.z = 7;

	//gEngfuncs.pEfxAPI->R_SparkShower( tr.endpos );

	//gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 200 );

	//int type, float * start, float * end, int modelIndex, int count, float life, float size, float amplitude, int renderamt, float speed 
}


//copy to show that agrunts are powered up. Periodically emits, floats, and fades.
void EV_ShowBallsPowerup(event_args_t* args)
{
	/*
	if(CVAR_GET_FLOAT("sparkMod")){

	}
	*/

	int ballsToSpawn = args->iparam1;

	/*
	if(global_sparkBallAmmountMulti != 1){
		//multiplying by 1 is useless, so don't if it is.
		ballsToSpawn =  (int) ((float)ballsToSpawn * global_sparkBallAmmountMulti);
	}
	*/
	//...not working out, just trust this CVar affected "args->iparam1".
	//Check for 0 balls, however.

	if (ballsToSpawn == 0) {
		return;  //nothing to do here
	}


	//pmtrace_t tr;
	int m_iBeam, m_iGlow, m_iBalls;
	m_iBalls = m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/hotglow.spr");


	vec3_t origin;
	//vec3_origin
	VectorCopy_f(args->origin, origin);


	//vec3_t fwd;
	//VectorAdd_f( tr.endpos, tr.plane.normal, fwd );


	//gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 200 );

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, origin, origin, m_iBalls, 8, 0.1f, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 40 );
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//...( float *pos, float speed, float life, int count, int modelIndex );
	//gEngfuncs.pEfxAPI->R_TempSphereModel(origin, 20, 30, 8, m_iBalls);

	//void	( *R_Sprite_Spray )				( float * pos, float * dir, int modelIndex, int count, int speed, int iRand );


	//try render modes 3 & 5?
	//gEngfuncs.pEfxAPI->R_Spray(origin, rot, m_iBalls, 8, 10, 3, 3);
	//

	int balls = ballsToSpawn;
	for (int i = 0; i < balls; i++) {
		createBallPowerup(&m_iBalls, &origin);
	}

	//eh->die = 1;
	//easyPrintLine("??? %.2f", eh->fadeSpeed);

	//eh->entity.dir = 7;
	//eh->entity.curstate.velocity.y = 7;
	//eh->entity.curstate.velocity.z = 7;

	//gEngfuncs.pEfxAPI->R_SparkShower( tr.endpos );

	//gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 200 );

	//int type, float * start, float * end, int modelIndex, int count, float life, float size, float amplitude, int renderamt, float speed 
}

void EV_HLDM_DecalGunshotCustomEvent(event_args_t* args)
{
	vec3_t origin;
	//vec3_origin
	VectorCopy_f(args->origin, origin);

	//EV_HLDM_FireBullets( idx, forward, right, up, 1, vecSrc, vecAiming, 8192, BULLET_PLAYER_9MM, 0, 0, args->fparam1, args->fparam2 );
	//void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int *tracerCountChoice, float flSpreadX, float flSpreadY )

	//pmtrace_t tr;
	//gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, FILLIN_TRACEFLAGS_STUDIO_BOX, -1, &tr );


//	EV_HLDM_DecalGunshot( origin, BULLET_PLAYER_9MM );

}

















//If this is called, make sure a "log" folder exists under the same folder as hl.exe, NOT the mod folder (Absolute Zero)!
void writeColorPickerChoices(void) {
	int r, g, b;
	FILE* myFile;

	myFile = fopen("log/r.txt", "w");  //"a+t");
	if (myFile) {
		for (int i = 0; i <= 255; i++) {
			r = i;
			g = 0;
			b = 0;
			fprintf(myFile, "%3d, %3d, %3d :::%3d\n", r, g, b, gEngfuncs.pEfxAPI->R_LookupColor(r, g, b));
		}
		fclose(myFile);
	}
	myFile = fopen("log/g.txt", "w");  //"a+t");
	if (myFile) {
		for (int i = 0; i <= 255; i++) {
			r = 0;
			g = i;
			b = 0;
			fprintf(myFile, "%3d, %3d, %3d :::%3d\n", r, g, b, gEngfuncs.pEfxAPI->R_LookupColor(r, g, b));
		}
		fclose(myFile);
	}
	myFile = fopen("log/b.txt", "w");  //"a+t");
	if (myFile) {
		for (int i = 0; i <= 255; i++) {
			r = 0;
			g = 0;
			b = i;
			fprintf(myFile, "%3d, %3d, %3d :::%3d\n", r, g, b, gEngfuncs.pEfxAPI->R_LookupColor(r, g, b));
		}
		fclose(myFile);
	}
}//END OF wirteColorPickerChoices












//0.27f;
#define TEST_PARTICLE_LIFE 0.18f;
//0.0167f;
#define TEST_PARTICLE_SPAWN_CYCLE_TIME 0.0f;

//MODDD - methods slightly modified from entity.cpp, found commented out.
void TEST_ParticleCallback(struct particle_s* particle, float frametime)
{
	int i;



	const float timeStart = particle->die - TEST_PARTICLE_LIFE;
	const float timeSinceStart = gEngfuncs.GetClientTime() - timeStart;


	int r;
	int g;
	int b;

	b = 0;

	//warp from that bright yellow (255, 255, 0) to a smokey gray (121, 121, 121);

	//how far along am I?
	const float timeFract = timeSinceStart / TEST_PARTICLE_LIFE;

	/*
	r = 255 + (121 - 255)*timeFract;
	g = 255 + (121 - 255)*timeFract;
	b = 0;
	*/



	/*

	r = 255 + (0 - 255)*timeFract;
	//g = 255 + (0 - 255)*timeFract;
	g = 255 + (0 - 255)*timeFract;
	b = 0;
	*/

	//Unreliable, ugh.
	//Just look at the palette itself and see if there's a row that looks good.
	//Indexes work as left to right, top to bottom. 16 per row, starting at 0.  So if the count goes over 16 (like 16 itself including #0),
	//that picks the next row's first color. Up to the last #255.
	//particle->color = 	gEngfuncs.pEfxAPI->R_LookupColor( r, g, b );

	particle->color = 96 + (int)ceil(15.0f * (1.0f - timeFract));



	//

	/*
	r = 255;
	g = 255;
	b = 0;


	p->color = 	gEngfuncs.pEfxAPI->R_LookupColor( r, g, b );
	//What is the point of R_GetPackedColor?? Just setting p->color above seems to work alone?
	gEngfuncs.pEfxAPI->R_GetPackedColor( &p->packedColor, p->color );
	*/



	for (i = 0; i < 3; i++)
	{
		particle->org[i] += particle->vel[i] * frametime;
	}
}




//cvar_t *color = NULL;
void TEST_Particles(const Vector& v_origin, const Vector& v_velocity)
{
	//static float lasttime;
	//float curtime;

	//curtime = gEngfuncs.GetClientTime();

	//if ( ( curtime - lasttime ) < 2.0 )
	//	return;

	//lasttime = curtime;

	// Create a few particles


	if (v_velocity.Length() > 0) {
		particle_t* p;
		int i, j;
		//const int numParticles = 26;
		//const int numParticles = 32;
		const int numParticles = 24;

		//LETS TEST.
		Vector forward = v_velocity.Normalize();
		Vector right;
		Vector up;

		if (forward == Vector(0, 0, 1)) {
			//matches the up vector exactly?  A cross product with 0, 0, 1 would not work then.  It would be (0, 0, 0), or no direction at all.
			//Just use fixed vectors for the rest for this special case.

			//My "right" can be one way floor-wise, my "up" is another floor-wise. X and Y respectively, either order.
			right = Vector(1, 0, 0);
			up = Vector(0, 1, 0);

		}
		else {
			//Determine what the right and up vectors are. This helps with the trigonometry to place points around me in a circle, equidistantly spaced in degrees.

			//matrixMult(forward, matrix_rot_z, right);
			//matrixMult(right, matrix_rot_y, right);

			right = CrossProduct(forward, Vector(0, 0, 1));

			//Yes, this produces a good relative up vector. It does feel just diabolical.
			up = CrossProduct(right, forward);

			//Normalize both.
			right = right.Normalize();
			up = up.Normalize();

		}

		//At this point it is established what vector goes forward, up, and right from that.  Up and right and all possible combinations of each
		//in a circular fashion form a plane for drawing circles around. I'm sure that makes perfect sense.



		for (i = 0; i < numParticles; i++) {
			int r, g, b;
			p = gEngfuncs.pEfxAPI->R_AllocParticle(TEST_ParticleCallback);
			if (!p)
				break;




			//for ( j = 0; j < 3; j++ )
			//{
				//p->org[ j ] = v_origin[ j ] + gEngfuncs.pfnRandomFloat( -32.0, 32.0 );;
				//p->vel[ j ] = gEngfuncs.pfnRandomFloat( -100.0, 100.0 );
			//}

			//First of all, get whatever portion around the circle this is.


			//Each 1/#'th will be covered.
			//For instance, if numParticles were 12, it would be 1/12'th.  so 0/12, 1/12, 2/12, 3/12, ... 9/12, 10/12, 11/12.
			//Exclude the last whole one (12/12) since that wraps around to the start of the circle, redundant with 0/12.
			float circleRad = 2.0f * M_PI * (((float)i) / ((float)numParticles));

			float circle_x = cos(circleRad);
			float circle_y = sin(circleRad);





			//p->org = v_origin + (circle_x * v_right) * 3 + (circle_y * v_up) * 3;
			//p->vel = (circle_x * v_right) + (circle_y * v_up);
			p->org = v_origin + (circle_x * right) * 2 + (circle_y * up) * 2;

			p->vel = (circle_x * right) * 68 + (circle_y * up) * 68;
			//p->vel = Vector(0, 0, 0);


			//2, 1, 3
			//

	/*
	  2,  3, -1    A
	x 0,  0,  1
	= 3, -2,  0    B

	   3, -2,  0    B
	x  2,  3, -1    A
	=  2,  3, 13    C
	*/


	//p->org = v_origin;
	//p->vel = Vector(1, 1, 1);
	//CrossProduct(v_velocity, Vector(0, 0, 1));


			r = 255;
			g = 255;  //why is green so gray looking??
			b = 0;

			//wtf?
			/*
			someBeam->r = colorReceive[0]/65025.0f;
			someBeam->g = colorReceive[1]/65025.0f;
			someBeam->b = colorReceive[2]/65025.0f;
			*/

			//p->color = 	gEngfuncs.pEfxAPI->R_LookupColor( r, g, b );
			p->color = 96 + 15;

			//What is the point of R_GetPackedColor?? Just setting p->color above seems to work alone?
			gEngfuncs.pEfxAPI->R_GetPackedColor(&p->packedColor, p->color);


			// p->die is set to current time so all you have to do is add an additional time to it
			//p->die += 1.5;
			//nah...
			//p->die = gEngfuncs.GetClientTime() + 0.3;
			p->die += TEST_PARTICLE_LIFE;


		}//END OF for each particle to spawn


	}//END OF velocity length check




}//END OF TEST_Particles



void EV_imitation7_think(struct tempent_s* ent, float frametime, float currenttime)
{
	if (currenttime < ent->entity.baseline.fuser1)
		return;


	if (ent->entity.origin == ent->entity.attachment[0]) {
		//HAVE LENIENCY.  Wait for this to be frozen for 0.15 seconds, sheesh.
		if (ent->entity.baseline.fuser2 != -1) {
			if (ent->entity.baseline.fuser2 < gEngfuncs.GetClientTime()) {
				ent->die = gEngfuncs.GetClientTime();
			}
		}
		else {
			//countdown...
			ent->entity.baseline.fuser2 = gEngfuncs.GetClientTime() + 0.3f;
		}

	}
	else {
		ent->entity.baseline.fuser2 = -1;  //not dying now.
		VectorCopy_f(ent->entity.origin, ent->entity.attachment[0]);
	}



	//gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage * n / 255.0, flDamage * n * 0.5 * 0.1, FTENT_FADEOUT );

				//vec3_t fwd;
				//VectorAdd_f( tr.endpos, tr.plane.normal, fwd );

				//gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 100 );

	/*
	int eckz = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/explode1.spr" );

	//not sure if "life" is necessary, seems to expire at the end of the last frame without any looping and/or cycle tags.  (framerate is 10, I assume,  10 frames per second:  9 frames, so  9 / 10 = 0.9 seconds to finish the anim.
	TEMPENTITY* eh = gEngfuncs.pEfxAPI->R_TempSprite( ent->entity.origin, vec3_origin, EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale), eckz, kRenderGlow, kRenderFxNoDissipation, 250.0 / 255.0, 0.91f, FTENT_SPRANIMATE );
	//eh->fadeSpeed = 3.3f;  ???
	//eh->entity.curstate.scale = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale);
	*/


	//Instead of the little explosion puffs, radiate particles.

	vec3_t vecSrc, angles, forward, right, up;

	//I... guess this is velocity.
	vec3_t velo = ent->entity.origin - ent->entity.prevstate.origin;

	VectorCopy_f(ent->entity.origin, vecSrc);

	/*
	//VectorCopy_f( ent->entity.angles, angles );  //or "ent->entity.baseline.angles" ?
	VectorCopy_f( ent->entity.baseline.angles, angles );
	AngleVectors ( angles, forward, right, up );
	*/

	TEST_Particles(vecSrc, velo);




	//TEST - real slow for now!
	ent->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + TEST_PARTICLE_SPAWN_CYCLE_TIME;
	//ent->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 1.0f;
}//END OF EV_imitation7_think


void EV_imitation7(event_args_t* args) {
	int iEntIndex = args->iparam1;
	TEMPENTITY* pTrailSpawner = NULL;


	pTrailSpawner = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel(args->origin);


	if (pTrailSpawner != NULL) {
		pTrailSpawner->entity.baseline.fuser2 = -1; //if that is fine?
		//pTrailSpawner->flags |= ( FTENT_PLYRATTACHMENT );
		pTrailSpawner->flags |= (FTENT_PLYRATTACHMENT | FTENT_COLLIDEKILL | FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD);

		pTrailSpawner->clientIndex = iEntIndex;
		pTrailSpawner->callback = &EV_imitation7_think;

		//pTrailSpawner->entity.baseline.sequence = args->iparam2 + 68;



		pTrailSpawner->die = gEngfuncs.GetClientTime() + 10; // Just in case

		//When is my first think cycle?
		pTrailSpawner->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 0.02f;
	}


}//END OF EV_imitation7







//MODDD - new, testing some DMC - quake stuff.

void EV_RocketTrailCallback(struct tempent_s* ent, float frametime, float currenttime)
{
	if (currenttime < ent->entity.baseline.fuser1)
		return;

	/*
	if ( ent->entity.origin == ent->entity.attachment[0] )
		ent->die = gEngfuncs.GetClientTime();
	else
		VectorCopy_f ( ent->entity.origin, ent->entity.attachment[0] );
	*/


	if (ent->entity.origin == ent->entity.attachment[0]) {
		//HAVE LENIENCY.  Wait for this to be frozen for 0.15 seconds, sheesh.
		if (ent->entity.baseline.fuser2 != -1) {
			if (ent->entity.baseline.fuser2 < gEngfuncs.GetClientTime()) {
				ent->die = gEngfuncs.GetClientTime();
			}
		}
		else {
			//countdown...
			ent->entity.baseline.fuser2 = gEngfuncs.GetClientTime() + 0.15;
		}

	}
	else {
		ent->entity.baseline.fuser2 = -1;  //not dying now.
		VectorCopy_f(ent->entity.origin, ent->entity.attachment[0]);
	}


	//Not set up to re-think. Or is that automatic..?
	//that is set ent->entity.baseline.fuser1 to some GetClientTime() + 0.03 or something.

	/*
	//Make the Rocket light up. ( And only rockets, no Grenades ).
	if ( ent->entity.baseline.sequence == 70 )
	{
		dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight ( 0 );
		VectorCopy_f ( ent->entity.origin, dl->origin );

		dl->radius = 160;
		dl->dark = TRUE;
		dl->die = gEngfuncs.GetClientTime() + 0.001; //Kill it right away

		dl->color.r = 255;
		dl->color.g = 255;
		dl->color.b = 255;
	}
	*/
}


// trail stuff.

#define GRENADE_TRAIL 1
#define ROCKET_TRAIL 2

void EV_Trail_EngineChoice_Think(struct tempent_s* ent, float frametime, float currenttime) {

	if (currenttime < ent->entity.baseline.fuser1)
		return;

	if (ent->entity.origin == ent->entity.attachment[0]) {
		//HAVE LENIENCY.  Wait for this to be frozen for 0.15 seconds, sheesh.
		if (ent->entity.baseline.fuser2 != -1) {
			if (ent->entity.baseline.fuser2 < gEngfuncs.GetClientTime()) {
				ent->die = gEngfuncs.GetClientTime();
			}
		}
		else {
			//countdown...
			ent->entity.baseline.fuser2 = gEngfuncs.GetClientTime() + 0.3f;
		}

	}
	else {
		ent->entity.baseline.fuser2 = -1;  //not dying now.
		VectorCopy_f(ent->entity.origin, ent->entity.attachment[0]);
	}



	gEngfuncs.pEfxAPI->R_RocketTrail(ent->entity.prevstate.origin, ent->entity.origin, (int)ent->entity.baseline.iuser1);


	//TEST - real slow for now!
	ent->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 0;//+ 0.0167;
	//ent->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 1.0f;



}//END OF EV_imitation7



//Similar to EV_Trail but pick what trail # to use from the engine (quake particles?) for choices 0 to 7 inclusive in iParam2 (the second whole number one, typically after giving entity index)
void EV_Trail_EngineChoice(event_args_t* args) {

	int iEntIndex = args->iparam1;
	TEMPENTITY* pTrailSpawner = NULL;


	pTrailSpawner = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel(args->origin);



	if (pTrailSpawner != NULL)
	{
		pTrailSpawner->entity.baseline.fuser2 = -1; //if that is fine?

		//pTrailSpawner->flags |= ( FTENT_PLYRATTACHMENT | FTENT_COLLIDEKILL | FTENT_CLIENTCUSTOM | FTENT_SMOKETRAIL | FTENT_COLLIDEWORLD );
		pTrailSpawner->flags |= (FTENT_SPIRAL | FTENT_PLYRATTACHMENT | FTENT_COLLIDEKILL | FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD);

		if (args->iparam2 == 6) {
			//orange trail.
			pTrailSpawner->flags |= FTENT_ORANGETRAIL;
		}
		else {
			//smoky?
			pTrailSpawner->flags |= FTENT_SMOKETRAIL;
		}

		pTrailSpawner->callback = &EV_Trail_EngineChoice_Think;
		pTrailSpawner->clientIndex = iEntIndex;

		//save me!
		pTrailSpawner->entity.baseline.iuser1 = args->iparam2;

		//pTrailSpawner->entity.baseline.sequence = args->iparam2 + 68;

		pTrailSpawner->die = gEngfuncs.GetClientTime() + 10; // Just in case
		pTrailSpawner->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 0; //+ 0.0167;
	}


}//END OF EV_Trail_EngineChoice





//NOTE - I am  trail.sc
//ALSO - yes, trailTypeTest may be sent to me as iParam2 but it clearly isn't having an influence here in that case.
//See entitiy.cpp's place where it checks for trailTypeTest.  That is much more inclusive of any partical effect ever.
void EV_Trail(event_args_t* args)
{


	int iEntIndex = args->iparam1;
	TEMPENTITY* pTrailSpawner = NULL;


	pTrailSpawner = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel(args->origin);


	//gEngfuncs.pEfxAPI->R_RocketTrail(0

	if (pTrailSpawner != NULL)
	{
		pTrailSpawner->entity.baseline.fuser2 = -1; //if that is fine?

		//pTrailSpawner->flags |= ( FTENT_PLYRATTACHMENT | FTENT_COLLIDEKILL | FTENT_CLIENTCUSTOM | FTENT_SMOKETRAIL | FTENT_COLLIDEWORLD );
		pTrailSpawner->flags |= (FTENT_SPIRAL | FTENT_PLYRATTACHMENT | FTENT_COLLIDEKILL | FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD);

		if (args->iparam2 == 6) {
			//orange trail.
			pTrailSpawner->flags |= FTENT_ORANGETRAIL;
		}
		else {
			//smoky?
			pTrailSpawner->flags |= FTENT_SMOKETRAIL;
		}

		pTrailSpawner->callback = &EV_RocketTrailCallback;
		pTrailSpawner->clientIndex = iEntIndex;



		/*
		//???
		if ( args->iparam2 == GRENADE_TRAIL )
			pTrailSpawner->entity.baseline.sequence = 69;
		else if ( args->iparam2 == ROCKET_TRAIL )
			pTrailSpawner->entity.baseline.sequence = 70;
			*/
			//pTrailSpawner->entity.baseline.sequence = args->iparam2 + 68;

		pTrailSpawner->die = gEngfuncs.GetClientTime() + 10; // Just in case
		pTrailSpawner->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + 0.5; // Don't try to die till 500ms ahead
	}


}








void EV_rocketAlphaTrailThink(struct tempent_s* ent, float frametime, float currenttime)
{
	if (currenttime < ent->entity.baseline.fuser1)
		return;


	if (ent->entity.origin == ent->entity.attachment[0]) {
		//HAVE LENIENCY.  Wait for this to be frozen for 0.15 seconds, sheesh.
		if (ent->entity.baseline.fuser2 != -1) {
			if (ent->entity.baseline.fuser2 < gEngfuncs.GetClientTime()) {
				ent->die = gEngfuncs.GetClientTime();
			}
		}
		else {
			//countdown...
			ent->entity.baseline.fuser2 = gEngfuncs.GetClientTime() + 0.15;
		}

	}
	else {
		ent->entity.baseline.fuser2 = -1;  //not dying now.
		VectorCopy_f(ent->entity.origin, ent->entity.attachment[0]);
	}


	//gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage * n / 255.0, flDamage * n * 0.5 * 0.1, FTENT_FADEOUT );

				//vec3_t fwd;
				//VectorAdd_f( tr.endpos, tr.plane.normal, fwd );

				//gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 100 );

	int eckz = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/explode1.spr");

	//not sure if "life" is necessary, seems to expire at the end of the last frame without any looping and/or cycle tags.  (framerate is 10, I assume,  10 frames per second:  9 frames, so  9 / 10 = 0.9 seconds to finish the anim.
	TEMPENTITY* eh = gEngfuncs.pEfxAPI->R_TempSprite(ent->entity.origin, vec3_origin, EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale), eckz, kRenderGlow, kRenderFxNoDissipation, 250.0 / 255.0, 0.91f, FTENT_SPRANIMATE);
	if (eh != NULL) {
		//eh->fadeSpeed = 3.3f;  ???
		//eh->entity.curstate.scale = EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaScale);
	}

	ent->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval);
}



//NOTE - I am  trailra.sc
void EV_rocketAlphaTrail(event_args_t* args)
{


	//gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/hotglow.spr" );
	//TEMPENTITY* eh = gEngfuncs.pEfxAPI->R_TempSprite( *loc, rot, 0.12f, *sprite, kRenderGlow, kRenderFxNoDissipation, 250.0 / 255.0, 0.22f, FTENT_GRAVITY | FTENT_COLLIDEWORLD | FTENT_FADEOUT );


	int iEntIndex = args->iparam1;
	TEMPENTITY* pTrailSpawner = NULL;



	pTrailSpawner = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel(args->origin);


	//gEngfuncs.pEfxAPI->R_RocketTrail(0

	if (pTrailSpawner != NULL)
	{
		pTrailSpawner->entity.baseline.fuser2 = -1; //if that is fine?

		//pTrailSpawner->flags |= ( FTENT_PLYRATTACHMENT | FTENT_COLLIDEKILL | FTENT_CLIENTCUSTOM | FTENT_SMOKETRAIL | FTENT_COLLIDEWORLD );
		pTrailSpawner->flags |= (FTENT_PLYRATTACHMENT | FTENT_CLIENTCUSTOM);


		pTrailSpawner->callback = &EV_rocketAlphaTrailThink;
		pTrailSpawner->clientIndex = iEntIndex;


		//pTrailSpawner->entity.baseline.sequence = args->iparam2 + 68;



		pTrailSpawner->die = gEngfuncs.GetClientTime() + 10; // Just in case

		pTrailSpawner->entity.baseline.fuser1 = gEngfuncs.GetClientTime() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(rocketTrailAlphaInterval); // Don't try to die till 500ms ahead
	}

}









//MODDDMIRROR
//======================
//	   MIRROR START
//======================
void EV_Mirror(event_args_t* args)
{
	//MODDD - iterator declared at the top now because VS6 be wack, yo.
	// (declarations inside for-loops collide with declarations elsewhere.)
	int ic;
	vec3_t org_target;

	//BOOL bNew = TRUE;
	//MODDD - using this instead.  Idea is if it gets set while looking through existing mirrors,
	// force the stats from this call into that one.
	// Remaining -1 means a new mirror element has to be used for the stats.
	int mirrorTargetIndex = -1;


	VectorCopy_f(args->origin, org_target);

	//MODDD - changed what parameters went where.
	// Radius getting mapped to one of the "int" params when neither float param was used?
	//float dist = (float)args->iparam1;
	//int type = args->iparam2;
	float dist = (float)args->fparam1;
	int type = args->iparam1;


	int bEnabled = args->bparam1;

	//we have mirror
	//...thanks.  

	// See if the coords we're given are already taken by an existing mirror.
	// If no existing mirror with matching coords is found, bNew remains TRUE.

	// I... really don't get this.
	// Say EV_Mirror is called 4 times to create 4 different mirrors, say "m" is gHUD.Mirrors:
	// m[0].enabled = TRUE (A)
	// m[1].enabled = TRUE (B)
	// m[2].enabled = TRUE (C)
	// m[3].enabled = TRUE (D)
	// m[4].enabled = FALSE
	// ...
	// And so, numMirrors is 4 (indices 0 to 3 valid for checks).

	// Then, say we send a call for the mirror at [1] and disable it.
	// m[0].enabled = TRUE
	// m[1].enabled = FALSE
	// m[2].enabled = TRUE
	// m[3].enabled = TRUE
	// m[4].enabled = FALSE
	// by the as-is logic, this also reduces numMirrors by 1. It is now 3 (indices 0 to 2 are valid).

	// See the problem?  m[3] is still enabled, but not one of the indices checked by most other for-loops
	// (which go from 0 to numMirrors-1 ).
	// When numMirrors is reduced by 1 and nothing else, it is assumed the last valid index is no longer
	// wanted, but that just isn't the case.

	// ANOTHER ISSUE, maybe.   On finding an existing mirror, what if any other part of its member vars
	// (type and radius) changed?  On finding an existing mirror, the script that sends the new type and
	// dist (for radius) above never runs.  Just seems odd to me.

	// If the point is to forget about disabled mirrors (to stop enabling/disabling mirrors from reaching)
	// the max mirror count pointlessly fast),
	// all mirrors above the currently disabled mirror should be copied to the previous one.
	// In the above example,

	// m[0].enabled = TRUE (A)
	// m[1].enabled = FALSE (B)
	// m[2].enabled = TRUE (C)
	// m[3].enabled = TRUE (D)
	// m[4].enabled = FALSE

	//...becomes

	// m[0].enabled = TRUE (A)
	// m[1].enabled = TRUE (C)
	// m[2].enabled = TRUE (D)
	// m[3].enabled = TRUE
	// m[4].enabled = FALSE

	//...that is, disabling [1], labeled (B), pushes all mirrors after to cover that index.
	// [3] is now safe to ignore, as it's an old copy of (D) that can be overwritten the next
	// time a new mirror is needed (we don't care about memory not referred to or safe for 
	// writing to).

	// IF I made a colossal <doink> up on understanding the original further below, let me know, thanks.
	// Maybe I'm paranoid and none of this was necessary.

	// In any case...
	// Few edits to make all the stats sent over be copied over (otherwise why would they be sent),
	// and numMirrors is not affected by changing the 'enabled' status of existing mirrors.
	// Also, a compromise for the "running out of mirrors" idea.  If the max mirror count is reached, 
	// we'll go to the earliest mirror whose 'enabled' is false and overwrite that index.  If any are
	// not enabled, otherwise admit failure (no disabled spaces to use).



	if (gHUD.numMirrors > 0)
	{
		for (ic = 0; ic < MIRROR_MAX; ic++)
		{
			if (gHUD.Mirrors[ic].origin[0] == org_target[0] && gHUD.Mirrors[ic].origin[1] == org_target[1] && gHUD.Mirrors[ic].origin[2] == org_target[2])
			{
				/*
				//MODDD - disabling.  Check all mirrors ever recorded still even on disabling one.
				// Because disabling one says nothing about mirrors after its index.
				if (bEnabled && !gHUD.Mirrors[ic].enabled ){
					gHUD.numMirrors++;
				}else if (!bEnabled && gHUD.Mirrors[ic].enabled ){
					gHUD.numMirrors--;
				}
				*/

				//MODDD - why not take all besides just 'bEnabled'?
				mirrorTargetIndex = ic;
				break;
			}
		}
	}

	// MODDD NOTE - only happens on not finding an existing mirror (matching origin) to use earlier.
	// Also, if the mirrorTargetIndex was set earlier, that means we found an existing mirror to use
	// already (skip trying to find a new element).
	if (mirrorTargetIndex == -1)
	{
		//MODDD - seciton reworked a bit, compare with the original further down.
		if (gHUD.numMirrors < MIRROR_MAX) {
			// easy, we have an unused mirror to take: the one at numMirrors.
			// And bump up numMirrors.
			mirrorTargetIndex = gHUD.numMirrors;
			gHUD.numMirrors++;
		}
		else {
			// Find the first disabled mirror to replace then.
			for (ic = 0; ic < MIRROR_MAX; ic++)
			{
				if (!gHUD.Mirrors[ic].enabled) {
					// woopee, use this one.
					mirrorTargetIndex = ic;
					break;
				}
			}
		}
	}//END OF mirrorTargetIndex check

	if (mirrorTargetIndex != -1) {
		VectorCopy_f(org_target, gHUD.Mirrors[mirrorTargetIndex].origin);
		gHUD.Mirrors[mirrorTargetIndex].type = type;
		gHUD.Mirrors[mirrorTargetIndex].enabled = bEnabled;
		gHUD.Mirrors[mirrorTargetIndex].radius = dist;
	}
	else {
		// Couldn't find an existing mirror or a place for a disabled one to overwrite?
		// Admit failure.
		easyForcePrintLine("ERROR: Can't register mirror, maximum %d allowed and no disabled mirrors found to replace!", MIRROR_MAX);
	}


	//MODDD - original version here.
	/*
	BOOL bNew = TRUE;

	if (gHUD.numMirrors > 0)
	{
		for (ic = 0; ic < MIRROR_MAX; ic++)
		{
			if (gHUD.Mirrors[ic].origin[0] == org_target[0] && gHUD.Mirrors[ic].origin[1] == org_target[1] && gHUD.Mirrors[ic].origin[2] == org_target[2])
			{
				if (bEnabled && !gHUD.Mirrors[ic].enabled ){
					gHUD.numMirrors++;
				}else if (!bEnabled && gHUD.Mirrors[ic].enabled ){
					gHUD.numMirrors--;
				}

				gHUD.Mirrors[ic].enabled = bEnabled;
				bNew = FALSE;
				break;
			}
		}
	}

	if (bNew)
	{
		if (gHUD.numMirrors >= MIRROR_MAX){
			easyForcePrintLine("ERROR: Can't register mirror, maximum %d allowed!", MIRROR_MAX);
		}
		else{
			VectorCopy_f(org_target, gHUD.Mirrors[gHUD.numMirrors].origin);
			gHUD.Mirrors[gHUD.numMirrors].type = type;
			gHUD.Mirrors[gHUD.numMirrors].enabled = bEnabled;
			gHUD.Mirrors[gHUD.numMirrors].radius = dist;
			gHUD.numMirrors++;
		}
	}
	*/
}
//======================
//	   MIRROR END
//======================

