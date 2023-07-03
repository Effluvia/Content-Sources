//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2_playerlocaldata.h"
#include "hl2_player.h"
#include "mathlib/mathlib.h"
#include "entitylist.h"

#ifdef HOE_DLL
#include "dt_utlvector_send.h"
#include "saverestore_utlvector.h"

// see env_screenoverlay.cpp
extern void SendProxy_String_tToString( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );
#endif // HOE_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef HOE_DLL
BEGIN_SEND_TABLE_NOBASE(CPlayerSquadInfo, DT_PlayerSquadInfo)
	SendPropInt(SENDINFO(frac), 8, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(health), 7, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(flags), 8, SPROP_UNSIGNED),
END_SEND_TABLE()
#endif // HOE_DLL

BEGIN_SEND_TABLE_NOBASE( CHL2PlayerLocalData, DT_HL2Local )
	SendPropFloat( SENDINFO(m_flSuitPower), 10, SPROP_UNSIGNED | SPROP_ROUNDUP, 0.0, 100.0 ),
	SendPropInt( SENDINFO(m_bZooming), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_bitsActiveDevices), MAX_SUIT_DEVICES, SPROP_UNSIGNED ),
#ifdef HOE_DLL
	SendPropUtlVectorDataTable( m_SquadMembers, 16, DT_PlayerSquadInfo ),
#else // HOE_DLL
	SendPropInt( SENDINFO(m_iSquadMemberCount) ),
	SendPropInt( SENDINFO(m_iSquadMedicCount) ),
	SendPropBool( SENDINFO(m_fSquadInFollowMode) ),
#endif // HOE_DLL
	SendPropBool( SENDINFO(m_bWeaponLowered) ),
	SendPropEHandle( SENDINFO(m_hAutoAimTarget) ),
	SendPropVector( SENDINFO(m_vecAutoAimPoint) ),
	SendPropEHandle( SENDINFO(m_hLadder) ),
	SendPropBool( SENDINFO(m_bDisplayReticle) ),
	SendPropBool( SENDINFO(m_bStickyAutoAim) ),
	SendPropBool( SENDINFO(m_bAutoAimTarget) ),
#ifdef HL2_EPISODIC
	SendPropFloat( SENDINFO(m_flFlashBattery) ),
	SendPropVector( SENDINFO(m_vecLocatorOrigin) ),
#endif
#ifdef HOE_DLL
	// Using SendPropArray() does not give reliable transmission???
	SendPropArray3( SENDINFO_ARRAY3(m_iszLetterNames), SendPropString( SENDINFO_ARRAY(m_iszLetterNames), 0, SendProxy_String_tToString ) ),
#endif // HOE_DLL
END_SEND_TABLE()

BEGIN_SIMPLE_DATADESC( CHL2PlayerLocalData )
	DEFINE_FIELD( m_flSuitPower, FIELD_FLOAT ),
	DEFINE_FIELD( m_bZooming, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bitsActiveDevices, FIELD_INTEGER ),
#ifdef HOE_DLL
//	DEFINE_UTLVECTOR( m_SquadMembers, FIELD_EMBEDDED ),
#else // HOE_DLL
	DEFINE_FIELD( m_iSquadMemberCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSquadMedicCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_fSquadInFollowMode, FIELD_BOOLEAN ),
#endif // HOE_DLL
	DEFINE_FIELD( m_bWeaponLowered, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDisplayReticle, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bStickyAutoAim, FIELD_BOOLEAN ),
#ifdef HL2_EPISODIC
	DEFINE_FIELD( m_flFlashBattery, FIELD_FLOAT ),
	DEFINE_FIELD( m_vecLocatorOrigin, FIELD_POSITION_VECTOR ),
#endif
#ifdef HOE_DLL
	DEFINE_AUTO_ARRAY( m_iszLetterNames, FIELD_STRING ),
#endif // HOE_DLL
	// Ladder related stuff
	DEFINE_FIELD( m_hLadder, FIELD_EHANDLE ),
	DEFINE_EMBEDDED( m_LadderMove ),
END_DATADESC()

CHL2PlayerLocalData::CHL2PlayerLocalData()
{
	m_flSuitPower = 0.0;
	m_bZooming = false;
	m_bWeaponLowered = false;
	m_hAutoAimTarget.Set(NULL);
	m_hLadder.Set(NULL);
	m_vecAutoAimPoint.GetForModify().Init();
	m_bDisplayReticle = false;
#ifdef HL2_EPISODIC
	m_flFlashBattery = 0.0f;
#endif
}

