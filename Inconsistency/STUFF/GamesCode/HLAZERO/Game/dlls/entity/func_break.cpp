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
/*

===== bmodels.cpp ========================================================

  spawn, think, and use functions for entities that use brush models
*/
// Breakables look to have models specific to map by a code (like "*28").  The given map has something in mind for that.
// Must mean some special offset for its origin too.  Origin starts at 0,0,0 or 0,0,1 for pushables that have never been moved,
// even though that doesn't make much sense.  Nothing else does that, always an absolute origin.
// Spawning a breakable with a wildcard model at (0,0,0) will thus be easier to manage / work with, probably put it back
// where it was at the start of the map.
// The absolute origin can be retrieved though, just use VecBModelOrigin(pev).  Same as Center().

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "func_break.h"
#include "decals.h"
#include "explode.h"
#include "player.h"
#include "basemonster.h"
#include "r_efx.h"
#include "util_debugdraw.h"

EASY_CVAR_EXTERN_DEBUGONLY(sparksComputerHitMulti)

extern DLL_GLOBAL Vector g_vecAttackDir;

// =================== FUNC_Breakable ==============================================

// Just add more items to the bottom of this array and they will automagically be supported
// This is done instead of just a classname in the FGD so we can control which entities can
// be spawned, and still remain fairly flexible
const char *CBreakable::pSpawnObjects[] =
{
	NULL,				// 0
	"item_battery",		// 1
	"item_healthkit",	// 2
	"weapon_9mmhandgun",// 3
	"ammo_9mmclip",		// 4
	"weapon_9mmAR",		// 5
	"ammo_9mmAR",		// 6
	"ammo_ARgrenades",	// 7
	"weapon_shotgun",	// 8
	"ammo_buckshot",	// 9
	"weapon_crossbow",	// 10
	"ammo_crossbow",	// 11
	"weapon_357",		// 12
	"ammo_357",			// 13
	"weapon_rpg",		// 14
	"ammo_rpgclip",		// 15
	"ammo_gaussclip",	// 16
	"weapon_handgrenade",// 17
	"weapon_tripmine",	// 18
	"weapon_satchel",	// 19
	"weapon_snark",		// 20
	"weapon_hornetgun",	// 21
	//MODDD!!!
	"item_antidote",		// 22
	"item_adrenaline",	// 23
	"item_radiation",	// 24
	//MODDD - NOTE.  This used to be named "item_longjumpcharge", but support for item_longjump and "charge" removed, now there is only item_longjump.  Use that.
	"item_longjump",	// 25
	"weapon_chumtoad",  //26
};



const char *CBreakable::pSoundsWood[] = 
{
	"debris/wood1.wav",
	"debris/wood2.wav",
	"debris/wood3.wav",
};

const char *CBreakable::pSoundsFlesh[] = 
{
	"debris/flesh1.wav",
	"debris/flesh2.wav",
	"debris/flesh3.wav",
	"debris/flesh5.wav",
	"debris/flesh6.wav",
	"debris/flesh7.wav",
};

const char *CBreakable::pSoundsMetal[] = 
{
	"debris/metal1.wav",
	"debris/metal2.wav",
	"debris/metal3.wav",
};

const char *CBreakable::pSoundsConcrete[] = 
{
	"debris/concrete1.wav",
	"debris/concrete2.wav",
	"debris/concrete3.wav",
};


const char *CBreakable::pSoundsGlass[] = 
{
	"debris/glass1.wav",
	"debris/glass2.wav",
	"debris/glass3.wav",
};

//
// func_breakable - bmodel that breaks into pieces after taking damage
//
LINK_ENTITY_TO_CLASS( func_breakable, CBreakable );


TYPEDESCRIPTION CBreakable::m_SaveData[] =
{
	DEFINE_FIELD( CBreakable, m_Material, FIELD_INTEGER ),
	DEFINE_FIELD( CBreakable, m_Explosion, FIELD_INTEGER ),
// Don't need to save/restore these because we precache after restore
//	DEFINE_FIELD( CBreakable, m_idShard, FIELD_INTEGER ),
	DEFINE_FIELD( CBreakable, m_angle, FIELD_FLOAT ),
	DEFINE_FIELD( CBreakable, m_iszGibModel, FIELD_STRING ),
	DEFINE_FIELD( CBreakable, m_iszSpawnObject, FIELD_STRING ),
	// Explosion magnitude is stored in pev->impulse
};
IMPLEMENT_SAVERESTORE( CBreakable, CBaseEntity );


void CBreakable::KeyValue( KeyValueData* pkvd )
{
	// UNDONE_WC: explicitly ignoring these fields, but they shouldn't be in the map file!
	if (FStrEq(pkvd->szKeyName, "explosion"))
	{
		if (!stricmp(pkvd->szValue, "directed"))
			m_Explosion = expDirected;
		else if (!stricmp(pkvd->szValue, "random"))
			m_Explosion = expRandom;
		else
			m_Explosion = expRandom;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "material"))
	{
		int i = atoi( pkvd->szValue);

		// 0:glass, 1:metal, 2:flesh, 3:wood

		if ((i < 0) || (i >= matLastMaterial))
			m_Material = matWood;
		else
			m_Material = (Materials)i;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "deadmodel"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "shards"))
	{
//			m_iShards = atof(pkvd->szValue);
			pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "gibmodel") )
	{
		m_iszGibModel = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnobject") )
	{
		int object = atoi( pkvd->szValue );
		if ( object > 0 && object < ARRAYSIZE(pSpawnObjects) )
			m_iszSpawnObject = MAKE_STRING( pSpawnObjects[object] );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "explodemagnitude") )
	{
		ExplosionSetMagnitude( atoi( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "lip") )
		pkvd->fHandled = TRUE;
	else
		CBaseDelay::KeyValue( pkvd );
}

BOOL CBreakable::IsWorldAffiliated(){
	//Counting this, but other things may want to look for func_breakable's specifically.
	return TRUE;
}

void CBreakable::Spawn( void )
{
    Precache( );

	if ( FBitSet( pev->spawnflags, SF_BREAK_TRIGGER_ONLY ) )
		pev->takedamage	= DAMAGE_NO;
	else
		pev->takedamage	= DAMAGE_YES;
  
	pev->solid		= SOLID_BSP;
    pev->movetype	= MOVETYPE_PUSH;
    m_angle			= pev->angles.y;
	pev->angles.y	= 0;

	// HACK:  matGlass can receive decals, we need the client to know about this
	//  so use class to store the material flag
	if ( m_Material == matGlass )
	{
		pev->playerclass = 1;
	}

	setModel(STRING(pev->model) );//set size and link into world.

	//safety.
	pev->classname = MAKE_STRING("func_breakable");


	SetTouch( &CBreakable::BreakTouch );
	if ( FBitSet( pev->spawnflags, SF_BREAK_TRIGGER_ONLY ) )		// Only break on trigger
		SetTouch( NULL );

	// Flag unbreakable glass as "worldbrush" so it will block ALL tracelines
	//MODDD NOTE - does this FL_WORLDBRUSH have other useful properties for checks then?  Or to refer to / check in our own tracelines?
	if ( !IsBreakable() && pev->rendermode != kRenderNormal )
		pev->flags |= FL_WORLDBRUSH;
}


const char **CBreakable::MaterialSoundList( Materials precacheMaterial, int &soundCount )
{
	const char	**pSoundList = NULL;

    switch ( precacheMaterial ) 
	{
	case matWood:
		pSoundList = pSoundsWood;
		soundCount = ARRAYSIZE(pSoundsWood);
		break;
	case matFlesh:
		pSoundList = pSoundsFlesh;
		soundCount = ARRAYSIZE(pSoundsFlesh);
		break;
	case matComputer:
	case matUnbreakableGlass:
	case matGlass:
		pSoundList = pSoundsGlass;
		soundCount = ARRAYSIZE(pSoundsGlass);
		break;

	case matMetal:
		pSoundList = pSoundsMetal;
		soundCount = ARRAYSIZE(pSoundsMetal);
		break;

	case matCinderBlock:
	case matRocks:
		pSoundList = pSoundsConcrete;
		soundCount = ARRAYSIZE(pSoundsConcrete);
		break;
	case matCeilingTile:
	case matNone:
	default:
		soundCount = 0;
		break;
	}

	return pSoundList;
}

void CBreakable::MaterialSoundPrecache( Materials precacheMaterial )
{
	const char	**pSoundList;
	int		i, soundCount = 0;

	pSoundList = MaterialSoundList( precacheMaterial, soundCount );

	for ( i = 0; i < soundCount; i++ )
	{
		PRECACHE_SOUND( (char *)pSoundList[i] );
	}
}

void CBreakable::MaterialSoundRandom( edict_t *pEdict, Materials soundMaterial, float volume )
{
	const char	**pSoundList;
	int		soundCount = 0;

	pSoundList = MaterialSoundList( soundMaterial, soundCount );

	if ( soundCount )
		EMIT_SOUND( pEdict, CHAN_BODY, pSoundList[ RANDOM_LONG(0,soundCount-1) ], volume, 1.0 );
}


//MODDD
CBreakable::CBreakable(){
	m_idShardText = NULL;
}

void CBreakable::Precache( void )
{
	const char *pGibName;

    switch (m_Material) 
	{
	case matWood:
		pGibName = "models/woodgibs.mdl";
		
		PRECACHE_SOUND("debris/bustcrate1.wav");
		PRECACHE_SOUND("debris/bustcrate2.wav");
		break;
	case matFlesh:
		pGibName = "models/fleshgibs.mdl";
		
		PRECACHE_SOUND("debris/bustflesh1.wav");
		PRECACHE_SOUND("debris/bustflesh2.wav");
		break;
	case matComputer:
		PRECACHE_SOUND("buttons/spark5.wav");
		PRECACHE_SOUND("buttons/spark6.wav");
		pGibName = "models/computergibs.mdl";
		
		PRECACHE_SOUND("debris/bustmetal1.wav");
		PRECACHE_SOUND("debris/bustmetal2.wav");
		break;

	case matUnbreakableGlass:
	case matGlass:
		pGibName = "models/glassgibs.mdl";
		
		PRECACHE_SOUND("debris/bustglass1.wav");
		PRECACHE_SOUND("debris/bustglass2.wav");
		break;
	case matMetal:
		pGibName = "models/metalplategibs.mdl";
		//pGibName = "models/woodgibs.mdl";
		
		PRECACHE_SOUND("debris/bustmetal1.wav");
		PRECACHE_SOUND("debris/bustmetal2.wav");
		break;
	case matCinderBlock:
		pGibName = "models/cindergibs.mdl";
		
		PRECACHE_SOUND("debris/bustconcrete1.wav");
		PRECACHE_SOUND("debris/bustconcrete2.wav");
		break;
	case matRocks:
		pGibName = "models/rockgibs.mdl";
		
		PRECACHE_SOUND("debris/bustconcrete1.wav");
		PRECACHE_SOUND("debris/bustconcrete2.wav");
		break;
	case matCeilingTile:
		pGibName = "models/ceilinggibs.mdl";
		
		PRECACHE_SOUND ("debris/bustceiling.wav");  
		break;

		//ventgibs?

	default:
		easyPrintLine("WHAT DEFAULTED? (note: unused if the map still forces something)   unexpected material # : %d", m_Material);
		//MODDD - need a break sound?
		pGibName = "models/shrapnel.mdl";
		//sound?

	break;
	}
	MaterialSoundPrecache( m_Material );
	if ( m_iszGibModel )
		pGibName = STRING(m_iszGibModel);

	//easyPrintLine("YES %s", pGibName);
	m_idShard = PRECACHE_MODEL( (char *)pGibName );

	m_idShardText =  (char *)pGibName ;

	// Precache the spawn item's data
	if ( m_iszSpawnObject )
		UTIL_PrecacheOther( (char *)STRING( m_iszSpawnObject ) );
}

// play shard sound when func_breakable takes damage.
// the more damage, the louder the shard sound.


void CBreakable::DamageSound( void )
{
	int pitch;
	float fvol;
	char *rgpsz[6];
	int i;
	int material = m_Material;

//	if (RANDOM_LONG(0,1))
//		return;

	if (RANDOM_LONG(0,2))
		pitch = PITCH_NORM;
	else
		pitch = 95 + RANDOM_LONG(0,34);

	fvol = RANDOM_FLOAT(0.75, 1.0);

	if (material == matComputer && RANDOM_LONG(0,1))
		material = matMetal;

	switch (material)
	{
	case matComputer:
	case matGlass:
	case matUnbreakableGlass:
		rgpsz[0] = "debris/glass1.wav";
		rgpsz[1] = "debris/glass2.wav";
		rgpsz[2] = "debris/glass3.wav";
		i = 3;
		break;

	case matWood:
		rgpsz[0] = "debris/wood1.wav";
		rgpsz[1] = "debris/wood2.wav";
		rgpsz[2] = "debris/wood3.wav";
		i = 3;
		break;

	case matMetal:
		rgpsz[0] = "debris/metal1.wav";
		rgpsz[1] = "debris/metal3.wav";
		rgpsz[2] = "debris/metal2.wav";
		i = 2;
		break;

	case matFlesh:
		rgpsz[0] = "debris/flesh1.wav";
		rgpsz[1] = "debris/flesh2.wav";
		rgpsz[2] = "debris/flesh3.wav";
		rgpsz[3] = "debris/flesh5.wav";
		rgpsz[4] = "debris/flesh6.wav";
		rgpsz[5] = "debris/flesh7.wav";
		i = 6;
		break;

	case matRocks:
	case matCinderBlock:
		rgpsz[0] = "debris/concrete1.wav";
		rgpsz[1] = "debris/concrete2.wav";
		rgpsz[2] = "debris/concrete3.wav";
		i = 3;
		break;

	case matCeilingTile:
		// UNDONE: no ceiling tile shard sound yet
		i = 0;
		break;
	}

	if (i)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, rgpsz[RANDOM_LONG(0,i-1)], fvol, ATTN_NORM, 0, pitch);
}

void CBreakable::ReportGeneric(){

	CBaseDelay::ReportGeneric();
	easyForcePrintLine("My model: %s", STRING(pev->model));
	::UTIL_printLineVector("Origin", pev->origin);
	
	/*
	Vector Amins = pev->absmin;
	Vector Amaxs = pev->absmax;
	Vector mins = pev->mins;
	Vector maxs = pev->maxs;
	*/
	
	//int x = 666;

}//END OF ReportGeneric


void CBreakable::BreakTouch( CBaseEntity *pOther )
{
	float flDamage;
	entvars_t*	pevToucher = pOther->pev;
	
	// only players can break these right now
	if ( !pOther->IsPlayer() || !IsBreakable() )
	{
        return;
	}

	if ( FBitSet ( pev->spawnflags, SF_BREAK_TOUCH ) )
	{// can be broken when run into 
		flDamage = pevToucher->velocity.Length() * 0.01;

		if (flDamage >= pev->health)
		{
			SetTouch( NULL );
			TakeDamage(pevToucher, pevToucher, flDamage, DMG_CRUSH);

			// do a little damage to player if we broke glass or computer
			pOther->TakeDamage( pev, pev, flDamage/4, DMG_SLASH );
		}
	}

	if ( FBitSet ( pev->spawnflags, SF_BREAK_PRESSURE ) && pevToucher->absmin.z >= pev->maxs.z - 2 )
	{// can be broken when stood upon
		
		// play creaking sound here.
		DamageSound();

		SetThink ( &CBreakable::Die );
		SetTouch( NULL );
		
		if ( m_flDelay == 0 )
		{// !!!BUGBUG - why doesn't zero delay work?
			m_flDelay = 0.1;
		}

		pev->nextthink = pev->ltime + m_flDelay;

	}
}

//
// Smash the our breakable object
//

// Break when triggered
void CBreakable::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( IsBreakable() )
	{
		pev->angles.y = m_angle;
		UTIL_MakeVectors(pev->angles);
		g_vecAttackDir = gpGlobals->v_forward;

		Die();
	}
}


GENERATE_TRACEATTACK_IMPLEMENTATION(CBreakable)
{
	// random spark if this is a 'computer' object
	if (RANDOM_LONG(0,1) )
	{
		switch( m_Material )
		{
			case matComputer:
			{
				//MODDD!
				//UTIL_Sparks( ptr->vecEndPos, DEFAULT_SPARK_BALLS, EASY_CVAR_GET_DEBUGONLY(sparksComputerHitMulti) );

				float flVolume = RANDOM_FLOAT ( 0.7 , 1.0 );//random volume range
				switch ( RANDOM_LONG(0,1) )
				{
					case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark5.wav", flVolume, ATTN_NORM);	break;
					case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark6.wav", flVolume, ATTN_NORM);	break;
				}
			}
			break;
			
			case matUnbreakableGlass:
				UTIL_Ricochet( ptr->vecEndPos, RANDOM_FLOAT(0.5,1.5) );
			break;
		}
	}

	GENERATE_TRACEATTACK_PARENT_CALL(CBaseDelay);
}


//=========================================================
// Special takedamage for func_breakable. Allows us to make
// exceptions that are breakable-specific
// bitsDamageType indicates the type of damage sustained ie: DMG_CRUSH
//=========================================================
GENERATE_TAKEDAMAGE_IMPLEMENTATION(CBreakable)
{
	Vector	vecTemp;

	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin). 
	if ( pevAttacker == pevInflictor )	
	{
		vecTemp = pevInflictor->origin - ( pev->absmin + ( pev->size * 0.5 ) );
		
		// if a client hit the breakable with a crowbar, and breakable is crowbar-sensitive, break it now.
		if ( FBitSet ( pevAttacker->flags, FL_CLIENT ) &&
				 FBitSet ( pev->spawnflags, SF_BREAK_CROWBAR ) && (bitsDamageType & DMG_CLUB))
			flDamage = pev->health;
	}
	else
	// an actual missile was involved.
	{
		vecTemp = pevInflictor->origin - ( pev->absmin + ( pev->size * 0.5 ) );
	}
	
	if (!IsBreakable())
		return 0;

	// Breakables take double damage from the crowbar
	if ( bitsDamageType & DMG_CLUB )
		flDamage *= 2;

	// Boxes / glass / etc. don't take much poison damage, just the impact of the dart - consider that 10%
	if ( bitsDamageType & DMG_POISON )
		flDamage *= 0.1;

// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();
		
// do the damage
	pev->health -= flDamage;
	if (pev->health <= 0)
	{
		Killed( pevInflictor, pevAttacker, GIB_NORMAL );
		Die();
		return 0;
	}

	// Make a shard noise each time func breakable is hit.
	// Don't play shard noise if cbreakable actually died.

	DamageSound();

	return 1;
}


void CBreakable::Die( void )
{
	Vector vecSpot;// shard origin
	Vector vecVelocity;// shard velocity
	CBaseEntity *pEntity = NULL;
	char cFlag = 0;
	int pitch;
	float fvol;
	float fattn;
	
	pitch = 95 + RANDOM_LONG(0,29);

	if (pitch > 97 && pitch < 103)
		pitch = 100;

	// The more negative pev->health, the louder
	// the sound should be.



	//MODDD - achieve higher volumes at lower damages now, barely anything will peak at 100,
	// not even direct explosives (some lost from distance to the origin, like 95 to 98 at best).
	// There's fully-charged gauss hits doing a bit over 200 but eh.  Overkill always was overkill.
	// Also using fabs now.
	// Wait, come to think if it wasn't this kinda pointless?  Already a random added from 0.85 to 1.0.
	// That means even at minimum, the volume starts at 0.85, and very rarely that close.
	// A reasonable average of 0.92 to 0.93 does not leave much room for overkill to make much of a 
	// difference.  CHANGED.
	// How about affecting attenuation too.
	//fvol = RANDOM_FLOAT(0.85, 1.0) + (abs(pev->health) / 100.0);
	
	// don't exceed 1!
	float overkillVolumeMulti = min(fabs(pev->health), 60.0f) / 60.0f;
	
	fvol = RANDOM_FLOAT(0.74f, 0.82f) + overkillVolumeMulti * 0.26f;
	// ATTN_NORM is 0.8.  Lower it is, further the sound carries, can create the illusion of loudness.
	fattn = 0.75f + -(overkillVolumeMulti * 0.20f);

	if (fvol > 1.0f){
		fvol = 1.0f;
	}


	switch (m_Material)
	{
	case matGlass:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass1.wav", fvol, fattn, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass2.wav", fvol, fattn, 0, pitch);	
			break;
		}
		cFlag = BREAK_GLASS;
		break;

	case matWood:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate1.wav", fvol, fattn, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate2.wav", fvol, fattn, 0, pitch);	
			break;
		}
		cFlag = BREAK_WOOD;
		break;

	case matComputer:
	case matMetal:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal1.wav", fvol, fattn, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal2.wav", fvol, fattn, 0, pitch);	
			break;
		}
		cFlag = BREAK_METAL;
		//MODDD - THIS produces the spark when interpreted in entity.cpp:
		cFlag |= FTENT_SMOKETRAIL;

		break;

	case matFlesh:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh1.wav", fvol, fattn, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh2.wav", fvol, fattn, 0, pitch);	
			break;
		}
		cFlag = BREAK_FLESH;
		break;

	case matRocks:
	case matCinderBlock:
		switch ( RANDOM_LONG(0,1) )
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete1.wav", fvol, fattn, 0, pitch);	
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete2.wav", fvol, fattn, 0, pitch);	
			break;
		}
		cFlag = BREAK_CONCRETE;
		break;

	case matCeilingTile:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustceiling.wav", fvol, fattn, 0, pitch);
		break;
	}
    
	if (m_Explosion == expDirected)
		vecVelocity = g_vecAttackDir * 200;
	else
	{
		vecVelocity.x = 0;
		vecVelocity.y = 0;
		vecVelocity.z = 0;
	}

	/*
	easyPrintLine("WHAT WAS MAT: %d", m_Material);
	easyPrintLine("MAT NAME: %s", m_idShardText);
	easyPrintLine("FLAGS W/O TRAIL: %d", cFlag & ~FTENT_SMOKETRAIL);
	*/


	//easyPrintLine("SHARD ID %d", m_idShard);
	//m_idShard = 258;   Not that, can be odd (makes gibs into RPG rounds?).
	
	//MODDD - How to force gibs:
	
	/*
	char* pGibName = "models/woodgibs.mdl";
	m_idShard = PRECACHE_MODEL( (char *)pGibName );
	cFlag = BREAK_WOOD;
	
	cFlag = BREAK_WOOD;
	*/
	//cFlag = BREAK_WOOD;


	vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
	// ???????
	//Vector vecSpot2 = pev->maxs - pev->mins;
	//return;


	/*
	MESSAGE_BEGIN( MSG_ONE, gmsgDrowning, NULL, pev );
			WRITE_BYTE( drowning );
		MESSAGE_END();
		*/


	//easyPrintLine("WHAT ARE THEY %.2f, %.2f, %.2f,     %.2f, %.2f, %.2f", vecSpot.x, vecSpot.y, vecSpot.z, vecSpot2.x, vecSpot2.y, vecSpot2.z); 

	//Vector modVect1 = Vector(8, 8, 8);
	//Vector modVect2 = Vector(-16, -16, -16);
	Vector modVect1 = Vector(0, 0, 0);
	Vector modVect2 = Vector(0, 0, 0);


	//get bounds?

	//Spawns gibs ordinarilly.
	
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSpot );
		WRITE_BYTE( TE_BREAKMODEL);

		// position
		WRITE_COORD( vecSpot.x );
		WRITE_COORD( vecSpot.y );
		WRITE_COORD( vecSpot.z );

		// size
		WRITE_COORD( pev->size.x);
		WRITE_COORD( pev->size.y);
		WRITE_COORD( pev->size.z);

		// velocity
		WRITE_COORD( vecVelocity.x ); 
		WRITE_COORD( vecVelocity.y );
		WRITE_COORD( vecVelocity.z );

		// randomization
		WRITE_BYTE( 10 ); 

		// Model
		WRITE_SHORT( m_idShard );	//model id#

		// # of shards
		WRITE_BYTE( 0 );	// let client decide

		// duration
		WRITE_BYTE( 25 );// 2.5 seconds

		// flags
		WRITE_BYTE( cFlag );
	MESSAGE_END();
	


	////PLAYBACK_EVENT_FULL (FEV_GLOBAL, pGrenade->edict(), g_sTrail, 0.0, 
	//(float *)&pGrenade->pev->origin, (float *)&pGrenade->pev->angles, 0.7, 0.0, pGrenade->entindex(), ROCKET_TRAIL, 0, 0);


	float size = pev->size.x;
	if ( size < pev->size.y )
		size = pev->size.y;
	if ( size < pev->size.z )
		size = pev->size.z;



	//MODDD - NOTE.  OHHhhhhhh.   This is checking to see if anything above the recently destroyed
	// breakable was standing on it, so drop its 'FL_ONGROUND' flag and let it fall.  Some stuff
	// needs that update, clearly.  Could be a useful idea for other places, turn into a util method,
	// something like "UTIL_CheckStandingOnDeleted(somePEV)" ?
	
	// !!! HACK  This should work!
	// Build a box above the entity that looks like an 8 pixel high sheet
	Vector mins = pev->absmin;
	Vector maxs = pev->absmax;
	mins.z = pev->absmax.z;
	maxs.z += 8;

	// BUGBUG -- can only find 256 entities on a breakable -- should be enough
	CBaseEntity *pList[256];
	int count = UTIL_EntitiesInBox( pList, 256, mins, maxs, FL_ONGROUND );
	if ( count )
	{
		for ( int i = 0; i < count; i++ )
		{
			ClearBits( pList[i]->pev->flags, FL_ONGROUND );
			pList[i]->pev->groundentity = NULL;
		}
	}

	// Don't fire something that could fire myself
	pev->targetname = 0;

	pev->solid = SOLID_NOT;
	// Fire targets on break
	SUB_UseTargets( NULL, USE_TOGGLE, 0 );

	SetThink( &CBaseEntity::SUB_Remove );
	pev->nextthink = pev->ltime + 0.1;
	if ( m_iszSpawnObject ){
		CBaseEntity* theEnt = CBaseEntity::Create( (char *)STRING(m_iszSpawnObject), VecBModelOrigin(pev), pev->angles, edict() );
		/*
		// IDEA:  things in the breakable start mid-air and fall to the ground?
		// Dunno if that would look cheezy
		if(theEnt != NULL){
			theEnt->pev->flags &= ~(FL_ONGROUND); //FL_MONSTERCLIP
			theEnt->pev->groundentity = NULL;
			theEnt->pev->origin.z += 16;
			theEnt->pev->velocity.z = -3;
		}
		*/
	}//'spawn object given' check

	if ( Explodable() )
	{
		ExplosionCreate( Center(), pev->angles, edict(), ExplosionMagnitude(), TRUE );
	}
}


//MODDD - NOTE - this was here, as-is in retail.  Just checking that the mat isn't matUnbreakableGlass
//(although, a spawnflag can make this or pushables still impossible to destroy.  See "isDestructibleInanimate" for more accuracy,
//which the AI may need to not look stupid)
BOOL CBreakable::IsBreakable( void ) 
{ 
	return m_Material != matUnbreakableGlass;
}

//MODDD
BOOL CBreakable::isBreakableOrChild(void){
    return TRUE;
}
//MODDD
BOOL CBreakable::isDestructibleInanimate(void){
	//we're destructible if the mat isn't "unbreakableGlass" and we are missing the BREAK_TRIGGER_ONLY spawnflag.
	return (m_Material != matUnbreakableGlass && !(pev->spawnflags & SF_BREAK_TRIGGER_ONLY) );
}


int CBreakable::DamageDecal( int bitsDamageType )
{
	return DamageDecal(bitsDamageType, 0);
}
int CBreakable::DamageDecal( int bitsDamageType, int bitsDamageTypeMod )
{
	if ( m_Material == matGlass  )
		return DECAL_GLASSBREAK1 + RANDOM_LONG(0,2);

	if ( m_Material == matUnbreakableGlass )
		return DECAL_BPROOF1;

	return CBaseEntity::DamageDecal( bitsDamageType, bitsDamageTypeMod );
}



class CPushable : public CBreakable
{
public:

	static	TYPEDESCRIPTION m_SaveData[];

	static char* m_soundNames[3];

	int	m_lastSound;	// no need to save/restore, just keeps the same sound from playing twice in a row
	float m_maxSpeed;
	float m_soundTime;
	//MODDD - nevermind
	//float m_spawnFriction;
	float blockForceVelocityTime;



	CPushable(void);

	//MODDD
	virtual BOOL IsBreakable(void);
	virtual BOOL isBreakableOrChild(void);
	virtual BOOL isDestructibleInanimate(void);

	virtual float massInfluence(void);


	void Spawn ( void );
	void Precache( void );
	void Touch ( CBaseEntity *pOther );
	void Move( CBaseEntity *pMover, int push, int useValue );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT StopSound( void );
//	virtual void SetActivator( CBaseEntity *pActivator ) { m_pPusher = pActivator; }

	//MODDD NOTE - it may be tempting to let pushables move across transitions, but be very careful.  It seems they still
	//             depend on some offset given by the map, or maybe not.  But it could turn invisible too if there isn't
	//             a model at that number ( "*27" for instance).  If a different models is suggested for that number,
	//             it would suddenly change just by going to a new map.
	//MODDD - ALSO, added the FCAP_ONOFF_USE cap.  On use being stopped, cancel velocity.
	// No sense in a pushed obect still sliding, you just would've pushed it further if you wanted it there.
	virtual int ObjectCaps( void ) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE; }
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );

	inline float MaxSpeed( void ) { return m_maxSpeed; }

	void ReportGeneric();
	
	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL

};

CPushable::CPushable(void) {
	blockForceVelocityTime = 0;
}


TYPEDESCRIPTION	CPushable::m_SaveData[] = 
{
	DEFINE_FIELD( CPushable, m_maxSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CPushable, m_soundTime, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CPushable, CBreakable );

LINK_ENTITY_TO_CLASS( func_pushable, CPushable );

char *CPushable::m_soundNames[3] = { "debris/pushbox1.wav", "debris/pushbox2.wav", "debris/pushbox3.wav" };


void CPushable::Spawn( void )
{
	if ( pev->spawnflags & SF_PUSH_BREAKABLE )
		CBreakable::Spawn();
	else
		Precache( );

	pev->movetype	= MOVETYPE_PUSHSTEP;
	pev->solid		= SOLID_BBOX;
	setModel( STRING(pev->model) );

	//safety.
	pev->classname = MAKE_STRING("func_pushable");

	easyForcePrintLine("CPushable: Spawn. Model: %s Spawnflags: %d", STRING(pev->model), pev->spawnflags);

	

	if ( pev->friction > 399 )
		pev->friction = 399;

	m_maxSpeed = 400 - pev->friction;
	SetBits( pev->flags, FL_FLOAT );
	
	//MODDD - nevermind, this looked useless.
	/*
	// Why so low?  Surely a small number can't hurt.
	// Boxes need not behave like they're on ice after all.
	//pev->friction = 0;
	pev->friction = m_maxSpeed * 0.05;
	// and remember what the friction here was.
	m_spawnFriction = pev->friction;
	*/
	
	pev->origin.z += 1;	// Pick up off of the floor
	UTIL_SetOrigin( pev, pev->origin );

	// Multiply by area of the box's cross-section (assume 1000 units^3 standard volume)
	pev->skin = ( pev->skin * (pev->maxs.x - pev->mins.x) * (pev->maxs.y - pev->mins.y) ) * 0.0005;
	m_soundTime = 0;

	//pev->spawnflags &= ~SF_PUSH_BREAKABLE;
}


void CPushable::Precache( void )
{
	for ( int i = 0; i < 3; i++ )
		PRECACHE_SOUND( m_soundNames[i] );

	if ( pev->spawnflags & SF_PUSH_BREAKABLE )
		CBreakable::Precache( );
}


void CPushable::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq(pkvd->szKeyName, "size") )
	{
		int bbox = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		switch( bbox )
		{
		case 0:	// Point
			UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
			break;

		case 2: // Big Hull!?!?	!!!BUGBUG Figure out what this hull really is
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN*2, VEC_DUCK_HULL_MAX*2);
			break;

		case 3: // Player duck
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
			break;

		default:
		case 1: // Player
			UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
			break;
		}
	}
	else if ( FStrEq(pkvd->szKeyName, "buoyancy") )
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBreakable::KeyValue( pkvd );
}


// Pull the func_pushable
void CPushable::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() )
	{
		if ( pev->spawnflags & SF_PUSH_BREAKABLE )
			this->CBreakable::Use( pActivator, pCaller, useType, value );
		return;
	}

	if ( pActivator->pev->velocity != g_vecZero )
		Move( pActivator, 0, value);
}


void CPushable::Touch( CBaseEntity *pOther )
{
	if (pOther->IsWorld()) return;

	/*
	// ALSO, push movements (not 'use') from any player is only allowed so often.
	// NEVERMIND.  Capping the max speed works fine.
	if (pOther->IsPlayer()) {
		if (gpGlobals->time <= flNextPlayerPushAllowed) {
			return;  //STOP
		}
		else {
			// allowed this time
			flNextPlayerPushAllowed = gpGlobals->time + 0.2;
		}
	}
	*/
	Move( pOther, 1, 1 );
}


//MODDD - new parameter, "useValue".  Tell whether the use key is being
// used continually (1) or released (0).
// Only useful if 'push' is 0, since push = 1 means the use-key isn't
// the one making this call.
void CPushable::Move( CBaseEntity *pOther, int push, int useValue )
{
	entvars_t*	pevToucher = pOther->pev;
	int playerTouch = 0;
	int maxSpeedTemp;
	float length;

	//MODDD - new.  Whether to allow this object to be affected by vertical force,
	//        namely floatables being pushed down by the player underwater.
	//BOOL allowVerticalPush = FALSE;
	

	float pushSpeedFactor = ((m_maxSpeed) / 400) + 0.1;
	
	if(pushSpeedFactor < 0.16)pushSpeedFactor = 0.16;  //no less than this allowed.
	if(pushSpeedFactor > 1.0)pushSpeedFactor = 1.0;

	
	//MODDDN NOTE - the "VARS" part below effectly makes it,
	//if(... &pevToucher->groundentity->v == pev){
	//
	//}

	// Is entity standing on this pushable ?
	if ( FBitSet(pevToucher->flags,FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev )
	{
		// Only push if floating
		//MODDD NOTICE - something can still have a waterlevel of 3, the max depth, even if the top of it is poking above the surface like the 
		//               floating barrels in c2a3 when unlocked from near the floor.
		//               The pushable table in the flooded room in c1a2 or a1a2 has a waterlevel of 1.
		if ( pev->waterlevel > 0 ){
			//MODDD - don't do this exactly, fall down to other behavior but mark a flag allowing vertical momentum for this push.
			//allowVerticalPush = TRUE;

			//No, maintain retail behavior, it's fine I guess.
			pev->velocity.z += pevToucher->velocity.z * 0.1;
			
		}else{
			//waterlevel of 0? not underwater at all? don't proceed.
			return;
		}

		//MODDD - retail behavior is to stop this method early if the toucher is touching this object from the top by being grounded to it.
		//        But jumping off this object, like the table in the flooded room, still counts as a "touch" that isn't grounded somehow. Careful.
		//        ...nevermind, let's still do it this way.
		return;
	}


	if ( pOther->IsPlayer() )
	{
		//MODDD NOTE - this means, a player making physical contact and not holding down the use or forward buttons will skip the rest of this method.
		if ( push && !(pevToucher->button & (IN_FORWARD|IN_USE)) )	// Don't push unless the player is pushing forward and NOT use (pull)
			return;
		playerTouch = 1;
	}

	float factor;

	if ( playerTouch )
	{
		if ( !(pevToucher->flags & FL_ONGROUND) )	// Don't push away from jumping/falling players unless in water
		{
			if ( pev->waterlevel < 1 )
				return;
			else 
				factor = 0.1;
		}
		else
			factor = 1;
	}
	else 
		factor = 0.25;


	//will be needed soon.
	maxSpeedTemp = MaxSpeed();

	//Hold on. Are we trying to jump off? It's possible for a frame to be not marked as grounded with this entity as the groundentity, but still be touching.
	//Check for that...
	if(push && pOther->pev->absmin.z >= this->pev->absmax.z - 8){
		return;
	}

	//apply friction?
	factor = factor * pushSpeedFactor;
	float timo = gpGlobals->time;

	if(push){
		// physical contact? rather plain.
		if(playerTouch){
			if (pOther->pev->button & IN_USE) {
				// same thing?
				//VecBModelOrigin(pev)
				//vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
				float toucherSpeed = pevToucher->velocity.Length2D();
				Vector realOrigin = VecBModelOrigin(pev);

				// old way:
				//if (toucherSpeed < 20 && toucherSpeed > 0.04) {

				// ALSO, if the player is pressing forward, and looking at me, the intent is clearly
				// to move the box forward.  Don't get stuck on pushing while too close to a box in front.
				// This happens because the box's velocity is tied to the player's velocity while 'use' is 
				// pressed, yet the box blocks the player from changing velocity towards it at all when close
				// enough.  So, detect if the player is trying to push forward on the box.  If so, give it a 
				// jolt forward and disable the 'use' velocity clone for a very short time to let the box advance
				// forward to be out of the way, then go back to copying player velocity while the player 
				// moves forward.
				// Yes... deluxe pushable logic, we have everything.
				if( (pOther->pev->button & IN_FORWARD && UTIL_IsFacing(pOther->pev, realOrigin, 0.707)) ){
					
					//::DebugLine_ClearAll();
					//DebugLine_SetupPoint(0, realOrigin + Vector(0,0,12), 0, 255, 0);
					//DebugLine_SetupPoint(1, pOther->pev->origin, 255, 0, 0);

					Vector forceIntent = ::UTIL_YawToVec(pevToucher->angles.y) * toucherSpeed * 4.3;
					pev->velocity.x += forceIntent.x; //* pushSpeedFactor;
					pev->velocity.y += forceIntent.y; //* pushSpeedFactor;

					blockForceVelocityTime = gpGlobals->time + 0.21;

					// And let the pusher get back some lost velocity.
					// If this even works.
					if (toucherSpeed < 30) {
						pOther->pev->velocity.x *= 2;
						pOther->pev->velocity.y *= 2;
					}
					else {
						pOther->pev->velocity.x *= 1.2;
						pOther->pev->velocity.y *= 1.2;
					}
						
				}
				else {
					// Not pressing forward and looking at me enough?  Normal contact push then.
					pev->velocity.x += pevToucher->velocity.x * 0.72;
					pev->velocity.y += pevToucher->velocity.y * 0.72;
					pOther->pev->velocity.x *= 1.2;
					pOther->pev->velocity.y *= 1.2;
				}
			}
			else {
				// normal way then.
				pev->velocity.x += pevToucher->velocity.x * 0.72;
				pev->velocity.y += pevToucher->velocity.y * 0.72;
				//pev->velocity.x = pevToucher->velocity.x * 1.3;
				//pev->velocity.y = pevToucher->velocity.y * 1.3;
				pOther->pev->velocity.x *= 1.2;
				pOther->pev->velocity.y *= 1.2;
			}
		}else{
			// Not the player? Use its mass to influence how far it pushes me.
			// This isn't specified for most monsters as monsters pushing this very often isn't expected.
			// But projectiles (arrows) or hornets could hit this and going flying looks kinda silly.
			pev->velocity.x += pevToucher->velocity.x * pOther->massInfluence() * pushSpeedFactor;
			pev->velocity.y += pevToucher->velocity.y * pOther->massInfluence() * pushSpeedFactor;
				
			// the other object will slow down any frame it touches me.  IF it is not another pusable.
			// And trains, they can look weird if thrown off.  This should be a virtual method like massInfluence (pushSlowdown?) if much more needs it.
			// Any other fixed moving things like this should probably be ignored for velocity changes by pushables.
			if(!FClassnameIs(pevToucher, "func_pushable") && !FClassnameIs(pevToucher, "func_train")){
				pOther->pev->velocity.x *= 0.6;
				pOther->pev->velocity.y *= 0.6;
			}
		}
	}else{ //ELSE OF push check
		// USE-KEY DRAGGING.
		if (gpGlobals->time >= blockForceVelocityTime) {
			if (useValue != 0) {
				// It is possible the player is against the box and needs to shove it a little further to start a real push than just staying stopped in front.
				// Also don't apply factor or pushSpeedFactor. Those already slow the player down.
				// Let the crate keep up with the player mostly
				Vector forceIntent;

				if (playerTouch) {
					// move as the player intended.
					// There's probably a fancier way to blend into the player velocity instead but this works.
					forceIntent = pevToucher->velocity;
				}
				else {
					// wait.. only the player can do "use"? whatever.
					forceIntent = pev->velocity + pevToucher->velocity * 0.8;
				}

				pev->velocity.x = forceIntent.x;
				pev->velocity.y = forceIntent.y;
				//pev->velocity.x += forceIntent.x * 0.6;
				//pev->velocity.y += forceIntent.y * 0.6;
				//pev->friction = 1;
			}
			else {
				// Released? STOP... mostly.
				pev->velocity.x = 0.3;
				pev->velocity.y = 0.3;
			}
		}
	}// END OF push check


	//MODDD - original. was cumulative. Above matches player velocity instead.
	/*
	pev->velocity.x += pevToucher->velocity.x * factor;
	pev->velocity.y += pevToucher->velocity.y * factor;
	*/

	length = sqrt( pev->velocity.x * pev->velocity.x + pev->velocity.y * pev->velocity.y );

	//MODD - now why do ya think we have a m_maxSpeed ???
	// Cap it!
	// WRRROOONNNNNNNG.
	// Cap to a m_maxSpeed of 1?  You want to do that?  No.
	// Just force it 400, a good upper limit for player speed.
	// (all cases of m_maxSpeed below replaced with 400)

	float actualMax = 400 * pushSpeedFactor + 40;

	if(length > actualMax){
		pev->velocity = (pev->velocity / pev->velocity.Length()) * actualMax;
		//pev->velocity = pev->velocity - pev->velocity * (pev->velocity.Length() - 400);
		length = actualMax;
	}


	//easyForcePrintLine("PUSHVEL len:%.2f phys?%d pushervel:%.2f", length, push, pevToucher->velocity.Length2D());

	//if ( push && (length > maxSpeedTemp) )
	//MODDD - always require the length check now!
	//MODDD - remove the speed length check.  The new system doesn't need this, the box can only move as fast as the player.
	//        Tossing around not supported, loses control too much.
	/*
	if ((length > maxSpeedTemp) )
	{
		pev->velocity.x = (pev->velocity.x * maxSpeedTemp / length );
		pev->velocity.y = (pev->velocity.y * maxSpeedTemp / length );
	}
	*/


	if ( playerTouch )
	{
		//MODDD - tell the player to slow down based on my friction (to simulate difficulty moving this around... heavier / more friction slows the player down much more)
		CBasePlayer* playerRef = static_cast<CBasePlayer*>( CBaseEntity::Instance(pevToucher) );
		playerRef->pushSpeedMulti = pushSpeedFactor;
		playerRef->framesUntilPushStops = 12;

		//easyForcePrintLine("SPEED FACTO: %.2f", pushSpeedFactor);
		
		
		//CBasePlayer *pl = ( CBasePlayer *) CBasePlayer::Instance( pev );
		//CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);


		//???
		//pevToucher->velocity.x = pev->velocity.x;
		//pevToucher->velocity.y = pev->velocity.y;
		if ( (gpGlobals->time - m_soundTime) > 0.7 )
		{
			m_soundTime = gpGlobals->time;
			if ( length > 0 && FBitSet(pev->flags,FL_ONGROUND) )
			{
				m_lastSound = RANDOM_LONG(0,2);
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound], 0.5, ATTN_NORM);
	//			SetThink( StopSound );
	//			pev->nextthink = pev->ltime + 0.1;
			}
			else
				UTIL_StopSound( ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound] );
		}
	}
}


#if 0
void CPushable::StopSound( void )
{
	Vector dist = pev->oldorigin - pev->origin;
	if ( dist.Length() <= 0 )
		UTIL_StopSound( ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound] );
}
#endif


void CPushable::ReportGeneric(){
	CBaseDelay::ReportGeneric();
	easyForcePrintLine("My model: %s", STRING(pev->model));
}


GENERATE_TRACEATTACK_IMPLEMENTATION(CPushable)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBreakable);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CPushable)
{

	//"IsBreakable()" is a little more accurate. matUnbreakableGlass check included in there now for pushables.
	//if ( pev->spawnflags & SF_PUSH_BREAKABLE )
	if(IsBreakable())
		return GENERATE_TAKEDAMAGE_PARENT_CALL(CBreakable);

	return 1;
}

BOOL CPushable::IsBreakable(){
	//Mark this as able to take damage if we have the SF_PUSH_BREAKABLE spawnflag. And aren't unbreakableglass as usual.
	//Pushables are, by default, invincible as they are necessary for puzzles. Or so I assume.
	//This is still not quite as accurate as isDestructibleInanimate below, as this version ignores the trigger spawnflag.
	return (m_Material != matUnbreakableGlass && pev->spawnflags & SF_PUSH_BREAKABLE);
}

//MODDD
BOOL CPushable::isBreakableOrChild(void){
    return TRUE;
}
//MODDD
BOOL CPushable::isDestructibleInanimate(void){
	//Pushables are NON-destructible by default.  Require SF_PUSH_BREAKABLE to return TRUE.
	//...However, any of the above conditions being met for breakables being indestructible (mat is
	//unbreakableGlass, or SF_BREAK_TRIGGER_ONLY is set), it looks like damage won't be taken.  So,
	//for parent conditions, FORCE us to return false.
	return (m_Material != matUnbreakableGlass && !(pev->spawnflags & SF_BREAK_TRIGGER_ONLY) ) && (pev->spawnflags & SF_PUSH_BREAKABLE);
}

float CPushable::massInfluence(void){
	return 0.82f;
}

