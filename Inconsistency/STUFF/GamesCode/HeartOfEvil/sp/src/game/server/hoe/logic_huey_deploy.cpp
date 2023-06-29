#include "cbase.h"
#include "ai_basenpc.h"
#include "logic_huey_deploy.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( logic_huey_deploy, CLogicHueyDeploy );

BEGIN_DATADESC( CLogicHueyDeploy )
	DEFINE_KEYFIELD( m_iszHueyName,		FIELD_STRING, "HueyName" ),
	DEFINE_KEYFIELD( m_iszPathTrack,	FIELD_STRING, "path_track" ),
	DEFINE_KEYFIELD( m_iszSquadName,	FIELD_STRING, "squadname" ),

	DEFINE_KEYFIELD( m_iMaxGrunts,		FIELD_INTEGER, "MaxGrunts" ),
	DEFINE_KEYFIELD( m_iMaxMedics,		FIELD_INTEGER, "MaxMedics" ),
	DEFINE_KEYFIELD( m_iMaxBoth,		FIELD_INTEGER, "MaxBoth" ),
	DEFINE_KEYFIELD( m_iPerDeploy,		FIELD_INTEGER, "PerDeploy" ),

	DEFINE_OUTPUT( m_OnSpawnNPC,		"OnSpawnNPC" ),
END_DATADESC()

//-----------------------------------------------------------------------------
void CLogicHueyDeploy::Activate( void )
{
	BaseClass::Activate();

	if ( GetEntityName() == NULL_STRING )
	{
		DevWarning("%s with no targetname removed\n", HUEY_DEPLOY_CLASSNAME );
		UTIL_Remove( this );
	}
}

//-----------------------------------------------------------------------------
bool CLogicHueyDeploy::GetDeployCounts( int *deploy, int *grunts, int *medics )
{
	int numGrunts = 0;
	int numMedics = 0;

//	if ( !m_bEnabled ) return 0;

	// Example: if MaxBoth=3 MaxGrunts=3 MaxMedic=1, there will never be more than 3 total,
	// never more than 1 medic total, never more than 3 grunts total. All 3 could be grunts,
	// or 2 grunts/1 medic.
	if ( m_DeployedGrunts.Count() + m_DeployedMedics.Count() < m_iMaxBoth )
	{
		if ( (m_iMaxGrunts > 0) && (m_DeployedGrunts.Count() < m_iMaxGrunts) )
		{
			numGrunts = m_iMaxGrunts - m_DeployedGrunts.Count();
		}
		if ( (m_iMaxMedics > 0) && (m_DeployedMedics.Count() < m_iMaxMedics) )
		{
			numMedics = m_iMaxMedics - m_DeployedMedics.Count();
		}
	}

	if (deploy) *deploy = m_iPerDeploy;
	if (grunts) *grunts = numGrunts;
	if (medics) *medics = numMedics;
	return (m_iPerDeploy > 0) && (numGrunts + numMedics > 0);
}

//-----------------------------------------------------------------------------
void CLogicHueyDeploy::RegisterNPC( string_t iszDeploy, CAI_BaseNPC *pNPC )
{
	ASSERT( iszDeploy != NULL_STRING && pNPC != NULL );
	if ( iszDeploy == NULL_STRING || pNPC == NULL )
		return;

	// Register the NPC with *all* logic_huey_deploys that share the same targetname
	for ( CBaseEntity *pEnt = gEntList.FindEntityByName( NULL, STRING(iszDeploy) );
		pEnt != NULL;
		pEnt = gEntList.FindEntityByName( pEnt, STRING(iszDeploy) ) )
	{
		CLogicHueyDeploy *pDeploy = dynamic_cast<CLogicHueyDeploy *>(pEnt);
		if ( pDeploy )
		{
			CUtlVector<EHANDLE> &list = pNPC->IsMedic() ? pDeploy->m_DeployedMedics : pDeploy->m_DeployedGrunts;
			if ( list.Find( pNPC ) == -1 )
				list.AddToTail( pNPC );
			else
				DevWarning("CLogicHueyDeploy::RegisterNPC trying to register NPC twice\n");
		}
	}
}

//-----------------------------------------------------------------------------
void CLogicHueyDeploy::UnregisterNPC( string_t iszDeploy, CAI_BaseNPC *pNPC )
{
	ASSERT( iszDeploy != NULL_STRING && pNPC != NULL );
	if ( iszDeploy == NULL_STRING || pNPC == NULL )
		return;

	for ( CBaseEntity *pEnt = gEntList.FindEntityByName( NULL, STRING(iszDeploy) );
		pEnt != NULL;
		pEnt = gEntList.FindEntityByName( pEnt, STRING(iszDeploy) ) )
	{
		CLogicHueyDeploy *pDeploy = dynamic_cast<CLogicHueyDeploy *>(pEnt);
		if ( pDeploy )
		{
			CUtlVector<EHANDLE> &list = pNPC->IsMedic() ? pDeploy->m_DeployedMedics : pDeploy->m_DeployedGrunts;
			if ( list.Find( pNPC ) != -1 )
				list.FindAndRemove( pNPC );
			else
				DevWarning("CLogicHueyDeploy::UnregisterNPC NPC wasn't registered\n");
		}
	}
}