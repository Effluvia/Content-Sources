//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "NPCevent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "grenade_ar2.h"
#include "AI_Memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "hoe_human.h" // hack for telling friends about grenade impact

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar    sk_plr_9mmAR_grenade;	
extern ConVar    sk_npc_9mmAR_grenade;	

class CWeaponM79 : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponM79, CBaseHLCombatWeapon );

	CWeaponM79();

	DECLARE_SERVERCLASS();
	
	void	Precache( void );
	void	AddViewKick( void );
	void	PrimaryAttack( void );

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 1; }

	void Equip( CBaseCombatCharacter *pOwner );
	bool Deploy( void );
	bool Reload( void );
	void ItemPostFrame( void );

	float	GetFireRate( void ) { return 3.0f; } // viewmodel reload anim takes 3 seconds
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	bool	CheckThrowFromPosition( const Vector &shootPos, const Vector &targetPos, Vector *pVecOut );
	bool	WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
	int		WeaponRangeAttack1Condition( float flDot, float flDist );
	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_5DEGREES; // FIXME
		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	Vector PredictImpactPosition( const Vector& origin, const Vector& velocity );

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
	float	m_flNextGrenadeToss;
	bool	m_bMustReload;
	bool	m_bFriendInBlastZone;

	friend bool M79FriendInBlastZone( CBaseCombatWeapon *pWeapon );
};

IMPLEMENT_SERVERCLASS_ST(CWeaponM79, DT_WeaponM79)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_m79, CWeaponM79 );
PRECACHE_WEAPON_REGISTER(weapon_m79);

BEGIN_DATADESC( CWeaponM79 )

	DEFINE_FIELD( m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( m_flNextGrenadeToss, FIELD_TIME ),
	DEFINE_FIELD( m_bMustReload,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFriendInBlastZone,	FIELD_BOOLEAN ),

END_DATADESC()

acttable_t	CWeaponM79::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true  },
	
// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_AR2_LOW,		true },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};

IMPLEMENT_ACTTABLE(CWeaponM79);

// FIXME: sync with AR2 grenade radius
#define	COMBINE_MIN_GRENADE_CLEAR_DIST 256

//=========================================================
CWeaponM79::CWeaponM79( )
{
	m_fMinRange1 = m_fMinRange2 = COMBINE_MIN_GRENADE_CLEAR_DIST;
	m_fMaxRange1 = m_fMaxRange2 = 1400;

	m_bReloadsSingly	= true;
	m_bFiresUnderwater = false;
	m_bMustReload		= false;

	m_flNextGrenadeToss = FLT_MIN;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM79::Precache( void )
{
	UTIL_PrecacheOther("grenade_ar2");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponM79::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
bool CWeaponM79::Deploy( void )
{
	m_bMustReload = (Clip1() <= 0);

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM79::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM79::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
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
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM79::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_AR2_GRENADE:
		{
			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			if ( !npc || !npc->GetEnemy() )
				break;

			WeaponSound( SINGLE_NPC );

			// Use the same exact spot that WeaponLOSCondition was passed. This is the
			// enemy's eyes or a random BodyTarget().
			Vector vecTarget = npc->m_vSavePosition;

			// Try to shoot from the muzzle of the weapon.  If that fails shoot
			// from Weapon_ShootPosition() which was used by WeaponRangeAttack1Condition.
			Vector vecShootOrigin, vecThrow;
			QAngle angShootDir;
			GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
			if ( !CheckThrowFromPosition( vecShootOrigin, vecTarget, &vecThrow ) )
			{
				// This looks bad but stops us blowing up our operator
				vecShootOrigin = npc->Weapon_ShootPosition();
				vecThrow = m_vecTossVelocity;
			}

			CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create( "grenade_ar2", vecShootOrigin, vec3_angle, npc );
			pGrenade->SetAbsVelocity( vecThrow );
			pGrenade->SetLocalAngularVelocity( QAngle( 0, 400, 0 ) );
			pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY ); 
			pGrenade->SetThrower( npc );
//			pGrenade->m_pMyWeaponAR2	= this;
			pGrenade->SetDamage(sk_npc_9mmAR_grenade.GetFloat());

#if 1
			// Find what our vertical theta is to estimate the time we'll impact the ground
			Vector vecToTarget = ( vecTarget - vecShootOrigin );
			VectorNormalize( vecToTarget );
			float flVelocity = VectorNormalize( vecThrow );
			float flCosTheta = DotProduct( vecToTarget, vecThrow );
			float flTime = (vecShootOrigin-vecTarget).Length2D() / ( flVelocity * flCosTheta );

			// Emit a sound where this is going to hit so that targets get a chance to act correctly
			CSoundEnt::InsertSound( SOUND_DANGER, vecTarget, 300, flTime, this );
#endif

			// wait N seconds before even looking again to see if a grenade can be thrown.
			m_flNextGrenadeToss = gpGlobals->curtime + GetFireRate();

			m_iClip1--;
		}
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponM79::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponM79::Reload( void )
{
	if ( BaseClass::Reload() )
	{
		m_bMustReload = false;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM79::ItemPostFrame( void )
{
	if ( m_bMustReload && HasWeaponIdleTimeElapsed() )
	{
		Reload();
	}

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM79::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle	viewPunch;

	viewPunch.x = -1.0f;
	viewPunch.y = 0.0f;
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch( viewPunch );
}

//-----------------------------------------------------------------------------
Vector CWeaponM79::PredictImpactPosition( const Vector& origin, const Vector& velocity )
{
	Vector vecMins = Vector(-3);
	Vector vecMaxs = Vector(3);

	Vector vecFrom = origin;
	Vector curVelocity = velocity;
	Vector gravity( 0, 0, 400 ); // must match CGrenadeAR2

	int count = 0;

	float flTime, timeStep = 0.15;
	for ( flTime = 0; /**/; flTime += timeStep )
	{
		// Calculate my position after the time step (average velocity over this time step)
		Vector nextPos = vecFrom + (curVelocity - 0.5 * gravity * timeStep) * timeStep;

		trace_t tr;
		AI_TraceHull( vecFrom, nextPos, vecMins, vecMaxs, MASK_SOLID_BRUSHONLY, GetOwner(), COLLISION_GROUP_NONE, &tr );

		if ( tr.startsolid || tr.fraction < 1.0 )
		{
			// Emit a sound where this is going to hit so that targets get a chance to act correctly
			CSoundEnt::InsertSound( SOUND_DANGER, tr.endpos, 300, flTime + 0.5f, this );

			return tr.endpos;
		}

		curVelocity = curVelocity - gravity * timeStep;
		vecFrom		= nextPos;

		++count;
	}

	return vec3_origin;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
// ConVar hoe_m79_muzzle("hoe_m79_muzzle", "4.5 23 -2");  // With 0,0,0 ViewModelFudge
ConVar hoe_m79_muzzle("hoe_m79_muzzle", "2.6 23 -3.6"); // With -2 0 -2 ViewModelFudge
ConVar hoe_m79_muzzle_ironsight("hoe_m79_muzzle_ironsight", "0 23 -2");
void CWeaponM79::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	//Must have ammo
	if ( ( Clip1() /*pPlayer->GetAmmoCount( m_iPrimaryAmmoType )*/ <= 0 ) || ( pPlayer->GetWaterLevel() == 3 ) )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	if( m_bInReload )
		m_bInReload = false;

	// MUST call sound before removing a round from the clip of a CMachineGun
	BaseClass::WeaponSound( SINGLE );

	pPlayer->RumbleEffect( RUMBLE_357, 0, RUMBLE_FLAGS_NONE );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();

#if 1
	// Launch the grenade from the apparent position of the muzzle.
	{
		Vector muzzleOffset;
		if ( IsIronSightsActive() )
			UTIL_StringToVector( muzzleOffset.Base(), hoe_m79_muzzle_ironsight.GetString() );
		else
			UTIL_StringToVector( muzzleOffset.Base(), hoe_m79_muzzle.GetString() );
		Vector	vForward, vRight, vUp;
		pPlayer->EyeVectors( &vForward, &vRight, &vUp );
		vecSrc = pPlayer->EyePosition() + vRight * muzzleOffset.x + vForward * muzzleOffset.y + vUp * muzzleOffset.z;
	}
#endif

	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors( pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow );
	VectorScale( vecThrow, 1000.0f, vecThrow );
	
	//Create the grenade
	QAngle angles;
	VectorAngles( vecThrow, angles );
	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create( "grenade_ar2", vecSrc, angles, pPlayer );
#if 1
	// Add the speed of any moving ground entity (i.e., the truck)
	CBaseEntity *pGroundEnt = pPlayer->GetGroundEntity();
	if ( pGroundEnt )
		pGrenade->SetAbsVelocity( vecThrow + pGroundEnt->GetAbsVelocity() );
	else
		pGrenade->SetAbsVelocity( vecThrow );
#else
	pGrenade->SetAbsVelocity( vecThrow );
#endif

	pGrenade->SetLocalAngularVelocity( RandomAngle( -400, 400 ) );
	pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE ); 
	pGrenade->SetThrower( GetOwner() );
	pGrenade->SetDamage( sk_plr_9mmAR_grenade.GetFloat() );

	// Guess where the grenade will impact and play a danger sound there to alert NPCs
	PredictImpactPosition( vecSrc, pGrenade->GetAbsVelocity() );

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Decrease ammo
	//pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );
	m_iClip1--;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;

	// Register a muzzleflash for the AI.
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );	

	// Signal a reload
	m_bMustReload = true;

	// FIXME: SendWeaponAnim() does this, remove for all weapons...
//	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	AddViewKick();
}

ConVar hoe_m79_los_debug( "hoe_m79_los_debug", "0" );

//-----------------------------------------------------------------------------
bool CWeaponM79::CheckThrowFromPosition( const Vector &shootPos, const Vector &targetPos, Vector *pVecOut )
{
	CAI_BaseNPC *npcOwner = GetOwner()->MyNPCPointer();
	if ( npcOwner == NULL )
		return false;

	// CGrenadeAR2 size
	Vector vecMins = Vector(-3,-3,-3);
	Vector vecMaxs = Vector(3,3,3);

#if 1
	// ----- First calculate the velocity like VecCheckThrow() -----
	float flSpeed = 600.0f;
	float flGravity = 400.0f;

	Vector vecGrenadeVel = (targetPos - shootPos);

	// throw at a constant time
	float time = vecGrenadeVel.Length() / flSpeed;
	vecGrenadeVel = vecGrenadeVel * (1.0 / time);

	// adjust upward toss to compensate for gravity loss
	vecGrenadeVel.z += flGravity * time * 0.5;

	// ----- Now do expensive check like CAI_BaseNPC::ThrowLimit
	Vector vecStart		= shootPos;
	Vector vecEnd		= targetPos;
	Vector vecFrom		= vecStart;

	Vector rawJumpVel = vecGrenadeVel;

	// Calculate the total time of the jump minus a tiny fraction
	float jumpTime		= (vecStart - vecEnd).Length2D()/rawJumpVel.Length2D();
	float timeStep		= jumpTime / 10.0;

	Vector gravity = Vector(0,0,flGravity);

	// this loop takes single steps to the goal.
	for (float flTime = 0; flTime < jumpTime-0.1; flTime += timeStep )
	{
		// Calculate my position after the time step (average velocity over this time step)
		Vector nextPos = vecFrom + (rawJumpVel - 0.5 * gravity * timeStep) * timeStep;

		// If last time step make next position the target position
		if ((flTime + timeStep) > jumpTime)
		{
			nextPos = vecEnd;
		}

		trace_t tr;
		AI_TraceHull( vecFrom, nextPos, vecMins, vecMaxs, MASK_SOLID, npcOwner, COLLISION_GROUP_NONE, &tr );

		if (tr.startsolid || tr.fraction < 1.0)
		{
			if ( tr.m_pEnt == npcOwner->GetEnemy() || flTime >= jumpTime * 0.9 )
			{
				if ( hoe_m79_los_debug.GetBool() )
					NDebugOverlay::Line( vecFrom, nextPos, 255, 255, 255, true, 1.0 );

				if ( pVecOut )
					*pVecOut = vecGrenadeVel;
				return true;
			}

			if ( hoe_m79_los_debug.GetBool() )
				NDebugOverlay::Line( vecFrom, nextPos, 255, 0, 0, true, 1.0 );

			return false;
		}
		else if ( hoe_m79_los_debug.GetBool() )
		{
			NDebugOverlay::Line( vecFrom, nextPos, 255, 255, 255, true, 1.0 );
		}

		rawJumpVel  = rawJumpVel - gravity * timeStep;
		vecFrom		= nextPos;
	}

	if ( pVecOut )
		*pVecOut = vecGrenadeVel;
	return true;

#else
	float flGravity = UTIL_ScaleForGravity( 400 ); // NOTE: must match AR2

	Vector vecToss = VecCheckThrow( npcOwner, shootPos, targetPos, 600.0, flGravity, &vecMins, &vecMaxs );
	if ( vecToss != vec3_origin )
	{
		if ( pVecOut )
			*pVecOut = vecToss;
		return true;
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool CWeaponM79::WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
	CAI_BaseNPC *npcOwner = GetOwner()->MyNPCPointer();

	// NOTE: This method doesn't check if friendlies are in the blast zone
	// since this gets called to test various nodes for weapon los and it
	// may take some time for the NPC to run to the node.

	// ---------------------------------------------------------------------
	// Check that throw is legal and clear
	// ---------------------------------------------------------------------

	// Find its relative shoot position
	Vector vecRelativeShootPosition;
	VectorSubtract( npcOwner->Weapon_ShootPosition(), npcOwner->GetAbsOrigin(), vecRelativeShootPosition );
	Vector barrelPos = ownerPos + vecRelativeShootPosition;

#if 1
	// Do a quick LOS check because CheckThrowFromPosition is expensive
	VectorSubtract( npcOwner->EyePosition(), npcOwner->GetAbsOrigin(), vecRelativeShootPosition );
	Vector ownerEyePos = ownerPos + vecRelativeShootPosition;

	trace_t tr;
	CTraceFilterLOS traceFilter( npcOwner, COLLISION_GROUP_NONE, npcOwner->GetEnemy() );
	UTIL_TraceLine( ownerEyePos, targetPos, MASK_BLOCKLOS, &traceFilter, &tr );
	if ( tr.fraction != 1.0 || tr.startsolid )
	{
		if ( hoe_m79_los_debug.GetBool() )
			NDebugOverlay::Line( ownerEyePos, targetPos, 255, 0, 0, true, 1.0 );
		if ( bSetConditions )
			npcOwner->SetCondition( COND_WEAPON_SIGHT_OCCLUDED );
		return false;
	}

	if ( CheckThrowFromPosition( barrelPos, targetPos, NULL ) )
	{
		if ( bSetConditions )
			npcOwner->m_vSavePosition = targetPos;
		return true;
	}
#else
	// CGrenadeAR2 size
	Vector vecMins = Vector(-3,-3,-3);
	Vector vecMaxs = Vector(3,3,3);

	float flGravity = UTIL_ScaleForGravity( 400 ); // NOTE: must match AR2

	Vector vecToss = VecCheckThrow( npcOwner, barrelPos, targetPos, 600.0, flGravity, &vecMins, &vecMaxs );
	if ( vecToss != vec3_origin )
	{
		if ( bSetConditions )
			npcOwner->m_vSavePosition = targetPos;
		return true;
	}
#endif
	else if ( bSetConditions )
	{
		npcOwner->SetCondition( COND_WEAPON_SIGHT_OCCLUDED );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponM79::WeaponRangeAttack1Condition( float flDot, float flDist )
{
	CAI_BaseNPC *npcOwner = GetOwner()->MyNPCPointer();

	m_bFriendInBlastZone = false;

#if 0 // this prevents shooting when running to establish line-of-fire
	// -----------------------
	// If moving, don't check.
	// -----------------------
	if ( npcOwner->IsMoving())
		return COND_NONE;
#endif

	CBaseEntity *pEnemy = npcOwner->GetEnemy();

	if (!pEnemy)
		return COND_NONE;

	Vector vecEnemyLKP = npcOwner->GetEnemyLKP();
	if ( !( pEnemy->GetFlags() & FL_ONGROUND ) && pEnemy->GetWaterLevel() == 0 && vecEnemyLKP.z > (GetAbsOrigin().z + WorldAlignMaxs().z) )
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		return COND_NONE;
	}

#if 1
	// --------------------------------------
	//  Get target vector
	// --------------------------------------
	// Use the same exact spot that WeaponLOSCondition was passed. This is the
	// enemy's eyes or a random BodyTarget().
	Vector vecTarget = npcOwner->m_vSavePosition;
#else
	// --------------------------------------
	//  Get target vector
	// --------------------------------------
	Vector vecTarget;
	if (random->RandomInt(0,1))
	{
		// magically know where they are
		vecTarget = pEnemy->WorldSpaceCenter();
	}
	else
	{
		// toss it to where you last saw them
		vecTarget = vecEnemyLKP;
	}
#endif
	// vecTarget = m_vecEnemyLKP + (pEnemy->BodyTarget( GetLocalOrigin() ) - pEnemy->GetLocalOrigin());
	// estimate position
	// vecTarget = vecTarget + pEnemy->m_vecVelocity * 2;

	// Shouldn't the weapon min/max range checks catch this?
	if ( ( vecTarget - npcOwner->GetLocalOrigin() ).Length2D() <= COMBINE_MIN_GRENADE_CLEAR_DIST )
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
//		return (COND_NONE);
		return COND_TOO_CLOSE_TO_ATTACK;
	}

	// ---------------------------------------------------------------------
	// Are any friendlies near the intended grenade impact area?
	// ---------------------------------------------------------------------
	CBaseEntity *pTarget = NULL;

	while ( ( pTarget = gEntList.FindEntityInSphere( pTarget, vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST ) ) != NULL )
	{
		// The above check doesn't always catch this.
		if ( pTarget == npcOwner )
		{
			// crap, I don't want to blow myself up
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
			return COND_TOO_CLOSE_TO_ATTACK;
		}

		if ( npcOwner->IRelationType( pTarget ) == D_LI )
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.

			// Tell my squad members to clear out so I can get a grenade in
			CBaseCombatCharacter *pBCC = pTarget->MyCombatCharacterPointer();
			if ( pBCC != NULL )
			{
				HumanGrenadeInteractionData data;
				data.flRadius = COMBINE_MIN_GRENADE_CLEAR_DIST * 2;
				data.vecOrigin = vecTarget;
				pBCC->DispatchInteraction( g_interactionHumanGrenade, &data, npcOwner->MyCombatCharacterPointer() );
			}

			m_bFriendInBlastZone = true;
			return (COND_WEAPON_BLOCKED_BY_FRIEND);
		}
	}

#if 1
	Vector vecToss;
	if ( CheckThrowFromPosition( npcOwner->Weapon_ShootPosition(), vecTarget, &vecToss ) )
#else // CheckThrowFromPosition
	// ---------------------------------------------------------------------
	// Check that throw is legal and clear
	// ---------------------------------------------------------------------
	// FIXME: speed is based on difficulty...

	Vector vecShoot = npcOwner->Weapon_ShootPosition();

	// CGrenadeAR2 size
	Vector vecMins = Vector(-3,-3,-3);
	Vector vecMaxs = Vector(3,3,3);

	float flGravity = UTIL_ScaleForGravity( 400 ); // NOTE: must match AR2

	Vector vecToss = VecCheckThrow( npcOwner, vecShoot, vecTarget, 600.0, flGravity, &vecMins, &vecMaxs );
	if ( vecToss != vec3_origin )
#endif // CheckThrowFromPosition
	{
#if 1
		// Toss a grenade every N seconds
		if ( gpGlobals->curtime < m_flNextGrenadeToss )
			return COND_NONE;
#endif

		m_vecTossVelocity = vecToss;

		// don't check again for a while.
		// JAY: HL1 keeps checking - test?
		//m_flNextGrenadeCheck = gpGlobals->curtime;
		m_flNextGrenadeCheck = gpGlobals->curtime + 0.3; // 1/3 second.
#if 0
		// Tell my friends to clear out so I can get a grenade in
		bool isFriendOfPlayer = false; // FIXME
		int type = isFriendOfPlayer ? SOUND_CONTEXT_ALLIES_ONLY : SOUND_CONTEXT_COMBINE_ONLY;
		CSoundEnt::InsertSound( SOUND_MOVE_AWAY | type, vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST, 0.1 );
#endif
		return COND_CAN_RANGE_ATTACK1;
	}
	else
	{
		// don't check again for a while.
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return COND_WEAPON_SIGHT_OCCLUDED;
	}
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponM79::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 1.00,		1.0		},
		{ 1.00,		1.0		},
		{ 1.00,		1.0		},
		{ 1.00,		1.0		},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}

bool M79FriendInBlastZone( CBaseCombatWeapon *pWeapon )
{
	if ( !pWeapon )
		return false;
	CWeaponM79 *pM79 = dynamic_cast<CWeaponM79 *>(pWeapon);
	if ( !pM79 )
		return false;
	return pM79->m_bFriendInBlastZone;
}