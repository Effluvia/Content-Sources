class CCodplay : public CHalfLifeMultiplay
{
public:
	CCodplay();
	virtual BOOL IsCOD( void );
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
};