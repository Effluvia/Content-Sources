#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "basebludgeonweapon.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "te_effect_dispatch.h"
#include "rumble_shared.h"
#include "func_break.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_plr_dmg_machete( "sk_plr_dmg_machete", "0");
ConVar sk_npc_dmg_machete( "sk_npc_dmg_machete", "0");

int g_interactionMacheteHeadChop = 0;

//-----------------------------------------------------------------------------
// CWeaponMachete
//-----------------------------------------------------------------------------

#define	MACHETE_RANGE	75.0f
#define	MACHETE_REFIRE	0.4f

//-----------------------------------------------------------------------------
// CWeaponMachete
//-----------------------------------------------------------------------------

class CWeaponMachete : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponMachete, CBaseHLBludgeonWeapon );

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponMachete();

	float		GetRange( void )		{	return	MACHETE_RANGE;	}
	float		GetFireRate( void )		{	return	MACHETE_REFIRE;	}

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );

	virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );
	void		PrimaryAttack( void );
	void		SecondaryAttack( void )	{	return;	}

	// Animation event
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	// Animation event handlers
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	virtual void ImpactEffect( trace_t &traceHit );
	virtual void OnHitEntity( const CTakeDamageInfo &info, const trace_t &trace );
};

IMPLEMENT_SERVERCLASS_ST(CWeaponMachete, DT_WeaponMachete)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_machete, CWeaponMachete );
PRECACHE_WEAPON_REGISTER( weapon_machete );

acttable_t CWeaponMachete::m_acttable[] = 
{
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_RUN_AIM,			ACT_HL2MP_RUN_MELEE, false },
};
IMPLEMENT_ACTTABLE(CWeaponMachete);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponMachete::CWeaponMachete( void )
{
	// DECLARE_INTERACTION
	if ( g_interactionMacheteHeadChop == 0 )
		g_interactionMacheteHeadChop = CBaseCombatCharacter::GetInteractionID();
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponMachete::GetDamageForActivity( Activity hitActivity )
{
	if ( ( GetOwner() != NULL ) && ( GetOwner()->IsPlayer() ) )
		return sk_plr_dmg_machete.GetFloat();

	return sk_npc_dmg_machete.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponMachete::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat( 1.0f, 2.0f );
	punchAng.y = random->RandomFloat( -2.0f, -1.0f );
	punchAng.z = 0.0f;
	
	pPlayer->ViewPunch( punchAng ); 
}

//-----------------------------------------------------------------------------
void CWeaponMachete::PrimaryAttack( void )
{
#if 0
	extern bool g_bMacheteOrChainsawAttack;
	g_bMacheteOrChainsawAttack = true;
#endif
	BaseClass::PrimaryAttack();
#if 0
	g_bMacheteOrChainsawAttack = false;
#endif
}

//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
//-----------------------------------------------------------------------------
ConVar sk_machete_lead_time( "sk_machete_lead_time", "0.9" );

int CWeaponMachete::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity( );

	// Project where the enemy will be in a little while
	float dt = sk_machete_lead_time.GetFloat();
	dt += random->RandomFloat( -0.3f, 0.2f );
	if ( dt < 0.0f )
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA( pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos );

	Vector vecDelta;
	VectorSubtract( vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta );

	if ( fabs( vecDelta.z ) > 70 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D( );
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize( vecDelta.AsVector2D() );
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D( vecDelta.AsVector2D(), vecForward.AsVector2D() );
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}


//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponMachete::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors( GetAbsAngles(), &vecDirection );

	CBaseEntity *pEnemy = pOperator->MyNPCPointer() ? pOperator->MyNPCPointer()->GetEnemy() : NULL;
	if ( pEnemy )
	{
		Vector vecDelta;
		VectorSubtract( pEnemy->WorldSpaceCenter(), pOperator->Weapon_ShootPosition(), vecDelta );
		VectorNormalize( vecDelta );
		
		Vector2D vecDelta2D = vecDelta.AsVector2D();
		Vector2DNormalize( vecDelta2D );
		if ( DotProduct2D( vecDelta2D, vecDirection.AsVector2D() ) > 0.8f )
		{
			vecDirection = vecDelta;
		}
	}

	Vector vecEnd;
	VectorMA( pOperator->Weapon_ShootPosition(), 50, vecDirection, vecEnd );
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack( pOperator->Weapon_ShootPosition(), vecEnd, 
		Vector(-16,-16,-16), Vector(36,36,36), sk_npc_dmg_machete.GetFloat(), DMG_SLASH, 0.75 );
	
	// did I hit someone?
	if ( pHurt )
	{
		// play sound
//		WeaponSound( MELEE_HIT );

		// Fake a trace impact, so the effects work out like a player's crowbaw
		trace_t traceHit;
		UTIL_TraceLine( pOperator->Weapon_ShootPosition(), pHurt->GetAbsOrigin(), MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit );
		ImpactEffect( traceHit );
	}
	else
	{
//		WeaponSound( MELEE_MISS );
	}
}

//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponMachete::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit( pEvent, pOperator );
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

//-----------------------------------------------------------------------------
void CWeaponMachete::ImpactEffect( trace_t &traceHit )
{
	// See if we hit water (we don't do the other impact effects in this case)
	if ( ImpactWater( traceHit.startpos, traceHit.endpos ) )
		return;

	//FIXME: need new decals
	CBaseEntity *pEnt = traceHit.m_pEnt;
	if ( pEnt && pEnt->BloodColor() != DONT_BLEED )
		UTIL_ImpactTrace( &traceHit, DMG_BULLET );
	else
		UTIL_ImpactTrace( &traceHit, DMG_SLASH );
}

//-----------------------------------------------------------------------------
void CWeaponMachete::OnHitEntity( const CTakeDamageInfo &info, const trace_t &trace)
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	// Swing() does not return the hitbox that was struck because it uses MASK_SHOT_HULL
	// not MASK_SHOT. In order to chop off heads with the machete we perform a trace from the player's eyes
	// forward to figure out if the player was aiming for the head.
	CBaseCombatCharacter *pBCC = trace.m_pEnt->MyCombatCharacterPointer();
	if ( pBCC != NULL )
	{
		Vector forward;
		pPlayer->EyeVectors( &forward, NULL );

		trace_t traceHit;
		UTIL_TraceLine( pPlayer->EyePosition(), pPlayer->EyePosition() + forward * 64, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &traceHit );
		if ( traceHit.m_pEnt == trace.m_pEnt && traceHit.hitgroup == HITGROUP_HEAD )
		{
			pBCC->DispatchInteraction( g_interactionMacheteHeadChop, NULL, pPlayer );
		}
		return;
	}

	// This gets called after the regular TraceAttack so make sure the breakable wasn't
	// broken by the machete swing itself before telling always-break-when-hit-by-machete breakables
	// to break
	CBreakable *pBreak = dynamic_cast< CBreakable * >(trace.m_pEnt);
	if ( pBreak != NULL && pBreak->IsAlive() )
	{
		if ( pBreak->HasSpawnFlags( 256 ) )
			pBreak->Break( pPlayer );
	}
}