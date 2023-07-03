#ifndef HOE_HUMAN_H
#define HOE_HUMAN_H

#include "ai_baseactor.h"
#include "ai_behavior_assault.h"
#include "ai_behavior_rappel.h"
#include "ai_behavior_standoff.h"
#include "hoe_behavior_passenger.h"
#include "ai_squad.h"
#include "engine/IEngineSound.h"
#include "hoe_behavior_rappel_huey.h"
#include "hoe_human_speech.h"

#define ENEMY_TELEPORTED_STUFF
#define HUMAN_STRAFE
#define HUMAN_UNSHOOTABLE

extern float g_flHumanSpeechTime;

struct HumanGrenadeInteractionData
{
	Vector vecOrigin; // where the grenade is aimed at
	float flRadius; // blast radius
	float flTime; // time we were told
	bool bRunning; // true if TASK_MOVE_AWAY_PATH should use flRadius
};

#ifdef HUMAN_UNSHOOTABLE
struct UnshootableEnt_t
{
	EHANDLE	hUnshootableEnt;	// Entity that's we couldn't find weapon los to
	float	fExpireTime;		// Time to forget this information
	Vector	vLocationWhenUnshootable;
	
	DECLARE_SIMPLE_DATADESC();
};
#endif

class CHOEHuman : public CAI_BaseActor
{
public:
	DECLARE_CLASS( CHOEHuman, CAI_BaseActor );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn( void );
	void Precache( void );
	virtual void SelectModel( void ) = 0;
	virtual void SelectModelGroups( void ) {};
	virtual bool CreateBehaviors( void );

	void OnRestore( void );

	int DrawDebugTextOverlays(void);

	CHOEHuman *HumanPointer( CBaseEntity *pEnt ) { return dynamic_cast<CHOEHuman *>( pEnt ); }

	Disposition_t IRelationType( CBaseEntity *pTarget );
	int IRelationPriority( CBaseEntity *pTarget );

	int GetSoundInterests( void )
	{
		return SOUND_COMBAT |
			SOUND_WORLD |
			SOUND_PLAYER |
			SOUND_DANGER |
			SOUND_BULLET_IMPACT |
			SOUND_PHYSICS_DANGER |
			SOUND_MOVE_AWAY |
			ALL_SCENTS; // so they can comment about stink
	}
	bool QueryHearSound( CSound *pSound );

	float MaxYawSpeed( void );

	bool ShouldGib( const CTakeDamageInfo &info );
	bool HasHumanGibs( void ) { return true; }
	bool HasAlienGibs( void ) { return false; }

	// This is the distance an NPC looks around for a cover node.
	// It is 1024 by default in ai_basenpc.h.
	// I changed CAI_TacticalServices::FindCoverNode to find a node
	// by path length rather than radius to stop NPCs running a long
	// way to the floor above/below.
	// I increased this distance to compensate.
	virtual float CoverRadius( void ) { return 2048; }

	bool FindCoverPos( CSound *pSound, Vector *pResult );

	void GatherConditions( void );
	void GatherEnemyConditions( CBaseEntity *pEnemy );
	void PrescheduleThink( void );
	void BuildScheduleTestBits( void );
	int SelectSchedule( void );
	virtual int SelectScheduleAlert( void );
	virtual int SelectScheduleCombat( void );
	virtual int SelectScheduleIdle( void );
	virtual int SelectScheduleDanger( void );
	virtual int SelectSchedulePriority( void );
	int SelectScheduleFromSquadCommand( void );
	int TranslateSchedule( int scheduleType );
	int SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	void SetTurnActivity( void );
	Activity NPC_TranslateActivity( Activity eNewActivity );
	virtual bool CanFlinch( void );

	bool AnyEnemyHasRangedAttack( void );

#ifdef HUMAN_STRAFE
	virtual bool ShouldStrafe( void );
	CSimpleSimTimer m_StrafeTimer;
#endif

	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );
	void HandleAnimEvent( animevent_t *pEvent );
	void TrackFootSplashes( int iAttachment, bool &bPrevInWater, Vector &prevOrigin );

	bool FInAimCone( const Vector &vecSpot );
	void MaintainLookTargets( float flInterval );

	bool ShouldIgnoreSound( CSound *pSound );

	bool HandleInteraction( int interactionType, void *data, CBaseCombatCharacter* sourceEnt );
	bool WasHealedRecently( void );
	bool OkForMedicToHealMe( CAI_BaseNPC *pMedic );

	bool IsWaitingToRappel( void ) { return m_RappelBehavior.IsWaitingToRappel() || m_HueyRappelBehavior.IsWaitingToRappel(); }
	void BeginRappel() { DevMsg("CHOEHuman::BeginRappel\n"); m_RappelBehavior.BeginRappel(); }

	virtual int GetHeadGroupNum( void ) = 0;
	virtual int GetNumHeads( void ) = 0;
	virtual const char *GetHeadModelName( void ) = 0;
	bool HasAHead( void ) { return m_nHeadNum != GetNumHeads(); }
	bool GetHeadPose( CBaseAnimating *pHead, Vector &origin, QAngle &angles );
	virtual void Behead( void );
	void InputBehead( inputdata_t &inputdata );
	EHANDLE m_hHead;

	virtual const char *GetHelmetModelName( void ) { return NULL; }
	bool GetHelmetPose( CBaseAnimating *pHelmet, Vector &origin, QAngle &angles );
	void SpawnHelmet( const Vector& impactDir );
	virtual bool DropHelmet( void ) { return false; }
	EHANDLE m_hHelmet;

	void InputGiveWeapon( inputdata_t &inputdata );
	void InputDropWeapon( inputdata_t &inputdata );

	virtual bool CanEnterVehicle( void );
	virtual bool CanExitVehicle( void );
	void EnterVehicle( CBaseEntity *pEntityVehicle, string_t sRole, bool bImmediateEnter );
	void InputEnterVehicle( inputdata_t &inputdata );
	void InputEnterVehicleImmediate( inputdata_t &inputdata );
	void InputExitVehicle( inputdata_t &inputdata );
	void EnterVehicleInputHelper( inputdata_t &inputdata, bool bImmediate );
	void InputFinishedEnterVehicle( inputdata_t &inputdata ); // called by passenger behavior, not map
	void InputFinishedExitVehicle( inputdata_t &inputdata ); // called by passenger behavior, not map
	COutputEvent	m_OnFinishedEnterVehicle;	// Passenger has completed entering the vehicle
	COutputEvent	m_OnFinishedExitVehicle;	// Passenger has completed exiting the vehicle

	void InputCrouch( inputdata_t &inputdata );
	bool IsCrouchedActivity( Activity activity );

	virtual const char **GetWeaponClasses( void ) const = 0;
	bool ShouldLookForBetterWeapon( void );
	bool Weapon_CanUse( CBaseCombatWeapon *pWeapon );
	bool WeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
	WeaponProficiency_t CalcWeaponProficiency( CBaseCombatWeapon *pWeapon );

	// close caption image
	virtual const char *GetCCImageNameCStr( void ) { return ""; }

	void AnalyzeGunfireSound( CSound *pSound );

	int MeleeAttack1Conditions( float flDot, float flDist );
	int GetKickDamage( void ) { return 10; }

	float GetZombieSafeDistance( void ) { return 100.0; }
	float GetZombieSafeDistanceSqr( void ) { return GetZombieSafeDistance() * GetZombieSafeDistance(); }
	float HealthFraction( CBaseEntity *pEnt )
	{
		if (pEnt->GetMaxHealth() == 0)
			return 1.0f;

		float flFraction = (float)pEnt->GetHealth() / (float)pEnt->GetMaxHealth();
		flFraction = clamp( flFraction, 0.0f, 1.0f );
		return flFraction;
	}
	float HealthFraction( void ) { return HealthFraction( this ); }
	float IsHealthInRange( float fracMin, float fracMax ) { return IsHealthInRange( this, fracMin, fracMax ); }
	float IsHealthInRange( CBaseEntity *pEnt, float fracMin, float fracMax )
	{
		float frac = HealthFraction( pEnt );
		return frac > fracMin && frac <= fracMax;
	}

	virtual const char *GetSentenceGroup( void ) const = 0;
#define HOE_HUMAN_RR // response rules
#ifdef HOE_HUMAN_RR
	CHumanSpeechManager *GetSpeechManager( void );
	virtual SpeechManagerID_t GetSpeechManagerID( void ) = 0;

	// CAI_ExpresserSink
	void OnSpokeConcept( AIConcept_t concept, AI_Response *response );
	void OnStartSpeaking( void );

	short m_nSpeakClosedCaptionID;

	void LookAtTalkTarget( float flDuration );
	void LookAtListenTarget( float flDuration );

	void PostSpeakDispatchResponse( AIConcept_t concept, AI_Response *response );
	int PlayScriptedSentence( const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, bool bConcurrent, CBaseEntity *pListener );

	virtual const char **GetFriendClasses( void ) const = 0;
	void FriendsSpokeConcept( AIConcept_t concept );

	CBaseEntity *FindNearestSpeechTarget( bool bPlayerOK );
	bool IsValidSpeechTarget( CBaseEntity *pTarget );

	CBaseEntity *FindNearestDeadFriend( void );
	CBaseEntity *FindNearestHead( void );
	bool CanSeeSmellOrigin( void );

	bool IsInAScript( void );
	bool OkToShout( void );
	bool OkToSpeak( void );
	void SetTalkTarget( CBaseEntity *pTarget ) { m_hTalkTarget = pTarget; }
	CBaseEntity *GetTalkTarget( void ) { return m_hTalkTarget.Get(); }
	void SetListenTarget( CBaseEntity *pTarget ) { m_hListenTarget = pTarget; }
	CBaseEntity *GetListenTarget( void ) { return m_hListenTarget.Get(); }

	void OnStateChange( NPC_STATE OldState, NPC_STATE NewState );
	virtual bool IdleSpeech( void );
	float m_flIdleSpeechDelay; // Don't yap for a while after spawning
	float m_flTimeInjuriesMentioned; // Time I or someone else commented on my injuries
	float m_flTimeScannedCorpses; // Time I looked around for dead friends
	float m_flTimeScannedSmells; // Time I looked around for smell sources

	bool SelectSpeechResponse( AIConcept_t concept, const char *pszModifiers, SpeechSelection_t &selection );
	bool DispatchSpeechSelection( SpeechSelection_t &selection );
	virtual bool SelectIdleSpeech( SpeechSelection_t &selection );
	virtual bool SelectInjurySpeech( SpeechSelection_t &selection );
#else // HOE_HUMAN_RR
	void InitSentences( void );
	int PlaySentence( const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, CBaseEntity *pListener );
	int PlayScriptedSentence( const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, bool bConcurrent, CBaseEntity *pListener );
	void Speak( const char *pSentence, int concept = 0, CBaseEntity *pListener = 0 );
	bool SpokeConcept( int concept ) { return (m_iSpokenConcepts & concept) != 0; }
	void FriendsSpokeConcept( int concept );

	virtual const char **GetFriendClasses( void ) const = 0;
	CBaseEntity *FindNearestSpeechTarget( bool bPlayerOK );
	bool IsValidSpeechTarget( CBaseEntity *pTarget );

	bool IsInAScript( void );
	bool OkToShout( void );
	bool OkToSpeak( void );
	bool IsTalking( void ) { return m_flStopTalkTime > gpGlobals->curtime; }
	bool SpokeRecently( float seconds ) { return m_flLastTalkTime > gpGlobals->curtime - seconds; }
	static bool IsAnyoneTalking( void ) { return g_flHumanSpeechTime > gpGlobals->curtime; }
	void SetQuestioner( CHOEHuman *pSpeaker, float timeToAnswer );
	void SetTalkTarget( CBaseEntity *pTarget ) { m_hTalkTarget = pTarget; }
	CBaseEntity *GetTalkTarget( void ) { return m_hTalkTarget.Get(); }
	void SetListenTarget( CBaseEntity *pTarget ) { m_hListenTarget = pTarget; }
	CBaseEntity *GetListenTarget( void ) { return m_hListenTarget.Get(); }
#endif // HOE_HUMAN_RR

	float GetTimePlayerStaring( void ) { return ( m_flTimePlayerStartStare != 0 ) ? gpGlobals->curtime - m_flTimePlayerStartStare : 0; }

	void AlertSound( void );
	bool AnnounceAttack( void );
	void IdleSound( void );
	void PainSound( const CTakeDamageInfo &info );
	void DelayedPainSound( void );
	void DeathSound( const CTakeDamageInfo &info );
	void FoundEnemySound( void );
	void GrenadeSound( void );
	
	void Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info );
	void OnKilledNPC( CBaseCombatCharacter *pKilled );
	void NotifyDeadFriend( CBaseEntity* pFriend );
	void OnFriendDamaged( CBaseCombatCharacter *pSquadmate, CBaseEntity *pAttacker);
	bool QuerySeeEntity( CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC );

	bool AfraidOfHuey( void );
	void DecalTrace( trace_t *pTrace, char const *decalName );
	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);
	int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void Event_Killed( const CTakeDamageInfo &info );
//	void ExplFlyTouch( CBaseEntity *pOther );
	bool ShouldPickADeathPose( void );
	bool CanBecomeRagdoll( void );
	bool BecomeRagdoll( const CTakeDamageInfo &info, const Vector &forceVector );
	bool BecomeRagdollOnClient( const Vector &force );
	CBaseEntity *CreateRagdollCorpse( const Vector &force, bool bUseLRURetirement, bool bSmell );

#define LOGIC_HUEY_DEPLOY
#ifdef LOGIC_HUEY_DEPLOY
	void Activate( void );
	void UpdateOnRemove( void );
	string_t m_iszDeployLogic;
#endif

	void DisplayDeathMessage( void );
	virtual string_t GetDeathMessage( void ) { return m_iszDeathMessage; }
	void InputSetGameEndAlly( inputdata_t &inputdata );
	bool m_bGameEndAlly;
	string_t m_iszDeathMessage;

	// Health regeneration for friendly allies
	virtual float RegenerateTime( void ) { return 0.0f; }
	
	bool EnemyIsZombie( void );
	bool EnemyIsBullseye( void );

	bool NoFriendlyFire( void );

	virtual bool IsPlayerSquadLeader( void );
	virtual CBaseEntity *GetFollowTarget( void )
	{
		return m_bFollowingTarget ? GetTarget() : NULL;
	}
	virtual bool IsFollowing( void ) { return IsFollowingPlayer() || IsFollowingHuman(); }
	virtual bool IsFollowingPlayer( void ) { return GetFollowTarget() != NULL && GetFollowTarget()->IsPlayer(); }
	virtual bool IsFollowingHuman( void ) { return GetFollowTarget() != NULL && HumanPointer( GetFollowTarget() ) != NULL; }
	virtual float GetFollowTargetDistance( void ) { return 0.0; }
	virtual void StartFollowing( CBaseEntity *pLeader ) { SetTarget( pLeader ); m_bFollowingTarget = true; }
	virtual void StopFollowing( void ) { SetTarget( NULL ); m_bFollowingTarget = false; }

	bool CanGrenadeEnemy( bool bUseFreeKnowledge = true );
	bool CanThrowGrenade( const Vector &vecTarget );
	bool CheckCanThrowGrenade( const Vector &vecTarget );
	bool CheckThrowFromPosition( const Vector &shootPos, const Vector &targetPos, Vector *pVecOut );

	void InputThrowGrenadeAtTarget( inputdata_t &inputdata );

#ifdef HUMAN_UNSHOOTABLE
	void RememberUnshootable( CBaseEntity *pEntity, float flDuration = -1 );
	bool IsUnshootable( CBaseEntity *pEntity );
	CUtlVector<UnshootableEnt_t> m_UnshootableEnts;
#endif

	CAI_AssaultBehavior m_AssaultBehavior;
	CAI_StandoffBehavior m_StandoffBehavior;
	CAI_RappelBehavior m_RappelBehavior;
	CHueyRappelBehavior m_HueyRappelBehavior;
	CHOEHumanPassengerBehavior m_PassengerBehavior;
	int m_nHeadNum;
#ifndef HOE_HUMAN_RR
	float m_flStopTalkTime;
	float m_flLastTalkTime;
	float m_flPainTime;
	int m_iSpokenConcepts;
#endif
	float m_flDelayedPainSoundTime;
	float m_flScriptedSentence;
	CTakeDamageInfo m_explosionDmgInfo;
	float m_flAttackedByHueyTime;
	float m_flTimeLastRegen;		// Last time I regenerated a bit of health.
	EHANDLE m_hTalkTarget; // Who to look at while talking
	EHANDLE m_hListenTarget; // Who to look at while being spoken to
	EHANDLE m_hMedicThatHealedMe; // Medic that healed this NPC last
	float m_flCrouchTime;		// How long have I been crouching for?
	bool m_bStopCrouching;
	bool m_bCrouchLocked; // InputCrouch
	float m_flLastMedicSearch;	// Last time I searched for a medic
	float m_flLastMedicHealed; // Last time I was healed by a medic
	float m_flKilledEnemyTime; // Last time I killed an enemy
	float m_flLastKick;
	float m_flLastFledZombieTime; // Last time I ran from too-close zombie
	float m_flFailedBackAwayTime; // Last time SCHED_BACK_AWAY_FROM_ENEMY failed
	HumanGrenadeInteractionData m_GrenadeInteraction;
	bool m_bFollowingTarget; // TRUE if GetTarget() is someone we are chasing/following
	float m_flTimePlayerStartStare;
	bool m_bShouldBoogie; // Should become a ragdoll boogie

	bool m_bLeftFootInWater;
	bool m_bRightFootInWater;
	int m_iLeftFootAttachment;
	int m_iRightFootAttachment;
	Vector m_vLeftFootOrigin;
	Vector m_vRightFootOrigin;

	float m_flWaterSoundTime;

#define TimePassed(v) (gpGlobals->curtime >= v)
#define TimeNotPassed(v) (gpGlobals->curtime < v)
#define TimeRecent(v,t) ((v != 0) && (gpGlobals->curtime - v < t))
#define TimeNotRecent(v,t) ((v == 0) || (gpGlobals->curtime - v >= t))

	typedef enum
	{
		BARNACLE_STATE_NONE = 0,
		BARNACLE_STATE_HIT,
		BARNACLE_STATE_PULL,
		BARNACLE_STATE_CHOMP,
		BARNACLE_STATE_CHEW,
		BARNACLE_STATE_RELEASED
	} BarnacleState;
	BarnacleState m_nBarnacleState;

	bool m_fHandGrenades;
	Vector m_vecTossVelocity;
	float m_flNextGrenadeCheck;

#ifdef ENEMY_TELEPORTED_STUFF
	virtual void EnemyTeleported( CBaseEntity *pEnemy );
	EHANDLE m_hEnemyThatTeleported;
	Vector m_vEnemyThatTeleportedPosition;
	bool m_bEnemyThatTeleportedSeen;
#endif

	//=====================
	// Squad Commands
	// Priority ordered - lowest number is lowest priority
	//=====================
	typedef enum
	{
		SQUADCMD_NONE = 0,				// Just do your own thing - lowest priority, the group is more important!
		SQUADCMD_CHECK_IN,				// Check in - i.e. inform squad leader if I have an enemy or not
		SQUADCMD_SEARCH_AND_DESTROY,	// Search for a target to attack - more immediate goals like attack are more important
		SQUADCMD_OUTTA_MY_WAY,			// Bugger off - unlikely to be life-threatening so low priority
		SQUADCMD_FOUND_ENEMY,			// I have found an enemy
		SQUADCMD_DISTRESS,				// When I'm hurt or injured or something
		SQUADCMD_COME_HERE,				// Come to me - not as important as attacking or retreating
		SQUADCMD_SURPRESSING_FIRE,		// Fire at a designated place - not as important as attack but still important
		SQUADCMD_ATTACK,				// Attack the leader's target, if you are not attacking something else
		SQUADCMD_DEFENSE,				// Defend squad member with lowest health - we look after each other so highest priority
		SQUADCMD_RETREAT,				// Run away from whatever you are attacking - high priority as we want to live
		SQUADCMD_GET_DOWN,				// Crouch - can have high priority as it doesn't alter m_nLastSquadCommand
		SQUADCMD_BEHIND_YOU,			// Turn around - can have high priority as it doesn't alter m_nLastSquadCommand
	} SquadCommand;

#define SQUAD_COMMAND_MEMORY_TIME 30	// How long I remember it for
#define SQUAD_COMMAND_RANGE 1024		// How close they have to be in order to recieve a command

	SquadCommand m_nLastSquadCommand;	// Last known command given by squad leader
	bool m_fSquadCmdAcknowledged; // Have I replied yet?
	float m_flLastSquadCmdTime; // Time since last squad command was given

	CBaseCombatCharacter *GetSquadLeader( void );
	bool HasSquadLeader( void );
	bool IsSquadLeader( void );
	void SquadIssueCommand( SquadCommand Cmd );
	void SquadReceiveCommand( SquadCommand Cmd );
	bool SquadCmdLegalForNonLeader( SquadCommand Cmd );
	bool SquadAnyIdle( void );
	bool SquadIsHealthy( void );
	bool SquadIsScattered( void );
	bool SquadGetWounded( bool bMedic = false );
	bool SquadGetCommanderEnemy( void );
	bool SquadGetCommanderLineOfSight( void );

	enum
	{
		HUMAN_SC_HELLO = 1 << 0,
		HUMAN_SC_DMG_LIGHT = 1 << 1,
		HUMAN_SC_DMG_MEDIUM = 1 << 2,
		HUMAN_SC_DMG_HEAVY = 1 << 3,
		HUMAN_SC_WOUND = 1 << 4,
		HUMAN_SC_MORTAL = 1 << 5,
		HUMAN_SC_QUESTION = 1 << 6,
		HUMAN_SC_IDLE = 1 << 7,
		HUMAN_SC_STARE = 1 << 8,
		HUMAN_SC_DEAD_ALLY = 1 << 9,
		HUMAN_SC_HEAR = 1 << 10,
	};

	//-----------------------------------------------------------------------------
	// Conditions.
	//-----------------------------------------------------------------------------
	enum 
	{
		COND_ZOMBIE_TOO_CLOSE = BaseClass::NEXT_CONDITION,
		COND_HUMAN_ENEMY_CLOSE,
		COND_HUMAN_MEDIC_WANTS_TO_HEAL_ME,
		COND_HUMAN_BECOMING_PASSENGER,
#ifdef ENEMY_TELEPORTED_STUFF
		COND_ENEMY_TELEPORTED,
#endif
		NEXT_CONDITION
	};

	//-----------------------------------------------------------------------------
	// Schedules
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_HUMAN_ALERT_REACT_TO_COMBAT_SOUND = BaseClass::NEXT_SCHEDULE,
		SCHED_HUMAN_BARNACLE_HIT,
		SCHED_HUMAN_BARNACLE_PULL,
		SCHED_HUMAN_BARNACLE_CHOMP,
		SCHED_HUMAN_BARNACLE_CHEW,
		SCHED_HUMAN_BARNACLE_RELEASED_HIGH,
		SCHED_HUMAN_BARNACLE_RELEASED_LOW,
		SCHED_HUMAN_CHECK_IN,
		SCHED_HUMAN_COWER,
		SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE,
		SCHED_HUMAN_EXPLOSION_DIE,
		SCHED_HUMAN_EXPLOSIVE_ELOF,
		SCHED_HUMAN_FACE_TARGET,
		SCHED_HUMAN_FAIL,
		SCHED_HUMAN_FIND_MEDIC,
		SCHED_HUMAN_FIND_MEDIC_COMBAT,
		SCHED_HUMAN_FOLLOW,
		SCHED_HUMAN_FOLLOW_CLOSE,
		SCHED_HUMAN_FOUND_ENEMY,
		SCHED_HUMAN_GRENADE_COVER,
		SCHED_HUMAN_GRENADE_INTERACTION,
		SCHED_HUMAN_HEAR_SOUND,
		SCHED_HUMAN_HIDE_RELOAD,
		SCHED_HUMAN_IDLE_RESPONSE,
		SCHED_HUMAN_IDLE_STAND,
		SCHED_HUMAN_INVESTIGATE_SOUND,
		SCHED_HUMAN_MELEE_ELOF,
		SCHED_HUMAN_MOVE_TO_ENEMY_LKP,
		SCHED_HUMAN_MOVE_TO_WEAPON_RANGE,
		SCHED_HUMAN_POPUP_ATTACK,
		SCHED_HUMAN_PRIMARY_MELEE_ATTACK1,
		SCHED_HUMAN_RANGE_ATTACK1,
		SCHED_HUMAN_RANGE_ATTACK2,
		SCHED_HUMAN_RELOAD,
		SCHED_HUMAN_REPEL,
		SCHED_HUMAN_REPEL_ATTACK,
		SCHED_HUMAN_REPEL_LAND,
		SCHED_HUMAN_REPEL_LAND_SEARCH,
		SCHED_HUMAN_RETREAT,
		SCHED_HUMAN_SEARCH_AND_DESTROY,
		SCHED_HUMAN_SIGNAL_ATTACK,
		SCHED_HUMAN_SIGNAL_CHECK_IN,
		SCHED_HUMAN_SIGNAL_COME_TO_ME,
		SCHED_HUMAN_SIGNAL_RETREAT,
		SCHED_HUMAN_SIGNAL_SEARCH_AND_DESTROY,
		SCHED_HUMAN_SIGNAL_SURPRESS,
		SCHED_HUMAN_STANDOFF,
#ifdef HUMAN_STRAFE
		SCHED_HUMAN_STRAFE,
#endif
		SCHED_HUMAN_SURPRESS,
		SCHED_HUMAN_TAKE_COVER,
		SCHED_HUMAN_TAKE_COVER_FROM_BEST_SOUND,
		SCHED_HUMAN_TAKECOVER_FAILED,
		SCHED_HUMAN_TOSS_GRENADE_COVER,
		SCHED_HUMAN_TURN_ROUND,
		SCHED_HUMAN_UNCROUCH,
		SCHED_HUMAN_UNCROUCH_SCRIPT,
		SCHED_HUMAN_VICTORY_DANCE,
		SCHED_HUMAN_WAIT_HEAL,
		SCHED_HUMAN_WAIT_IN_COVER,
		NEXT_SCHEDULE,
	};

	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		TASK_HUMAN_CHECK_FIRE = BaseClass::NEXT_TASK,
		TASK_HUMAN_CROUCH,
		TASK_HUMAN_EXPLOSION_FLY,
		TASK_HUMAN_EXPLOSION_LAND,
		TASK_HUMAN_EYECONTACT,
		TASK_HUMAN_FACE_TOSS_DIR,
		TASK_HUMAN_FIND_MEDIC,
		TASK_HUMAN_GET_EXPLOSIVE_PATH_TO_ENEMY,
		TASK_HUMAN_GET_MELEE_PATH_TO_ENEMY,
		TASK_HUMAN_GET_PATH_TO_RANDOM_NODE,
		TASK_HUMAN_IDEALYAW,
		TASK_HUMAN_PLAY_SEQUENCE_WAIT,
		TASK_HUMAN_SOUND_ATTACK,
		TASK_HUMAN_SOUND_CHARGE,
		TASK_HUMAN_SOUND_CHECK_IN,
		TASK_HUMAN_SOUND_CLEAR,
		TASK_HUMAN_SOUND_COME_TO_ME,
		TASK_HUMAN_SOUND_COVER,
		TASK_HUMAN_SOUND_EXPL,
		TASK_HUMAN_SOUND_FOUND_ENEMY,
		TASK_HUMAN_SOUND_GRENADE,
		TASK_HUMAN_SOUND_HEALED,
		TASK_HUMAN_SOUND_HEAR,
		TASK_HUMAN_SOUND_HELP,
		TASK_HUMAN_SOUND_MEDIC,
		TASK_HUMAN_SOUND_RESPOND,
		TASK_HUMAN_SOUND_RETREAT,
		TASK_HUMAN_SOUND_RETREATING,
		TASK_HUMAN_SOUND_SEARCHING,
		TASK_HUMAN_SOUND_SEARCH_AND_DESTROY,
		TASK_HUMAN_SOUND_SURPRESS,
		TASK_HUMAN_SOUND_SURPRESSING,
		TASK_HUMAN_SOUND_TAUNT,
		TASK_HUMAN_SOUND_THROW,
		TASK_HUMAN_SOUND_VICTORY,
		TASK_HUMAN_UNCROUCH,
		TASK_HUMAN_WAIT_GOAL_VISIBLE,
		TASK_FIND_FAR_NODE_COVER_FROM_ORIGIN,
		TASK_HUMAN_DEFER_SQUAD_GRENADES,
#ifdef HUMAN_STRAFE
		TASK_GET_STRAFE_PATH,
#endif
		NEXT_TASK,
	};

	//-----------------------------------------------------------------------------
	// Activities
	//-----------------------------------------------------------------------------
	static Activity ACT_BARNACLE_RELEASED;
	static Activity ACT_BARNACLE_HIT_GROUND;
	static Activity ACT_EXPLOSION_HIT;
	static Activity ACT_EXPLOSION_FLY;
	static Activity ACT_WALK_CROUCH_HURT;
	static Activity ACT_RUN_CROUCH_HURT;
	static Activity ACT_TURN_LEFT_LOW;
	static Activity ACT_TURN_RIGHT_LOW;
	static Activity ACT_SIGNAL1_LOW;
	static Activity ACT_SIGNAL2_LOW;
	static Activity ACT_SIGNAL3_LOW;
	static Activity ACT_SMALL_FLINCH_LOW;
	static Activity ACT_IDLE_ANGRY_LOW;

	//-----------------------------------------------------------------------------
	// Animation events.
	//-----------------------------------------------------------------------------
	static Animevent AE_NPC_KICK;
	static Animevent AE_HUMAN_GRENADE_TOSS;
	static Animevent AE_HUMAN_GRENADE_DROP;
	static Animevent AE_HUMAN_OPEN_VEHICLE;
	static Animevent AE_HUMAN_CLOSE_VEHICLE;

	// -----------------------------------------------
	//	Squad slots
	// -----------------------------------------------
	enum SquadSlot_T
	{
		SQUAD_SLOT_HUMAN_ENGAGE1 = LAST_SHARED_SQUADSLOT,
		SQUAD_SLOT_HUMAN_ENGAGE2,
		SQUAD_SLOT_HUMAN_GRENADE1,
		SQUAD_SLOT_HUMAN_GRENADE2,
		NEXT_SQUAD_SLOT,
	};
};

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------
#define SF_HUMAN_FRIENDLY (1 << 16)
#define SF_HUMAN_LEADER (1 << 17)
#define SF_HUMAN_GRENADES (1 << 18)
#define SF_HUMAN_PREDISASTER (1 << 19)
#define SF_HUMAN_SERVERSIDE_RAGDOLL (1 << 20)

#define HUMAN_SPEECH_RADIUS 500.0
#define HUMAN_KICK_RANGE 64
#define HUMAN_KICK_INTERVAL 10	// Min interval between kicks
#define HUMAN_EXPLOSIVE_MAX_RANGE 4096
#define HUMAN_EXPLOSIVE_MIN_RANGE 256
#define HUMAN_CROUCH_TIME 8	// Time spent crouching even if nothing is apparently going on
#define HUMAN_MEDIC_SEARCH_TIME 30	// After I've called for a medic, how soon I can call again

#define bits_MEMORY_HUMAN_NO_COVER bits_MEMORY_CUSTOM1

extern int g_interactionHumanMedicHeal;
extern int g_interactionHumanGrenade;

#endif // HOE_HUMAN_H
