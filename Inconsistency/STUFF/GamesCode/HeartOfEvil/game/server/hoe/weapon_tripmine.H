//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		SLAM 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	WEAPONTRIPMINE_H
#define	WEAPONTRIPMINE_H
#ifndef HL2MP

#include "basegrenade_shared.h"
#include "basehlcombatweapon.h"

class CWeaponTripmine : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponTripmine, CBaseHLCombatWeapon );

	DECLARE_SERVERCLASS();

	bool				m_bClearReload;
	bool				m_bNeedReload;
	bool				m_bAttachTripmine;

//	void				Spawn( void );
	void				Precache( void );

	int					CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	void				PrimaryAttack( void );
	void				SecondaryAttack( void );
	void				WeaponIdle( void );
//	void				WeaponSwitch( void );
//	void				TripmineThink( void );
	
//	void				SetPickupTouch( void );
//	void				TripmineTouch( CBaseEntity *pOther );	// default weapon touch

	void				ItemPostFrame( void );	
	bool				Reload( void );

//	void				SetViewModelBodygroup( int iGroup, int iValue );

	bool				CanAttach( trace_t &tr );		// In position where can attach SLAM?
	void				StartAttach( void );
	void				Attach( const trace_t &tr );

	bool				Deploy( void );
	bool				Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	void				RegisterPrivateActivities( void );

	CWeaponTripmine();

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
};

#endif  //HL2MP
#endif	//WEAPONTRIPMINE_H
