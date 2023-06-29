#include "cbase.h"
#include "basehlcombatweapon.h"
#include "in_buttons.h"
#include "activitylist.h"
#include "gib.h"
#include "te_effect_dispatch.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "soundent.h"
#ifdef HOE_THIRDPERSON
#include "hl2_player.h"
#endif // HOE_THIRDPERSON

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_chainsaw_dmg( "sk_chainsaw_dmg", "15.0" );

#define CHAINSAW_DMG_PER_TICK sk_chainsaw_dmg.GetFloat()
#define CHAINSAW_RANGE 48.0

#define SF_CHAINSAW_LEVITATE 1024

#define CHAINSAW_LEVITATE_THINK_CONTEXT "ChainsawLevitateThinkContext"

#define CHAINSAW_LEVITATE_OFFSET 4.0f
#define CHAINSAW_ANGULAR_VELOCITY QAngle(0,25,0)

class CWeaponChainsaw : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponChainsaw, CBaseHLCombatWeapon );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponChainsaw();
	void Spawn( void );
	void Precache( void );
#ifdef PRIVATE_ACTIVITY
	void RegisterPrivateActivities( void );
#endif
	int CapabilitiesGet( void ) { return bits_CAP_WEAPON_MELEE_ATTACK1 | bits_CAP_WEAPON_MELEE_ATTACK2; }

	void PrimaryAttack( void );
	void SecondaryAttack( void );

	float GetRange( void ) { return CHAINSAW_RANGE; }

	bool Holster( CBaseCombatWeapon *pSwitchingTo );

	void ItemPostFrame( void );
	void WeaponIdle( void );

	bool TurnOn( void );
	void TurnOff( void );
	bool UseFuel( int amount );
	void CutSomeMeat( void );

	void AddViewKick( float flKick );

	enum ChainsawStatus_t
	{
		OFF,
		START_READY,
		STARTING,
		ON,
		ATTACK
	};

	ChainsawStatus_t m_iChainsawStatus;

	void SetStatus( ChainsawStatus_t status ) { m_iChainsawStatus = status; }
	ChainsawStatus_t GetStatus( void ) const { return m_iChainsawStatus; }

	void LevitateThink( void );
	void Equip( CBaseCombatCharacter *pOwner );

	int m_iFuel;
	float m_flTimeCheckDamage;
	float m_flTimeHitSound;
	float m_flLevitateZ;
	float m_flLevitateOffset;

#ifdef PRIVATE_ACTIVITY
	static Activity
		ACT_CHAINSAW_IDLE_OFF,
		ACT_CHAINSAW_START_FAIL,
		ACT_CHAINSAW_START_EMPTY,
		ACT_CHAINSAW_START_READY,
		ACT_CHAINSAW_START,
		ACT_CHAINSAW_IDLE,
		ACT_CHAINSAW_IDLE_HIT,
		ACT_CHAINSAW_ATTACK;
#endif
};

IMPLEMENT_SERVERCLASS_ST(CWeaponChainsaw, DT_WeaponChainsaw)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_chainsaw, CWeaponChainsaw );
PRECACHE_WEAPON_REGISTER(weapon_chainsaw);

BEGIN_DATADESC( CWeaponChainsaw )
	DEFINE_FIELD( m_flTimeCheckDamage, FIELD_TIME ),
	DEFINE_FIELD( m_flTimeHitSound, FIELD_TIME ),
	DEFINE_FIELD( m_iFuel, FIELD_INTEGER ),
	DEFINE_FIELD( m_iChainsawStatus, FIELD_INTEGER ),
	DEFINE_FIELD( m_flLevitateZ, FIELD_FLOAT ),
	DEFINE_FIELD( m_flLevitateOffset, FIELD_FLOAT ),
	DEFINE_THINKFUNC( LevitateThink ),
END_DATADESC()

acttable_t CWeaponChainsaw::m_acttable[] = 
{
#ifdef PRIVATE_ACTIVITY
	{ ACT_IDLE_ANGRY,				ACT_HL2MP_IDLE_PHYSGUN,					true },
	{ ACT_RUN_AIM,					ACT_HL2MP_RUN_PHYSGUN,					true },
#else
	{ ACT_IDLE_ANGRY,				ACT_CHAINSAW_IDLE_AIM,					true },
	{ ACT_RUN_AIM,					ACT_CHAINSAW_RUN,						true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_CHAINSAW_ATTACK,					true },
#endif
};
IMPLEMENT_ACTTABLE(CWeaponChainsaw);

#ifdef PRIVATE_ACTIVITY
Activity
	CWeaponChainsaw::ACT_CHAINSAW_IDLE_OFF,
	CWeaponChainsaw::ACT_CHAINSAW_START_FAIL,
	CWeaponChainsaw::ACT_CHAINSAW_START_EMPTY,
	CWeaponChainsaw::ACT_CHAINSAW_START_READY,
	CWeaponChainsaw::ACT_CHAINSAW_START,
	CWeaponChainsaw::ACT_CHAINSAW_IDLE,
	CWeaponChainsaw::ACT_CHAINSAW_IDLE_HIT,
	CWeaponChainsaw::ACT_CHAINSAW_ATTACK;
#endif

int g_interactionChainsawed = 0;

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponChainsaw::CWeaponChainsaw()
{
	// DECLARE_INTERACTION
	if ( g_interactionChainsawed == 0 )
		g_interactionChainsawed = CBaseCombatCharacter::GetInteractionID();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the weapon
//-----------------------------------------------------------------------------
void CWeaponChainsaw::Spawn( void )
{
	m_fMinRange1	= 0;
	m_fMinRange2	= 0;
	m_fMaxRange1	= 64;
	m_fMaxRange2	= 64;

	m_iFuel = 60; // 1 or 2 points used every 1/10th of a second

	BaseClass::Spawn();

	SetStatus( OFF );

	if ( HasSpawnFlags( SF_CHAINSAW_LEVITATE ) )
	{
		m_flLevitateZ = GetAbsOrigin().z;
		m_flLevitateOffset = CHAINSAW_LEVITATE_OFFSET;
		VPhysicsDestroyObject();
		SetMoveType( MOVETYPE_FLY );
		SetLocalAngularVelocity( CHAINSAW_ANGULAR_VELOCITY );
		SetContextThink( &CWeaponChainsaw::LevitateThink, gpGlobals->curtime, CHAINSAW_LEVITATE_THINK_CONTEXT );
	}
}

//-----------------------------------------------------------------------------
void CWeaponChainsaw::Precache( void )
{
#ifdef PRIVATE_ACTIVITY
	RegisterPrivateActivities();
#endif

	PrecacheModel( "models/stickygib.mdl" );

	PrecacheScriptSound( "Weapon_Chainsaw.Gibs" );

	BaseClass::Precache();
}

#ifdef PRIVATE_ACTIVITY
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponChainsaw::RegisterPrivateActivities( void )
{
	static bool bRegistered = false;

	if (bRegistered)
		return;

	REGISTER_PRIVATE_ACTIVITY( ACT_CHAINSAW_IDLE_OFF );
	REGISTER_PRIVATE_ACTIVITY( ACT_CHAINSAW_START_FAIL );
	REGISTER_PRIVATE_ACTIVITY( ACT_CHAINSAW_START_EMPTY );
	REGISTER_PRIVATE_ACTIVITY( ACT_CHAINSAW_START_READY );
	REGISTER_PRIVATE_ACTIVITY( ACT_CHAINSAW_START );
	REGISTER_PRIVATE_ACTIVITY( ACT_CHAINSAW_IDLE );
	REGISTER_PRIVATE_ACTIVITY( ACT_CHAINSAW_IDLE_HIT );
	REGISTER_PRIVATE_ACTIVITY( ACT_CHAINSAW_ATTACK );

//	bRegistered = true;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponChainsaw::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL )
		return;

	if ( !TurnOn() )
		return;

	WeaponSound( SINGLE );
	SendWeaponAnim( ACT_CHAINSAW_ATTACK );
	m_flNextPrimaryAttack = GetWeaponIdleTime();
//	SetWeaponIdleTime( m_flNextPrimaryAttack );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
#ifdef HOE_THIRDPERSON
	ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
#endif

	Assert( GetStatus() == ON );
	SetStatus( ATTACK );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponChainsaw::SecondaryAttack( void )
{
	if ( GetStatus() == OFF )
		TurnOn();
	else if ( GetStatus() > STARTING )
	{
		TurnOff();
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.4f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponChainsaw::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( GetStatus() != OFF )
	{
		StopWeaponSound( SINGLE ); // stop "attack" sound
		StopWeaponSound( SPECIAL1 ); // stop "bad-start" sound
		StopWeaponSound( SPECIAL2 ); // stop "idle" sound
		StopWeaponSound( WPN_DOUBLE ); // stop "start" sound

#ifdef HOE_THIRDPERSON
		ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_RESET_GESTURE_SLOT, GESTURE_SLOT_CUSTOM );
#endif
	}

	SetStatus( OFF );
	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponChainsaw::TurnOn( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL )
		return false;

	// No gas
	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return false;

	// Already running
	if ( GetStatus() >= ON )
		return true;

	// Starting up
	if ( GetStatus() >= START_READY )
		return false;

	// Reach for the ripcord
	SendWeaponAnim( ACT_CHAINSAW_START_READY );
//	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
#ifdef HOE_THIRDPERSON
	ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_CUSTOM_GESTURE, ACT_CHAINSAW_START_READY );
#endif

	m_flNextSecondaryAttack = GetWeaponIdleTime();

	SetStatus( START_READY );

	// Return false because we can't attack until startup is complete
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponChainsaw::TurnOff( void )
{
	Assert( GetStatus() != OFF );
	SendWeaponAnim( ACT_CHAINSAW_IDLE_OFF );
//	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
	SetStatus( OFF );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponChainsaw::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
		return;

	// Check for animations ending
	WeaponIdle();

	if ( (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime) )
	{
		PrimaryAttack();
	}
	else if ( (pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime) )
	{
		SecondaryAttack();
	}

	// Shut it off if the owner is in deep water
	if ( GetStatus() != OFF && pOwner->GetWaterLevel() == 3 )
	{
		TurnOff();
	}

	// Use gas and do damage every 1/10th of a second
	if ( GetStatus() != OFF && m_flTimeCheckDamage <= gpGlobals->curtime )
	{
		if ( GetStatus() > STARTING )
			CutSomeMeat();

		m_flTimeCheckDamage = gpGlobals->curtime + 0.1;

		// Constant combat sound while the chainsaw is running. No sneaking up on zombies.
		CSoundEnt::InsertSound( SOUND_COMBAT | SOUND_CONTEXT_REACT_TO_SOURCE,
			GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, GetOwner(),
			SOUNDENT_CHANNEL_REPEATING );

		int fuel = ( GetStatus() == ATTACK ) ? 2 : 1;
		if ( !UseFuel( fuel ) )
		{
			TurnOff();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponChainsaw::WeaponIdle( void )
{
	// Ready to switch animations?
 	if ( !HasWeaponIdleTimeElapsed() )
		return;

	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return;

	// Reached for the ripcord
	if ( GetStatus() == START_READY )
	{
		// Must have fuel and not be under water
		if ( ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 ) || ( pOwner->GetWaterLevel() == 3 ) )
		{
			WeaponSound( EMPTY );
			SendWeaponAnim( ACT_CHAINSAW_START_EMPTY );
#ifdef HOE_THIRDPERSON
			ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_CUSTOM_GESTURE, ACT_CHAINSAW_START_EMPTY );
#endif
			SetStatus( OFF );

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetWeaponIdleTime();
//			SetWeaponIdleTime( m_flNextPrimaryAttack );
		}

		// Randomly fail to start
		else if (random->RandomInt(0, 4) == 0)
		{
			WeaponSound( SPECIAL1 );
			SendWeaponAnim( ACT_CHAINSAW_START_FAIL );
#ifdef HOE_THIRDPERSON
			ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_CUSTOM_GESTURE, ACT_CHAINSAW_START_FAIL );
#endif
//			SetStatus( OFF );

//			m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetWeaponIdleTime();
//			SetWeaponIdleTime( m_flNextPrimaryAttack );
		}

		// Begin starting up
		else
		{
			WeaponSound( WPN_DOUBLE );
			SendWeaponAnim( ACT_CHAINSAW_START );
#ifdef HOE_THIRDPERSON
			ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_CUSTOM_GESTURE, ACT_CHAINSAW_START );
#endif
			SetStatus( STARTING );

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetWeaponIdleTime();
			m_flTimeCheckDamage = gpGlobals->curtime; // start using gas now
//			SetWeaponIdleTime( m_flNextPrimaryAttack );
		}
	}

	// See if the attack animation ended
	else if ( GetStatus() == ATTACK )
	{
		WeaponSound( SPECIAL2 ); // idle
		SendWeaponAnim( ACT_CHAINSAW_IDLE );
//		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
		SetStatus( ON );
	}

	// Finished starting up
	else if ( GetStatus() == STARTING )
	{
		WeaponSound( SPECIAL2 ); // idle
		SendWeaponAnim( ACT_CHAINSAW_IDLE );
//		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
		SetStatus( ON );
	}

	// Idle again
	else if ( GetStatus() == ON )
	{
		WeaponSound( SPECIAL2 ); // idle
		SendWeaponAnim( ACT_CHAINSAW_IDLE );
//		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
	}

	// Idle off
	else
	{
		SendWeaponAnim( ACT_CHAINSAW_IDLE_OFF );
//		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
	}

//	BaseClass::WeaponIdle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponChainsaw::UseFuel( int amount )
{
	Assert( GetStatus() != OFF );

	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;

	if ( m_iFuel < amount )
	{
		pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
		m_iFuel += 60 - amount;
	}
	else
	{
		m_iFuel -= amount;
	}

	return pOwner->GetAmmoCount( m_iPrimaryAmmoType ) > 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponChainsaw::CutSomeMeat( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return;

	Vector vecStart = pOwner->Weapon_ShootPosition( );
	Vector forward;
//	forward = pOwner->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT, GetRange() );
	AngleVectors( pOwner->GetLocalAngles(), &forward );
	Vector vecEnd = vecStart + forward * GetRange();
	trace_t traceHit;
	UTIL_TraceLine( vecStart, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );

	// Damage every 1/10th of a second
	float flDamage = CHAINSAW_DMG_PER_TICK;
	if ( GetStatus() == ATTACK )
		flDamage *= 2.0;
#if 0
	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( GetOwner(), GetOwner(), flDamage, DMG_SLASH );
	triggerInfo.SetDamagePosition( traceHit.startpos );
	triggerInfo.SetDamageForce( forward );
	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, forward );
#endif

	int damageType = DMG_SLASH; // apply slash to everything except bleeders

	if ( traceHit.fraction < 1.0 )
	{
		CBaseEntity	*pHitEntity = traceHit.m_pEnt;

		// Apply damage to a hit target
		if ( pHitEntity != NULL )
		{
			CBaseCombatCharacter *pBCC = pHitEntity->MyCombatCharacterPointer();
			if ( pBCC != NULL )
			{
				pBCC->DispatchInteraction( g_interactionChainsawed, NULL, pOwner );
			}

			CTakeDamageInfo info( GetOwner(), GetOwner(), flDamage, DMG_SLASH );

			if ( pOwner->IsPlayer() && pHitEntity->IsNPC() )
			{
				// If bonking an NPC, adjust damage.
				info.AdjustPlayerDamageInflictedForSkillLevel();
			}


			CalculateMeleeDamageForce( &info, forward, traceHit.endpos );

#if 0
			extern bool g_bMacheteOrChainsawAttack;
			g_bMacheteOrChainsawAttack = true; // hack on
#endif
			pHitEntity->DispatchTraceAttack( info, forward, &traceHit ); 
#if 0
			g_bMacheteOrChainsawAttack = false; // hack off
#endif
			ApplyMultiDamage();

			if ( pHitEntity->BloodColor() == DONT_BLEED )
			{
#if 0
				CEffectData data;
				data.m_vOrigin = traceHit.endpos;
				data.m_vAngles = GetAbsAngles();
				data.m_vNormal = ( traceHit.plane.normal /* + velocity */ ) * 0.5;
				DispatchEffect( "ManhackSparks", data );
#endif
			}
			else
			{
				if ( m_flTimeHitSound < gpGlobals->curtime )
				{
					CSoundParameters params;
					if ( GetParametersForSound( "Weapon_Chainsaw.Gibs", params, NULL ) )
					{
						float flDuration, soundtime = 0.0f;
						// WeaponSound() doesn't return the duration
						CPASAttenuationFilter filter( this, params.soundlevel );
						EmitSound( filter, entindex(), "Weapon_Chainsaw.Gibs", NULL, soundtime, &flDuration);
						m_flTimeHitSound = gpGlobals->curtime + flDuration;
					}
				}

				// Gibs
				if ( !random->RandomInt(0, 4) )
				{
					CGib::SpawnStickyGibs( pHitEntity, traceHit.endpos, 1 );
				}
				// TraceAttack() spawns some blood
//				SpawnBlood( traceHit.endpos, g_vecAttackDir, pHitEntity->BloodColor(), 6 );

				// This applies blood instead of slash
				damageType = DMG_BULLET;
			}

			// Now hit all triggers along the ray that... 
			TraceAttackToTriggers( info, traceHit.startpos, traceHit.endpos, forward );
		}

		// Apply an impact effect
//		ImpactEffect( traceHit );
		UTIL_ImpactTrace( &traceHit, damageType );

		// Do jiggery animation
		if ( ( GetActivity() == ACT_CHAINSAW_IDLE ) || 
			( GetActivity() == ACT_CHAINSAW_IDLE_HIT && HasWeaponIdleTimeElapsed() ) )
		{
			SendWeaponAnim( ACT_CHAINSAW_IDLE_HIT );
//			SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
#ifdef HOE_THIRDPERSON
			ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_CUSTOM_GESTURE, ACT_CHAINSAW_IDLE_HIT );
#endif
		}

		// Screen shake
		AddViewKick( 2.0 );
	}
	else
	{
		// Not hitting anything

		// Minor screen shake
		AddViewKick( 0.3 );

		if ( m_flTimeHitSound >= gpGlobals->curtime )
		{
			// Make the gibs or screech noise stop
			StopSound( entindex(), "Weapon_Chainsaw.Gibs" ); // see StopWeaponSound
			m_flTimeHitSound = gpGlobals->curtime;
		}

		// Make the jiggery animation stop
		if ( GetActivity() == ACT_CHAINSAW_IDLE_HIT )
		{
			SetWeaponIdleTime( gpGlobals->curtime );
		}
	}
}

//-----------------------------------------------------------------------------
void CWeaponChainsaw::AddViewKick( float flKick )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat( -flKick, flKick );
	punchAng.y = 0.0f;
	punchAng.z = random->RandomFloat( -flKick, flKick );
	
	pPlayer->ViewPunch( punchAng ); 
}

//-----------------------------------------------------------------------------
void CWeaponChainsaw::LevitateThink( void )
{
	if ( !UTIL_FindClientInPVS( edict() ) )
	{
		SetNextThink( gpGlobals->curtime + random->RandomFloat( 1.0f, 1.5f ), CHAINSAW_LEVITATE_THINK_CONTEXT );
		return;
	}

	float flHeight = GetAbsOrigin().z;
	float flDeltaZ = m_flLevitateZ + m_flLevitateOffset - flHeight;
	Vector velocity = GetAbsVelocity();
	if ( flDeltaZ > 0 )
	{
		velocity.z += 5.0;
		velocity.z = min( velocity.z, 10 );
		m_flLevitateOffset = CHAINSAW_LEVITATE_OFFSET;
	}
	else
	{
		velocity.z -= 5.0;
		velocity.z = max( velocity.z, -10 );
		m_flLevitateOffset = -CHAINSAW_LEVITATE_OFFSET;
	}
	SetAbsVelocity( velocity );

	// If something touches the chainsaw it stops rotating
	SetLocalAngularVelocity( CHAINSAW_ANGULAR_VELOCITY );

	SetNextThink( gpGlobals->curtime + 0.1, CHAINSAW_LEVITATE_THINK_CONTEXT );
}

//-----------------------------------------------------------------------------
void CWeaponChainsaw::Equip( CBaseCombatCharacter *pOwner )
{
	SetContextThink( NULL, TICK_NEVER_THINK, CHAINSAW_LEVITATE_THINK_CONTEXT );

	SetMoveType( MOVETYPE_VPHYSICS );

	BaseClass::Equip( pOwner );
}
