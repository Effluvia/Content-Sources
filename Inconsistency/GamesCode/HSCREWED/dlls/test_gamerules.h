//
// test_gamerules
// Game rules for the game mode, for testing weapons
// and other beta testing.
//

#define JASON 0;

class CTestplay : public CHalfLifeMultiplay
{
public:
	CTestplay();
	virtual BOOL IsHeavyRain( void );
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
	virtual BOOL IsTest( void );
	//virtual int JasonsStolen( int jason );
};