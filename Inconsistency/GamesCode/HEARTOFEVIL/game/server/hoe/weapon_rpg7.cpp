//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "movie_explosion.h"
#include "soundent.h"
#include "player.h"
#include "rope.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "explode.h"
#include "util.h"
#include "in_buttons.h"
#include "weapon_rpg7.h"
#include "shake.h"
#include "AI_BaseNPC.h"
#include "AI_Squad.h"
#include "te_effect_dispatch.h"
#include "triggers.h"
#include "smoke_trail.h"
#include "collisionutils.h"
#include "hl2_shareddefs.h"
#include "rumble_shared.h"

#ifdef HL2_DLL
	extern int g_interactionPlayerLaunchedRPG;
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	RPG_SPEED	1500

#ifdef RPG7_LASER

//const char *g_pLaserDotThink = "LaserThinkContext";
extern const char *g_pLaserDotThink;

//-----------------------------------------------------------------------------
// Laser Dot
//-----------------------------------------------------------------------------
class CRPG7LaserDot : public CSprite 
{
	DECLARE_CLASS( CRPG7LaserDot, CSprite );
public:

	CRPG7LaserDot( void );
	~CRPG7LaserDot( void );

	static CRPG7LaserDot *Create( const Vector &origin, CBaseEntity *pOwner = NULL, bool bVisibleDot = true );

	void	SetTargetEntity( CBaseEntity *pTarget ) { m_hTargetEnt = pTarget; }
	CBaseEntity *GetTargetEntity( void ) { return m_hTargetEnt; }

	void	SetLaserPosition( const Vector &origin, const Vector &normal );
	Vector	GetChasePosition();
	void	TurnOn( void );
	void	TurnOff( void );
	bool	IsOn() const { return m_bIsOn; }

	void	Toggle( void );

	void	LaserThink( void );

	int		ObjectCaps() { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

	void	MakeInvisible( void );

protected:
	Vector				m_vecSurfaceNormal;
	EHANDLE				m_hTargetEnt;
	bool				m_bVisibleLaserDot;
	bool				m_bIsOn;

	DECLARE_DATADESC();
public:
	CRPG7LaserDot			*m_pNext;
};

// a list of laser dots to search quickly
CEntityClassList<CRPG7LaserDot> g_LaserDotList;
CRPG7LaserDot *CEntityClassList<CRPG7LaserDot>::m_pClassList = NULL;
CRPG7LaserDot *GetLaserDotList()
{
	return g_LaserDotList.m_pClassList;
}

#endif // RPG7_LASER

BEGIN_DATADESC( CRPG7Rocket )

	DEFINE_FIELD( m_hOwner,					FIELD_EHANDLE ),
	DEFINE_FIELD( m_hRocketTrail,			FIELD_EHANDLE ),
#ifdef RPG7_LASER
	DEFINE_FIELD( m_flAugerTime,			FIELD_TIME ),
#endif
	DEFINE_FIELD( m_flMarkDeadTime,			FIELD_TIME ),
	DEFINE_FIELD( m_flGracePeriodEndsAt,	FIELD_TIME ),
	DEFINE_FIELD( m_flDamage,				FIELD_FLOAT ),
	DEFINE_FIELD( m_bCreateDangerSounds,	FIELD_BOOLEAN ),
	
	// Function Pointers
	DEFINE_FUNCTION( MissileTouch ),
	DEFINE_FUNCTION( AccelerateThink ),
	DEFINE_FUNCTION( AugerThink ),
	DEFINE_FUNCTION( IgniteThink ),
	DEFINE_FUNCTION( SeekThink ),

END_DATADESC()
LINK_ENTITY_TO_CLASS( rpg7_rocket, CRPG7Rocket );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CRPG7Rocket::CRPG7Rocket()
{
	m_hRocketTrail = NULL;
	m_bCreateDangerSounds = true; // was false -- HOE
}

CRPG7Rocket::~CRPG7Rocket()
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CRPG7Rocket::Precache( void )
{
	PrecacheModel( "models/RPG7/rpg7rocket_nam/rpg7rocket.mdl" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CRPG7Rocket::Spawn( void )
{
	Precache();

	SetSolid( SOLID_BBOX );
	SetModel("models/RPG7/rpg7rocket_nam/rpg7rocket.mdl");
	UTIL_SetSize( this, -Vector(4,4,4), Vector(4,4,4) );

	SetTouch( &CRPG7Rocket::MissileTouch );

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	SetThink( &CRPG7Rocket::IgniteThink );
	
#ifdef RPG7_LASER
	SetNextThink( gpGlobals->curtime + 0.3f );
#else
	SetNextThink( gpGlobals->curtime );
#endif
	SetDamage( 100.0f );

	m_takedamage = DAMAGE_YES;
	m_iHealth = m_iMaxHealth = 100;
	m_bloodColor = DONT_BLEED;
	m_flGracePeriodEndsAt = 0;

	AddFlag( FL_OBJECT );
}


//---------------------------------------------------------
//---------------------------------------------------------
void CRPG7Rocket::Event_Killed( const CTakeDamageInfo &info )
{
	m_takedamage = DAMAGE_NO;

	ShotDown();
}

unsigned int CRPG7Rocket::PhysicsSolidMaskForEntity( void ) const
{ 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

//---------------------------------------------------------
//---------------------------------------------------------
int CRPG7Rocket::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	if ( ( info.GetDamageType() & (DMG_MISSILEDEFENSE | DMG_AIRBOAT) ) == false )
		return 0;

	bool bIsDamaged;
	if( m_iHealth <= AugerHealth() )
	{
		// This missile is already damaged (i.e., already running AugerThink)
		bIsDamaged = true;
	}
	else
	{
		// This missile isn't damaged enough to wobble in flight yet
		bIsDamaged = false;
	}
	
	int nRetVal = BaseClass::OnTakeDamage_Alive( info );

	if( !bIsDamaged )
	{
		if ( m_iHealth <= AugerHealth() )
		{
			ShotDown();
		}
	}

	return nRetVal;
}


//-----------------------------------------------------------------------------
// Purpose: Stops any kind of tracking and shoots dumb
//-----------------------------------------------------------------------------
void CRPG7Rocket::DumbFire( void )
{
	SetThink( &CRPG7Rocket::SeekThink ); // HOE: was NULL, but want danger sounds from Huey rockets
	SetMoveType( MOVETYPE_FLY );

	SetModel("models/RPG7/rpg7rocket_nam/rpg7rocket.mdl");
	UTIL_SetSize( this, vec3_origin, vec3_origin );

#ifdef HOE_DLL
	EmitSound( "Weapon_RPG7.Rocket1" );
#else
	EmitSound( "Missile.Ignite" );
#endif

	// Smoke trail.
	CreateSmokeTrail();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRPG7Rocket::SetGracePeriod( float flGracePeriod )
{
	m_flGracePeriodEndsAt = gpGlobals->curtime + flGracePeriod;

	// Go non-solid until the grace period ends
	AddSolidFlags( FSOLID_NOT_SOLID );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CRPG7Rocket::AccelerateThink( void )
{
	Vector vecForward;

	// !!!UNDONE - make this work exactly the same as HL1 RPG, lest we have looping sound bugs again!
	EmitSound( "Missile.Accelerate" );

	// SetEffects( EF_LIGHT );

	AngleVectors( GetLocalAngles(), &vecForward );
	SetAbsVelocity( vecForward * RPG_SPEED );

	SetThink( &CRPG7Rocket::SeekThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

#define AUGER_YDEVIANCE 20.0f
#define AUGER_XDEVIANCEUP 8.0f
#define AUGER_XDEVIANCEDOWN 1.0f

//---------------------------------------------------------
//---------------------------------------------------------
void CRPG7Rocket::AugerThink( void )
{
	// If we've augered long enough, then just explode
	if ( m_flAugerTime < gpGlobals->curtime )
	{
		Explode();
		return;
	}

	if ( m_flMarkDeadTime < gpGlobals->curtime )
	{
		m_lifeState = LIFE_DYING;
	}

	QAngle angles = GetLocalAngles();

	angles.y += random->RandomFloat( -AUGER_YDEVIANCE, AUGER_YDEVIANCE );
	angles.x += random->RandomFloat( -AUGER_XDEVIANCEDOWN, AUGER_XDEVIANCEUP );

	SetLocalAngles( angles );

	Vector vecForward;

	AngleVectors( GetLocalAngles(), &vecForward );
	
	SetAbsVelocity( vecForward * 1000.0f );

	SetNextThink( gpGlobals->curtime + 0.05f );
}

//-----------------------------------------------------------------------------
// Purpose: Causes the missile to spiral to the ground and explode, due to damage
//-----------------------------------------------------------------------------
void CRPG7Rocket::ShotDown( void )
{
	CEffectData	data;
	data.m_vOrigin = GetAbsOrigin();

	DispatchEffect( "RPGShotDown", data );

	if ( m_hRocketTrail != NULL )
	{
		m_hRocketTrail->m_bDamaged = true;
	}

	SetThink( &CRPG7Rocket::AugerThink );
	SetNextThink( gpGlobals->curtime );
	m_flAugerTime = gpGlobals->curtime + 1.5f;
	m_flMarkDeadTime = gpGlobals->curtime + 0.75;

	// Let the RPG start reloading immediately
	if ( m_hOwner != NULL )
	{
#ifdef RPG7_LASER
		m_hOwner->NotifyRocketDied();
#endif
		m_hOwner = NULL;
	}
}


//-----------------------------------------------------------------------------
// The actual explosion 
//-----------------------------------------------------------------------------
void CRPG7Rocket::DoExplosion( void )
{
	// Explode
	ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), GetDamage(), CRPG7Rocket::EXPLOSION_RADIUS, 
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRPG7Rocket::Explode( void )
{
	// Don't explode against the skybox. Just pretend that 
	// the missile flies off into the distance.
	Vector forward;

	GetVectors( &forward, NULL, NULL );

	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + forward * 16, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	m_takedamage = DAMAGE_NO;
	SetSolid( SOLID_NONE );
	if( tr.fraction == 1.0 || !(tr.surface.flags & SURF_SKY) )
	{
		DoExplosion();
	}

	if( m_hRocketTrail )
	{
		m_hRocketTrail->SetLifetime(0.1f);
		m_hRocketTrail = NULL;
	}

	if ( m_hOwner != NULL )
	{
#ifdef RPG7_LASER
		m_hOwner->NotifyRocketDied();
#endif
		m_hOwner = NULL;
	}

#ifdef HOE_DLL
	StopSound( "Weapon_RPG7.Rocket1" );
#else
	StopSound( "Missile.Ignite" );
#endif
	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CRPG7Rocket::MissileTouch( CBaseEntity *pOther )
{
	Assert( pOther );
	
	// Don't touch triggers (but DO hit weapons)
	if ( pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS) && pOther->GetCollisionGroup() != COLLISION_GROUP_WEAPON )
		return;

	Explode();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRPG7Rocket::CreateSmokeTrail( void )
{
	if ( m_hRocketTrail )
		return;

	// Smoke trail.
	if ( (m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL )
	{
		m_hRocketTrail->m_Opacity = 0.2f;
		m_hRocketTrail->m_SpawnRate = 100;
		m_hRocketTrail->m_ParticleLifetime = 0.5f;
		m_hRocketTrail->m_StartColor.Init( 0.65f, 0.65f , 0.65f );
		m_hRocketTrail->m_EndColor.Init( 0.0, 0.0, 0.0 );
		m_hRocketTrail->m_StartSize = 8;
		m_hRocketTrail->m_EndSize = 32;
		m_hRocketTrail->m_SpawnRadius = 4;
		m_hRocketTrail->m_MinSpeed = 2;
		m_hRocketTrail->m_MaxSpeed = 16;
		
		m_hRocketTrail->SetLifetime( 999 );
		m_hRocketTrail->FollowEntity( this, "0" );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRPG7Rocket::IgniteThink( void )
{
	SetMoveType( MOVETYPE_FLY );
	SetModel("models/RPG7/rpg7rocket_nam/rpg7rocket.mdl");
	UTIL_SetSize( this, vec3_origin, vec3_origin );
 	RemoveSolidFlags( FSOLID_NOT_SOLID );

	//TODO: Play opening sound

	Vector vecForward;

#ifdef HOE_DLL
	EmitSound( "Weapon_RPG7.Rocket1" );
#else
	EmitSound( "Missile.Ignite" );
#endif

	AngleVectors( GetLocalAngles(), &vecForward );
	SetAbsVelocity( vecForward * RPG_SPEED );

	SetThink( &CRPG7Rocket::SeekThink );
	SetNextThink( gpGlobals->curtime );

	if ( m_hOwner && m_hOwner->GetOwner() )
	{
		CBasePlayer *pPlayer = ToBasePlayer( m_hOwner->GetOwner() );

		if ( pPlayer )
		{
			color32 white = { 255,225,205,64 };
			UTIL_ScreenFade( pPlayer, white, 0.1f, 0.0f, FFADE_IN );

			pPlayer->RumbleEffect( RUMBLE_RPG_MISSILE, 0, RUMBLE_FLAG_RESTART );

#if 1
			// Add the speed of any moving ground entity (i.e., the truck)
			CBaseEntity *pGroundEnt = pPlayer->GetGroundEntity();
			if ( pGroundEnt ) // if the truck is a func_tracktrain
				SetAbsVelocity( GetAbsVelocity() + pGroundEnt->GetAbsVelocity() );
			else if ( pPlayer->IsInAVehicle() ) // if the truck is a prop_vehicle_xxx
				SetAbsVelocity( GetAbsVelocity() + pPlayer->GetVehicleEntity()->GetAbsVelocity() );
#endif
		}
	}

	CreateSmokeTrail();
}

#ifdef RPG7_LASER

//-----------------------------------------------------------------------------
// Gets the shooting position 
//-----------------------------------------------------------------------------
void CRPG7Rocket::GetShootPosition( CRPG7LaserDot *pLaserDot, Vector *pShootPosition )
{
	if ( pLaserDot->GetOwnerEntity() != NULL )
	{
		//FIXME: Do we care this isn't exactly the muzzle position?
		*pShootPosition = pLaserDot->GetOwnerEntity()->WorldSpaceCenter();
	}
	else
	{
		*pShootPosition = pLaserDot->GetChasePosition();
	}
}

	
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#define	RPG_HOMING_SPEED	0.125f

void CRPG7Rocket::ComputeActualDotPosition( CRPG7LaserDot *pLaserDot, Vector *pActualDotPosition, float *pHomingSpeed )
{
	*pHomingSpeed = RPG_HOMING_SPEED;
	if ( pLaserDot->GetTargetEntity() )
	{
		*pActualDotPosition = pLaserDot->GetChasePosition();
		return;
	}

	Vector vLaserStart;
	GetShootPosition( pLaserDot, &vLaserStart );

	//Get the laser's vector
	Vector vLaserDir;
	VectorSubtract( pLaserDot->GetChasePosition(), vLaserStart, vLaserDir );
	
	//Find the length of the current laser
	float flLaserLength = VectorNormalize( vLaserDir );
	
	//Find the length from the missile to the laser's owner
	float flMissileLength = GetAbsOrigin().DistTo( vLaserStart );

	//Find the length from the missile to the laser's position
	Vector vecTargetToMissile;
	VectorSubtract( GetAbsOrigin(), pLaserDot->GetChasePosition(), vecTargetToMissile ); 
	float flTargetLength = VectorNormalize( vecTargetToMissile );

	// See if we should chase the line segment nearest us
	if ( ( flMissileLength < flLaserLength ) || ( flTargetLength <= 512.0f ) )
	{
		*pActualDotPosition = UTIL_PointOnLineNearestPoint( vLaserStart, pLaserDot->GetChasePosition(), GetAbsOrigin() );
		*pActualDotPosition += ( vLaserDir * 256.0f );
	}
	else
	{
		// Otherwise chase the dot
		*pActualDotPosition = pLaserDot->GetChasePosition();
	}

//	NDebugOverlay::Line( pLaserDot->GetChasePosition(), vLaserStart, 0, 255, 0, true, 0.05f );
//	NDebugOverlay::Line( GetAbsOrigin(), *pActualDotPosition, 255, 0, 0, true, 0.05f );
//	NDebugOverlay::Cross3D( *pActualDotPosition, -Vector(4,4,4), Vector(4,4,4), 255, 0, 0, true, 0.05f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRPG7Rocket::SeekThink( void )
{
	CBaseEntity	*pBestDot	= NULL;
	float		flBestDist	= MAX_TRACE_LENGTH;
	float		dotDist;

	// If we have a grace period, go solid when it ends
	if ( m_flGracePeriodEndsAt )
	{
		if ( m_flGracePeriodEndsAt < gpGlobals->curtime )
		{
			RemoveSolidFlags( FSOLID_NOT_SOLID );
			m_flGracePeriodEndsAt = 0;
		}
	}

	//Search for all dots relevant to us
	for( CRPG7LaserDot *pEnt = GetLaserDotList(); pEnt != NULL; pEnt = pEnt->m_pNext )
	{
		if ( !pEnt->IsOn() )
			continue;

		if ( pEnt->GetOwnerEntity() != GetOwnerEntity() )
			continue;

		dotDist = (GetAbsOrigin() - pEnt->GetAbsOrigin()).Length();

		//Find closest
		if ( dotDist < flBestDist )
		{
			pBestDot	= pEnt;
			flBestDist	= dotDist;
		}
	}

	if( hl2_episodic.GetBool() )
	{
		if( flBestDist <= ( GetAbsVelocity().Length() * 2.5f ) && FVisible( pBestDot->GetAbsOrigin() ) )
		{
			// Scare targets
			CSoundEnt::InsertSound( SOUND_DANGER, pBestDot->GetAbsOrigin(), CRPG7Rocket::EXPLOSION_RADIUS, 0.2f, pBestDot, SOUNDENT_CHANNEL_REPEATED_DANGER, NULL );
		}
	}

	//If we have a dot target
	if ( pBestDot == NULL )
	{
		//Think as soon as possible
		SetNextThink( gpGlobals->curtime );
		return;
	}

	CRPG7LaserDot *pLaserDot = (CRPG7LaserDot *)pBestDot;
	Vector	targetPos;

	float flHomingSpeed; 
	Vector veCRPG7LaserDotPosition;
	ComputeActualDotPosition( pLaserDot, &targetPos, &flHomingSpeed );

	if ( IsSimulatingOnAlternateTicks() )
		flHomingSpeed *= 2;

	Vector	vTargetDir;
	VectorSubtract( targetPos, GetAbsOrigin(), vTargetDir );
	float flDist = VectorNormalize( vTargetDir );

	if( pLaserDot->GetTargetEntity() != NULL && flDist <= 240.0f && hl2_episodic.GetBool() )
	{
		// Prevent the missile circling the Strider like a Halo in ep1_c17_06. If the missile gets within 20
		// feet of a Strider, tighten up the turn speed of the missile so it can break the halo and strike. (sjb 4/27/2006)
		if( pLaserDot->GetTargetEntity()->ClassMatches( "npc_strider" ) )
		{
			flHomingSpeed *= 1.75f;
		}
	}

	Vector	vDir	= GetAbsVelocity();
	float	flSpeed	= VectorNormalize( vDir );
	Vector	vNewVelocity = vDir;
	if ( gpGlobals->frametime > 0.0f )
	{
		if ( flSpeed != 0 )
		{
			vNewVelocity = ( flHomingSpeed * vTargetDir ) + ( ( 1 - flHomingSpeed ) * vDir );

			// This computation may happen to cancel itself out exactly. If so, slam to targetdir.
			if ( VectorNormalize( vNewVelocity ) < 1e-3 )
			{
				vNewVelocity = (flDist != 0) ? vTargetDir : vDir;
			}
		}
		else
		{
			vNewVelocity = vTargetDir;
		}
	}

	QAngle	finalAngles;
	VectorAngles( vNewVelocity, finalAngles );
	SetAbsAngles( finalAngles );

	vNewVelocity *= flSpeed;
	SetAbsVelocity( vNewVelocity );

	if( GetAbsVelocity() == vec3_origin )
	{
		// Strange circumstances have brought this missile to halt. Just blow it up.
		Explode();
		return;
	}

	// Think as soon as possible
	SetNextThink( gpGlobals->curtime );

#ifdef HL2_EPISODIC

	if ( m_bCreateDangerSounds == true )
	{
		trace_t tr;
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 0.5, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

		CSoundEnt::InsertSound( SOUND_DANGER, tr.endpos, 100, 0.2, this, SOUNDENT_CHANNEL_REPEATED_DANGER );
	}
#endif
}

#else

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRPG7Rocket::SeekThink( void )
{
	// If we have a grace period, go solid when it ends
	if ( m_flGracePeriodEndsAt )
	{
		if ( m_flGracePeriodEndsAt < gpGlobals->curtime )
		{
			RemoveSolidFlags( FSOLID_NOT_SOLID );
			m_flGracePeriodEndsAt = 0;
		}
	}

	if ( GetAbsVelocity() == vec3_origin )
	{
		// Strange circumstances have brought this missile to halt. Just blow it up.
		Explode();
		return;
	}

	// Think as soon as possible
	SetNextThink( gpGlobals->curtime );

#ifdef HL2_EPISODIC

	if ( m_bCreateDangerSounds == true )
	{
		trace_t tr;
#ifdef HOE_SOUND_SHAPE

		// Warn from our current position to 3 seconds straight ahead.
		Vector vecRay = GetAbsVelocity() * 3.0f;

		// Don't warn past the solid we could impact on (ignore NPCs)
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + vecRay, MASK_SOLID_BRUSHONLY,
			this, COLLISION_GROUP_NONE, &tr );
		if ( vecRay.Length() > tr.endpos.DistTo(tr.startpos) )
		{
			float scale = tr.endpos.DistTo(tr.startpos) / vecRay.Length();
			vecRay *= scale;
		}

		// This is a small radius just to have NPCs dodge the rocket.
		// NOTE: uses a sound ray not a point!
		CSoundEnt::InsertSoundRay( SOUND_DANGER, GetAbsOrigin(), vecRay,
			64, 0.2, this, SOUNDENT_CHANNEL_REPEATING );

		// Now do a full-size danger sound where we are going to impact (including NPCs).
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + vecRay * 1.1, MASK_SOLID,
			this, COLLISION_GROUP_NONE, &tr );
		if ( tr.DidHit() )
		{
			CSoundEnt::InsertSound( SOUND_DANGER, tr.endpos, CRPG7Rocket::EXPLOSION_RADIUS, 0.2, this,
				SOUNDENT_CHANNEL_REPEATED_DANGER );
		}

#else // HOE_SOUND_SHAPE
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 0.5, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

		CSoundEnt::InsertSound( SOUND_DANGER, tr.endpos, CRPG7Rocket::EXPLOSION_RADIUS, 0.2, this, SOUNDENT_CHANNEL_REPEATED_DANGER );
#endif // HOE_SOUND_SHAPE
	}
#endif
}

#endif // !RPG7_LASER

//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : &vecOrigin - 
//			&vecAngles - 
//			NULL - 
//
// Output : CRPG7Rocket
//-----------------------------------------------------------------------------
CRPG7Rocket *CRPG7Rocket::Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner = NULL )
{
	//CRPG7Rocket *pMissile = (CRPG7Rocket *)CreateEntityByName("rpg_missile" );
	CRPG7Rocket *pMissile = (CRPG7Rocket *) CBaseEntity::Create( "rpg7_rocket", vecOrigin, vecAngles, CBaseEntity::Instance( pentOwner ) );
	pMissile->SetOwnerEntity( Instance( pentOwner ) );
	pMissile->Spawn();
	pMissile->AddEffects( EF_NOSHADOW );
	
	Vector vecForward;
	AngleVectors( vecAngles, &vecForward );

	pMissile->SetAbsVelocity( vecForward * 300 /* + Vector( 0,0, 128 ) */ );

	return pMissile;
}


#define	RPG_BEAM_SPRITE		"effects/laser1_noz.vmt"
#define	RPG_LASER_SPRITE	"sprites/redglow1.vmt"

//=============================================================================
// RPG
//=============================================================================

// These bodygroups are used by the world model and viewmodel
enum
{
	RPG7_BODYGROUP_LAUNCHER = 0,
	RPG7_BODYGROUP_ROCKET
};
enum
{
	RPG7_ROCKET_SHOW = 0,
	RPG7_ROCKET_HIDE
};

BEGIN_DATADESC( CWeaponRPG7 )

	DEFINE_FIELD( m_bInitialStateUpdate,FIELD_BOOLEAN ),
#ifndef RPG7_LASER
	DEFINE_FIELD( m_bMustReload,		FIELD_BOOLEAN ),
#else
	DEFINE_FIELD( m_bGuiding,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecNPCLaserDot,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_hLaserDot,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_hMissile,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_hLaserMuzzleSprite, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hLaserBeam,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_bHideGuiding,		FIELD_BOOLEAN ),
#endif

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponRPG7, DT_WeaponRPG7)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_rpg7, CWeaponRPG7 );
PRECACHE_WEAPON_REGISTER(weapon_rpg7);

acttable_t	CWeaponRPG7::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_RPG, true },
	{ ACT_GESTURE_RELOAD, ACT_DOD_RELOAD_BAZOOKA, true },

	{ ACT_IDLE_RELAXED,				ACT_IDLE_RPG_RELAXED,			true },
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_ANGRY_RPG,				true },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_RPG,				true },

	{ ACT_IDLE,						ACT_IDLE_RPG,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_RPG,				true },
	{ ACT_WALK,						ACT_WALK_RPG,					true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RPG,			true },
	{ ACT_RUN,						ACT_RUN_RPG,					true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RPG,				true },
	{ ACT_COVER_LOW,				ACT_COVER_LOW_RPG,				true },
};

IMPLEMENT_ACTTABLE(CWeaponRPG7);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponRPG7::CWeaponRPG7()
{
	m_bReloadsSingly = true;
	m_bInitialStateUpdate= false;
#ifndef RPG7_LASER
	m_bMustReload		= false;
#else
	m_bHideGuiding = false;
	m_bGuiding = false;
#endif
	m_fMinRange1 = m_fMinRange2 = CRPG7Rocket::EXPLOSION_RADIUS * 1.25; // HOE was 40*12
	m_fMaxRange1 = m_fMaxRange2 = 500*12;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponRPG7::~CWeaponRPG7()
{
#ifdef RPG7_LASER
	if ( m_hLaserDot != NULL )
	{
		UTIL_Remove( m_hLaserDot );
		m_hLaserDot = NULL;
	}

	if ( m_hLaserMuzzleSprite )
	{
		UTIL_Remove( m_hLaserMuzzleSprite );
	}

	if ( m_hLaserBeam )
	{
		UTIL_Remove( m_hLaserBeam );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG7::Precache( void )
{
	BaseClass::Precache();

#ifdef HOE_DLL
	PrecacheScriptSound( "Weapon_RPG7.Rocket1" );
#endif
	PrecacheScriptSound( "Missile.Ignite" );
	PrecacheScriptSound( "Missile.Accelerate" );

	// Laser dot...
	PrecacheModel( "sprites/redglow1.vmt" );
	PrecacheModel( RPG_LASER_SPRITE );
	PrecacheModel( RPG_BEAM_SPRITE );

	UTIL_PrecacheOther( "rpg7_rocket" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG7::Activate( void )
{
	BaseClass::Activate();
#ifdef RPG7_LASER
	// Restore the laser pointer after transition
	if ( m_bGuiding )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		
		if ( pOwner == NULL )
			return;

		if ( pOwner->GetActiveWeapon() == this )
		{
			StartGuiding();
		}
	}
#endif
}

//-----------------------------------------------------------------------------
void CWeaponRPG7::Equip( CBaseCombatCharacter *pOwner )
{
	if ( FClassnameIs( pOwner, "npc_superzombie" ) )
	{
		m_fMinRange1 = m_fMinRange2 = 0;
	}
	else
	{
		m_fMinRange1 = m_fMinRange2 = CRPG7Rocket::EXPLOSION_RADIUS * 1.25; // HOE was 40*12
	}

	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponRPG7::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	if ( (pEvent->type & AE_TYPE_NEWEVENTSYSTEM) && (pEvent->type & AE_TYPE_SERVER) )
	{
		if ( pEvent->event == AE_SV_BODYGROUP_SET_VALUE )
		{
			char szBodygroupName[256];
			int value = 0;

			char token[256];

			const char *p = pEvent->options;

			// Bodygroup Name
			p = nexttoken(token, p, ' ');
			if ( token[0] ) // BUG IN SDK: was "if (token)" which is always true
			{
				Q_strncpy( szBodygroupName, token, sizeof(szBodygroupName) );
			}

			// Get the desired value
			p = nexttoken(token, p, ' ');
			if ( token[0] ) // BUG IN SDK: was "if (token)" which is always true
			{
				value = atoi( token );
			}

#ifdef HOE_WEAPONMODEL_FIX
			CBaseViewModel *vm = GetViewModelPtr();
			int index = vm ? vm->FindBodygroupByName( szBodygroupName ) : -1;
#else
			int index = FindBodygroupByName( szBodygroupName );
#endif
			if ( index >= 0 )
			{
				SetViewModelBodygroup( index, value );
			}
			return;
		}
	}

	switch( pEvent->event )
	{
		case EVENT_WEAPON_SMG1:
		{
#ifdef RPG7_LASER
			if ( m_hMissile != NULL )
				return;
#endif
			Vector	muzzlePoint;
			QAngle	vecAngles;

			muzzlePoint = GetOwner()->Weapon_ShootPosition();

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );

			Vector vecShootDir = npc->GetActualShootTrajectory( muzzlePoint );

			// look for a better launch location
			Vector altLaunchPoint;
			if (GetAttachment( "missile", altLaunchPoint ))
			{
				// check to see if it's relativly free
				if ( CheckForClearShot( altLaunchPoint, vecShootDir ) )
				{
					muzzlePoint = altLaunchPoint;
				}
				else DevMsg( "RPG7 'missile' attachment point blocked\n" );
			}

			VectorAngles( vecShootDir, vecAngles );

#ifndef RPG7_LASER
			CRPG7Rocket *pMissile;
			pMissile = CRPG7Rocket::Create( muzzlePoint, vecAngles, GetOwner()->edict() );		
			pMissile->m_hOwner = this;

			// NPCs always get a grace period
			pMissile->SetGracePeriod( 0.5 );

			pOperator->DoMuzzleFlash();

			WeaponSound( SINGLE_NPC );
#else
			m_hMissile = CRPG7Rocket::Create( muzzlePoint, vecAngles, GetOwner()->edict() );		
			m_hMissile->m_hOwner = this;

			// NPCs always get a grace period
			m_hMissile->SetGracePeriod( 0.5 );

			pOperator->DoMuzzleFlash();

			WeaponSound( SINGLE_NPC );

			// Make sure our laserdot is off
			m_bGuiding = false;

			if ( m_hLaserDot )
			{
				m_hLaserDot->TurnOff();
			}
#endif
			m_iClip1--; // Use a round so the NPC will reload

			// Hide the rocket
			SetBodygroup( RPG7_BODYGROUP_ROCKET, RPG7_ROCKET_HIDE );
		}
		break;

		// This event fires in the viewmodel reload animation when the rocket is inserted
		case EVENT_WEAPON_RELOAD:
			FinishReload(); m_bInReload = true;
//			WeaponSound( RELOAD );
#ifdef HOE_THIRDPERSON
			SetBodygroup( RPG7_BODYGROUP_ROCKET, RPG7_ROCKET_SHOW );
#endif
			break;

		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponRPG7::HasAnyAmmo( void )
{
#ifdef RPG7_LASER
	if ( m_hMissile != NULL )
		return true;
#endif
	return BaseClass::HasAnyAmmo();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponRPG7::WeaponShouldBeLowered( void )
{
	// Lower us if we're out of ammo
	if ( !HasAnyAmmo() )
		return true;
	
	return BaseClass::WeaponShouldBeLowered();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ConVar hoe_rpg7_muzzle_ironsight("hoe_rpg7_muzzle_ironsight", "0 8.0 -3.0");
void CWeaponRPG7::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

#ifdef RPG7_LASER
	// Can't have an active missile out
	if ( m_hMissile != NULL )
		return;
#endif
	// Can't be reloading
	if ( GetActivity() == ACT_VM_RELOAD )
		return;

	// Must have ammo
	if ( ( Clip1() <= 0 ) || ( pPlayer->GetWaterLevel() == 3 ) )
	{
//		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	if( m_bInReload )
		m_bInReload = false;

	Vector vecOrigin;
	Vector vecForward;

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	Vector	vForward, vRight, vUp;

	pOwner->EyeVectors( &vForward, &vRight, &vUp );

#ifdef HOE_IRONSIGHTS
	Vector muzzleOffset = Vector( 6.0f, 12.0f, -3.0f );
	if ( IsIronSightsActive() )
		UTIL_StringToVector( muzzleOffset.Base(), hoe_rpg7_muzzle_ironsight.GetString() );
	Vector muzzlePoint = pOwner->Weapon_ShootPosition()
		 + vRight * muzzleOffset.x + vForward * muzzleOffset.y + vUp * muzzleOffset.z;
#else
	Vector	muzzlePoint = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;
#endif

	QAngle vecAngles;
	VectorAngles( vForward, vecAngles );
#ifndef RPG7_LASER
	CRPG7Rocket *pMissile;
	pMissile = CRPG7Rocket::Create( muzzlePoint, vecAngles, GetOwner()->edict() );
	pMissile->m_hOwner = this;
#else
	m_hMissile = CRPG7Rocket::Create( muzzlePoint, vecAngles, GetOwner()->edict() );

	m_hMissile->m_hOwner = this;
#endif
	// If the shot is clear to the player, give the missile a grace period
	trace_t	tr;
	Vector vecEye = pOwner->EyePosition();
	UTIL_TraceLine( vecEye, vecEye + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction == 1.0 )
	{
#ifndef RPG7_LASER
		pMissile->SetGracePeriod( 0.3 );
#else
		m_hMissile->SetGracePeriod( 0.3 );
#endif
	}

	m_iClip1--;
//	DecrementAmmo( GetOwner() );

	// Register a muzzleflash for the AI
	pOwner->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

#ifdef HOE_DLL
	SetViewModelBodygroup( RPG7_BODYGROUP_ROCKET, RPG7_ROCKET_HIDE );
#endif
#ifdef HOE_THIRDPERSON
	SetBodygroup( RPG7_BODYGROUP_ROCKET, RPG7_ROCKET_HIDE );
#endif
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	WeaponSound( SINGLE );

#ifndef RPG7_LASER
//	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	// Signal a reload
	m_bMustReload = true;
#endif
	pOwner->RumbleEffect( RUMBLE_SHOTGUN_SINGLE, 0, RUMBLE_FLAG_RESTART );

	// Check to see if we should trigger any RPG firing triggers
	int iCount = g_hWeaponFireTriggers.Count();
	for ( int i = 0; i < iCount; i++ )
	{
		if ( g_hWeaponFireTriggers[i]->IsTouching( pOwner ) )
		{
			if ( FClassnameIs( g_hWeaponFireTriggers[i], "trigger_rpgfire" ) )
			{
				g_hWeaponFireTriggers[i]->ActivateMultiTrigger( pOwner );
			}
		}
	}
#if 0
	if( hl2_episodic.GetBool() )
	{
		CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
		int nAIs = g_AI_Manager.NumAIs();

		string_t iszStriderClassname = AllocPooledString( "npc_strider" );

		for ( int i = 0; i < nAIs; i++ )
		{
			if( ppAIs[ i ]->m_iClassname == iszStriderClassname )
			{
				ppAIs[ i ]->DispatchInteraction( g_interactionPlayerLaunchedRPG, NULL, m_hMissile );
			}
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponRPG7::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	// Take away our primary ammo type
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

#ifdef RPG7_LASER
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
void CWeaponRPG7::SuppressGuiding( bool state )
{
	m_bHideGuiding = state;

	if ( m_hLaserDot == NULL )
	{
		StartGuiding();

		//STILL!?
		if ( m_hLaserDot == NULL )
			 return;
	}

	if ( state )
	{
		m_hLaserDot->TurnOff();
	}
	else
	{
		m_hLaserDot->TurnOn();
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Override this if we're guiding a missile currently
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponRPG7::Lower( void )
{
#ifdef RPG7_LASER
	if ( m_hMissile != NULL )
		return false;
#endif
	return BaseClass::Lower();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG7::ItemPostFrame( void )
{
#ifndef RPG7_LASER
	if ( m_bMustReload && HasWeaponIdleTimeElapsed() )
	{
		Reload();
	}

	BaseClass::ItemPostFrame();
#else
	BaseClass::ItemPostFrame();

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	//If we're pulling the weapon out for the first time, wait to draw the laser
	if ( ( m_bInitialStateUpdate ) && ( GetActivity() != ACT_VM_DRAW ) )
	{
		StartGuiding();
		m_bInitialStateUpdate = false;
	}

	// Supress our guiding effects if we're lowered
	if ( GetIdealActivity() == ACT_VM_IDLE_LOWERED || GetIdealActivity() == ACT_VM_RELOAD )
	{
		SuppressGuiding();
	}
	else
	{
		SuppressGuiding( false );
	}

	//Player has toggled guidance state
	//Adrian: Players are not allowed to remove the laser guide in single player anymore, bye!
	if ( g_pGameRules->IsMultiplayer() == true )
	{
		if ( pPlayer->m_afButtonPressed & IN_ATTACK2 )
		{
			ToggleGuiding();
		}
	}

	//Move the laser
	UpdateLaserPosition();
	UpdateLaserEffects();
#endif
}

#ifdef RPG7_LASER
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Vector
//-----------------------------------------------------------------------------
Vector CWeaponRPG7::GetLaserPosition( void )
{
	CreateLaserPointer();

	if ( m_hLaserDot != NULL )
		return m_hLaserDot->GetAbsOrigin();

	//FIXME: The laser dot sprite is not active, this code should not be allowed!
	assert(0);
	return vec3_origin;
}

//-----------------------------------------------------------------------------
// Purpose: NPC RPG users cheat and directly set the laser pointer's origin
// Input  : &vecTarget - 
//-----------------------------------------------------------------------------
void CWeaponRPG7::UpdateNPCLaserPosition( const Vector &vecTarget )
{
	CreateLaserPointer();
	// Turn the laserdot on
	m_bGuiding = true;
	m_hLaserDot->TurnOn();

	Vector muzzlePoint = GetOwner()->Weapon_ShootPosition();
	Vector vecDir = (vecTarget - muzzlePoint);
	VectorNormalize( vecDir );
	vecDir = muzzlePoint + ( vecDir * MAX_TRACE_LENGTH );
	UpdateLaserPosition( muzzlePoint, vecDir );

	SetNPCLaserPosition( vecTarget );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG7::SetNPCLaserPosition( const Vector &vecTarget ) 
{ 
	m_vecNPCLaserDot = vecTarget; 
	//NDebugOverlay::Box( m_vecNPCRPG7LaserDot, -Vector(10,10,10), Vector(10,10,10), 255,0,0, 8, 3 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const Vector &CWeaponRPG7::GetNPCLaserPosition( void )
{
	return m_vecNPCLaserDot;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true if the rocket is being guided, false if it's dumb
//-----------------------------------------------------------------------------
bool CWeaponRPG7::IsGuiding( void )
{
	return m_bGuiding;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponRPG7::Deploy( void )
{
	m_bInitialStateUpdate = true;
#ifndef RPG7_LASER
	m_bMustReload = ( Clip1() <= 0 );
#endif
#ifdef HOE_DLL
	if ( BaseClass::Deploy() )
	{
		// Handle weapon loading during ItemHolsterFrame and deploying empty.
		SetViewModelBodygroup( RPG7_BODYGROUP_ROCKET, ( Clip1() <= 0 ) ?
			RPG7_ROCKET_HIDE : RPG7_ROCKET_SHOW );
#ifdef HOE_THIRDPERSON
		SetBodygroup( RPG7_BODYGROUP_ROCKET, ( Clip1() <= 0 ) ?
			RPG7_ROCKET_HIDE : RPG7_ROCKET_SHOW );
#endif
		return true;
	}
	return false;
#else
	return BaseClass::Deploy();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponRPG7::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef RPG7_LASER
	//Can't have an active missile out
	if ( m_hMissile != NULL )
		return false;
	StopGuiding();
#endif
	return BaseClass::Holster( pSwitchingTo );
}

#ifdef RPG7_LASER
//-----------------------------------------------------------------------------
// Purpose: Turn on the guiding laser
//-----------------------------------------------------------------------------
void CWeaponRPG7::StartGuiding( void )
{
	// Don't start back up if we're overriding this
	if ( m_bHideGuiding )
		return;

	m_bGuiding = true;

	WeaponSound(SPECIAL1);

	CreateLaserPointer();
	StartLaserEffects();
}

//-----------------------------------------------------------------------------
// Purpose: Turn off the guiding laser
//-----------------------------------------------------------------------------
void CWeaponRPG7::StopGuiding( void )
{
	m_bGuiding = false;

	WeaponSound( SPECIAL2 );

	StopLaserEffects();

	// Kill the dot completely
	if ( m_hLaserDot != NULL )
	{
		m_hLaserDot->TurnOff();
		UTIL_Remove( m_hLaserDot );
		m_hLaserDot = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Toggle the guiding laser
//-----------------------------------------------------------------------------
void CWeaponRPG7::ToggleGuiding( void )
{
	if ( IsGuiding() )
	{
		StopGuiding();
	}
	else
	{
		StartGuiding();
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG7::Drop( const Vector &vecVelocity )
{
#ifdef RPG7_LASER
	StopGuiding();
#endif

	BaseClass::Drop( vecVelocity );
}

#ifdef RPG7_LASER
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
extern ConVar hl2_xbox_aiming;
void CWeaponRPG7::UpdateLaserPosition( Vector vecMuzzlePos, Vector vecEndPos )
{
	if ( vecMuzzlePos == vec3_origin || vecEndPos == vec3_origin )
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if ( !pPlayer )
			return;

		vecMuzzlePos = pPlayer->Weapon_ShootPosition();
		Vector	forward;

		if( hl2_xbox_aiming.GetBool() == true && hl2_episodic.GetBool() == false )
		{
			forward = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );	
		}
		else
		{
			pPlayer->EyeVectors( &forward );
		}

		vecEndPos = vecMuzzlePos + ( forward * MAX_TRACE_LENGTH );
	}

	//Move the laser dot, if active
	trace_t	tr;
	
	// Trace out for the endpoint
	UTIL_TraceLine( vecMuzzlePos, vecEndPos, (MASK_SHOT & ~CONTENTS_WINDOW), this, COLLISION_GROUP_NONE, &tr );

	// Move the laser sprite
	if ( m_hLaserDot != NULL )
	{
		Vector	laserPos = tr.endpos;
		m_hLaserDot->SetLaserPosition( laserPos, tr.plane.normal );
		
		if ( tr.DidHitNonWorldEntity() )
		{
			CBaseEntity *pHit = tr.m_pEnt;

			if ( ( pHit != NULL ) && ( pHit->m_takedamage ) )
			{
				m_hLaserDot->SetTargetEntity( pHit );
			}
			else
			{
				m_hLaserDot->SetTargetEntity( NULL );
			}
		}
		else
		{
			m_hLaserDot->SetTargetEntity( NULL );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponRPG7::CreateLaserPointer( void )
{
	if ( m_hLaserDot != NULL )
		return;

	m_hLaserDot = CRPG7LaserDot::Create( GetAbsOrigin(), GetOwnerEntity() );
	m_hLaserDot->TurnOff();

	UpdateLaserPosition();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG7::NotifyRocketDied( void )
{
	m_hMissile = NULL;

	Reload();
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponRPG7::Reload( void )
{
#ifndef RPG7_LASER
	if ( BaseClass::Reload() )
	{
#ifdef HOE_DLLxxx // now done by .qc via animation event
		SetViewModelBodygroup( RPG7_BODYGROUP_ROCKET, RPG7_ROCKET_SHOW );
#endif
		m_bMustReload = false;
		return true;
	}

	return false;
#else
	CBaseCombatCharacter *pOwner = GetOwner();
	
	if ( pOwner == NULL )
		return false;

	if ( pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
		return false;

	WeaponSound( RELOAD );
	
	SendWeaponAnim( ACT_VM_RELOAD );

	return true;
#endif
}

//-----------------------------------------------------------------------------
bool CWeaponRPG7::CheckForClearShot( const Vector &muzzlePos, const Vector &shootDir )
{
	if ( GetOwner() == NULL )
		return false;

	CAI_BaseNPC *npcOwner = GetOwner()->MyNPCPointer();
	if ( npcOwner == NULL )
		return false;

	float hullSize = 24;

	// In a few places (alamo) Charlie can't shoot down because the giant box hits the edge he
	// is standing on.  So if shooting downwards use a smaller box.
	QAngle angShoot;
	VectorAngles( shootDir, angShoot );
	if ( angShoot.x >= 20 && angShoot.x <= 60 )
	{
		hullSize = RemapVal( angShoot.x, 20, 60, 12, 6 );
	}
//	DevMsg( "CWeaponRPG7::CheckForClearShot pitch %f\n", angShoot.x );

	// HOE: stopped working right in EP2 because owner isn't ignored???
	trace_t tr;
	AI_TraceHull( muzzlePos, muzzlePos + shootDir * (10.0f*12.0f), Vector(-hullSize),
		Vector(hullSize), MASK_NPCSOLID, npcOwner, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction != 1.0f )
	{
		// Superzombies happily shoot at point-blank range so check that we are
		// at least hitting our enemy.
		if ( FClassnameIs( npcOwner, "npc_superzombie" ) )
		{
			if ( npcOwner->GetEnemy() && tr.m_pEnt != npcOwner->GetEnemy() )
				return false;
			return true;
		}
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool CWeaponRPG7::WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
	// Note: this wasn't correct originally since rockets don't go through CONTENTS_GRATE.
	// i.e., the base class uses MASK_SHOT not MASK_SHOT_HULL.
	bool bResult = BaseClass::WeaponLOSCondition( ownerPos, targetPos, bSetConditions );

	if( bResult )
	{
		CAI_BaseNPC* npcOwner = GetOwner()->MyNPCPointer();

		if( npcOwner )
		{
			trace_t tr;

			Vector vecRelativeShootPosition;
			VectorSubtract( npcOwner->Weapon_ShootPosition(), npcOwner->GetAbsOrigin(), vecRelativeShootPosition );
			Vector vecMuzzle = ownerPos + vecRelativeShootPosition;
			Vector vecShootDir = npcOwner->GetActualShootTrajectory( vecMuzzle );

			// Make sure I have a good 10 feet of wide clearance in front, or I'll blow my teeth out.
			bResult = CheckForClearShot( vecMuzzle, vecShootDir );
		}
	}

	return bResult;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponRPG7::WeaponRangeAttack1Condition( float flDot, float flDist )
{
#ifdef RPG7_LASER
	if ( m_hMissile != NULL )
		return 0;
#endif
#ifndef HOE_DLL // why ignore Z?
	// Ignore vertical distance when doing our RPG distance calculations
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	if ( pNPC )
	{
		CBaseEntity *pEnemy = pNPC->GetEnemy();
		Vector vecToTarget = (pEnemy->GetAbsOrigin() - pNPC->GetAbsOrigin());
		vecToTarget.z = 0;
		flDist = vecToTarget.Length();
	}
#endif // HOE_DLL
	if ( flDist < min( m_fMinRange1, m_fMinRange2 ) )
		return COND_TOO_CLOSE_TO_ATTACK;

	if ( m_flNextPrimaryAttack > gpGlobals->curtime )
		return COND_NONE;

	// See if there's anyone in the way!
	CAI_BaseNPC *pOwner = GetOwner()->MyNPCPointer();
	ASSERT( pOwner != NULL );

	if( pOwner )
	{
		// Make sure I don't shoot the world!
		trace_t tr;

		Vector vecMuzzle = pOwner->Weapon_ShootPosition();
		Vector vecShootDir = pOwner->GetActualShootTrajectory( vecMuzzle );

		// Make sure I have a good 10 feet of wide clearance in front, or I'll blow my teeth out.
		if ( !CheckForClearShot( vecMuzzle, vecShootDir ) )
			return COND_WEAPON_SIGHT_OCCLUDED;
	}

#if 1
	// ---------------------------------------------------------------------
	// Are any friendlies near the intended impact area?
	// ---------------------------------------------------------------------
	CBaseEntity *pTarget = NULL;
	Vector vecTarget = pOwner->GetEnemy()->WorldSpaceCenter();

	while ( ( pTarget = gEntList.FindEntityInSphere( pTarget, vecTarget, CRPG7Rocket::EXPLOSION_RADIUS ) ) != NULL )
	{
		if ( pTarget == pOwner )
		{
			// crap, I don't want to blow myself up
			m_flNextPrimaryAttack = gpGlobals->curtime + 1; // one full second.
			return COND_TOO_CLOSE_TO_ATTACK;
		}

		if ( pOwner->IRelationType( pTarget ) == D_LI )
		{
			// crap, I might blow my own guy up. Don't attack and don't check again for a while.
			m_flNextPrimaryAttack = gpGlobals->curtime + 1; // one full second.

			return COND_WEAPON_BLOCKED_BY_FRIEND;
		}
	}
#endif

	return COND_CAN_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
void CWeaponRPG7::WeaponIdle( void )
{
	// HOE: support ACT_VM_IDLE_EMPTY
	if ( HasWeaponIdleTimeElapsed() ) 
	{
		Activity act = ACT_VM_IDLE;
#ifndef RPG7_LASER
		if ( Clip1() <= 0 )
#else
		if ( m_hMissile != NULL || !HasPrimaryAmmo() )
#endif
		{
			if ( SelectWeightedSequence( ACT_VM_IDLE_EMPTY ) != ACTIVITY_NOT_AVAILABLE )
				act = ACT_VM_IDLE_EMPTY;
		}
		SendWeaponAnim( act );
	}
}

//-----------------------------------------------------------------------------
void CWeaponRPG7::WeaponSound( WeaponSound_t sound_type, float soundtime /* = 0.0f */ )
{
	if ( sound_type == RELOAD_NPC )
		SetBodygroup( RPG7_BODYGROUP_ROCKET, RPG7_ROCKET_SHOW );

	BaseClass::WeaponSound( sound_type, soundtime );
}

//-----------------------------------------------------------------------------
void CWeaponRPG7::SetViewModel( void )
{
	BaseClass::SetViewModel();

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	CBaseViewModel *pVM = pOwner->GetViewModel( m_nViewModelIndex );
	if ( !pVM )
		return;

	pVM->m_nBody = m_nViewModelBody;
}

//-----------------------------------------------------------------------------
void CWeaponRPG7::SetViewModelBodygroup( int iGroup, int iValue )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	CBaseViewModel *pVM = pOwner->GetViewModel( m_nViewModelIndex );
	if ( !pVM )
		return;

	pVM->SetBodygroup( iGroup, iValue );

	m_nViewModelBody = pVM->m_nBody;
}

#ifdef RPG7_LASER
//-----------------------------------------------------------------------------
// Purpose: Start the effects on the viewmodel of the RPG
//-----------------------------------------------------------------------------
void CWeaponRPG7::StartLaserEffects( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
		return;

	CBaseViewModel *pBeamEnt = static_cast<CBaseViewModel *>(pOwner->GetViewModel());

	if ( m_hLaserBeam == NULL )
	{
		m_hLaserBeam = CBeam::BeamCreate( RPG_BEAM_SPRITE, 1.0f );
		
		if ( m_hLaserBeam == NULL )
		{
			// We were unable to create the beam
			Assert(0);
			return;
		}

		m_hLaserBeam->EntsInit( pBeamEnt, pBeamEnt );

		int	startAttachment = LookupAttachment( "laser" );
		int endAttachment	= LookupAttachment( "laser_end" );

		m_hLaserBeam->FollowEntity( pBeamEnt );
		m_hLaserBeam->SetStartAttachment( startAttachment );
		m_hLaserBeam->SetEndAttachment( endAttachment );
		m_hLaserBeam->SetNoise( 0 );
		m_hLaserBeam->SetColor( 255, 0, 0 );
		m_hLaserBeam->SetScrollRate( 0 );
		m_hLaserBeam->SetWidth( 0.5f );
		m_hLaserBeam->SetEndWidth( 0.5f );
		m_hLaserBeam->SetBrightness( 128 );
		m_hLaserBeam->SetBeamFlags( SF_BEAM_SHADEIN );
	}
	else
	{
		m_hLaserBeam->SetBrightness( 128 );
	}

	if ( m_hLaserMuzzleSprite == NULL )
	{
		m_hLaserMuzzleSprite = CSprite::SpriteCreate( RPG_LASER_SPRITE, GetAbsOrigin(), false );

		if ( m_hLaserMuzzleSprite == NULL )
		{
			// We were unable to create the sprite
			Assert(0);
			return;
		}

		m_hLaserMuzzleSprite->SetAttachment( pOwner->GetViewModel(), LookupAttachment( "laser" ) );
		m_hLaserMuzzleSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation );
		m_hLaserMuzzleSprite->SetBrightness( 255, 0.5f );
		m_hLaserMuzzleSprite->SetScale( 0.25f, 0.5f );
		m_hLaserMuzzleSprite->TurnOn();
	}
	else
	{
		m_hLaserMuzzleSprite->TurnOn();
		m_hLaserMuzzleSprite->SetScale( 0.25f, 0.25f );
		m_hLaserMuzzleSprite->SetBrightness( 255 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stop the effects on the viewmodel of the RPG
//-----------------------------------------------------------------------------
void CWeaponRPG7::StopLaserEffects( void )
{
	if ( m_hLaserBeam != NULL )
	{
		m_hLaserBeam->SetBrightness( 0 );
	}
	
	if ( m_hLaserMuzzleSprite != NULL )
	{
		m_hLaserMuzzleSprite->SetScale( 0.01f );
		m_hLaserMuzzleSprite->SetBrightness( 0, 0.5f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Pulse all the effects to make them more... well, laser-like
//-----------------------------------------------------------------------------
void CWeaponRPG7::UpdateLaserEffects( void )
{
	if ( !m_bGuiding )
		return;

	if ( m_hLaserBeam != NULL )
	{
		m_hLaserBeam->SetBrightness( 128 + random->RandomInt( -8, 8 ) );
	}

	if ( m_hLaserMuzzleSprite != NULL )
	{
		m_hLaserMuzzleSprite->SetScale( 0.1f + random->RandomFloat( -0.025f, 0.025f ) );
	}
}

//=============================================================================
// Laser Dot
//=============================================================================

LINK_ENTITY_TO_CLASS( rpg7_laserdot, CRPG7LaserDot );

BEGIN_DATADESC( CRPG7LaserDot )
	DEFINE_FIELD( m_vecSurfaceNormal,	FIELD_VECTOR ),
	DEFINE_FIELD( m_hTargetEnt,			FIELD_EHANDLE ),
	DEFINE_FIELD( m_bVisibleLaserDot,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bIsOn,				FIELD_BOOLEAN ),

	//DEFINE_FIELD( m_pNext, FIELD_CLASSPTR ),	// don't save - regenerated by constructor
	DEFINE_THINKFUNC( LaserThink ),
END_DATADESC()

#if 0

//-----------------------------------------------------------------------------
// Finds missiles in cone
//-----------------------------------------------------------------------------
CBaseEntity *CreateLaserDot( const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot )
{
	return CRPG7LaserDot::Create( origin, pOwner, bVisibleDot );
}

void SetLaserDotTarget( CBaseEntity *pLaserDot, CBaseEntity *pTarget )
{
	CRPG7LaserDot *pDot = assert_cast< CRPG7LaserDot* >(pLaserDot );
	pDot->SetTargetEntity( pTarget );
}

void EnableLaserDot( CBaseEntity *pLaserDot, bool bEnable )
{
	CRPG7LaserDot *pDot = assert_cast< CRPG7LaserDot* >(pLaserDot );
	if ( bEnable )
	{
		pDot->TurnOn();
	}
	else
	{
		pDot->TurnOff();
	}
}

#endif

CRPG7LaserDot::CRPG7LaserDot( void )
{
	m_hTargetEnt = NULL;
	m_bIsOn = true;
	g_LaserDotList.Insert( this );
}

CRPG7LaserDot::~CRPG7LaserDot( void )
{
	g_LaserDotList.Remove( this );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
// Output : CRPG7LaserDot
//-----------------------------------------------------------------------------
CRPG7LaserDot *CRPG7LaserDot::Create( const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot )
{
	CRPG7LaserDot *pLaserDot = (CRPG7LaserDot *) CBaseEntity::Create( "env_laserdot", origin, QAngle(0,0,0) );

	if ( pLaserDot == NULL )
		return NULL;

	pLaserDot->m_bVisibleLaserDot = bVisibleDot;
	pLaserDot->SetMoveType( MOVETYPE_NONE );
	pLaserDot->AddSolidFlags( FSOLID_NOT_SOLID );
	pLaserDot->AddEffects( EF_NOSHADOW );
	UTIL_SetSize( pLaserDot, vec3_origin, vec3_origin );

	//Create the graphic
	pLaserDot->SpriteInit( "sprites/redglow1.vmt", origin );

	pLaserDot->SetName( AllocPooledString("TEST") );

	pLaserDot->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
	pLaserDot->SetScale( 0.5f );

	pLaserDot->SetOwnerEntity( pOwner );

	pLaserDot->SetContextThink( &CRPG7LaserDot::LaserThink, gpGlobals->curtime + 0.1f, g_pLaserDotThink );
	pLaserDot->SetSimulatedEveryTick( true );

	if ( !bVisibleDot )
	{
		pLaserDot->MakeInvisible();
	}

	return pLaserDot;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRPG7LaserDot::LaserThink( void )
{
	SetNextThink( gpGlobals->curtime + 0.05f, g_pLaserDotThink );

	if ( GetOwnerEntity() == NULL )
		return;

	Vector	viewDir = GetAbsOrigin() - GetOwnerEntity()->GetAbsOrigin();
	float	dist = VectorNormalize( viewDir );

	float	scale = RemapVal( dist, 32, 1024, 0.01f, 0.5f );
	float	scaleOffs = random->RandomFloat( -scale * 0.25f, scale * 0.25f );

	scale = clamp( scale + scaleOffs, 0.1f, 32.0f );

	SetScale( scale );
}

void CRPG7LaserDot::SetLaserPosition( const Vector &origin, const Vector &normal )
{
	SetAbsOrigin( origin );
	m_vecSurfaceNormal = normal;
}

Vector CRPG7LaserDot::GetChasePosition()
{
	return GetAbsOrigin() - m_vecSurfaceNormal * 10;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRPG7LaserDot::TurnOn( void )
{
	m_bIsOn = true;
	if ( m_bVisibleLaserDot )
	{
		BaseClass::TurnOn();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRPG7LaserDot::TurnOff( void )
{
	m_bIsOn = false;
	if ( m_bVisibleLaserDot )
	{
		BaseClass::TurnOff();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRPG7LaserDot::MakeInvisible( void )
{
	BaseClass::TurnOff();
}

#endif // RPG7_LASER