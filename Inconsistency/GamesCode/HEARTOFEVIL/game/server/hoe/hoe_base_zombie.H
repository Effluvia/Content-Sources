#include "ai_baseactor.h"
#include "ai_behavior_follow.h"
#include "engine/IEngineSound.h"
#include "npcevent.h"

class CHOEBaseZombie;

class CZombieFollowBehavior : public CAI_FollowBehavior
{
	DECLARE_CLASS( CZombieFollowBehavior, CAI_FollowBehavior );
public:

	int FollowCallBaseSelectSchedule( void );
	int SelectSchedule( void );
	int TranslateScheduleHack( int scheduleType );
	CHOEBaseZombie *GetOuter( void ) { return (CHOEBaseZombie *)BaseClass::GetOuter(); }
};

class CHOEBaseZombie : public CAI_BaseActor
{
public:
	DECLARE_CLASS( CHOEBaseZombie, CAI_BaseActor );
//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	virtual void SelectModel( void ) = 0;
	virtual void SelectModelGroups( void ) = 0;
	bool CreateBehaviors( void );

	bool ShouldGib( const CTakeDamageInfo &info );
	bool HasHumanGibs( void ) { return true; }
	bool HasAlienGibs( void ) { return false; }
	
	int GetSoundInterests( void )
	{
		return SOUND_COMBAT |
			SOUND_WORLD |
			SOUND_PLAYER |
			SOUND_DANGER |
			SOUND_BULLET_IMPACT |
			SOUND_PHYSICS_DANGER |
			SOUND_MOVE_AWAY |
			ALL_SCENTS;
	}
	void RemoveIgnoredConditions( void );

	bool HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sourceEnt );

	virtual bool CheckFollowPlayer( void ) = 0;

	virtual const char *GetSentenceGroup( void ) const = 0;
	void InitSentences( void )
	{
		enginesound->PrecacheSentenceGroup( GetSentenceGroup() );
	}
	void Speak( const char *pSentence );

	int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);

	virtual int GetHeadGroupNum( void ) = 0;
	virtual int GetNumHeads( void ) = 0;
	virtual const char *GetHeadModelName( void ) = 0;
	bool HasAHead( void ) { return m_nHeadNum < GetNumHeads(); }
	bool GetHeadPosition( Vector &origin, QAngle &angles );
	virtual void Behead( void );

	void AlertSound( void );
	void AttackSound( void );
	void IdleSound( void );
	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info );


	Disposition_t IRelationType( CBaseEntity *pTarget );
	bool AfraidOfHuey( void );

	void HandleAnimEvent( animevent_t *pEvent );

	bool FInAimCone( const Vector &vecSpot );

	int MeleeAttack1Conditions( float flDot, float flDist );
	int MeleeAttack2Conditions( float flDot, float flDist );

	float MaxYawSpeed( void ) { return 35.0f; }

	int m_nHeadNum;
	float m_flChainsawedTime; // Last time we were cut by a chainsaw
	float m_flHungryTime;
	float m_flPainTime;
	float m_flAttackedByHueyTime;
	float m_flLastKick;
	float m_flChaseEnemyTime;

	//-----------------------------------------------------------------------------
	// Custom schedules.
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_ZOMBIE_CHAINSAWED = BaseClass::NEXT_SCHEDULE,
		SCHED_ZOMBIE_EAT,
		SCHED_ZOMBIE_RELOAD,
		NEXT_SCHEDULE,
	};

	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		TASK_ZOMBIE_DONT_EAT = BaseClass::NEXT_TASK,
		TASK_ZOMBIE_EAT,
		TASK_ZOMBIE_SOUND_EAT,
		NEXT_TASK
	};

	//-----------------------------------------------------------------------------
	// Activities
	//-----------------------------------------------------------------------------
	static Activity ACT_ZOMBIE_CHAINSAWED;
	static Activity ACT_ZOMBIE_EAT;

	//-----------------------------------------------------------------------------
	// Animation events.
	//-----------------------------------------------------------------------------
	static Animevent AE_ZOMBIE_BITE;
	static Animevent AE_ZOMBIE_KICK;

	//
	// This next stuff is from CHOEHumanFollower
	//

	int ObjectCaps( void );

	void GatherConditions( void );
	int SelectSchedule( void );
	int SelectSchedulePlayerPush( void );
	int SelectSchedulePostBehavior( void );
	void BuildScheduleTestBits( void );
	int TranslateSchedule( int scheduleType );
	void StartTask( const Task_t *pTask );
	bool IgnorePlayerPushing( void );
	bool ShouldDeferToFollowBehavior( void );
	void SpeedBoost( void );

	void Touch( CBaseEntity *pOther );
	void PredictPlayerPush( void );

	float GetIdealSpeed() const;
	float GetIdealAccel() const;
	bool OnObstructionPreSteer( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult );

	void UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	CZombieFollowBehavior &GetFollowBehavior( void ) { return m_FollowBehavior; }
	bool IsFollowingPlayer( void );

	void OnStateChange( NPC_STATE oldState, NPC_STATE newState );

	CZombieFollowBehavior m_FollowBehavior;
	COutputEvent m_OnPlayerUse;
	bool m_bMovingAwayFromPlayer;
	float m_flBoostSpeed;

#define PLAYER_SQUAD_STUFF // code for telling our squad where to go ALA HL2 ( c and cc commands ).
#ifdef PLAYER_SQUAD_STUFF

	bool ShouldAlwaysThink( void );
	void PrescheduleThink( void );

	void ClearFollowTarget( void );
	bool IsFollowingCommandPoint( void );
	void UpdateFollowCommandPoint( void );
	bool HaveCommandGoal( void ) const;	

	// BASE NPC OVERRIDES
	//{
	// Can the player order this member to go places
	virtual bool IsCommandable( void );

	// Nearest squad member that sees the player
	virtual CAI_BaseNPC *GetSquadCommandRepresentative( void );

	virtual bool TargetOrder( CBaseEntity *pTarget, CAI_BaseNPC **Allies, int numAllies );

	virtual void MoveOrder( const Vector &vecDest, CAI_BaseNPC **Allies, int numAllies );

	virtual void OnMoveToCommandGoalFailed( void );
	//}

#endif // PLAYER_SQUAD_STUFF
};