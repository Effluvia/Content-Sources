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

#include "extdll.h"
#include "crowbar.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"
#include "util_debugdraw.h"

//MODDD NOTE - the crowbar doesn't seem to make any TextureHit call clientside, only serverside.
//             Not necessarily an issue but just pointing that out.


EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteclip)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_infiniteammo)
//EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)
EASY_CVAR_EXTERN_DEBUGONLY(multiplayerCrowbarHitSoundMode)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds)
EASY_CVAR_EXTERN_DEBUGONLY(drawDebugCrowbar)


//MODDD - some things moved to the new crowbar.h.

LINK_ENTITY_TO_CLASS( weapon_crowbar, CCrowbar );



void CCrowbar::Spawn( )
{
	Precache( );
	m_iId = WEAPON_CROWBAR;
	SET_MODEL(ENT(pev), "models/w_crowbar.mdl");

	m_iClip = -1;

	FallInit();// get ready to fall down.
}


void CCrowbar::Precache( void )
{
	PRECACHE_MODEL("models/v_crowbar.mdl");
	PRECACHE_MODEL("models/w_crowbar.mdl");
	PRECACHE_MODEL("models/p_crowbar.mdl");
	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	precacheGunPickupSound();

	m_usCrowbar = PRECACHE_EVENT ( 1, "events/crowbar.sc" );
}

int CCrowbar::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_CROWBAR;
	p->iWeight = CROWBAR_WEIGHT;
	return 1;
}


BOOL CCrowbar::Deploy( )
{
	return DefaultDeploy( "models/v_crowbar.mdl", "models/p_crowbar.mdl", CROWBAR_DRAW, "crowbar", 0, 0, (13.0/24.0), -1 );
}

void CCrowbar::Holster( int skiplocal /* = 0 */ )
{
	//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	//SendWeaponAnim( CROWBAR_HOLSTER );

	DefaultHolster(CROWBAR_HOLSTER, skiplocal, 0, (13.0f/24.0f) );
}


#ifndef CLIENT_DLL
//MODDD - little issue here. Never bother to report whether we found ANY intersection.
// In such a case, the trace result is returned in the way it was found but there is no way to tell that this is due to failure.
// Checking to see if the trace changed at all seems hacky and may have a 1 in a billion chance of being wrong.
// Just return a Boolean: found a point at all so it can be trusted that the trace now holds that, or no suitable point was found.
// also, MODDD - BAD BAD BAD!  Assuming vecEnd from the existing trace is BAAAAAAAAD.  it could be centered on the origin (player's eyes)
// instead of knowing to go outward in the direction we're facing!
BOOL FindHullIntersection( const Vector &vecSrc, const Vector &vecEnd, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	BOOL success = FALSE;
	int i, j, k;
	float distance;
	float* minmaxs[2] = {mins, maxs};
	TraceResult tmpTrace;
	
	// HHHHIIIIIIIISSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
	//Vector		vecHullEnd = tr.vecEndPos;
	Vector vecHullEnd = vecEnd;

	// Renamed!  From vecEnd to vecEndTarget.
	Vector vecEndTarget;

	distance = 1e6f;


	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace );
	if ( tmpTrace.flFraction < 1.0 )
	{
		//alread hit a point? That must mean it's ok.
		tr = tmpTrace;
		return TRUE;
	}


	int debugOffIndex = 0;

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( k = 0; k < 2; k++ )
			{
				vecEndTarget.x = vecHullEnd.x + minmaxs[i][0];
				vecEndTarget.y = vecHullEnd.y + minmaxs[j][1];
				vecEndTarget.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEndTarget, dont_ignore_monsters, pEntity, &tmpTrace );

				//DebugLine_ClearAll();
				//DebugLine_Setup(debugOffIndex, vecSrc, (tr.vecEndPos + (tr.vecEndPos - vecSrc) * 4), tr.flFraction);
				//DebugLine_Setup(debugOffIndex+1, vecSrc, vecSrc + (vecEndTarget - vecSrc) * 2, tr.flFraction);
				if (EASY_CVAR_GET_DEBUGONLY(drawDebugCrowbar) == 1) {
					DebugLine_Setup(vecSrc, vecEndTarget, tr.flFraction);
				}
				//debugOffIndex += 2;

				if ( tmpTrace.flFraction < 1.0 )
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if ( thisDistance < distance )
					{
						success = TRUE; //found something.
						tr = tmpTrace;
						distance = thisDistance;
					}
					//easyForcePrintLine("FindHullIntersection: YAY!!!!!");
				}else{
					//easyForcePrintLine("FindHullIntersection: NO.");
				}
			}
		}
	}

	return success;
}//END OF FindHullIntersection
#endif


void CCrowbar::PrimaryAttack()
{
	if (! Swing( 1 ))
	{
		SetThink( &CCrowbar::SwingAgain );
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CCrowbar::Smack( )
{
	DecalGunshot( &m_trHit, BULLET_PLAYER_CROWBAR );
}


void CCrowbar::SwingAgain( void )
{
	Swing( 0 );
}



#ifndef CLIENT_DLL
// MODDD - clone created just for putting some printouts around.  This is otherwise useless.
Vector FireBulletsPlayerEh ( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y, z;

	entvars_t* otherPev = pevAttacker;

	//always send!
	//if ( pevAttacker == NULL )
	//	pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{
		//Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ) , -0.5, 0.5 );
		y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5, 0.5 );
		z = x * x + y * y;

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(otherPev)/*pentIgnore*/, &tr);
		
		//easyPrintLine("flFraction?", tr.flFraction); 
		// do damage, paint decals

		//easyPrintLine("IS IT NULL????? %d", (CBaseEntity::Instance(tr.pHit)  == NULL) );
		if(CBaseEntity::Instance(tr.pHit)  != NULL){
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

			
			//easyPrintLine("NAME:::%s", STRING(pEntity->pev->classname) );
		}

		if (tr.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

			pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_CLUB | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB) );
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );

			/*
			//easyPrintLine("PLAYER BULLET TYPE?! %d", iBulletType);
			if ( iDamage )
			{
				
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB) );
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );
				
			} 
			else switch(iBulletType)
			{
			default:
			case BULLET_PLAYER_9MM:	
				//easyPrintLine("BULLET TRACE?!");
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &tr, DMG_BULLET); 
				break;

			case BULLET_PLAYER_MP5:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMP5, vecDir, &tr, DMG_BULLET); 
				break;

			case BULLET_PLAYER_BUCKSHOT:	
				 // make distance based!
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgBuckshot, vecDir, &tr, DMG_BULLET); 
				break;
			
			case BULLET_PLAYER_357:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &tr, DMG_BULLET); 
				break;
				
			case BULLET_NONE: // FIX
				pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					UTIL_DecalTrace( &tr,    25   + RANDOM_LONG(0,2) ); // "25" was "DECAL_GLASSBREAK1"
				}
				break;
			}
			
			//MODDD
			if (pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				//easyPrintLine("WHAT IS THE THING I HIT %s", STRING(pEntity->pev->classname) );

			}else{

				//if ( FNullEnt(tr.pHit))
				{
					//IF REVERTING TO THIS, try to use global_muteRicochetSound instead!
					if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteRicochetSound) != 2){
						MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos );
							WRITE_BYTE( TE_GUNSHOT );
							WRITE_COORD( tr.vecEndPos.x );
							WRITE_COORD( tr.vecEndPos.y );
							WRITE_COORD( tr.vecEndPos.z );
						MESSAGE_END();
					}
					
				}
			}
			*/
		}//END OF if (tr.flFraction != 1.0)

		//easyPrintLine("NULL?? %d", FNullEnt(tr.pHit) );

		//COME BACK

		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0 );
	}
	ApplyMultiDamage(otherPev, pevAttacker);

	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}
#endif


int CCrowbar::Swing( int fFirst )
{
#ifndef CLIENT_DLL
	//if muteCrowbarSounds is 0 or 1, we didn't hute hit sounds.
	const BOOL canPlayHitSound = (EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds) != 2 && EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(muteCrowbarSounds) != 3);
#endif

	BOOL specialMiss = FALSE;
	int fDidHit = FALSE;

	TraceResult tr;

	UTIL_MakeVectors (m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition( );
	//MODDD - NOTICE!  Use a slightly longer trace for the initial straight-line trace.
	// That way, head-hits count at a slightly longer distance (having to re-do with the
	// TraceHull call does not pick up on hitgroups for dealing extra damage).
	Vector vecEndDIRECT = vecSrc + gpGlobals->v_forward * 52; //32;
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32; //32;

	UTIL_TraceLine( vecSrc, vecEndDIRECT, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	//MODDD - idle in one second from an attack.
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;


#ifndef CLIENT_DLL

	if (EASY_CVAR_GET_DEBUGONLY(drawDebugCrowbar) == 1) {
		DebugLine_ClearAll();
	}

	//easyForcePrintLine("-------------------------------------------------");
	//easyForcePrintLine("HERE HE GO A fl:%.2f ss:%d io:%d fas:%d hit:%s", tr.flFraction, tr.fStartSolid, tr.fInOpen, tr.fAllSolid, (tr.pHit!=NULL)?FClassname(CBaseEntity::Instance(tr.pHit)):"NULL");
	

	// It don't matter.  None of this matters.
	/*
	if ( tr.flFraction >= 1.0 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );
		if ( tr.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			
			//easyForcePrintLine("HERE HE GO B1 fl:%.2f ss:%d io:%d fas:%d hit:%s", tr.flFraction, tr.fStartSolid, tr.fInOpen, tr.fAllSolid, (tr.pHit!=NULL)?FClassname(CBaseEntity::Instance(tr.pHit)):"NULL");

			//DebugLine_Setup(0, Vector(tr.vecEndPos) + Vector(7, 0, 0), Vector(tr.vecEndPos) + Vector(-7, 0, 0), 255, 0, 0 );
			//DebugLine_Setup(1, Vector(tr.vecEndPos) + Vector(0, -7, 0), Vector(tr.vecEndPos) + Vector(0, 7, 0), 255, 0, 0 );

			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );

			BOOL temp_isBSPModel = FALSE;
			
			if (pHit != NULL) {
				temp_isBSPModel = pHit->IsBSPModel();
				
				//HACK - force the check below!  I want my hitgroup dammit
				//temp_isBSPModel = TRUE;
			}

			// was (!pHit or || temp_isBSPModel)
			if (!pHit || temp_isBSPModel){
				
				//if(pHit){
				//	easyForcePrintLine("!!!!!!!!!!!!!!!!!! %d", pHit->IsBSPModel() );
				//}
				BOOL hullSuccess = FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
				if(!hullSuccess){
					// If the HullIntersection failed to map a point, this is a failure to hit.
					// This can only happen for BSP Models, however.  Non-BSP models were still hit if the
					// UTIL_TraceHull check above passed (which to reach here it clearly did).
					specialMiss = TRUE;
				}
				else {

				}
			}
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)

			//easyForcePrintLine("HERE HE GO B2 fl:%.2f ss:%d io:%d fas:%d hit:%s", tr.flFraction, tr.fStartSolid, tr.fInOpen, tr.fAllSolid, (tr.pHit!=NULL)?FClassname(CBaseEntity::Instance(tr.pHit)):"NULL");
	
			//if(tr.flFraction == 0){
			//	//Moved no distance from the player? This is suspect. Count as a miss.
			//	//This can happen from attacking close to the edge of a wall but parallel to it, particularly around a corner ever so slightly.
			//	//The hull just collides at its first point without moving at all. Not really possible for a real hit, especially after that FindHullIntersection.
			//	//OUT OF DATE WAY OF CHECKING, look above for that specialMiss = TRUE line.
			//	specialMiss = TRUE;
			//}
			

		}
	}
	*/


#ifndef CLIENT_DLL
	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			
			//if (!pHit || pHit->IsBSPModel())
			BOOL hitSuccess = FindHullIntersection(vecSrc, vecEnd, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());

			if (!hitSuccess) {
				// aw man.  No particular point hit.
				specialMiss = TRUE;
				//tr.vecEndPos = vecSrc + gpGlobals->v_forward * 38;  //force it?
			}
			else {
				vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
			}
		}
	}
#endif


	if (tr.flFraction < 1.0 && !specialMiss) {
		// hit
		fDidHit = TRUE;
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage();

		BOOL useBulletHitSound = TRUE;

		if ((m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase()) || IsMultiplayer())
		{
			// first swing does full damage... or apparently, Multiplayer lets all swings do full damage?
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgCrowbar, gpGlobals->v_forward, &tr, DMG_CLUB, 0, TRUE, &useBulletHitSound);
		}
		else
		{
			// subsequent swings do half
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgCrowbar / 2, gpGlobals->v_forward, &tr, DMG_CLUB, 0, TRUE, &useBulletHitSound);
		}
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		int fHitWorld = TRUE;



		/*
		easyForcePrintLine("DID I HIT SOMETHING?");
		if(pEntity){
			easyForcePrintLine("YES: %d %d", pEntity->IsWorld(), pEntity->IsWorldAffiliated());
		}else{
			easyForcePrintLine("NO.");
		}
		*/


		if (EASY_CVAR_GET_DEBUGONLY(drawDebugCrowbar) == 1) {
			//::DebugLine_ClearAll();
			::DebugLine_Setup(vecSrc, (tr.vecEndPos + (tr.vecEndPos - vecSrc) * 4), tr.flFraction);
			::DebugLine_Setup(vecSrc, vecSrc + (vecEnd - vecSrc) * 2, tr.flFraction);
			//::DebugLine_SetupPoint(8, tr.vecEndPos, 0, 0, 255);
		}


		if (pEntity && !pEntity->IsWorld() && !pEntity->IsWorldAffiliated())
		{

			//hit some typical monster, or something clearly not part of the static map. As in, not even something like a prop.
			fHitWorld = FALSE;

			//MODDD - let's just have a Classify not NONE check and organic check instead.
			//if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			if (pEntity->Classify() != CLASS_NONE && pEntity->isOrganic() == TRUE)
			{

				if (canPlayHitSound) {

					//MODDD - if the traceattack method of whatever was hit told us not to play a sound (like a ricochet, which already handled it on its own), we won't.
					if (useBulletHitSound) {

						// play thwack or smack sound
						switch (RANDOM_LONG(0, 2))
						{
						case 0:
							EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod1.wav", 1, ATTN_NORM); break;
						case 1:
							EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod2.wav", 1, ATTN_NORM); break;
						case 2:
							EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod3.wav", 1, ATTN_NORM); break;
						}
					}//END OF useBulletHitSound check

				}//END OF canPlayHitSound check


				//This now always happens for flesh (organic) hits.
				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;

				if (!pEntity->IsAlive()) {
					//no, wait until the end...  use what it was going to return anyways instead.
					//also this was already set to TRUE by default. whatever.
					fDidHit = TRUE;
					//return TRUE;
				}
				else {
					flVol = 0.1;
				}
			}
			else {
				float fvolbar = 0;
				//doens't have any sort of Classify() section or inorganic? Make the usual crowbar metal-hitting sound.
				//easyForcePrintLine("ILL realize I just hit some metallic enemy");

				// Hitting a metalic foe? Do closer to full volume.
				fvolbar = 0.71;

				if (!useBulletHitSound) {
					//reduce the sound instead.
					fvolbar = 0.34;
				}

				if (canPlayHitSound) {
					//!!!!!
					// For now, metallics don't care about "useBulletHitSound", ricochet or not they play this. ... besides a little volume reduction on the hit sound.

					// also play crowbar strike
					switch (RANDOM_LONG(0, 1))
					{
					case 0:
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit1.wav", fvolbar, ATTN_NORM, 0, 103 + RANDOM_LONG(0, 3));
						break;
					case 1:
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit2.wav", fvolbar, ATTN_NORM, 0, 103 + RANDOM_LONG(0, 3));
						break;
					}

				}//END OF canPlayHitSound check

				//MODDD - moved here, also for hitting metalic monsters. 0.6 multiple since that is a common reduction given by textures on world hits anyways.
				m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME * 0.6;

			}//END OF else OF class / organic check

		}//END OF hit pEntity (not NULL) check

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			//return 1;
			//MODDD NOTE IMPORTANT - it is still possible to hit an object with a more indirect trace like a HULL trace, but this does not
			// tell exactly WHAT point was hit.  TEXTURETYPE_PlaySound for whatever reason, is much more strict and needs to hit an exact surface...
			// or be close enough, it's an engine call to find what type of texture (plane) is hit for what sound to play.
			// If it still doesn't find any texture / surface / plane, whatever you want to call it, we can just force it to play at the usual
			// crowbar volume anyways.

			BOOL forcedByMultiplayer = FALSE;

			//MODDD - TEST ME... above is the original, unsure if the alternative below has better overall results.
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			//float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, (tr.vecEndPos + (tr.vecEndPos - vecSrc) * 4), BULLET_PLAYER_CROWBAR);


			if (canPlayHitSound) {

				if (IsMultiplayer()) {
					//MODDD NOTE - check this out?  why is it that way if this is accurate still??
					// override the volume here, cause we don't play texture sounds in multiplayer, 
					// and fvolbar is going to be 0 from the above call.


					switch ((int)EASY_CVAR_GET_DEBUGONLY(multiplayerCrowbarHitSoundMode)) {
					case 0:
						//nothing.
						fvolbar = 0;
						forcedByMultiplayer = TRUE;
						break;
					case 1:
						//leave it up to TEXTURETYPE anyways. No edits here.

						break;
					case 2:
						//force it to 0.6 like a lot of crowbar sounds.
						fvolbar = 0.6;
						forcedByMultiplayer = TRUE;
						break;
					case 3:
						//retail.  Force it to max.
						fvolbar = 1;
						forcedByMultiplayer = TRUE;
						break;
					default:
						//nothing?

						break;
					}//END OF switch
				}//END OF IsMultiplayer check.


				//if forced by multiplayer, don't bother. We meant to make it 0 or whatever.
				if (fvolbar == 0 && !forcedByMultiplayer) {
					//NOTICE - it's possible we DID hit something (given taking this route from trace results, we did infact), but because
					//         an extra linetrace is needed to detect what surface was hit, and it might have come from a HULL trace instead,
					//         we aren't given the exact point that was hit, only that there was a hit.
					//soooo should we force it to a safe default like 0.6 in that case??
					//but how does it know to draw a decal at the same sometimes.. baffling.  Like it searches near the tr->vecEndPos (what it ultiamtely
					//sends off to UTIL_DecalTrace in util.cpp).

					//If we WERE to assume a texture sound were picked or something, we would need to enforce the default here too.
					//But the same sound (cbar_hit1 or 2.wav) is used always, so it's fine now.

					fvolbar = 0.6;
				}
				//fvolbar = 1;

				// also play crowbar strike
				switch (RANDOM_LONG(0, 1))
				{
				case 0:
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				case 1:
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				}

			}//END OF canPlayHitSound check

			/*
			//MODDD - new.  Also serverside, make the particle effect when hitting a non-monster:
			//NO.  The crowbar won't do this now.
			MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, tr.vecEndPos );
				WRITE_BYTE( TE_GUNSHOT );
				WRITE_COORD( tr.vecEndPos.x );
				WRITE_COORD( tr.vecEndPos.y );
				WRITE_COORD( tr.vecEndPos.z );
			MESSAGE_END();
			*/

			// delay the decal a bit
			m_trHit = tr;

			//MODDD - moved here, now just for hitting the world.
			m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;

		}//END OF if(fHitWorld)


		//??? INVESTIGATE W/ PRINTOUTS
		/*
		if(!(pEntity || fHitWorld)){
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7;
		}
		*/

		//MODDD - moved to the above if-then.
		//m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;

	}//END OF trace hit OR special miss checks

#endif
	

	//specialMiss = TRUE;

	//( int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
	//swingMissChoice

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usCrowbar, 
	0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, m_fInAttack, 0, FALSE, FALSE );


#ifndef CLIENT_DLL
	//Vector vecWhoCares = FireBulletsPlayerEh( 1, vecSrc, gpGlobals->v_forward, VECTOR_CONE_1DEGREES, 32, BULLET_PLAYER_9MM, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
#endif

#ifndef CLIENT_DLL
	//easyForcePrintLine("HERE HE GO C fl:%.2f ss:%d io:%d fas:%d hit:%s", tr.flFraction, tr.fStartSolid, tr.fInOpen, tr.fAllSolid, (tr.pHit!=NULL)?FClassname(CBaseEntity::Instance(tr.pHit)):"NULL");
#endif
	//easyForcePrintLine("OH my man WHAT YOU HIT %s", (tr.pHit!=NULL)?FClassname(CBaseEntity::Instance(tr.pHit)):"NULL");



	//MODDD - special way to miss.
	// ...not that specialMiss could even be set TRUE clientside anyway.
	if ( tr.flFraction >= 1.0 || specialMiss )
	{
		if (fFirst)
		{
			/*
			#if CLIENT_DLL == 0
			swingMissChoice = (m_iSwingMiss++) % 3;
			#endif
			*/

//#if CLIENT_DLL
			switch(m_fInAttack)
			{
			case 0:
				//SendWeaponAnimBypass( CROWBAR_ATTACK1MISS, 1 );
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (10.0/22.0) + randomIdleAnimationDelay();
			break;
			case 1:
				//SendWeaponAnimBypass( CROWBAR_ATTACK2MISS, 1 );
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (13.0/22.0) + randomIdleAnimationDelay();
			break;
			case 2:
				//SendWeaponAnimBypass( CROWBAR_ATTACK3MISS, 1 );
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (18.0/24.0) + randomIdleAnimationDelay();
			 break;
			}
//#endif
			if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
				// miss
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
			}else{
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
			}

			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
			
			
			#ifndef CLIENT_DLL
				m_fInAttack = (++m_fInAttack) % 3;
				//easyPrintLine("SERVAH");
			#else
				//easyPrintLine("CLIENTT");
			#endif
			
		}
	}
	else
	{
		//return TRUE;

		//int choicetemp2 = ((m_iSwing++) %3);
		//easyPrintLine("bee %d", choicetemp2);
		//switch( ((m_iSwing++) % 2) + 1 )
		switch( m_fireState )
		{
		case 0:
			SendWeaponAnimBypass( CROWBAR_ATTACK1HIT );
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (11.0/22.0) + randomIdleAnimationDelay();
			 break;
		case 1:
			SendWeaponAnimBypass( CROWBAR_ATTACK2HIT );
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (14.0/22.0) + randomIdleAnimationDelay();
			 break;
		case 2:
			SendWeaponAnimBypass( CROWBAR_ATTACK3HIT );
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (19.0/24.0) + randomIdleAnimationDelay();
			 break;
		}
		#ifndef CLIENT_DLL
			m_fireState = (++m_fireState) % 3;
			//easyPrintLine("SERVAH");
		#else
			//easyPrintLine("CLIENTT");
		#endif

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		// at least do these things?
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25;

		//easyPrintLine("efefefefefef");
		//return TRUE;



		//MODDD
		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelay) == 0){
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25;
		}else{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(cheat_minimumfiredelaycustom);
		}

		SetThink( &CCrowbar::Smack );
		pev->nextthink = UTIL_WeaponTimeBase() + 0.2;

	}
	return fDidHit;
}



//MODDD - added.
// I think idle2 or idle3 loops, so that time it should've been static at the end doesn't happen
// (replays the anim), so re-picking an idle causes a sudden jump in the crowbar.
// Not a huge deal but eh.
void CCrowbar::WeaponIdle( void )
{
	//ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );


	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	//MODDD NOTE - "m_flTimeWeaponIdle" changes per "iAnim" choice are additions.
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	
		iAnim = CROWBAR_IDLE;
		m_flTimeWeaponIdle = (35.0/13.0) + randomIdleAnimationDelay();
	break;
	default:
	case 1:
		iAnim = CROWBAR_IDLE2;
		m_flTimeWeaponIdle = (80.0/15.0) + randomIdleAnimationDelay();
	break;
	case 2:
		iAnim = CROWBAR_IDLE3;
		m_flTimeWeaponIdle = (80.0/15.0) + randomIdleAnimationDelay();
	break;
	}

	//MODDD - that's a good idea, yea?
	forceBlockLooping();

	SendWeaponAnim( iAnim );
	//MODDD - why was this random?
	//m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}

