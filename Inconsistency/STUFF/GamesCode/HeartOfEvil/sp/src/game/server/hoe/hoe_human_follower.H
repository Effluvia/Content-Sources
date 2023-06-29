#ifndef HOE_HUMAN_FOLLWER_H
#define HOE_HUMAN_FOLLWER_H

#include "ai_behavior_follow.h"
#include "ai_behavior_lead.h"
#include "hoe_human.h"


class CHOEFollowBehavior : public CAI_FollowBehavior
{
	DECLARE_CLASS( CHOEFollowBehavior, CAI_FollowBehavior );
public:

	int FollowCallBaseSelectSchedule( void );
	int SelectSchedule( void );
	int TranslateScheduleHack( int scheduleType );
	CHOEHuman *GetOuter( void ) { return (CHOEHuman *)BaseClass::GetOuter(); }
};

class CHOEHumanFollower : public CHOEHuman
{
public:
	DECLARE_CLASS( CHOEHumanFollower, CHOEHuman );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	bool CreateBehaviors( void );

	int ObjectCaps( void );

	Disposition_t IRelationType( CBaseEntity *pTarget );
	int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void Event_Killed( const CTakeDamageInfo &info );
	void NotifyFriendsOfPlayerDamage( CBaseEntity *pPlayer );

	bool IdleSpeech( void );

	bool AfraidOfHuey( void );

	bool ShouldAlwaysThink( void );

	void GatherConditions( void );
	void PrescheduleThink( void );
	int SelectSchedulePriority( void );
	int SelectSchedulePlayerPush( void );
	int TranslateSchedule( int scheduleType );
	void BuildScheduleTestBits( void );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );
	bool IgnorePlayerPushing( void );
	bool ShouldDeferToFollowBehavior( void );
	void SpeedBoost( void );

	void Touch( CBaseEntity *pOther );
	void PredictPlayerPush( void );

	float GetIdealSpeed() const;
	float GetIdealAccel() const;
	bool OnObstructionPreSteer( AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult );
	virtual bool ShouldBlockerMoveAway( CAI_BaseNPC *pBlocker );

	void UseFollower( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void InputAddToPlayerSquad( inputdata_t &inputdata );

	CHOEFollowBehavior &GetFollowBehavior( void ) { return m_FollowBehavior; }
	bool IsFollowingPlayer( void );

	void OnStateChange( NPC_STATE oldState, NPC_STATE newState );

	virtual bool CheckFollowPlayer( void );

	bool FValidateHintType( CAI_Hint *pHint );
	bool ShouldAlwaysTransition( void );
	void InputOutsideTransition( inputdata_t &inputdata );

	CHOEFollowBehavior m_FollowBehavior;
	CAI_LeadBehavior m_LeadBehavior;
	COutputEvent m_OnPlayerUse;
	bool m_bMovingAwayFromPlayer;
	float m_flBoostSpeed;
	float m_flOutWayTime;
	float m_flPlayerPushTime; // Last time the player pushed this NPC
	int m_iPlayerPushCount; // Number of close-in-time pushes by the player
	float m_flTimeShotByPlayer; // Last time player shot this NPC
	float m_flTimePlayerShotFriend; // Last time this NPC saw the player shoot a friend
	bool m_bAlwaysTransition;

#define PLAYER_SQUAD_STUFF // code for telling our squad where to go ALA HL2 ( c and cc commands ).
#ifdef PLAYER_SQUAD_STUFF

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

#endif // HOE_HUMAN_FOLLWER_H
