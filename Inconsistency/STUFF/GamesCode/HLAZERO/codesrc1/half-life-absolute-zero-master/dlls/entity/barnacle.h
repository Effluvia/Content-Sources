




#ifndef BARNACLE_H
#define BARNACLE_H


#include "util.h"
#include "cbase.h"
//#include "effects.h"
#include "basemonster.h"



class CBarnacle : public CBaseMonster
{
public:

	//MODDD - new
	CBarnacle();

	static int s_iStandardGibID;
	static BOOL s_fStandardGibDecal;

	static int BarnacleGetStandardGibSpawnID(void);

	
	int IRelationship ( CBaseEntity *pTarget );
	int forcedRelationshipWith(CBaseEntity *pWith);

	void StartReanimationPost(int preReviveSequence);

	void SetActivity ( Activity NewActivity );

	BOOL loweredPreviously;
	BOOL retractedPreviously;
	float getTentacleSuddenAnimFrameRate(void);

	void Spawn( void );
	void Precache( void );
	CBaseEntity *TongueTouchEnt ( float *pflLength );
	CBaseEntity *TongueTouchEnt ( float *pflLength, float *pflLengthMinimal );


	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void EXPORT BarnacleThink ( void );
	void EXPORT WaitTillDead ( void );
	
	GENERATE_GIBMONSTER_PROTOTYPE
	GENERATE_GIBMONSTERGIB_PROTOTYPE

	GENERATE_KILLED_PROTOTYPE
	
	//MODDD
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE
	
	

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flAltitude;
	float m_flKillVictimTime;
	int   m_cGibs;// barnacle loads up on gibs each time it kills something.
	BOOL  m_fTongueExtended;
	BOOL  m_fLiftingPrey;
	float m_flTongueAdj;

	BOOL smallerTest;

	//MODDD - am I dead?  If so, don't allow re-toggling the death anim.
	BOOL barnacleDeathActivitySet;

	float retractDelay;

};



#endif //END OF BARNACLE_H