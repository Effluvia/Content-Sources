class CLogicHueyDeploy : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicHueyDeploy, CLogicalEntity );
	DECLARE_DATADESC();

	void Activate( void );
	bool GetDeployCounts( int *perDeploy = NULL, int *grunts = NULL, int *medics = NULL );
	string_t GetSquadName( void ) const { return m_iszSquadName; }

	static void RegisterNPC( string_t iszDeploy, CAI_BaseNPC *pNPC );
	static void UnregisterNPC( string_t iszDeploy, CAI_BaseNPC *pNPC );

	string_t m_iszHueyName; // Name of the npc_huey that uses us
	string_t m_iszPathTrack; // path_track to deploy at
	string_t m_iszSquadName; // squad name for NPCs deployed here

	int m_iMaxGrunts; // Population of grunts to maintain
	int m_iMaxMedics; // Population of medics to maintain
	int m_iMaxBoth; // Population of grunts/medics to maintain
	int m_iPerDeploy; // Max number of grunts/medics to drop at once

	CUtlVector<EHANDLE> m_DeployedGrunts; // Currently deployed grunts. NOT SAVED/RESTORED
	CUtlVector<EHANDLE> m_DeployedMedics; // Currently deployed medics. NOT SAVED/RESTORED

	COutputEHANDLE m_OnSpawnNPC;
};

#define HUEY_DEPLOY_CLASSNAME "logic_huey_deploy"