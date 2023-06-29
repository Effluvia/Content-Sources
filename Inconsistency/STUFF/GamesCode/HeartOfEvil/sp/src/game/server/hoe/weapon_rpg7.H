//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_RPG7_H
#define WEAPON_RPG7_H

#ifdef _WIN32
#pragma once
#endif

#include "basehlcombatweapon.h"
#include "Sprite.h"
#include "npcevent.h"
#include "beam_shared.h"

class CWeaponRPG7;
class CRPG7LaserDot;
class RocketTrail;
 
//###########################################################################
//	>> CRPG7Rocket		(missile launcher class is below this one!)
//###########################################################################
class CRPG7Rocket : public CBaseCombatCharacter
{
	DECLARE_CLASS( CRPG7Rocket, CBaseCombatCharacter );

public:
	static const int EXPLOSION_RADIUS = 200;

	CRPG7Rocket();
	~CRPG7Rocket();

#ifdef HL1_DLL
	Class_T Classify( void ) { return CLASS_NONE; }
#else
	Class_T Classify( void ) { return CLASS_MISSILE; }
#endif
	
	void	Spawn( void );
	void	Precache( void );
	void	MissileTouch( CBaseEntity *pOther );
	void	Explode( void );
	void	ShotDown( void );
	void	AccelerateThink( void );
	void	AugerThink( void );
	void	IgniteThink( void );
	void	SeekThink( void );
	void	DumbFire( void );
	void	SetGracePeriod( float flGracePeriod );

	int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void	Event_Killed( const CTakeDamageInfo &info );
	
	virtual float	GetDamage() { return m_flDamage; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }

	unsigned int PhysicsSolidMaskForEntity( void ) const;

	CHandle<CWeaponRPG7>		m_hOwner;

	static CRPG7Rocket *Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner );

	void CreateDangerSounds( bool bState ){ m_bCreateDangerSounds = bState; }

protected:
	virtual void DoExplosion();	
#ifdef RPG7_LASER
	virtual void ComputeActualDotPosition( CRPG7LaserDot *pLaserDot, Vector *pActualDotPosition, float *pHomingSpeed );
#endif
	virtual int AugerHealth() { return m_iMaxHealth - 20; }

	// Creates the smoke trail
	void CreateSmokeTrail( void );

	// Gets the shooting position 
	void GetShootPosition( CRPG7LaserDot *pLaserDot, Vector *pShootPosition );

	CHandle<RocketTrail>	m_hRocketTrail;
	float					m_flAugerTime;		// Amount of time to auger before blowing up anyway
	float					m_flMarkDeadTime;
	float					m_flDamage;

private:
	float					m_flGracePeriodEndsAt;
	bool					m_bCreateDangerSounds;

	DECLARE_DATADESC();
};


#ifdef RPG7_LASER
//-----------------------------------------------------------------------------
// Laser dot control
//-----------------------------------------------------------------------------
CBaseEntity *CreateLaserDot( const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot );
void SetLaserDotTarget( CBaseEntity *pLaserDot, CBaseEntity *pTarget );
void EnableLaserDot( CBaseEntity *pLaserDot, bool bEnable );
#endif

//-----------------------------------------------------------------------------
// RPG
//-----------------------------------------------------------------------------
class CWeaponRPG7 : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponRPG7, CBaseHLCombatWeapon );
public:

	CWeaponRPG7();
	~CWeaponRPG7();

	DECLARE_SERVERCLASS();

	void	Precache( void );

	void	Equip( CBaseCombatCharacter *pOwner );

	void	PrimaryAttack( void );
	virtual float GetFireRate( void ) { return 1; };
	void	ItemPostFrame( void );

	void	Activate( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	bool	Reload( void );
	bool	WeaponShouldBeLowered( void );
	bool	Lower( void );

	virtual void Drop( const Vector &vecVelocity );

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 1; }
	float	GetMinRestTime() { return 2.0; } // 2 second reload + 2 second delay
	float	GetMaxRestTime() { return 2.0; }

	// Rockets do not go through CONTENTS_GRATE like bullets do
	int		WeaponLOSMask( void ) { return MASK_SHOT_HULL; }

	bool	CheckForClearShot( const Vector &muzzlePos, const Vector &shootDir );
	bool	WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
	int		WeaponRangeAttack1Condition( float flDot, float flDist );

	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#ifdef RPG7_LASER
	void	StartGuiding( void );
	void	StopGuiding( void );
	void	ToggleGuiding( void );
	bool	IsGuiding( void );

	void	NotifyRocketDied( void );
#endif

	bool	HasAnyAmmo( void );

#ifdef RPG7_LASER
	void	SuppressGuiding( bool state = true );

	void	CreateLaserPointer( void );
	void	UpdateLaserPosition( Vector vecMuzzlePos = vec3_origin, Vector vecEndPos = vec3_origin );
	Vector	GetLaserPosition( void );
	void	StartLaserEffects( void );
	void	StopLaserEffects( void );
	void	UpdateLaserEffects( void );

	// NPC RPG users cheat and directly set the laser pointer's origin
	void	UpdateNPCLaserPosition( const Vector &vecTarget );
	void	SetNPCLaserPosition( const Vector &vecTarget );
	const Vector &GetNPCLaserPosition( void );
#endif
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	void	WeaponIdle( void );

	virtual void			WeaponSound( WeaponSound_t sound_type, float soundtime = 0.0f );

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone = VECTOR_CONE_3DEGREES;
		return cone;
	}
	
#ifdef RPG7_LASER
	CBaseEntity *GetMissile( void ) { return m_hMissile; }
#endif

#ifdef HOE_DLL
	virtual void SetViewModel( void );
	void SetViewModelBodygroup( int iGroup, int iValue );
	int m_nViewModelBody;
#endif

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
	
protected:

	bool				m_bInitialStateUpdate;
#ifdef RPG7_LASER
	bool				m_bGuiding;
	bool				m_bHideGuiding;		//User to override the player's wish to guide under certain circumstances
	Vector				m_vecNPCLaserDot;
	CHandle<CRPG7LaserDot>	m_hLaserDot;
	CHandle<CRPG7Rocket>	m_hMissile;
	CHandle<CSprite>	m_hLaserMuzzleSprite;
	CHandle<CBeam>		m_hLaserBeam;
#else
	bool m_bMustReload;
#endif
};

#endif // WEAPON_RPG7_H
