#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#define PISTOL_WHIP
#ifdef PISTOL_WHIP
#include "te_effect_dispatch.h"
#include "rumble_shared.h"
#include "gamestats.h"
#endif
#ifdef HOE_THIRDPERSON
#include "hl2_player.h"
#endif // HOE_THIRDPERSON

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	PISTOL_FASTEST_REFIRE_TIME		0.75f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME	0.75f

#define	PISTOL_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

extern ConVar pistol_use_new_accuracy;

#ifdef PISTOL_WHIP
#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM,-BLUDGEON_HULL_DIM,-BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM,BLUDGEON_HULL_DIM,BLUDGEON_HULL_DIM);

ConVar sk_plr_pistolwhip( "sk_plr_pistolwhip", "0", FCVAR_REPLICATED ); 
#endif // PISTOL_WHIP

//-----------------------------------------------------------------------------
// CColt1911A1
//-----------------------------------------------------------------------------

class CColt1911A1 : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CColt1911A1, CBaseHLCombatWeapon );

	CColt1911A1(void);

	DECLARE_SERVERCLASS();

	void	Precache( void );
	void	RegisterPrivateActivities(void);
	void	ItemPostFrame( void );
	void	ItemPreFrame( void );
	void	ItemBusyFrame( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
#ifdef PISTOL_WHIP
	void	AddViewKick( bool bPrimary );
#else
	void	AddViewKick( void );
#endif
	void	DryFire( void );

	void	FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void	Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	void	UpdatePenaltyTime( void );

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	Activity	GetPrimaryAttackActivity( void );
	Activity	GetDrawActivity( void );
	Activity	GetHolsterActivity( void );
	Activity	GetIdleActivity( void );

	void	WeaponIdle( void );

	virtual bool Reload( void );

#ifdef HOE_IRONSIGHTS
	virtual bool CanToggleIronSights( void )
	{
#ifdef PISTOL_WHIP
		// The colt is deployable without ammo because of pistol-whip, but can't be ironsighted when empty
		if ( !IsIronSightsActive() && !HasPrimaryAmmo() )
			return false;
#endif
		return BaseClass::CanToggleIronSights();
	}
#endif // HOE_IRONSIGHTS

	virtual const Vector& GetBulletSpread( void )
	{		
		static const Vector coneNPC = VECTOR_CONE_6DEGREES;
		if ( GetOwner() && GetOwner()->IsNPC() )
			return coneNPC;
			
		static Vector cone;

		if ( pistol_use_new_accuracy.GetBool() )
		{
			float ramp = RemapValClamped(	m_flAccuracyPenalty, 
											0.0f, 
											PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME, 
											0.0f, 
											1.0f ); 

			// We lerp from very accurate to inaccurate over time
			VectorLerp( VECTOR_CONE_1DEGREES, VECTOR_CONE_6DEGREES, ramp, cone );
		}
		else
		{
			// Old value
			cone = VECTOR_CONE_4DEGREES;
		}

		return cone;
	}
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.5f; }

	// This prevents the weapon firing faster than its rate-of-fire.
	// CAI_ShotRegulator::Reset(false) will allow another shot as soon as the rest interval is over
	// even if the rest interval is shorter than GetFireRate().
	virtual float GetMinRestTime() { return GetFireRate(); }
	virtual float GetMaxRestTime() { return GetFireRate(); }

#ifdef PISTOL_WHIP
	virtual bool HasAnyAmmo( void );
	virtual bool Deploy( void );
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );

	float GetRange( void ) { return	64.0f; }
	float GetDamageForActivity( Activity hitActivity );

	void Swing( void );
	Activity ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner );
	bool ImpactWater( const Vector &start, const Vector &end );
	void ImpactEffect( trace_t &traceHit );
	void Hit( trace_t &traceHit, Activity nHitActivity );

	CNetworkVar( bool, m_fDrawbackStarted );
	CNetworkVar( bool, m_fDrawbackFinished );
	float m_flDrawbackStartTime;
#ifdef HOE_IRONSIGHTS
	bool m_fDrawbackWhileIronsighted;
#endif

#ifdef HOE_THIRDPERSON

#define ACT_SECONDARY_PREFIRE_STAND_COLT ACT_MP_ATTACK_STAND_MELEE_SECONDARY
#define ACT_SECONDARY_ATTACK_STAND_COLT ACT_MP_ATTACK_STAND_SECONDARYFIRE
#define ACT_SECONDARY_PREFIRE_MOVE_COLT ACT_SECONDARY_VM_PULLBACK
#define ACT_SECONDARY_ATTACK_MOVE_COLT ACT_SECONDARY_VM_SECONDARYATTACK

	CNetworkVar( bool, m_fSecondaryAttack );
	Activity ActivityOverride( Activity baseAct, bool *pRequired )
	{
		CHL2_Player *pPlayer = ToHL2Player( GetOwner() );

		if ( pPlayer && pPlayer->GetCurrentMainActivity() == ACT_RUN_RIFLE )
		{
			if ( baseAct == ACT_MP_GRENADE2_DRAW && m_fSecondaryAttack )
				return ACT_SECONDARY_PREFIRE_MOVE_COLT;
			if ( baseAct == ACT_GESTURE_RANGE_ATTACK2 && m_fSecondaryAttack )
				return ACT_SECONDARY_ATTACK_MOVE_COLT;
		}
//		if ( baseAct == ACT_RUN_AIM && m_fSecondaryAttack )
//			return ACT_HL2MP_RUN_MELEE;

		//HOE_SWITCH_GESTURE_WITH_MAIN_SEQUENCE
		if ( pPlayer && pPlayer->GetCurrentMainActivity() == ACT_IDLE )
		{
			if ( baseAct == ACT_SECONDARY_PREFIRE_MOVE_COLT )
				return ACT_SECONDARY_PREFIRE_STAND_COLT;
			if ( baseAct == ACT_SECONDARY_ATTACK_MOVE_COLT )
				return ACT_SECONDARY_ATTACK_STAND_COLT;
		}
		if ( pPlayer && pPlayer->GetCurrentMainActivity() == ACT_RUN_RIFLE )
		{
			if ( baseAct == ACT_SECONDARY_PREFIRE_STAND_COLT )
				return ACT_SECONDARY_PREFIRE_MOVE_COLT;
			if ( baseAct == ACT_SECONDARY_ATTACK_STAND_COLT )
				return ACT_SECONDARY_ATTACK_MOVE_COLT;
		}

		return BaseClass::ActivityOverride( baseAct, pRequired );
	}
#endif // HOE_THIRDPERSON

#endif // PISTOL_WHIP

	DECLARE_ACTTABLE();

private:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	float	m_flAccuracyPenalty;
	int		m_nNumShotsFired;
};


IMPLEMENT_SERVERCLASS_ST(CColt1911A1, DT_Colt1911A1)
#if defined( PISTOL_WHIP ) && defined( HOE_THIRDPERSON )
	SendPropBool( SENDINFO( m_fSecondaryAttack ) ),
#endif
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_colt1911a1, CColt1911A1 );
PRECACHE_WEAPON_REGISTER( weapon_colt1911a1 );

BEGIN_DATADESC( CColt1911A1 )

	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flLastAttackTime,		FIELD_TIME ),
	DEFINE_FIELD( m_flAccuracyPenalty,		FIELD_FLOAT ), //NOTENOTE: This is NOT tracking game time
	DEFINE_FIELD( m_nNumShotsFired,			FIELD_INTEGER ),
#ifdef PISTOL_WHIP
	DEFINE_FIELD( m_fDrawbackStarted,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fDrawbackFinished,		FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flDrawbackStartTime,	FIELD_TIME ),
#ifdef HOE_IRONSIGHTS
	DEFINE_FIELD( m_fDrawbackWhileIronsighted,		FIELD_BOOLEAN ),
#endif
#endif
#if defined( PISTOL_WHIP ) && defined( HOE_THIRDPERSON )
	DEFINE_FIELD( m_fSecondaryAttack,		FIELD_BOOLEAN ),
#endif
END_DATADESC()

static int ACT_COLT1911A1_PRIMARYATTACK_FIRELAST = 0;
static int ACT_COLT1911A1_HOLSTER_EMPTY = 0;

acttable_t CColt1911A1::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,	true },
#ifdef HOE_THIRDPERSON
	{ ACT_GESTURE_RANGE_ATTACK2,	ACT_SECONDARY_ATTACK_STAND_COLT,		true },
	{ ACT_MP_GRENADE2_DRAW,			ACT_SECONDARY_PREFIRE_STAND_COLT,	false },
#endif
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
};

IMPLEMENT_ACTTABLE( CColt1911A1 );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CColt1911A1::CColt1911A1( void )
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0.0f;

	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
#ifdef PISTOL_WHIP
	m_bAltFiresUnderwater = true;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CColt1911A1::Precache( void )
{
	BaseClass::Precache();
	RegisterPrivateActivities();
}

//-----------------------------------------------------------------------------
void CColt1911A1::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	WeaponSound( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0 );

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
void CColt1911A1::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}

//-----------------------------------------------------------------------------
void CColt1911A1::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_PISTOL_FIRE:
		{
			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator->Weapon_ShootPosition();

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );

			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );
#if 1
			FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
#else
			CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

			WeaponSound( SINGLE_NPC );
			pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
			pOperator->DoMuzzleFlash();
			m_iClip1 = m_iClip1 - 1;
#endif
		}
		break;
#ifdef PISTOL_WHIP
		case EVENT_WEAPON_SEQUENCE_FINISHED:
			m_fDrawbackFinished = true;
			m_flDrawbackStartTime = gpGlobals->curtime;
			break;
#endif
		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CColt1911A1::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flSoonestPrimaryAttack	= gpGlobals->curtime + PISTOL_FASTEST_DRY_REFIRE_TIME;
	m_flNextPrimaryAttack		= GetWeaponIdleTime();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CColt1911A1::PrimaryAttack( void )
{
	if ( ( gpGlobals->curtime - m_flLastAttackTime ) > 0.5f )
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		m_nNumShotsFired++;
	}

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_REFIRE_TIME;
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner() );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if( pOwner )
	{
		// Each time the player fires the pistol, reset the view punch. This prevents
		// the aim from 'drifting off' when the player fires very quickly. This may
		// not be the ideal way to achieve this, but it's cheap and it works, which is
		// great for a feature we're evaluating. (sjb)
		pOwner->ViewPunchReset();
	}

	BaseClass::PrimaryAttack();

	// Add an accuracy penalty which can move past our maximum penalty time if we're really spastic
	m_flAccuracyPenalty += PISTOL_ACCURACY_SHOT_PENALTY_TIME;

#ifdef PISTOL_WHIP
	m_flNextSecondaryAttack = GetWeaponIdleTime(); // end of attack animation
#endif
}

//-----------------------------------------------------------------------------
void CColt1911A1::SecondaryAttack( void )
{
#ifdef PISTOL_WHIP
	Assert( m_fDrawbackStarted == false );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

#ifdef HOE_IRONSIGHTS
	// Waiting for ironsights to exit before swinging
	if ( m_fDrawbackWhileIronsighted )
		return;
	if ( IsIronSightsActive() )
	{
		if ( CanToggleIronSights() ) // may be shooting
		{
			m_fDrawbackWhileIronsighted = true;
			ToggleIronSights();
		}
		return;
	}
#endif

	m_fDrawbackStarted = true;
#if defined( HOE_THIRDPERSON )
	m_fSecondaryAttack = true;
#endif
	// Send the anim
	SendWeaponAnim( ACT_VM_HAULBACK );

#ifdef HOE_THIRDPERSON
	ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_GRENADE2_DRAW );
#endif

#endif // PISTOL_WHIP
}

#ifdef PISTOL_WHIP
//-----------------------------------------------------------------------------
bool CColt1911A1::HasAnyAmmo( void )
{
	return true; // we can always pistol-whip
//	return BaseClass::HasAnyAmmo();
}

//-----------------------------------------------------------------------------
bool CColt1911A1::Deploy( void )
{
	m_fDrawbackStarted = false;
	m_fDrawbackFinished = false;
#ifdef HOE_IRONSIGHTS
	m_fDrawbackWhileIronsighted = false;
#endif
#if defined( HOE_THIRDPERSON )
	m_fSecondaryAttack = false;
#endif

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
bool CColt1911A1::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_fDrawbackStarted = false;
	m_fDrawbackFinished = false;
#ifdef HOE_IRONSIGHTS
	m_fDrawbackWhileIronsighted = false;
#endif
#ifdef HOE_THIRDPERSON
	m_fSecondaryAttack = false;
	if ( GetOwner() != NULL && GetOwner()->IsPlayer() )
		ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_RESET_GESTURE_SLOT, GESTURE_SLOT_ATTACK_AND_RELOAD );
#endif

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
float CColt1911A1::GetDamageForActivity( Activity hitActivity )
{
	float flDmg = sk_plr_pistolwhip.GetFloat();
	float flMult = RemapValClamped( gpGlobals->curtime - m_flDrawbackStartTime, 0, 2.0, 1.0, 3.0 );
	flDmg *= flMult;
	return flDmg;
}

//-----------------------------------------------------------------------------
void CColt1911A1::Swing( void )
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	pOwner->RumbleEffect( RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART );

	Vector swingStart = pOwner->Weapon_ShootPosition( );
	Vector forward;

	forward = pOwner->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT, GetRange() );

	Vector swingEnd = swingStart + forward * GetRange();
	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
#if 1
	Activity nHitActivity = ACT_VM_SWINGHARD;
#else
	Activity nHitActivity = ACT_VM_HITCENTER;
#endif

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( GetOwner(), GetOwner(), GetDamageForActivity( nHitActivity ), DMG_CLUB );
	triggerInfo.SetDamagePosition( traceHit.startpos );
	triggerInfo.SetDamageForce( forward );
	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, forward );

	if ( traceHit.fraction == 1.0 )
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull( swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
		if ( traceHit.fraction < 1.0 && traceHit.m_pEnt )
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize( vecToTarget );

			float dot = vecToTarget.Dot( forward );

			// YWB:  Make sure they are sort of facing the guy at least...
			if ( dot < 0.70721f )
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity( traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner );
			}
		}
	}
#if 1
	gamestats->Event_WeaponFired( pOwner, false, GetClassname() );
#else
	if ( !bIsSecondary )
	{
		m_iPrimaryAttacks++;
	} 
	else 
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired( pOwner, !bIsSecondary, GetClassname() );
#endif

	// -------------------------
	//	Miss
	// -------------------------
	if ( traceHit.fraction == 1.0f )
	{
#if 0
		nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER2 : ACT_VM_MISSCENTER;
#endif
		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange();
		
		// See if we happened to hit water
		ImpactWater( swingStart, testEnd );
	}
	else
	{
#if 1
		Hit( traceHit, nHitActivity );
#else
		Hit( traceHit, nHitActivity, bIsSecondary ? true : false );
#endif
	}

	// Send the anim
	SendWeaponAnim( nHitActivity );

#ifdef HOE_THIRDPERSON
	ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_SECONDARY );
#endif

	// Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = GetWeaponIdleTime();

	//Play swing sound
	WeaponSound( SPECIAL1 );
}

//-----------------------------------------------------------------------------
Activity CColt1911A1::ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner )
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = {mins.Base(), maxs.Base()};
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
	if ( tmpTrace.fraction == 1.0 )
	{
		for ( i = 0; i < 2; i++ )
		{
			for ( j = 0; j < 2; j++ )
			{
				for ( k = 0; k < 2; k++ )
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
					if ( tmpTrace.fraction < 1.0 )
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if ( thisDistance < distance )
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}

#if 1
	return ACT_VM_SECONDARYATTACK;
#else
	return ACT_VM_HITCENTER;
#endif
}

//-----------------------------------------------------------------------------
bool CColt1911A1::ImpactWater( const Vector &start, const Vector &end )
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...
	
	// We must start outside the water
	if ( UTIL_PointContents( start ) & (CONTENTS_WATER|CONTENTS_SLIME))
		return false;

	// We must end inside of water
	if ( !(UTIL_PointContents( end ) & (CONTENTS_WATER|CONTENTS_SLIME)))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine( start, end, (CONTENTS_WATER|CONTENTS_SLIME), GetOwner(), COLLISION_GROUP_NONE, &waterTrace );

	if ( waterTrace.fraction < 1.0f )
	{
		CEffectData	data;

		data.m_fFlags  = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if ( waterTrace.contents & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect( "watersplash", data );			
	}

	return true;
}

//-----------------------------------------------------------------------------
void CColt1911A1::ImpactEffect( trace_t &traceHit )
{
	// See if we hit water (we don't do the other impact effects in this case)
	if ( ImpactWater( traceHit.startpos, traceHit.endpos ) )
		return;

	//FIXME: need new decals
	CBaseEntity *pEnt = traceHit.m_pEnt;
	if ( pEnt && pEnt->BloodColor() != DONT_BLEED )
	{
		WeaponSound( SPECIAL2 ); // thump
		UTIL_ImpactTrace( &traceHit, DMG_BULLET );
	}
	else
		UTIL_ImpactTrace( &traceHit, DMG_SLASH );
}

//------------------------------------------------------------------------------
void CColt1911A1::Hit( trace_t &traceHit, Activity nHitActivity )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	//Do view kick
	AddViewKick( false );

	//Make sound for the AI
	CSoundEnt::InsertSound( SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer );

	// This isn't great, but it's something for when the crowbar hits.
	pPlayer->RumbleEffect( RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART );

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if ( pHitEntity != NULL )
	{
		Vector hitDirection;
		pPlayer->EyeVectors( &hitDirection, NULL, NULL );
		VectorNormalize( hitDirection );

		CTakeDamageInfo info( GetOwner(), GetOwner(), GetDamageForActivity( nHitActivity ), DMG_CLUB );

		if( pPlayer && pHitEntity->IsNPC() )
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}

		CalculateMeleeDamageForce( &info, hitDirection, traceHit.endpos );

		pHitEntity->DispatchTraceAttack( info, hitDirection, &traceHit ); 
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers( info, traceHit.startpos, traceHit.endpos, hitDirection );

		if ( ToBaseCombatCharacter( pHitEntity ) )
		{
			gamestats->Event_WeaponHit( pPlayer, false, GetClassname(), info );
		}
	}

	// Apply an impact effect
	ImpactEffect( traceHit );
}
#endif // PISTOL_WHIP

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CColt1911A1::UpdatePenaltyTime( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	// Check our penalty time decay
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp( m_flAccuracyPenalty, 0.0f, PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CColt1911A1::ItemPreFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CColt1911A1::ItemBusyFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CColt1911A1::ItemPostFrame( void )
{
#ifdef PISTOL_WHIP
	if ( m_fDrawbackStarted == false )
#endif
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
		return;
	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
		return;

#ifdef PISTOL_WHIP
#if defined( HOE_THIRDPERSON )
	if ( m_fSecondaryAttack && m_fDrawbackStarted == false && m_fDrawbackFinished == false )
	{
		if ( m_flNextSecondaryAttack <= gpGlobals->curtime )
			m_fSecondaryAttack = false;
	}
#endif

#ifdef HOE_IRONSIGHTS
	// Player alt-fired while ironsighted, wait for it to complete before swinging
	if ( m_fDrawbackWhileIronsighted && GetIronSightsState() == IRONSIGHT_STATE_NONE )
	{
		m_fDrawbackWhileIronsighted = false;
		SecondaryAttack();
		return;
	}
#endif

	if ( m_fDrawbackFinished )
	{
		if ( ( pOwner->m_nButtons & IN_ATTACK2 ) == false )
		{
			m_fDrawbackStarted = false;
			m_fDrawbackFinished = false;
			Swing();
		}
		return;
	}
	if ( m_fDrawbackStarted )
		return;
#endif

	//Allow a refire as fast as the player can click
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}
	else if ( ( pOwner->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack < gpGlobals->curtime ) && ( m_iClip1 <= 0 ) )
	{
		DryFire();
	}
}

//-----------------------------------------------------------------------------
Activity CColt1911A1::GetPrimaryAttackActivity( void )
{
#if 1
	if ( Clip1() == 1 )
		return (Activity) ACT_COLT1911A1_PRIMARYATTACK_FIRELAST;

	return BaseClass::GetPrimaryAttackActivity();
#else
	if ( m_nNumShotsFired < 1 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nNumShotsFired < 2 )
		return ACT_VM_RECOIL1;

	if ( m_nNumShotsFired < 3 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
#endif
}

//-----------------------------------------------------------------------------
Activity CColt1911A1::GetDrawActivity( void )
{
	if ( Clip1() == 0 )
		return (Activity) ACT_VM_DRAW_EMPTY;

	return ACT_VM_DRAW;
}

//-----------------------------------------------------------------------------
Activity CColt1911A1::GetHolsterActivity( void )
{
	if ( Clip1() == 0 )
		return (Activity) ACT_COLT1911A1_HOLSTER_EMPTY;

	return ACT_VM_HOLSTER;
}

//-----------------------------------------------------------------------------
bool CColt1911A1::Reload( void )
{
	bool fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), Clip1() ? ACT_VM_RELOAD : ACT_VM_RELOAD_EMPTY );
	if ( fRet )
	{
		WeaponSound( RELOAD );
		m_flAccuracyPenalty = 0.0f;
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef PISTOL_WHIP
void CColt1911A1::AddViewKick( bool bPrimary )
#else
void CColt1911A1::AddViewKick( void )
#endif
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle	viewPunch;

#ifdef PISTOL_WHIP
	if ( bPrimary )
	{
		viewPunch.x = random->RandomFloat( 0.25f, 0.5f );
		viewPunch.y = random->RandomFloat( -.6f, .6f );
		viewPunch.z = 0.0f;
	}
	else
	{
		viewPunch.x = random->RandomFloat( 1.0f, 2.0f );
		viewPunch.y = random->RandomFloat( -2.0f, -1.0f );
		viewPunch.z = 0.0f;
	}
#else
	viewPunch.x = random->RandomFloat( 0.25f, 0.5f );
	viewPunch.y = random->RandomFloat( -.6f, .6f );
	viewPunch.z = 0.0f;
#endif

	//Add it to the view punch
	pPlayer->ViewPunch( viewPunch );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CColt1911A1::RegisterPrivateActivities(void)
{
	static bool bRegistered = false;

	if (bRegistered)
		return;

	REGISTER_PRIVATE_ACTIVITY( ACT_COLT1911A1_PRIMARYATTACK_FIRELAST )
	REGISTER_PRIVATE_ACTIVITY( ACT_COLT1911A1_HOLSTER_EMPTY )

//	bRegistered = true;
}

//-----------------------------------------------------------------------------
Activity CColt1911A1::GetIdleActivity( void )
{
	if ( Clip1() == 0 )
	{
		if ( SelectWeightedSequence( ACT_VM_IDLE_EMPTY ) != ACTIVITY_NOT_AVAILABLE )
			return ACT_VM_IDLE_EMPTY;
	}

	return BaseClass::GetIdleActivity();
}

//-----------------------------------------------------------------------------
void CColt1911A1::WeaponIdle( void )
{
#ifdef PISTOL_WHIP
	if ( m_fDrawbackStarted )
		return;
#endif
#if 0
	// Support ACT_VM_IDLE_EMPTY
	if ( HasWeaponIdleTimeElapsed() ) 
	{
		Activity act = ACT_VM_IDLE;
		if ( Clip1() == 0 )
		{
			if ( SelectWeightedSequence( ACT_VM_IDLE_EMPTY ) != ACTIVITY_NOT_AVAILABLE )
				act = ACT_VM_IDLE_EMPTY;
		}
		SendWeaponAnim( act );
	}
#endif
	return BaseClass::WeaponIdle();
}
