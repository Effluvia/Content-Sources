//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_hl2_playerlocaldata.h"
#include "dt_recv.h"
#ifdef HOE_DLL
#include "gamestringpool.h"
#include "dt_utlvector_recv.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef HOE_DLL
void RecvProxy_StringToString_t( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	string_t *pStrOut = (string_t*)pOut;
	if ( pData->m_pRecvProp->m_StringBufferSize <= 0 )
	{
		*pStrOut = NULL_STRING;
		return;
	}
	*pStrOut = AllocPooledString( pData->m_Value.m_pString );
}

BEGIN_RECV_TABLE_NOBASE(C_PlayerSquadInfo, DT_PlayerSquadInfo)
	RecvPropInt( RECVINFO(frac) ),
	RecvPropInt( RECVINFO(health) ),
	RecvPropInt( RECVINFO(flags) )
END_RECV_TABLE()

#endif // HOE_DLL

BEGIN_RECV_TABLE_NOBASE( C_HL2PlayerLocalData, DT_HL2Local )
	RecvPropFloat( RECVINFO(m_flSuitPower) ),
	RecvPropInt( RECVINFO(m_bZooming) ),
	RecvPropInt( RECVINFO(m_bitsActiveDevices) ),
#ifdef HOE_DLL
	RecvPropUtlVectorDataTable( m_SquadMembers, 16, DT_PlayerSquadInfo ),
#else // HOE_DLL
	RecvPropInt( RECVINFO(m_iSquadMemberCount) ),
	RecvPropInt( RECVINFO(m_iSquadMedicCount) ),
	RecvPropBool( RECVINFO(m_fSquadInFollowMode) ),
#endif // HOE_DLL
	RecvPropBool( RECVINFO(m_bWeaponLowered) ),
	RecvPropEHandle( RECVINFO(m_hAutoAimTarget) ),
	RecvPropVector( RECVINFO(m_vecAutoAimPoint) ),
	RecvPropEHandle( RECVINFO(m_hLadder) ),
	RecvPropBool( RECVINFO(m_bDisplayReticle) ),
	RecvPropBool( RECVINFO(m_bStickyAutoAim) ),
#ifdef HOE_DLL
	RecvPropArray3( RECVINFO_ARRAY(m_iszLetterNames), RecvPropString( RECVINFO(m_iszLetterNames[0]), 0, RecvProxy_StringToString_t ) ),
#endif // HOE_DLL
	RecvPropBool( RECVINFO(m_bAutoAimTarget) ),
#ifdef HL2_EPISODIC
	RecvPropFloat( RECVINFO(m_flFlashBattery) ),
	RecvPropVector( RECVINFO(m_vecLocatorOrigin) ),
#endif
END_RECV_TABLE()

BEGIN_PREDICTION_DATA_NO_BASE( C_HL2PlayerLocalData )
	DEFINE_PRED_FIELD( m_hLadder, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

C_HL2PlayerLocalData::C_HL2PlayerLocalData()
{
	m_flSuitPower = 0.0;
	m_bZooming = false;
#ifdef HOE_DLL
	m_SquadMembers.RemoveAll(); // useless
#else // HOE_DLL
	m_iSquadMemberCount = 0;
	m_iSquadMedicCount = 0;
	m_fSquadInFollowMode = false;
#endif // HOE_DLL
	m_bWeaponLowered = false;
	m_hLadder = NULL;
#ifdef HL2_EPISODIC
	m_flFlashBattery = 0.0f;
	m_vecLocatorOrigin = vec3_origin;
#endif
}

