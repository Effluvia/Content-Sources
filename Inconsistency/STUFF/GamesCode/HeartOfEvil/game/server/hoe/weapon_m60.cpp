#include "cbase.h"
#include "basehlcombatweapon.h"
#include "NPCevent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "AI_Memory.h"
#include "soundent.h"
//#include "rumble_shared.h"
#ifdef HOE_THIRDPERSON
#include "hl2_player.h"
#endif // HOE_THIRDPERSON

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define M60_BODYGROUP_TRIPOD 1
#define M60_TRIPOD_DOWN 0
#define M60_TRIPOD_UP 1

class CWeaponM60 : public CHLMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponM60, CHLMachineGun );

	CWeaponM60();

	DECLARE_SERVERCLASS();
	
	void	Precache( void );
	void	RegisterPrivateActivities( void );
	void	AddViewKick( void );
	void	PrimaryAttack( void );
//	void	SecondaryAttack( void );

	int		GetMinBurst() { return 5; }
	int		GetMaxBurst() { return 10; }

	void Equip( CBaseCombatCharacter *pOwner );
	void Drop( const Vector &vecVelocity );
	bool	Reload( void );

	void	ItemPostFrame( void );

	virtual void SetViewModel( void );
	void SetViewModelBodygroup( int iGroup, int iValue );
	int	m_nViewModelBody;

	float	GetFireRate( void ) { return 0.15; } // HL1 M60_FIRE_DELAY = 0.1
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
//	int		WeaponRangeAttack2Condition( float flDot, float flDist );
	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector coneNPC = VECTOR_CONE_8DEGREES;
		if ( GetOwner() && GetOwner()->IsNPC() )
			return coneNPC;

		static const Vector cone = VECTOR_CONE_6DEGREES;
		return cone;
	}

	bool Deploy( void );

	void UpdateAmmoBelt( void );

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	DECLARE_ACTTABLE();

	int m_nBulletsInBelt; // number of bullets displayed in the ammo belt
	float m_flReload2Time; // when to play the final part of the "reload" animation

	static Activity ACT_VM_M60_RELOAD_FINISH;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponM60, DT_WeaponM60)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_m60, CWeaponM60 );
PRECACHE_WEAPON_REGISTER(weapon_m60);

BEGIN_DATADESC( CWeaponM60 )
//	DEFINE_FIELD( m_nBulletsInBelt, FIELD_INTEGER ), // don't save
	DEFINE_FIELD( m_flReload2Time, FIELD_TIME ),
END_DATADESC()

Activity CWeaponM60::ACT_VM_M60_RELOAD_FINISH;

// NOTE use of AR2 activities
acttable_t	CWeaponM60::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_RANGE_AIM_AR2_LOW,			true },

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

	{ ACT_WALK_AIM,					ACT_WALK_AIM_SHOTGUN,			true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,			true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_AR2_LOW,		true },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};

IMPLEMENT_ACTTABLE(CWeaponM60);

//=========================================================
CWeaponM60::CWeaponM60( )
{
	m_fMinRange1 = m_fMinRange2 = 0;// No minimum range. 
	m_fMaxRange1 = m_fMaxRange2 = 1024;

	m_bAltFiresUnderwater = false;

	m_nBulletsInBelt = 0;
	m_flReload2Time = 0.0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM60::Precache( void )
{
	BaseClass::Precache();
	RegisterPrivateActivities();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponM60::Equip( CBaseCombatCharacter *pOwner )
{
#if 0
	if( pOwner->Classify() == CLASS_PLAYER_ALLY )
	{
		m_fMaxRange1 = 3000;
	}
	else
	{
		m_fMaxRange1 = 1400;
	}
#endif
	BaseClass::Equip( pOwner );

#ifndef HOE_WEAPONMODEL_FIX
	if ( pOwner->IsNPC() )
#endif
	{
		SetBodygroup( M60_BODYGROUP_TRIPOD, M60_TRIPOD_UP );
	}
}

//-----------------------------------------------------------------------------
void CWeaponM60::Drop( const Vector &vecVelocity )
{
#ifndef HOE_WEAPONMODEL_FIX
	if ( GetOwner()->IsNPC() )
#endif
	{
		SetBodygroup( M60_BODYGROUP_TRIPOD, M60_TRIPOD_DOWN );
	}

	BaseClass::Drop( vecVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM60::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
#if 0
Vector vecDbg;
VectorMA(vecShootOrigin, 1000, vecShootDir, vecDbg);
NDebugOverlay::Line(vecShootOrigin, vecDbg, 0,0,255, true, .75);

Vector vecProjectedPosition = pOperator->MyNPCPointer()->GetActualShootPosition( vecShootOrigin );
NDebugOverlay::Line(vecShootOrigin, vecProjectedPosition, 255,0,0, true, .75);
#endif

// FIXME: use the returned number of bullets to account for >10hz firerate
//	WeaponSoundRealtime( SINGLE_NPC );
	WeaponSound( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0 );

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM60::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
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
void CWeaponM60::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_SMG1:
		{
			Vector vecShootOrigin, vecShootDir;
			QAngle angDiscard;

			// Support old style attachment point firing
			if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
			{
				vecShootOrigin = pOperator->Weapon_ShootPosition();
			}

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );
			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
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
Activity CWeaponM60::GetPrimaryAttackActivity( void )
{
	return BaseClass::GetPrimaryAttackActivity();
#if 0
	if ( m_nShotsFired < 2 )
		return ;

	if ( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;
	
	if ( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
#endif
}

//-----------------------------------------------------------------------------
void CWeaponM60::PrimaryAttack( void )
{
	BaseClass::PrimaryAttack();
}

//-----------------------------------------------------------------------------
bool CWeaponM60::Deploy( void )
{
	bool bRet = BaseClass::Deploy();

	UpdateAmmoBelt();

	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM60::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	UpdateAmmoBelt();

	if (m_flReload2Time != 0.0 && m_flReload2Time <= gpGlobals->curtime)
	{
		int ammo = pOwner->GetAmmoCount(m_iPrimaryAmmoType);
		int body = ( (ammo <= 0) ? 0 : (ammo < 18) ? ammo : 18 );
		SetViewModelBodygroup( 1, body );

		SendWeaponAnim( ACT_VM_M60_RELOAD_FINISH );
		m_flReload2Time = 0.0;

		pOwner->m_flNextAttack = gpGlobals->curtime;
		m_flNextPrimaryAttack = GetWeaponIdleTime();
	}
}

//-----------------------------------------------------------------------------
void CWeaponM60::SetViewModel( void )
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
void CWeaponM60::SetViewModelBodygroup( int iGroup, int iValue )
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

//-----------------------------------------------------------------------------
bool CWeaponM60::Reload( void )
{
	bool fRet;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), (m_iClip1 > 0) ? ACT_VM_RELOAD : ACT_VM_RELOAD_EMPTY);
	if ( fRet )
	{
//		WeaponSound( RELOAD );
		m_flReload2Time = GetWeaponIdleTime();
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM60::AddViewKick( void )
{
	#define	EASY_DAMPEN			0.5f
	#define	MAX_VERTICAL_KICK	2.0f	//Degrees
	#define	SLIDE_LIMIT			2.0f	//Seconds
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );

#ifndef HOE_THIRDPERSON // disable this in thirdperson cuz the legs start running
	// Shove backward (from HL1)
	if ( !pPlayer->IsInAVehicle() )
	{
		Vector forward;
		AngleVectors( pPlayer->GetLocalAngles(), &forward, NULL, NULL );
		pPlayer->ApplyAbsVelocityImpulse( forward * -50 );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM60::UpdateAmmoBelt( void )
{
	int nBullets = ( (m_iClip1 <= 0) ? 0 : (m_iClip1 < 18) ? m_iClip1 : 18 );
	if ( nBullets != m_nBulletsInBelt )
	{
		SetViewModelBodygroup( 1, nBullets );
		m_nBulletsInBelt = nBullets;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM60::RegisterPrivateActivities(void)
{
	static bool bRegistered = false;

	if (bRegistered)
		return;

	REGISTER_PRIVATE_ACTIVITY( ACT_VM_M60_RELOAD_FINISH )

//	bRegistered = true;
}
