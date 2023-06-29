class CNPCDeathSound : public CBaseEntity
{
public:
	DECLARE_CLASS( CNPCDeathSound, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CNPCDeathSound()
	{
	};

	void Spawn( void );

	int UpdateTransmitState( void )
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	CNetworkString( m_szSoundName, 128 );
};
