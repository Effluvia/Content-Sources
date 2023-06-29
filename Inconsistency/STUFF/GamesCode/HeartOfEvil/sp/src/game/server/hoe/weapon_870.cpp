//=============================================================================//
//
// Purpose: A shotgun.
//
//			Primary attack: fire buckshot or elephantshot.
//			Secondary attack: load a round of elephantshot.
//
//=============================================================================//

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "vstdlib/random.h"
#ifdef HOE_THIRDPERSON
#include "hl2_player.h"
#endif // HOE_THIRDPERSON

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sk_auto_reload_time;

#ifdef HOE_IRONSIGHTS
extern ConVar hoe_ironsights;
extern ConVar hoe_ironsights_auto;
#endif

// Viewmodel bodygroups and values
enum
{
	W870_BODYGROUP_SHOTTY = 0,
	W870_BODYGROUP_SHELL
};
enum
{
	W870_SHELL_BUCKSHOT = 0,
	W870_SHELL_ELEPHANT,
	W870_SHELL_BLANK
};

class CWeapon870 : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeapon870, CBaseHLCombatWeapon );

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeapon870(void);
	void	Spawn( void );
	void	Precache( void );

	void Equip( CBaseCombatCharacter *pOwner );
	
	int CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector vitalAllyCone = VECTOR_CONE_3DEGREES;
		static Vector cone = VECTOR_CONE_10DEGREES; // FIXME: 20 degrees for elephantshot
#if 0
		if ( GetOwner() && GetOwner()->ClassifyPlayerAllyVital() )
		{
			// Give Alyx's shotgun blasts more a more directed punch. She needs
			// to be at least as deadly as she would be with her pistol to stay interesting (sjb)
			return vitalAllyCone;
		}
#endif
		return cone;
	}

	virtual int				GetMinBurst() { return 1; }
	virtual int				GetMaxBurst() { return 1; } // was 3
	virtual float			GetFireRate( void ) { return 0.7; }; // useless for shotty?
	virtual float			GetMinRestTime() { return 0.9 /* 0.3 */; }
	virtual float			GetMaxRestTime() { return 1.1 /* 0.6 */; }

	bool StartReload( void );
	bool Reload( void );
	void FillClip( void );
	void FinishReload( void );
//	void CheckHolsterReload( void );
	void Pump( void );
//	void WeaponIdle( void );
	void ItemHolsterFrame( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void DryFire( void );

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	virtual void SetViewModel( void );
	void SetViewModelBodygroup( int iGroup, int iValue );
	int m_nViewModelBody;

	bool	m_bNeedPump;		// When emptied completely
	bool	m_bDelayedFire1;	// Fire primary when finished reloading
//	bool	m_bDelayedFire2;	// Fire secondary when finished reloading

	enum {
		Q_EMPTY = 0,
		Q_BUCKSHOT,
		Q_ELEPHANTSHOT
	};
	int m_AmmoType;			// Q_xxx ammo to be loaded
	int m_NextAmmoType;		// Q_XXX ammo to be loaded next
#define MAX_CLIP 10				// the actual clip size is in scripts/weapon_870.txt
	CNetworkArray( unsigned char, m_AmmoQueue, MAX_CLIP );	// Q_xxx ammo that is loaded

};

IMPLEMENT_SERVERCLASS_ST(CWeapon870, DT_Weapon870)
	SendPropArray3(SENDINFO_ARRAY3(m_AmmoQueue), SendPropInt(SENDINFO_ARRAY(m_AmmoQueue), 8, SPROP_UNSIGNED)),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_870, CWeapon870 );
PRECACHE_WEAPON_REGISTER(weapon_870);

BEGIN_DATADESC( CWeapon870 )
	DEFINE_FIELD( m_bNeedPump, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDelayedFire1, FIELD_BOOLEAN ),
//	DEFINE_FIELD( m_bDelayedFire2, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_AmmoType, FIELD_INTEGER ),
	DEFINE_FIELD( m_NextAmmoType, FIELD_INTEGER ),
	DEFINE_AUTO_ARRAY( m_AmmoQueue, FIELD_CHARACTER ),
END_DATADESC()

acttable_t	CWeapon870::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },	// FIXME: hook to shotgun unique

	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SHOTGUN,					false },
	{ ACT_WALK,						ACT_WALK_RIFLE,						true },
//	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SHOTGUN,				true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SHOTGUN_RELAXED,		false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SHOTGUN_STIMULATED,	false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_SHOTGUN_AGITATED,		false },//always aims

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

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,					true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,				true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,			true },
	{ ACT_RUN,						ACT_RUN_RIFLE,						true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,					true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,				true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SHOTGUN_LOW,		true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SHOTGUN_LOW,				false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SHOTGUN,			false },
};

IMPLEMENT_ACTTABLE(CWeapon870);

//-----------------------------------------------------------------------------
CWeapon870::CWeapon870( void )
{
	m_bReloadsSingly = true;

	m_bNeedPump		= false;
	m_bDelayedFire1 = false;
//	m_bDelayedFire2 = false;

	m_fMinRange1		= 0.0;
	m_fMaxRange1		= 640;
	m_fMinRange2		= 0.0;
	m_fMaxRange2		= 640;
}

//-----------------------------------------------------------------------------
void CWeapon870::Spawn( void )
{
	BaseClass::Spawn();

	for ( int i = 0; i < MAX_CLIP; i++ )
		m_AmmoQueue.Set( i, Q_EMPTY );
	for ( int i = 0; i < m_iClip1; i++ )
		m_AmmoQueue.Set( i, Q_BUCKSHOT );
	m_AmmoType = Q_BUCKSHOT;
	m_NextAmmoType = -1;
}

//-----------------------------------------------------------------------------
void CWeapon870::Precache( void )
{
	CBaseCombatWeapon::Precache();
}

//-----------------------------------------------------------------------------
void CWeapon870::Equip( CBaseCombatCharacter *pOwner )
{
	// HL1 superzombie had range of 1024 on all weapons.
	// This is mainly for the namf1 szombie-vs-szombie battle.
	if ( FClassnameIs( pOwner, "npc_superzombie" ) )
	{
		m_fMaxRange1 = m_fMaxRange2 = 800;
	}
	else
	{
		m_fMaxRange1 = m_fMaxRange2 = 600;
	}

	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
void CWeapon870::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT( npc != NULL );
	WeaponSound( SINGLE_NPC );
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;

	if ( bUseWeaponAngles )
	{
		QAngle	angShootDir;
		GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
		AngleVectors( angShootDir, &vecShootDir );
	}
	else 
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );
	}

	pOperator->FireBullets( 8, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon870::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack( pOperator, true );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon870::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_SHOTGUN_FIRE:
		{
			FireNPCPrimaryAttack( pOperator, false );
		}
		break;

		// This event fires in the viewmodel reload animation when the shell is inserted
		case EVENT_WEAPON_RELOAD:
			FillClip();
			WeaponSound( RELOAD );
			SetViewModelBodygroup(W870_BODYGROUP_SHELL, W870_SHELL_BLANK);
			break;

		default:
			CBaseCombatWeapon::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeapon870::StartReload( void )
{
#if defined(HOE_IRONSIGHTS)
	if ( IsIronSightsActive() )
	{
		ToggleIronSights();
		return false;
	}
#endif // HOE_IRONSIGHTS

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return false;

	if ( pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 && pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0 )
		return false;

	if ( m_iClip1 >= GetMaxClip1() )
		return false;

#if 0 // pump at end of every reload (ACT_SHOTGUN_RELOAD_FINISH == pump)
	// If shotgun totally emptied then a pump animation is needed
	
	//NOTENOTE: This is kinda lame because the player doesn't get strong feedback on when the reload has finished,
	//			without the pump.  Technically, it's incorrect, but it's good for feedback...

	if (m_iClip1 <= 0)
	{
		m_bNeedPump = true;
	}
#endif

	bool bElephant = m_AmmoType == Q_ELEPHANTSHOT;
	int j = min(1, pOwner->GetAmmoCount( bElephant ? (int)m_iSecondaryAmmoType : (int)m_iPrimaryAmmoType ));

	if (j <= 0)
		return false;

	m_NextAmmoType = -1;

	SendWeaponAnim( ACT_SHOTGUN_RELOAD_START );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = GetWeaponIdleTime();

	m_bInReload = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeapon870::Reload( void )
{
	// Check that StartReload was called first
	if (!m_bInReload)
	{
		Warning("ERROR: Shotgun Reload called incorrectly!\n");
	}

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return false;

	if ( m_AmmoType == Q_BUCKSHOT && pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		m_AmmoType = Q_ELEPHANTSHOT;
	if ( m_AmmoType == Q_ELEPHANTSHOT && pOwner->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 )
	{
		if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
			return false;
		m_AmmoType = Q_BUCKSHOT;
	}
	bool bElephant = m_AmmoType == Q_ELEPHANTSHOT;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	int j = min(1, pOwner->GetAmmoCount( bElephant ? (int)m_iSecondaryAmmoType : (int)m_iPrimaryAmmoType ));

	if (j <= 0)
		return false;

	// Make shotgun shell visible
	SetViewModelBodygroup(W870_BODYGROUP_SHELL, bElephant ? W870_SHELL_ELEPHANT : W870_SHELL_BUCKSHOT);

//	FillClip();

	// Play reload on different channel as otherwise steals channel away from fire sound
//	WeaponSound( RELOAD );
	SendWeaponAnim( ACT_VM_RELOAD );

#if defined( HOE_THIRDPERSON ) && !defined( CLIENT_DLL )
	ToHL2Player(pOwner)->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
#endif // HOE_THIRDPERSON

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = GetWeaponIdleTime();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon870::FinishReload( void )
{
	// Make shotgun shell invisible
//	SetViewModelBodygroup(W870_BODYGROUP_SHELL, W870_SHELL_BLANK);

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return;

	m_bInReload = false;

	// Finish reload animation
	SendWeaponAnim( ACT_SHOTGUN_RELOAD_FINISH );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = GetWeaponIdleTime();
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon870::FillClip( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return;

	bool bElephant = m_AmmoType == Q_ELEPHANTSHOT;

	// Add them to the clip
	if ( pOwner->GetAmmoCount( bElephant ? (int)m_iSecondaryAmmoType : (int)m_iPrimaryAmmoType) > 0 )
	{
		if ( Clip1() < GetMaxClip1() )
		{
			m_iClip1++;

			// Add to FRONT of queue
			m_AmmoQueue.Set( m_iClip1 - 1, m_AmmoType );
//			static char *strings[3] = { "0", "B", "E" }; DevMsg("CWeapon870::FillClip:"); for ( int i = 0; i < m_iClip1; i++ ) DevMsg("%s ", (m_AmmoQueue[i] >= 0 && m_AmmoQueue[i] < 3) ? strings[m_AmmoQueue[i]] : "ERROR"); DevMsg("\n");
			pOwner->RemoveAmmo( 1, bElephant ? (int)m_iSecondaryAmmoType : (int)m_iPrimaryAmmoType );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play weapon pump anim
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon870::Pump( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();

	if ( pOwner == NULL )
		return;
	
	m_bNeedPump = false;
	
//	WeaponSound( SPECIAL1 );  animation event plays the sound

	// Finish reload animation
	SendWeaponAnim( ACT_SHOTGUN_PUMP );

	pOwner->m_flNextAttack	= GetWeaponIdleTime();
	m_flNextPrimaryAttack	= GetWeaponIdleTime();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeapon870::DryFire( void )
{
	WeaponSound(EMPTY);
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flNextPrimaryAttack = GetWeaponIdleTime();
}

//-----------------------------------------------------------------------------
void CWeapon870::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	bool bElephant = m_AmmoQueue[m_iClip1 - 1] == Q_ELEPHANTSHOT;

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound( bElephant ? WPN_DOUBLE : SINGLE );

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( bElephant ? ACT_VM_SECONDARYATTACK : ACT_VM_PRIMARYATTACK );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
#ifdef HOE_THIRDPERSON
	ToHL2Player(GetOwner())->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
#endif

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = GetWeaponIdleTime();

	m_AmmoQueue.Set( m_iClip1 - 1, Q_EMPTY );
	m_iClip1 -= 1;

	Vector	vecSrc		= pPlayer->Weapon_ShootPosition( );
	Vector	vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );	

	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 1.0 );
	
	// Fire the bullets, and force the first shot to be perfectly accurate
	pPlayer->FireBullets( bElephant ? 30 : 6, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, bElephant ? (int) m_iSecondaryAmmoType : (int) m_iPrimaryAmmoType, 0, -1, -1, 0, NULL, true );
	
	pPlayer->ViewPunch( QAngle( random->RandomFloat( -2, -1 ), random->RandomFloat( -2, 2 ), 0 ) );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2, GetOwner() );

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	if( m_iClip1 )
	{
		// pump so long as some rounds are left.
		// 870 has pump animation after each shot
		//m_bNeedPump = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeapon870::SecondaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	m_AmmoType = Q_ELEPHANTSHOT;
	StartReload();
}

//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CWeapon870::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
	{
		return;
	}

	if (m_bInReload)
	{
		// If I'm primary firing and have one round stop reloading and fire
		if ((pOwner->m_nButtons & IN_ATTACK ) && (m_iClip1 >=1))
		{
			m_bInReload		= false;
			m_bNeedPump		= false;
			m_bDelayedFire1 = true;
		}
#if 0
		// If I'm secondary firing and have one round stop reloading and fire
		else if ((pOwner->m_nButtons & IN_ATTACK2 ) && (m_iClip1 >=2))
		{
			m_bInReload		= false;
			m_bNeedPump		= false;
			m_bDelayedFire2 = true;
		}
#endif
		else
		{
			// If I try to reload while reloading switch to loading buckshot
			if ( (pOwner->m_nButtons & IN_RELOAD) && (pOwner->GetAmmoCount( m_iPrimaryAmmoType ) >= 1) )
			{
				m_NextAmmoType = Q_BUCKSHOT;
			}
			// If I'm secondary firing and have elephantshot then switch to reloading that type
			else  if ( (pOwner->m_nButtons & IN_ATTACK2) && (pOwner->GetAmmoCount( m_iSecondaryAmmoType ) >= 1) )
			{
				m_NextAmmoType = Q_ELEPHANTSHOT;
			}

			// Reload animation finished
			if ( m_flNextPrimaryAttack <= gpGlobals->curtime )
			{
				// If out of ammo end reload
				if ( pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 && pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0 )
				{
					FinishReload();
					return;
				}
				// If clip not full reload again
				if (m_iClip1 < GetMaxClip1())
				{
					// Did the user change ammo types during reloading?
					if (m_NextAmmoType != -1)
					{
						m_AmmoType = m_NextAmmoType;
						m_NextAmmoType = -1;
					}
					Reload();
					return;
				}
				// Clip full, stop reloading
				else
				{
					FinishReload();
					return;
				}
			}
		}
	}
	else
	{			
		// Make shotgun shell invisible
		SetViewModelBodygroup(W870_BODYGROUP_SHELL, W870_SHELL_BLANK);
	}

	if ((m_bNeedPump) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		Pump();
		return;
	}
#if 0
	// Shotgun uses same timing and ammo for secondary attack
	if ((m_bDelayedFire2 || pOwner->m_nButtons & IN_ATTACK2)&&(m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		m_bDelayedFire2 = false;
		
		if ( (m_iClip1 <= 1 && UsesClipsForAmmo1()))
		{
			// If only one shell is left, do a single shot instead	
			if ( m_iClip1 == 1 )
			{
				PrimaryAttack();
			}
			else if (!pOwner->GetAmmoCount(m_iPrimaryAmmoType))
			{
				DryFire();
			}
			else
			{
				StartReload();
			}
		}

		// Fire underwater?
		else if (GetOwner()->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			if ( pOwner->m_afButtonPressed & IN_ATTACK )
			{
				 m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			SecondaryAttack();
		}
	}
	else 
#endif
	if ( (m_bDelayedFire1 || pOwner->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		m_bDelayedFire1 = false;
		if ( (m_iClip1 <= 0 && UsesClipsForAmmo1()) || ( !UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType) ) )
		{
			if ( !pOwner->GetAmmoCount(m_iPrimaryAmmoType) && !pOwner->GetAmmoCount(m_iSecondaryAmmoType) )
			{
				DryFire();
			}
			else
			{
				// Reset to loading buckshot not elephantshot
				m_AmmoType = Q_BUCKSHOT;
				if ( !pOwner->GetAmmoCount(m_iPrimaryAmmoType) )
					m_AmmoType = Q_ELEPHANTSHOT;

				StartReload();
			}
		}
		// Fire underwater?
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
#ifdef HOE_IRONSIGHTS
		// Switch to ironsight automatically when trying to shoot.
		else if ( hoe_ironsights.GetBool() && hoe_ironsights_auto.GetBool() &&
			HasIronSights() && GetIronSightsState() == IRONSIGHT_STATE_NONE )
		{
			ToggleIronSights();
		}
#endif // HOE_IRONSIGHTS
		else
		{
			// If the firing button was just pressed, reset the firing time
			CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
			if ( pPlayer && pPlayer->m_afButtonPressed & IN_ATTACK )
			{
				 m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			PrimaryAttack();
		}
	}

	if ( (pOwner->m_nButtons & IN_RELOAD) && UsesClipsForAmmo1() && !m_bInReload ) 
	{
		// Reset to loading buckshot not elephantshot
		m_AmmoType = Q_BUCKSHOT;
		if ( !pOwner->GetAmmoCount(m_iPrimaryAmmoType) )
			m_AmmoType = Q_ELEPHANTSHOT;

		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		StartReload();
	}
	else if ( (pOwner->m_nButtons & IN_ATTACK2) && !m_bInReload ) 
	{
		// Reload with elephantshot
		SecondaryAttack();
	}
	else 
	{
		// no fire buttons down
		m_bFireOnEmpty = false;

		if ( !HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime ) 
		{
			// weapon isn't useable, switch.
			if ( !(GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && pOwner->SwitchToNextBestWeapon( this ) )
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if ( m_iClip1 <= 0 && !(GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < gpGlobals->curtime )
			{
				// Reset to loading buckshot not elephantshot
				m_AmmoType = Q_BUCKSHOT;
				if ( !pOwner->GetAmmoCount(m_iPrimaryAmmoType) )
					m_AmmoType = Q_ELEPHANTSHOT;

				if ( StartReload() )
				{
					// if we've successfully started to reload, we're done
					return;
				}
			}
		}

#ifdef HOE_IRONSIGHTS
		if ( pOwner->m_afButtonPressed & IN_IRONSIGHTS )
			ToggleIronSights();
#endif // HOE_IRONSIGHTS

		WeaponIdle( );
		return;
	}
}

//-----------------------------------------------------------------------------
void CWeapon870::ItemHolsterFrame( void )
{
	// Must be player held
	if ( GetOwner() && GetOwner()->IsPlayer() == false )
		return;

	// We can't be active
	if ( GetOwner()->GetActiveWeapon() == this )
		return;

	// If it's been longer than three seconds, reload
	if ( ( gpGlobals->curtime - m_flHolsterTime ) > sk_auto_reload_time.GetFloat() )
	{
		// Reset the timer
		m_flHolsterTime = gpGlobals->curtime;
	
		if ( GetOwner() == NULL )
			return;

		if ( m_iClip1 == GetMaxClip1() )
			return;

		// Just load the clip with no animations
		int ammoFill = min( (GetMaxClip1() - m_iClip1), GetOwner()->GetAmmoCount( GetPrimaryAmmoType() ) );
		
		GetOwner()->RemoveAmmo( ammoFill, GetPrimaryAmmoType() );
		for ( int i = 0; i < ammoFill; i++ )
			m_AmmoQueue.Set( m_iClip1 + i, Q_BUCKSHOT );
		m_iClip1 += ammoFill;
	}
}

//==================================================
// Purpose: 
//==================================================
/*
void CWeapon870::WeaponIdle( void )
{
	//Only the player fires this way so we can cast
	CBasePlayer *pPlayer = GetOwner()

	if ( pPlayer == NULL )
		return;

	//If we're on a target, play the new anim
	if ( pPlayer->IsOnTarget() )
	{
		SendWeaponAnim( ACT_VM_IDLE_ACTIVE );
	}
}
*/

//-----------------------------------------------------------------------------
void CWeapon870::SetViewModel( void )
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
void CWeapon870::SetViewModelBodygroup( int iGroup, int iValue )
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
