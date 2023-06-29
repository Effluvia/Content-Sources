//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef VEHICLE_TRUCK_H
#define VEHICLE_TRUCK_H
#ifdef _WIN32
#pragma once
#endif

#include "vehicle_base.h"

#define TRUCK_WHEEL_COUNT	4

struct TruckWaterData_t
{
	bool		m_bWheelInWater[TRUCK_WHEEL_COUNT];
	bool		m_bWheelWasInWater[TRUCK_WHEEL_COUNT];
	Vector		m_vecWheelContactPoints[TRUCK_WHEEL_COUNT];
	float		m_flNextRippleTime[TRUCK_WHEEL_COUNT];
	bool		m_bBodyInWater;
	bool		m_bBodyWasInWater;

	DECLARE_SIMPLE_DATADESC();
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPropTruck : public CPropVehicleDriveable
{
	DECLARE_CLASS( CPropTruck, CPropVehicleDriveable );

public:

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CPropTruck( void );

	// CPropVehicle
	virtual void	ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData );
	virtual void	ItemPostFrame( CBasePlayer *pPlayer );
	virtual void	DriveVehicle( float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased );
	virtual void	SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move );
	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void	DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles );
	virtual bool	AllowBlockedExit( CBasePlayer *pPlayer, int nRole ) { return false; }
	virtual bool	CanExitVehicle( CBaseEntity *pEntity );
	virtual bool	IsVehicleBodyInWater() { return m_WaterData.m_bBodyInWater; }

	// CBaseEntity
	void			Think(void);
	void			Precache( void );
	void			Spawn( void ); 
	void			Activate( void );
	void			VPhysicsUpdate( IPhysicsObject *pPhysics );


	virtual void	CreateServerVehicle( void );
	virtual Vector	BodyTarget( const Vector &posSrc, bool bNoisy = true );
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );
	virtual float	PassengerDamageModifier( const CTakeDamageInfo &info );

	virtual void	EnterVehicle( CBaseCombatCharacter *pPassenger );
	virtual void	ExitVehicle( int nRole );

	// NPC Driving
	bool			NPC_HasPrimaryWeapon( void ) { return false; }

	bool HeadlightIsOn( void ) { return m_bHeadlightIsOn; }
	void HeadlightTurnOn( void ) { m_bHeadlightIsOn = true; }
	void HeadlightTurnOff( void ) { m_bHeadlightIsOn = false; }

	bool IsPlayerDriving( void ) { return m_bIsPlayerDriving; }
	bool ShouldForceExit() { return m_bForcedExit; }
	void ClearForcedExit() { m_bForcedExit = false; }

private:

	void		InitWaterData( void );
	void		HandleWater( void );
	bool		CheckWater( void );
	void		CheckWaterLevel( void );
	void		CreateSplash( const Vector &vecPosition );
	void		CreateRipple( const Vector &vecPosition );

	void		UpdateSteeringAngle( void );
	void		CreateDangerSounds( void );

	void		ComputePDControllerCoefficients( float *pCoefficientsOut, float flFrequency, float flDampening, float flDeltaTime );
	void		DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );
	void		DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );

	void		TruckSeagullThink( void );
	void		SpawnPerchedSeagull( void );
	void		AddSeagullPoop( const Vector &vecOrigin );

	void		InputShowHudHint( inputdata_t &inputdata );
	void		InputEnterVehicle( inputdata_t &inputdata );
	void		InputEnterVehicleImmediate( inputdata_t &inputdata );
	void		InputExitVehicle( inputdata_t &inputdata );
	void		InputExitVehicleImmediate( inputdata_t &inputdata );

private:

	float			m_flDangerSoundTime;
	float			m_throttleDisableTime;

	// handbrake after the fact to keep vehicles from rolling
	float			m_flHandbrakeTime;
	bool			m_bInitialHandbrake;

	float			m_flOverturnedTime;

	Vector			m_vecLastEyePos;
	Vector			m_vecLastEyeTarget;
	Vector			m_vecEyeSpeed;
	Vector			m_vecTargetSpeed;

	TruckWaterData_t	m_WaterData;

	int				m_iNumberOfEntries;

	// Seagull perching
	float			m_flPlayerExitedTime;	// Time at which the player last left this vehicle
	float			m_flLastSawPlayerAt;	// Time at which we last saw the player
	EHANDLE			m_hLastPlayerInVehicle;
	EHANDLE			m_hSeagull;
	bool			m_bHasPoop;

	CNetworkVar( bool, m_bHeadlightIsOn );

	CNetworkVar( bool, m_bIsPlayerDriving ); // Is player driver or passenger?
	bool			m_bLeanOutAnimOn;
	bool			m_bLeanInAnimOn;
	bool			m_bPlayerLeaning;
	float			m_flLeanStartTime;
	bool			m_bForcedExit;
};

#endif // VEHICLE_TRUCK_H
