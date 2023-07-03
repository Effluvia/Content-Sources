#include "ai_behavior_passenger.h"

struct dupFailPosition_t
{
	Vector	vecPosition;
	float	flTime;

	DECLARE_SIMPLE_DATADESC();
};

class CHOEHumanPassengerBehavior : public CAI_PassengerBehavior
{
public:
	DECLARE_CLASS( CHOEHumanPassengerBehavior, CAI_PassengerBehavior );
	DECLARE_DATADESC()
	DEFINE_CUSTOM_SCHEDULE_PROVIDER;

	CHOEHumanPassengerBehavior();
	virtual void Enable( CBaseEntity *pVehicle, bool bImmediateEnter /*= false*/ );

	Activity NPC_TranslateActivity( Activity activity );

	virtual int		SelectSchedule( void );
	virtual int		SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	virtual void	StartTask( const Task_t *pTask );
	virtual void	RunTask( const Task_t *pTask );
	virtual bool	IsNavigationUrgent( void );
	virtual	bool	IsCurTaskContinuousMove( void );
	virtual bool	IsCrouching( void );

	bool	UseRadialRouteToEntryPoint( const Vector &vecEntryPoint );
	float	GetArcToEntryPoint( const Vector &vecCenterPoint, const Vector &vecEntryPoint, bool &bClockwise );
	virtual int	SelectTransitionSchedule( void );
	int		SelectScheduleInsideVehicle( void );
	int		SelectScheduleOutsideVehicle( void );
	bool	FindPathToVehicleEntryPoint( void );

	bool			UpdateVehicleEntrancePath( void );
	bool			PointIsWithinEntryFailureRadius( const Vector &vecPosition );
	void			ResetVehicleEntryFailedState( void );
	void			MarkVehicleEntryFailed( const Vector &vecPosition );
	virtual int		FindEntrySequence( bool bNearest = false );

	float	m_flNextEnterAttempt;
	float	m_flEntranceUpdateTime;

	CUtlVector<dupFailPosition_t>	m_FailedEntryPositions;

	// CAI_PassengerBehavior has a hard-coded role of "passenger".  The huey
	// has pilot/copilot/sarge/barney passenger roles.
	virtual string_t GetRoleName( void );
	string_t m_sRoleName;

	enum
	{
		// Schedules
		SCHED_PASSENGER_RUN_TO_ENTER_VEHICLE_HOE = BaseClass::NEXT_SCHEDULE,
		SCHED_PASSENGER_RUN_TO_ENTER_VEHICLE_FAILED_HOE,
		SCHED_PASSENGER_ENTER_VEHICLE_PAUSE_HOE,
		NEXT_SCHEDULE,

		// Tasks
		TASK_GET_PATH_TO_VEHICLE_ENTRY_POINT_HOE = BaseClass::NEXT_TASK,
		TASK_GET_PATH_TO_NEAR_VEHICLE_HOE,
		TASK_RUN_TO_VEHICLE_ENTRANCE_HOE,
		NEXT_TASK,
	};
};
