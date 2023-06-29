//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "grenade_tripmine.h"
#include "entitylist.h"
#include "weapon_tripmine.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "activitylist.h"
#ifdef HOE_THIRDPERSON
#include "hl2_player.h"
#endif // HOE_THIRDPERSON

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TRIPMINE_ATTACH_DELAY 1.0 // how long to wait after attaching a tripmine before another may be attached

BEGIN_DATADESC( CWeaponTripmine )

	DEFINE_FIELD( m_bNeedReload, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bClearReload, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAttachTripmine, FIELD_BOOLEAN ),

	// Function Pointers
//	DEFINE_FUNCTION( TripmineThink ),
//	DEFINE_FUNCTION( TripmineTouch ),

END_DATADESC()


IMPLEMENT_SERVERCLASS_ST(CWeaponTripmine, DT_WeaponTripmine)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_tripmine, CWeaponTripmine );
PRECACHE_WEAPON_REGISTER(weapon_tripmine);

acttable_t	CWeaponTripmine::m_acttable[] = 
{
	{ ACT_MP_ATTACK_STAND_PREFIRE, ACT_SLAM_TRIPMINE_TO_STICKWALL_ND, false },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_TRIPWIRE, true },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_RUN_AIM,			ACT_HL2MP_RUN_MELEE, false },
};

IMPLEMENT_ACTTABLE(CWeaponTripmine);

static int ACT_VM_TRIPMINE_IDLE1 = 0;
static int ACT_VM_TRIPMINE_IDLE2 = 0;
static int ACT_VM_TRIPMINE_ARM1 = 0;
static int ACT_VM_TRIPMINE_PLACE = 0;
static int ACT_VM_TRIPMINE_FIDGET = 0;
static int ACT_VM_TRIPMINE_HOLSTER = 0;
static int ACT_VM_TRIPMINE_DRAW = 0;
static int ACT_VM_TRIPMINE_GROUND = 0;
static int ACT_VM_TRIPMINE_WORLD = 0;

#if 0
void CWeaponTripmine::Spawn( )
{
	BaseClass::Spawn();

	Precache( );

	UTIL_SetSize(this, Vector(-4,-4,-2),Vector(4,4,2));

	FallInit();// get ready to fall down

	SetThink( NULL );

	// Give 1 piece of default ammo when first picked up
	m_iClip1 = 1;
}
#endif
void CWeaponTripmine::Precache( void )
{
	BaseClass::Precache();

	RegisterPrivateActivities();

	UTIL_PrecacheOther( "npc_tripmine" );

	PrecacheScriptSound( "TripmineGrenade.Deploy" );
}

#if 0
//------------------------------------------------------------------------------
// Purpose : Override to use slam's pickup touch function
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWeaponTripmine::SetPickupTouch( void )
{
	SetTouch( &CWeaponTripmine::TripmineTouch );
}

//-----------------------------------------------------------------------------
// Purpose: Override so give correct ammo
// Input  : pOther - the entity that touched me
// Output :
//-----------------------------------------------------------------------------
void CWeaponTripmine::TripmineTouch( CBaseEntity *pOther )
{
	// Can I even pick stuff up?
	if ( pOther->IsEFlagSet( EFL_NO_WEAPON_PICKUP ) )
		return;

	// ---------------------------------------------------
	//  First give weapon to touching entity if allowed
	// ---------------------------------------------------
	BaseClass::DefaultTouch(pOther);

	// ----------------------------------------------------
	//  Give slam ammo if touching client
	// ----------------------------------------------------
	if (pOther->GetFlags() & FL_CLIENT)
	{
		// ------------------------------------------------
		//  If already owned weapon of this type remove me
		// ------------------------------------------------
		CBaseCombatCharacter* pBCC = ToBaseCombatCharacter( pOther );
		CWeaponTripmine* oldWeapon = (CWeaponTripmine*)pBCC->Weapon_OwnsThisType( GetClassname() );
		if (oldWeapon != this)
		{
			UTIL_Remove( this );
		}
		else
		{
			pBCC->GiveAmmo( 1, m_iPrimaryAmmoType );
			SetThink(NULL);
		}
	}
}
#endif

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
bool CWeaponTripmine::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bNeedReload = false;
//	SetThink(NULL);
#ifdef HOE_THIRDPERSON
	if ( GetOwner() != NULL && GetOwner()->IsPlayer() )
		ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_RESET_GESTURE_SLOT, GESTURE_SLOT_ATTACK_AND_RELOAD );
#endif
	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: SLAM has no reload, but must call weapon idle to update state
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponTripmine::Reload( void )
{
	if ( !HasPrimaryAmmo() )
		return false;

	if ( m_bNeedReload && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack	= GetWeaponIdleTime();
//		m_flNextSecondaryAttack	= GetWeaponIdleTime();
//		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

		//Mark this as done
		m_bNeedReload = false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponTripmine::PrimaryAttack( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
	{ 
		return;
	}

	if ( !HasPrimaryAmmo() )
	{
		return;
	}

	trace_t tr;
	if ( CanAttach( tr ) )
	{
		StartAttach();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Secondary attack switches between satchel charge and tripmine mode
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponTripmine::SecondaryAttack( void )
{
	return; // Nothin for now. SLAM's just a tripmine.
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponTripmine::Attach( const trace_t &tr )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	if (!pOwner)
	{
		return;
	}

	QAngle angles;
	VectorAngles(tr.plane.normal, angles);
//	angles.x += 90;
	
	CBaseEntity *pEnt = CBaseEntity::Create( "npc_tripmine", tr.endpos + tr.plane.normal * 0, angles, NULL );

	CTripmineGrenade *pMine = (CTripmineGrenade *)pEnt;
	pMine->m_hOwner = GetOwner();

	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );

	EmitSound( "TripmineGrenade.Deploy" );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponTripmine::StartAttach( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
	{
		return;
	}

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
#ifdef HOE_THIRDPERSON
	ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRE );
#endif

	// play "arming" animation
	SendWeaponAnim( ACT_VM_TRIPMINE_ARM1 );

	// When "arming" animation is finished, attach the tripmine
	m_bAttachTripmine	= true;

	// When "attach" is complete, draw another tripmine
	m_bNeedReload		= true;

	m_flNextPrimaryAttack	= FLT_MAX; // This gets updated after placing the tripmine
//	m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();
//	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
}

#if 0
//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponTripmine::TripmineThink( void )
{
	SetNextThink( gpGlobals->curtime + 0.1f );
}
#endif
//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponTripmine::CanAttach( trace_t &tr )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	if (!pOwner)
	{
		return false;
	}

	Vector vecSrc	 = pOwner->Weapon_ShootPosition( );
	Vector vecAiming = pOwner->EyeDirection3D( );

	Vector	vecEnd = vecSrc + (vecAiming * 128);
	UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
	
	if ( tr.fraction < 1.0 )
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if ( pEntity && !(pEntity->GetFlags() & FL_CONVEYOR) )
		{
			// Don't attach to a living creature
			CBaseCombatCharacter *pBCC		= ToBaseCombatCharacter( pEntity );
			if (pBCC)
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override so SLAM to so secondary attack when no secondary ammo
//			but satchel is in the world
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponTripmine::ItemPostFrame( void )
{
#if 1
	BaseClass::ItemPostFrame();
#else
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
	{
		return;
	}

	if (!m_bNeedReload && (pOwner->m_nButtons & IN_ATTACK) &&
			(m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		PrimaryAttack();
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	else 
	{
		WeaponIdle();
		return;
	}
#endif
}

#if 0
//-----------------------------------------------------------------------------
void CWeaponTripmine::SetViewModelBodygroup( int iGroup, int iValue )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner != NULL )
	{
		CBaseViewModel *vm = pOwner->GetViewModel();
		if ( vm != NULL )
		{
			vm->SetBodygroup( iGroup, iValue );
		}
	}
}
#endif

#if 0
//-----------------------------------------------------------------------------
// Purpose: Switch to next best weapon
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponTripmine::WeaponSwitch( void )
{
BaseClass::WeaponSwitch(); return;

	CBaseCombatCharacter *pOwner  = GetOwner();
	pOwner->SwitchToNextBestWeapon( pOwner->GetActiveWeapon() );

	// If not armed and have no ammo
	if (/*!m_bDetonatorArmed && */pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
	{
		pOwner->ClearActiveWeapon();
	}
}
#endif
//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponTripmine::WeaponIdle( void )
{
	// Ready to switch animations?
 	if ( !HasWeaponIdleTimeElapsed() )
		return;

	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return;

	// No more ammo
	if ( !HasPrimaryAmmo() )
	{
		pOwner->Weapon_Drop(this);
		UTIL_Remove(this);
		return;
	}

	if (m_bClearReload)
	{
		m_bClearReload = false;
		m_bNeedReload = false;
	}

	int iAnim = ACT_VM_IDLE; // idle1, idle2 or fidget

	// "arming" animation finished, so place it
	if (m_bAttachTripmine)
	{
		m_bAttachTripmine = false;

		// The player may have moved/turned during the "arming" animation so check that we can still attach
		trace_t tr;
		if ( CanAttach( tr ) )
		{
			Attach( tr );
			iAnim = ACT_VM_TRIPMINE_PLACE;
//			SetViewModelBodygroup( 1, 1 ); // hide the viewmodel mine
#ifdef HOE_THIRDPERSON
			ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
#endif
		}
		else
		{
#ifdef HOE_THIRDPERSON
			ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_RESET_GESTURE_SLOT, GESTURE_SLOT_ATTACK_AND_RELOAD );
#endif
		}

		m_flNextPrimaryAttack = gpGlobals->curtime + TRIPMINE_ATTACH_DELAY;
	}

	// m_bNeedReload means draw another
	else if ( m_bNeedReload )
	{
//		SetViewModelBodygroup( 1, 0 ); // show the viewmodel mine
		iAnim = ACT_VM_DRAW;
		m_bClearReload = true;
	}

	SendWeaponAnim( iAnim );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponTripmine::Deploy( void )
{
#if 1
	bool bRet = BaseClass::Deploy();

//	SetViewModelBodygroup( 1, 0 ); // show the viewmodel mine just in case
	m_bNeedReload = false;
	return bRet;
#else
	CBaseCombatCharacter *pOwner  = GetOwner();
	if (!pOwner)
	{
		return false;
	}

	SetThink( &CWeaponTripmine::TripmineThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	SetModel( GetViewModel() );

	m_bNeedReload = false;

	return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(),
		ACT_VM_DRAW, (char*)GetAnimPrefix() );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  :
// Output :
//-----------------------------------------------------------------------------
CWeaponTripmine::CWeaponTripmine(void)
{
	m_bNeedReload			= true;
	m_bClearReload			= false;
	m_bAttachTripmine		= false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponTripmine::RegisterPrivateActivities(void)
{
	static bool bRegistered = false;

	if (bRegistered)
		return;

	REGISTER_PRIVATE_ACTIVITY( ACT_VM_TRIPMINE_IDLE1 );
	REGISTER_PRIVATE_ACTIVITY( ACT_VM_TRIPMINE_IDLE2 );
	REGISTER_PRIVATE_ACTIVITY( ACT_VM_TRIPMINE_ARM1 );
	REGISTER_PRIVATE_ACTIVITY( ACT_VM_TRIPMINE_PLACE );
	REGISTER_PRIVATE_ACTIVITY( ACT_VM_TRIPMINE_FIDGET );
	REGISTER_PRIVATE_ACTIVITY( ACT_VM_TRIPMINE_HOLSTER );
	REGISTER_PRIVATE_ACTIVITY( ACT_VM_TRIPMINE_DRAW );
	REGISTER_PRIVATE_ACTIVITY( ACT_VM_TRIPMINE_GROUND );
	REGISTER_PRIVATE_ACTIVITY( ACT_VM_TRIPMINE_WORLD );

//	bRegistered = true;
}