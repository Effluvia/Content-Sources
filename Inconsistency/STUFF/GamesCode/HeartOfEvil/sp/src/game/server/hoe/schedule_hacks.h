#define HOE_BEGIN_CUSTOM_NPC( className, derivedClass ) \
	AI_BEGIN_CUSTOM_NPC( className, derivedClass ) \
	CUtlVector<char *> scheduleFilesToLoad;

#define DECLARE_SCHEDULE( id ) \
	scheduleIds.PushBack( #id, id );
#define LOAD_SCHEDULES_FILE( prefix ) \
	scheduleFilesToLoad.AddToTail( #prefix );

#define HOE_END_CUSTOM_NPC() \
		\
		int i; \
		\
		CNpc::AccessClassScheduleIdSpaceDirect().Init( pszClassName, BaseClass::GetSchedulingSymbols(), &BaseClass::AccessClassScheduleIdSpaceDirect() ); \
		CNpc::gm_SquadSlotIdSpace.Init( &BaseClass::gm_SquadSlotNamespace, &BaseClass::gm_SquadSlotIdSpace); \
		\
		scheduleIds.Sort(); \
		taskIds.Sort(); \
		conditionIds.Sort(); \
		squadSlotIds.Sort(); \
		\
		for ( i = 0; i < scheduleIds.Count(); i++ ) \
		{ \
			ADD_CUSTOM_SCHEDULE_NAMED( CNpc, scheduleIds[i].pszName, scheduleIds[i].localId );  \
		} \
		\
		for ( i = 0; i < taskIds.Count(); i++ ) \
		{ \
			ADD_CUSTOM_TASK_NAMED( CNpc, taskIds[i].pszName, taskIds[i].localId );  \
		} \
		\
		for ( i = 0; i < conditionIds.Count(); i++ ) \
		{ \
			if ( ValidateConditionLimits( conditionIds[i].pszName ) ) \
			{ \
				ADD_CUSTOM_CONDITION_NAMED( CNpc, conditionIds[i].pszName, conditionIds[i].localId );  \
			} \
		} \
		\
		for ( i = 0; i < squadSlotIds.Count(); i++ ) \
		{ \
			ADD_CUSTOM_SQUADSLOT_NAMED( CNpc, squadSlotIds[i].pszName, squadSlotIds[i].localId );  \
		} \
		\
		for ( i = 0; i < reqiredOthers.Count(); i++ ) \
		{ \
			(*reqiredOthers[i])();  \
		} \
		\
		for ( i = 0; i < schedulesToLoad.Count(); i++ ) \
		{ \
			if ( CNpc::gm_SchedLoadStatus.fValid ) \
			{ \
				CNpc::gm_SchedLoadStatus.fValid = g_AI_SchedulesManager.LoadSchedulesFromBuffer( pszClassName, schedulesToLoad[i], &AccessClassScheduleIdSpaceDirect() ); \
			} \
			else \
				break; \
		} \
		for ( i = 0; i < scheduleFilesToLoad.Count(); i++ ) \
		{ \
			if ( CNpc::gm_SchedLoadStatus.fValid ) \
			{ \
				CNpc::gm_SchedLoadStatus.fValid = g_AI_SchedulesManager.LoadSchedules( scheduleFilesToLoad[i], &AccessClassScheduleIdSpaceDirect() ); \
			} \
			else \
				break; \
		} \
	}