//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2_PLAYERLOCALDATA_H
#define HL2_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"

#include "hl_movedata.h"

#ifdef HOE_DLL
class CPlayerSquadInfo
{
public:
	DECLARE_CLASS_NOBASE( CPlayerSquadInfo );
//	DECLARE_SIMPLE_DATADESC();
//	DECLARE_EMBEDDED_NETWORKVAR();

	CPlayerSquadInfo()
	{
		Init( NULL );
	}

	void Init( CBasePlayer *pPlayer )
	{
		m_pPlayer = pPlayer;
		frac = 0;
		health = 0;
		flags = 0;
		m_hMember = NULL;
		m_flTime = 0;
		m_flDuration = 0;
	}

	// For CNetworkVars.
	void NetworkStateChanged();
	void NetworkStateChanged( void *pVar );

	CBasePlayer *m_pPlayer;

	CHandle<CAI_BaseNPC> m_hMember;
	float m_flTime; // time joined squad, left squad, or died
	float m_flDuration; // duration of join/leave/die

	
	CNetworkVar( int, frac ); // 0-100%, join/leave/die duration
	CNetworkVar( int, health ); // 0-100%

	enum
	{
		PSI_FOLLOW = 0x01,
		PSI_USEABLE = 0x02,
		PSI_MEDIC = 0x04,
		PSI_JOIN = 0x08,
		PSI_LEAVE = 0x10,
		PSI_DIED = 0x20,
	};
    CNetworkVar( int, flags );

};

inline void CPlayerSquadInfo::NetworkStateChanged()
{
	if ( m_pPlayer )
		m_pPlayer->NetworkStateChanged();
}

inline void CPlayerSquadInfo::NetworkStateChanged( void *pVar )
{
	if ( m_pPlayer )
		m_pPlayer->NetworkStateChanged();
}

#endif // HOE_DLL

//-----------------------------------------------------------------------------
// Purpose: Player specific data for HL2 ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CHL2PlayerLocalData
{
public:
	// Save/restore
	DECLARE_SIMPLE_DATADESC();
	DECLARE_CLASS_NOBASE( CHL2PlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CHL2PlayerLocalData();

	CNetworkVar( float, m_flSuitPower );
	CNetworkVar( bool,	m_bZooming );
	CNetworkVar( int,	m_bitsActiveDevices );
#ifdef HOE_DLL
	CUtlVector<CPlayerSquadInfo> m_SquadMembers;
#else // HOE_DLL
	CNetworkVar( int,	m_iSquadMemberCount );
	CNetworkVar( int,	m_iSquadMedicCount );
	CNetworkVar( bool,	m_fSquadInFollowMode );
#endif // HOE_DLL
	CNetworkVar( bool,	m_bWeaponLowered );
	CNetworkVar( EHANDLE, m_hAutoAimTarget );
	CNetworkVar( Vector, m_vecAutoAimPoint );
	CNetworkVar( bool,	m_bDisplayReticle );
	CNetworkVar( bool,	m_bStickyAutoAim );
	CNetworkVar( bool,	m_bAutoAimTarget );
#ifdef HL2_EPISODIC
	CNetworkVar( float, m_flFlashBattery );
	CNetworkVar( Vector, m_vecLocatorOrigin );
#endif
#ifdef HOE_DLL
	CNetworkArray( string_t, m_iszLetterNames, 32 );
#endif // HOE_DLL

	// Ladder related data
	CNetworkVar( EHANDLE, m_hLadder );
	LadderMove_t			m_LadderMove;
};

EXTERN_SEND_TABLE(DT_HL2Local);


#endif // HL2_PLAYERLOCALDATA_H
