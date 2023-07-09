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

===== generic grenade.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"


//===================grenade


LINK_ENTITY_TO_CLASS( flare, CFlare );

// Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges
#define SF_DETONATE		0x0001
#define XEN_PLANT_GLOW_SPRITE		"sprites/flare3.spr"
//
// Grenade Explode
//
void CFlare::Explode( Vector vecSrc, Vector vecAim )
{
	TraceResult tr;
	UTIL_TraceLine ( pev->origin, pev->origin + Vector ( 0, 0, -32 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}

// UNDONE: temporary scorching for PreAlpha - find a less sleazy permenant solution.
void CFlare::Explode( TraceResult *pTrace, int bitsDamageType )
{
	
	/*
//	float		flRndSound;// sound randomizer

	pev->model = iStringNull;//invisible
//	pev->solid = SOLID_NOT;// intangible
//
//	pev->takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
//	if ( pTrace->flFraction != 1.0 )
//	{
//		pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 0.6);
//	}

//	int iContents = UTIL_PointContents ( pev->origin );
	
//	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_EXPLOSION );		// This makes a dynamic light and the explosion sprites/sound
//		WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
		WRITE_COORD( pev->origin.y );
//		WRITE_COORD( pev->origin.z );
		if (iContents != CONTENTS_WATER)
		{
			WRITE_SHORT( g_sModelIndexFireball );
		}
		else
		{
			WRITE_SHORT( g_sModelIndexWExplosion );
		}
		WRITE_BYTE( (pev->dmg - 50) * .60  ); // scale * 10
		WRITE_BYTE( 15  ); // framerate
		WRITE_BYTE( TE_EXPLFLAG_NONE );
	MESSAGE_END();

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );
	entvars_t *pevOwner;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	pev->owner = NULL; // can't traceline attack owner if this is set

	RadiusDamage ( pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType );

	if ( RANDOM_FLOAT( 0 , 1 ) < 0.5 )
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH1 );
	}
	else
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH2 );
	}

	flRndSound = RANDOM_FLOAT( 0 , 1 );

	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM);	break;
		case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM);	break;
		case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM);	break;
	}
*/
//	pev->effects |= EF_NODRAW;
//	SetThink( Smoke );
//	pev->velocity = g_vecZero;
//	pev->nextthink = gpGlobals->time + 0.2;

//	if (iContents != CONTENTS_WATER)
//	{
//		int sparkCount = RANDOM_LONG(0,3);
	//	for ( int i = 0; i < sparkCount; i++ )
	//		Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL );
//	}
//	*/

					// lots of smoke
	/*
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD(  pev->origin.x );
			WRITE_COORD(  pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 12 ); // scale * 10
			WRITE_BYTE( 10 - 0 * 5); // framerate
		MESSAGE_END();


		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD( pev->origin.x);	// X
			WRITE_COORD( pev->origin.y);	// Y
			WRITE_COORD( pev->origin.z);	// Z
			WRITE_BYTE( 10 );		// radius * 0.1
			WRITE_BYTE( 0 );		// r
			WRITE_BYTE( 100 );		// g
			WRITE_BYTE( 0 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );
*/

}


void CFlare::Smoke( void )
{
	if (UTIL_PointContents ( pev->origin ) == CONTENTS_WATER)
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( (pev->dmg - 50) * 0.80 ); // scale * 10
			WRITE_BYTE( 12  ); // framerate
		MESSAGE_END();
	}
	UTIL_Remove( this );
}

void CFlare::Killed( entvars_t *pevAttacker, int iGib )
{
//	Detonate( );
}


// Timed grenade, this think is called when time runs out.
void CFlare::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
//	SetThink( Detonate );
	pev->nextthink = gpGlobals->time;
}

void CFlare::PreDetonate( void )
{
	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, 400, 0.3 );

//	SetThink( Detonate );
						// lots of smoke
	/*
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD(  pev->origin.x  );
			WRITE_COORD(  pev->origin.y  );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 12 ); // scale * 10
			WRITE_BYTE( 10 - 0 * 5); // framerate
		MESSAGE_END();


		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD( pev->origin.x);	// X
			WRITE_COORD( pev->origin.y);	// Y
			WRITE_COORD( pev->origin.z);	// Z
			WRITE_BYTE( 10 );		// radius * 0.1
			WRITE_BYTE( 0 );		// r
			WRITE_BYTE( 100 );		// g
			WRITE_BYTE( 0 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );
	pev->nextthink = gpGlobals->time + 0.1;
	*/
}


void CFlare::Detonate( void )
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT(pev), & tr);

//	Explode( &tr, DMG_BLAST );
}


//
// Contact grenade, explode when it touches something
// 
void CFlare::ExplodeTouch( CBaseEntity *pOther )
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	pev->enemy = pOther->edict();

	vecSpot = pev->origin - pev->velocity.Normalize() * 32;
	UTIL_TraceLine( vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr );

//	Explode( &tr, DMG_BLAST );
}


void CFlare::DangerSoundThink( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * 0.5, pev->velocity.Length( ), 0.2 );
	pev->nextthink = gpGlobals->time + 0.2;

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
	}
}


void CFlare::BounceTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// only do damage if we're moving fairly fast
	if (m_flNextAttack < gpGlobals->time && pev->velocity.Length() > 100)
	{
		entvars_t *pevOwner = VARS( pev->owner );
		if (pevOwner)
		{
			TraceResult tr = UTIL_GetGlobalTrace( );
			ClearMultiDamage( );
			pOther->TraceAttack(pevOwner, 1, gpGlobals->v_forward, &tr, DMG_CLUB ); 
			ApplyMultiDamage( pev, pevOwner);
		}
		m_flNextAttack = gpGlobals->time + 1.0; // debounce
	}

	Vector vecTestVelocity;
	// pev->avelocity = Vector (300, 300, 300);

	// this is my heuristic for modulating the grenade velocity because grenades dropped purely vertical
	// or thrown very far tend to slow down too quickly for me to always catch just by testing velocity. 
	// trimming the Z velocity a bit seems to help quite a bit.
	vecTestVelocity = pev->velocity; 
	vecTestVelocity.z *= 0.45;

	if ( !m_fRegisteredSound && vecTestVelocity.Length() <= 60 )
	{
		//ALERT( at_console, "Grenade Registered!: %f\n", vecTestVelocity.Length() );

		// grenade is moving really slow. It's probably very close to where it will ultimately stop moving. 
		// go ahead and emit the danger sound.
		
		// register a radius louder than the explosion, so we make sure everyone gets out of the way
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, pev->dmg / 0.4, 0.3 );
		m_fRegisteredSound = TRUE;
	}

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.8;

		pev->sequence = RANDOM_LONG( 1, 1 );
	}
	else
	{
		// play bounce sound
		BounceSound();
	}
	pev->framerate = pev->velocity.Length() / 200.0;
	if (pev->framerate > 1.0)
		pev->framerate = 1;
	else if (pev->framerate < 0.5)
		pev->framerate = 0;

}



void CFlare::SlideTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// pev->avelocity = Vector (300, 300, 300);

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;

		if (pev->velocity.x != 0 || pev->velocity.y != 0)
		{
			// maintain sliding sound
		}
	}
	else
	{
	//	BounceSound();
	}
}

void CFlare :: BounceSound( void )
{
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit1.wav", 0.25, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit2.wav", 0.25, ATTN_NORM);	break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit3.wav", 0.25, ATTN_NORM);	break;
	}
}

void CFlare :: TumbleThinkRed( void )
{
	/*
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	float sparktime;

	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 1;

	if (pev->dmgtime - 1 < gpGlobals->time)
	{
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink( Detonate );
	}
	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}

					// lots of smoke
	
	m_pGlow = CSprite::SpriteCreate( XEN_PLANT_GLOW_SPRITE, pev->origin + Vector(0,0,(pev->mins.z+pev->maxs.z)*0.5), TRUE );
	m_pGlow->SetTransparency( kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx );
	m_pGlow->SetAttachment( edict(), 1 );

	if(sparktime< gpGlobals->time)
	{
		UTIL_Sparks( pev->origin );
		UTIL_Sparks( pev->origin );
	sparktime = gpGlobals->time +0.1;
	}

		/*
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD(  pev->origin.x );
			WRITE_COORD(  pev->origin.y  );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 12 ); // scale * 10
			WRITE_BYTE( 10 - 0 * 5); // framerate
		MESSAGE_END();
*/

		//Luz Roja
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD( pev->origin.x);	// X
			WRITE_COORD( pev->origin.y);	// Y
			WRITE_COORD( pev->origin.z);	// Z
			WRITE_BYTE( 25 );		// radius * 0.1
			WRITE_BYTE( 90 );		// r
			WRITE_BYTE( 0 );		// g
			WRITE_BYTE( 0 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );





}


void CFlare :: TumbleThinkGreen( void )
{
	/*
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->dmgtime - 1 < gpGlobals->time)
	{
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink( Detonate );
	}
	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}

					// lots of smoke
	
	m_pGlow = CSprite::SpriteCreate( XEN_PLANT_GLOW_SPRITE, pev->origin + Vector(0,0,(pev->mins.z+pev->maxs.z)*0.5), TRUE );
	m_pGlow->SetTransparency( kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx );
	m_pGlow->SetAttachment( edict(), 1 );

		UTIL_Sparks( pev->origin );
		UTIL_Sparks( pev->origin );

		/*
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD(  pev->origin.x );
			WRITE_COORD(  pev->origin.y  );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 12 ); // scale * 10
			WRITE_BYTE( 10 - 0 * 5); // framerate
		MESSAGE_END();

*/
		//Luz Roja
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD( pev->origin.x);	// X
			WRITE_COORD( pev->origin.y);	// Y
			WRITE_COORD( pev->origin.z);	// Z
			WRITE_BYTE( 25 );		// radius * 0.1
			WRITE_BYTE( 0 );		// r
			WRITE_BYTE( 90 );		// g
			WRITE_BYTE( 0 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );





}


void CFlare :: TumbleThinkBlue( void )
{
	/*
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->dmgtime - 1 < gpGlobals->time)
	{
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink( Detonate );
	}
	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}

					// lots of smoke
	
	m_pGlow = CSprite::SpriteCreate( XEN_PLANT_GLOW_SPRITE, pev->origin + Vector(0,0,(pev->mins.z+pev->maxs.z)*0.5), TRUE );
	m_pGlow->SetTransparency( kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx );
	m_pGlow->SetAttachment( edict(), 1 );

		UTIL_Sparks( pev->origin );
		UTIL_Sparks( pev->origin );

		/*
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD(  pev->origin.x );
			WRITE_COORD(  pev->origin.y  );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 12 ); // scale * 10
			WRITE_BYTE( 10 - 0 * 5); // framerate
		MESSAGE_END();
*/

		//Luz azul
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD( pev->origin.x);	// X
			WRITE_COORD( pev->origin.y);	// Y
			WRITE_COORD( pev->origin.z);	// Z
			WRITE_BYTE( 25 );		// radius * 0.1
			WRITE_BYTE( 0 );		// r
			WRITE_BYTE( 0 );		// g
			WRITE_BYTE( 90 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );





}


void CFlare:: Spawn( void )
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->classname = MAKE_STRING( "flare" );
	
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/flare.mdl");
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));

	pev->dmg = 100;
	m_fRegisteredSound = FALSE;
		m_pGlow = CSprite::SpriteCreate( XEN_PLANT_GLOW_SPRITE, pev->origin + Vector(0,0,(pev->mins.z+pev->maxs.z)*0.5), TRUE );
	m_pGlow->SetTransparency( kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx );
	m_pGlow->SetAttachment( edict(), 1 );


}


CFlare *CFlare::ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CFlare *	pFlare = GetClassPtr( (CFlare *)NULL );
	pFlare->Spawn();
	// contact grenades arc lower
	pFlare->pev->gravity = 0.5;// lower gravity since grenade is aerodynamic and engine doesn't know it.
	UTIL_SetOrigin( pFlare->pev, vecStart );
	pFlare->pev->velocity = vecVelocity;
	pFlare->pev->angles = UTIL_VecToAngles (pFlare->pev->velocity);
	pFlare->pev->owner = ENT(pevOwner);
	
	// make monsters afaid of it while in the air
	pFlare->SetThink( DangerSoundThink );
	pFlare->pev->nextthink = gpGlobals->time;
	
	// Tumble in air
	pFlare->pev->avelocity.x = RANDOM_FLOAT ( -100, -500 );
	
	// Explode on contact
	pFlare->SetTouch( ExplodeTouch );

	pFlare->pev->dmg = gSkillData.plrDmgM203Grenade;

	return pFlare;
}


CFlare * CFlare:: ShootTimedBlue( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	/*
	CFlare *pFlare = GetClassPtr( (CFlare *)NULL );
	pFlare->Spawn();
	UTIL_SetOrigin( pFlare->pev, vecStart );
	pFlare->pev->velocity = vecVelocity;
//	pFlare->pev->angles = UTIL_VecToAngles(pFlare->pev->velocity);
	pFlare->pev->owner = ENT(pevOwner);
	
	pFlare->SetTouch( BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

				// lots of smoke
		UTIL_Sparks( pFlare->pev->origin );
		UTIL_Sparks( pFlare->pev->origin );
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pFlare->pev->origin.x );
			WRITE_COORD( pFlare->pev->origin.y );
			WRITE_COORD( pFlare->pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 5 ); // scale * 10
			WRITE_BYTE( 10 - 0 * 5); // framerate
		MESSAGE_END();


		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(pFlare->pev->origin.x);	// X
			WRITE_COORD(pFlare->pev->origin.y);	// Y
			WRITE_COORD(pFlare->pev->origin.z);	// Z
			WRITE_BYTE( 10 );		// radius * 0.1
			WRITE_BYTE( 0 );		// r
			WRITE_BYTE( 100 );		// g
			WRITE_BYTE( 0 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );

	pFlare->pev->dmgtime = gpGlobals->time + time;
	pFlare->SetThink( TumbleThinkBlue );
	pFlare->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pFlare->pev->nextthink = gpGlobals->time;
		pFlare->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pFlare->pev->sequence = RANDOM_LONG( 3, 6 );
	pFlare->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pFlare->pev->gravity = 0.5;
	pFlare->pev->friction = 0.8;

	SET_MODEL(ENT(pFlare->pev), "models/w_flare.mdl");
	pFlare->pev->dmg = 100;



*/
	return 0;
	
}



CFlare * CFlare:: ShootTimedRed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	/*
	CFlare *pFlare = GetClassPtr( (CFlare *)NULL );
	pFlare->Spawn();
	UTIL_SetOrigin( pFlare->pev, vecStart );
	pFlare->pev->velocity = vecVelocity;
//	pFlare->pev->angles = UTIL_VecToAngles(pFlare->pev->velocity);
	pFlare->pev->owner = ENT(pevOwner);
	
	pFlare->SetTouch( BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

				// lots of smoke
		UTIL_Sparks( pFlare->pev->origin );
		UTIL_Sparks( pFlare->pev->origin );
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pFlare->pev->origin.x );
			WRITE_COORD( pFlare->pev->origin.y );
			WRITE_COORD( pFlare->pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 5 ); // scale * 10
			WRITE_BYTE( 10 - 0 * 5); // framerate
		MESSAGE_END();


		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(pFlare->pev->origin.x);	// X
			WRITE_COORD(pFlare->pev->origin.y);	// Y
			WRITE_COORD(pFlare->pev->origin.z);	// Z
			WRITE_BYTE( 10 );		// radius * 0.1
			WRITE_BYTE( 0 );		// r
			WRITE_BYTE( 100 );		// g
			WRITE_BYTE( 0 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );

	pFlare->pev->dmgtime = gpGlobals->time + time;
	pFlare->SetThink( TumbleThinkRed );
	pFlare->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pFlare->pev->nextthink = gpGlobals->time;
		pFlare->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pFlare->pev->sequence = RANDOM_LONG( 3, 6 );
	pFlare->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pFlare->pev->gravity = 0.5;
	pFlare->pev->friction = 0.8;

	SET_MODEL(ENT(pFlare->pev), "models/w_flare.mdl");
	pFlare->pev->dmg = 100;



	
*/
	return 0;
	
}



CFlare * CFlare:: ShootTimedGreen( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{

	/*
	CFlare *pFlare = GetClassPtr( (CFlare *)NULL );
	pFlare->Spawn();
	UTIL_SetOrigin( pFlare->pev, vecStart );
	pFlare->pev->velocity = vecVelocity;
//	pFlare->pev->angles = UTIL_VecToAngles(pFlare->pev->velocity);
	pFlare->pev->owner = ENT(pevOwner);
	
	pFlare->SetTouch( BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

				// lots of smoke
		UTIL_Sparks( pFlare->pev->origin );
		UTIL_Sparks( pFlare->pev->origin );
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pFlare->pev->origin.x );
			WRITE_COORD( pFlare->pev->origin.y );
			WRITE_COORD( pFlare->pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 5 ); // scale * 10
			WRITE_BYTE( 10 - 0 * 5); // framerate
		MESSAGE_END();


		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(pFlare->pev->origin.x);	// X
			WRITE_COORD(pFlare->pev->origin.y);	// Y
			WRITE_COORD(pFlare->pev->origin.z);	// Z
			WRITE_BYTE( 10 );		// radius * 0.1
			WRITE_BYTE( 0 );		// r
			WRITE_BYTE( 100 );		// g
			WRITE_BYTE( 0 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );

	pFlare->pev->dmgtime = gpGlobals->time + time;
	pFlare->SetThink( TumbleThinkGreen );
	pFlare->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pFlare->pev->nextthink = gpGlobals->time;
		pFlare->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pFlare->pev->sequence = RANDOM_LONG( 3, 6 );
	pFlare->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pFlare->pev->gravity = 0.5;
	pFlare->pev->friction = 0.8;

	SET_MODEL(ENT(pFlare->pev), "models/w_flare.mdl");
	pFlare->pev->dmg = 100;



*/
	return 0;
	
}
