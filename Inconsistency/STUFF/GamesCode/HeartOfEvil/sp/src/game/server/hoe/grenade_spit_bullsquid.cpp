//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "grenade_spit_bullsquid.h"
#include "soundent.h"
#include "decals.h"
#include "smoke_trail.h"
#include "hl2_shareddefs.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "particle_parse.h"
#include "particle_system.h"
#include "soundenvelope.h"
#include "ai_utils.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_bullsquid_spit_grenade_dmg( "sk_bullsquid_spit_grenade_dmg", "20", FCVAR_NONE, "Total damage done by an individual bullsquid loogie.");
ConVar sk_bullsquid_spit_grenade_radius( "sk_bullsquid_spit_grenade_radius","40", FCVAR_NONE, "Radius of effect for an bullsquid spit grenade.");
ConVar sk_bullsquid_spit_grenade_poison_ratio( "sk_bullsquid_spit_grenade_poison_ratio","0.3", FCVAR_NONE, "Percentage of an bullsquid's spit damage done as poison (which regenerates)"); 

LINK_ENTITY_TO_CLASS( grenade_spit_bullsquid, CGrenadeSpitBullsquid );

BEGIN_DATADESC( CGrenadeSpitBullsquid )

	DEFINE_THINKFUNC( SpitThink ),
	DEFINE_ENTITYFUNC( GrenadeSpitTouch ),

END_DATADESC()

CGrenadeSpitBullsquid::CGrenadeSpitBullsquid( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeSpitBullsquid::Spawn( void )
{
	Precache( );
	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_FLYGRAVITY );
	SetSolidFlags( FSOLID_NOT_STANDABLE );

	SetModel( "models/spitball_large.mdl" );
	UTIL_SetSize( this, vec3_origin, vec3_origin );

	SetUse( &CBaseGrenade::DetonateUse );
	SetTouch( &CGrenadeSpitBullsquid::GrenadeSpitTouch );

	m_flDamage		= sk_bullsquid_spit_grenade_dmg.GetFloat();
	m_DmgRadius		= sk_bullsquid_spit_grenade_radius.GetFloat();
	m_takedamage	= DAMAGE_NO;
	m_iHealth		= 1;

	SetGravity( UTIL_ScaleForGravity( SPIT_GRAVITY ) );
	SetFriction( 0.8f );

	SetCollisionGroup( HL2COLLISION_GROUP_SPIT );

	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );

	// We're self-illuminating, so we don't take or give shadows
	AddEffects( EF_NOSHADOW|EF_NORECEIVESHADOW );

	// Create the dust effect in place
	m_hSpitEffect = (CParticleSystem *) CreateEntityByName( "info_particle_system" );
	if ( m_hSpitEffect != NULL )
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue( "start_active", "1" );
		m_hSpitEffect->KeyValue( "effect_name", "antlion_spit_trail" );
		m_hSpitEffect->SetParent( this );
		m_hSpitEffect->SetLocalOrigin( vec3_origin );
		DispatchSpawn( m_hSpitEffect );
		if ( gpGlobals->curtime > 0.5f )
			m_hSpitEffect->Activate();
	}

	// HOE: added a think function to detect when passing through CONTENTS_GRATE
	// as well as to remove mysteriously non-moving spits.
	SetThink( &CGrenadeSpitBullsquid::SpitThink );
	SetNextThink( gpGlobals->curtime );
	m_vecPrevPosition = vec3_origin;
}


void CGrenadeSpitBullsquid::SetSpitSize( int nSize )
{
	switch (nSize)
	{
		case SPIT_LARGE:
		{
			SetModel( "models/spitball_large.mdl" );
			break;
		}
		case SPIT_MEDIUM:
		{
			m_flDamage *= 0.5f;
			SetModel( "models/spitball_medium.mdl" );
			break;
		}
		case SPIT_SMALL:
		{
			m_flDamage *= 0.25f;
			SetModel( "models/spitball_small.mdl" );
			break;
		}
	}
}

void CGrenadeSpitBullsquid::Event_Killed( const CTakeDamageInfo &info )
{
	Detonate( );
}

//-----------------------------------------------------------------------------
void CGrenadeSpitBullsquid::SpitThink( void )
{
	if ( GetAbsVelocity() == vec3_origin )
	{
		// Strange circumstances have brought this spit to a halt. Just blow it up.
		// I see this happen when a leading spit kills an NPC which turns into a
		// server-side ragdoll; following spits seem to hit the ragdoll and come
		// to a halt in mid-air.  Or the spit is hitting the NPC's weapon...
		Detonate();
		return;
	}

	// Think again as soon as possible
	SetNextThink( gpGlobals->curtime );

	// Have we moved since spawning (or restoring from a savefile)?
	if ( m_vecPrevPosition == vec3_origin )
	{
		m_vecPrevPosition = GetAbsOrigin();
		return;
	}

	// Splatter a bit if passing through a fence.
	trace_t tr;
	UTIL_TraceEntity( this, m_vecPrevPosition, GetAbsOrigin(), CONTENTS_GRATE, &tr );
	if ( tr.DidHit() )
	{
		QAngle vecAngles;
		VectorAngles( tr.plane.normal, vecAngles );
		DispatchParticleEffect( "antlion_spit_player", tr.endpos, vecAngles );
	}

	m_vecPrevPosition = GetAbsOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: Handle spitting
//-----------------------------------------------------------------------------
void CGrenadeSpitBullsquid::GrenadeSpitTouch( CBaseEntity *pOther )
{
	if ( pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER) )
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ( ( pOther->m_takedamage == DAMAGE_NO ) || ( pOther->m_takedamage == DAMAGE_EVENTS_ONLY ) )
			return;
	}

	// Don't hit other spit
	if ( pOther->GetCollisionGroup() == HL2COLLISION_GROUP_SPIT )
		return;

	// We want to collide with water
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();

	// copy out some important things about this trace, because the first TakeDamage
	// call below may cause another trace that overwrites the one global pTrace points
	// at.
	bool bHitWater = ( ( pTrace->contents & CONTENTS_WATER ) != 0 );
	CBaseEntity *const pTraceEnt = pTrace->m_pEnt;
	const Vector tracePlaneNormal = pTrace->plane.normal;

	if ( bHitWater )
	{
		// Splash!
		CEffectData data;
		data.m_fFlags = 0;
		data.m_vOrigin = pTrace->endpos;
		data.m_vNormal = Vector( 0, 0, 1 );
		data.m_flScale = 8.0f;

		DispatchEffect( "watersplash", data );
	}
	else
	{
		// Make a splat decal
		trace_t *pNewTrace = const_cast<trace_t*>( pTrace );
		UTIL_DecalTrace( pNewTrace, "BeerSplash" );
	}

	// Part normal damage, part poison damage
	float poisonratio = sk_bullsquid_spit_grenade_poison_ratio.GetFloat();

	// Take direct damage if hit
	// NOTE: assume that pTrace is invalidated from this line forward!
	if ( pTraceEnt )
	{
		pTraceEnt->TakeDamage( CTakeDamageInfo( this, GetThrower(), m_flDamage * (1.0f-poisonratio), DMG_ACID ) );
		pTraceEnt->TakeDamage( CTakeDamageInfo( this, GetThrower(), m_flDamage * poisonratio, DMG_POISON ) );
	}

	CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin(), m_DmgRadius * 2.0f, 0.5f, GetThrower() );

	QAngle vecAngles;
	VectorAngles( tracePlaneNormal, vecAngles );
	
	if ( pOther->IsPlayer() || bHitWater )
	{
		// Do a lighter-weight effect if we just hit a player
		DispatchParticleEffect( "antlion_spit_player", GetAbsOrigin(), vecAngles );
	}
	else
	{
		DispatchParticleEffect( "antlion_spit", GetAbsOrigin(), vecAngles );
	}

	Detonate();
}

void CGrenadeSpitBullsquid::Detonate(void)
{
	m_takedamage = DAMAGE_NO;

	EmitSound( "NPC_Bullsquid.Acid" );	
	EmitSound( "NPC_Bullsquid.SpitHit" );	

	if ( m_hSpitEffect )
	{
		UTIL_Remove( m_hSpitEffect );
	}

	UTIL_Remove( this );
}

void CGrenadeSpitBullsquid::Precache( void )
{
	PrecacheModel( "models/spitball_large.mdl" ); 
	PrecacheModel("models/spitball_medium.mdl"); 
	PrecacheModel("models/spitball_small.mdl"); 

	PrecacheScriptSound( "NPC_Bullsquid.Acid" );
	PrecacheScriptSound( "NPC_Bullsquid.SpitHit" );

	PrecacheParticleSystem( "antlion_spit_player" );
	PrecacheParticleSystem( "antlion_spit" );
}
