#include "hoe_human_follower.h"

class CHOEHumanMedic : public CHOEHumanFollower
{
public:
	DECLARE_CLASS( CHOEHumanMedic, CHOEHumanFollower );
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	int SelectSchedulePriority( void );
	int SelectScheduleCombat( void );
	int SelectScheduleHeal( void );
	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );
	int SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	Activity NPC_TranslateActivity( Activity eNewActivity );
	void HandleAnimEvent( animevent_t *pEvent );

	void UseMedic( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void Heal( void );
	bool IsMedic( void ) { return true; }
	CBaseEntity *FindHealTarget( void );

	void Event_Killed( const CTakeDamageInfo &info );

	virtual bool ShouldBlockerMoveAway( CAI_BaseNPC *pBlocker );

	float m_flTimeHealedPlayer;

	//-----------------------------------------------------------------------------
	// Schedules
	//-----------------------------------------------------------------------------
	enum
	{
		SCHED_HUMAN_MEDIC_HEAL = BaseClass::NEXT_SCHEDULE,
		SCHED_HUMAN_MEDIC_CHASE,
		NEXT_SCHEDULE,
	};

	//-----------------------------------------------------------------------------
	// Tasks
	//-----------------------------------------------------------------------------
	enum
	{
		TASK_HUMAN_MEDIC_SOUND_HEAL = BaseClass::NEXT_TASK,
		TASK_HUMAN_MEDIC_CHECK_TARGET,
		NEXT_TASK,
	};

	//-----------------------------------------------------------------------------
	// Animation events.
	//-----------------------------------------------------------------------------
	static Animevent AE_HMEDIC_SYRINGE_HAND;
	static Animevent AE_HMEDIC_SYRINGE_HELMET;
	static Animevent AE_HMEDIC_HEAL;

	//-----------------------------------------------------------------------------
	// Activities
	//-----------------------------------------------------------------------------
	static Activity ACT_HMEDIC_HEAL;
	static Activity ACT_HMEDIC_HEAL_CROUCH;

	enum {
		HMEDIC_BODYGROUP_HEAD = 0,
		HMEDIC_BODYGROUP_BODY,
		HMEDIC_BODYGROUP_SYRINGE,
	};
};
