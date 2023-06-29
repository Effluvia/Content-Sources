#include "ai_behavior.h"

class CBeam;

class CHueyRappelBehavior : public CAI_SimpleBehavior
{
	DECLARE_CLASS( CHueyRappelBehavior, CAI_SimpleBehavior );

public:
	CHueyRappelBehavior();
	
	void Precache( void );
	virtual const char *GetName() {	return "HueyRappel"; }

	virtual bool KeyValue( const char *szKeyName, const char *szValue );

	bool ShouldAlwaysThink();

	virtual bool 	CanSelectSchedule();
	void GatherConditions();
	void CleanupOnDeath( CBaseEntity *pCulprit = NULL, bool bFireDeathOutput = true );
	
	//virtual void	BeginScheduleSelection();
	//virtual void	EndScheduleSelection();

	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	bool IsWaitingToRappel() { return m_bWaitingToRappel; }
	void BeginRappel( CBaseEntity *pHuey, int iAttachment, Activity iDeployActivity );
	void SetDescentSpeed();
	void AbortRappel( void );

	void CreateZipline();
	void CutZipline();

	//void BuildScheduleTestBits();
	//int TranslateSchedule( int scheduleType );
	//void OnStartSchedule( int scheduleType );

	//void InitializeBehavior();
	
	enum
	{
		SCHED_HUEY_RAPPEL_WAIT = BaseClass::NEXT_SCHEDULE,
		SCHED_HUEY_RAPPEL_DEPLOY,
		SCHED_HUEY_RAPPEL,
		SCHED_HUEY_CLEAR_RAPPEL_POINT, // Get out of the way for the next guy
		SCHED_HUEY_RAPPEL_FALL_HIGH,
		SCHED_HUEY_RAPPEL_FALL_LOW,
		NEXT_SCHEDULE,

		TASK_HUEY_RAPPEL_DEPLOY = BaseClass::NEXT_TASK,
		TASK_HUEY_RAPPEL,
		TASK_HUEY_HIT_GROUND,
		NEXT_TASK,

		COND_HUEY_BEGIN_RAPPEL = BaseClass::NEXT_CONDITION,
		COND_HUEY_ABORT_RAPPEL,
		NEXT_CONDITION,
	};

	DEFINE_CUSTOM_SCHEDULE_PROVIDER;

public:

private:
	virtual int		SelectSchedule();

	//---------------------------------
	bool				m_bWaitingToRappel;
	bool				m_bOnGround;
	CHandle<CBeam>		m_hLine;
	Vector				m_vecHueyRopeAnchor;
	EHANDLE				m_hHuey;
	int					m_iAttachment;
	Activity			m_iDeployActivity;
#ifdef RAPPEL_PHYSICS
	EHANDLE				m_hTipEnt;
#endif
	float				m_flFallVelocity;

	DECLARE_DATADESC();
};

extern Activity ACT_RAPPEL_JUMP0;
extern Activity ACT_RAPPEL_JUMP2;
