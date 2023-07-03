//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#if !defined( C_HL2_PLAYERLOCALDATA_H )
#define C_HL2_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif


#include "dt_recv.h"

#include "hl2/hl_movedata.h"

EXTERN_RECV_TABLE( DT_HL2Local );

#ifdef HOE_DLL
class C_PlayerSquadInfo
{
public:
	DECLARE_CLASS_NOBASE( C_PlayerSquadInfo );
	DECLARE_SIMPLE_DATADESC();
	DECLARE_EMBEDDED_NETWORKVAR();

	C_PlayerSquadInfo()
	{
	}
	
	int frac; // 0-100% duration of die/join/leave
	int health; // 0-100%
	enum
	{
		PSI_FOLLOW = 0x01,
		PSI_USEABLE = 0x02,
		PSI_MEDIC = 0x04,
		PSI_JOIN = 0x08,
		PSI_LEAVE = 0x10,
		PSI_DIED = 0x20,
	};
    int flags;

};
#endif // HOE_DLL

class C_HL2PlayerLocalData
{
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLASS_NOBASE( C_HL2PlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	C_HL2PlayerLocalData();

	float	m_flSuitPower;
	bool	m_bZooming;
	int		m_bitsActiveDevices;
#ifdef HOE_DLL
	CUtlVector<C_PlayerSquadInfo> m_SquadMembers;
#else // HOE_DLL
	int		m_iSquadMemberCount;
	int		m_iSquadMedicCount;
	bool	m_fSquadInFollowMode;
#endif // HOE_DLL
	bool	m_bWeaponLowered;
	EHANDLE m_hAutoAimTarget;
	Vector	m_vecAutoAimPoint;
	bool	m_bDisplayReticle;
	bool	m_bStickyAutoAim;
#ifdef HOE_DLL
	string_t m_iszLetterNames[32];
#endif // HOE_DLL
	bool	m_bAutoAimTarget;
#ifdef HL2_EPISODIC
	float	m_flFlashBattery;
	Vector	m_vecLocatorOrigin;
#endif

	// Ladder related data
	EHANDLE			m_hLadder;
	LadderMove_t	m_LadderMove;
};


#endif
