

#include "monstersavestate.h"


SimpleMonsterSaveState::SimpleMonsterSaveState(void) {

}
void SimpleMonsterSaveState::Save(CBaseMonster* toRead) {
	OLD_m_afMemory = toRead->m_afMemory;
	OLD_m_pSchedule = toRead->m_pSchedule;
	OLD_m_iScheduleIndex = toRead->m_iScheduleIndex;
	OLD_m_iTaskStatus = toRead->m_iTaskStatus;
	OLD_m_afConditions = toRead->m_afConditions & ~(bits_COND_NEW_ENEMY);
	OLD_m_afConditionsNextFrame = toRead->m_afConditionsNextFrame & ~(bits_COND_NEW_ENEMY);
	OLD_m_afConditionsMod = toRead->m_afConditionsMod;
	OLD_m_afConditionsModNextFrame = toRead->m_afConditionsModNextFrame;
	OLD_m_MonsterState = toRead->m_MonsterState;
	OLD_m_IdealMonsterState = toRead->m_IdealMonsterState;
	OLD_sequence = toRead->pev->sequence;
	OLD_body = toRead->pev->body;
	OLD_origin = toRead->pev->origin;
	OLD_angles = toRead->pev->angles;
	OLD_m_hEnemy = toRead->m_hEnemy;
	OLD_m_hTargetEnt = toRead->m_hTargetEnt;
	OLD_m_pCine = toRead->m_pCine;
	OLD_m_Activity = toRead->m_Activity;
	OLD_m_IdealActivity = toRead->m_IdealActivity;
	OLD_m_movementActivity = toRead->m_movementActivity;
	OLD_m_movementGoal = toRead->m_movementGoal;
}
void SimpleMonsterSaveState::Restore(CBaseMonster* toReceive) {
	toReceive->m_afMemory = OLD_m_afMemory;
	toReceive->m_pSchedule = OLD_m_pSchedule;
	toReceive->m_iScheduleIndex = OLD_m_iScheduleIndex;
	toReceive->m_iTaskStatus = OLD_m_iTaskStatus;
	toReceive->m_afConditions = OLD_m_afConditions;
	toReceive->m_afConditionsNextFrame = OLD_m_afConditionsNextFrame;
	toReceive->m_afConditionsMod = OLD_m_afConditionsMod;
	toReceive->m_afConditionsModNextFrame = OLD_m_afConditionsModNextFrame;
	toReceive->m_MonsterState = OLD_m_MonsterState;
	toReceive->m_IdealMonsterState = OLD_m_IdealMonsterState;
	toReceive->pev->sequence = OLD_sequence;
	toReceive->pev->body = OLD_body;
	toReceive->pev->origin = OLD_origin;
	toReceive->pev->angles = OLD_angles;
	toReceive->m_hEnemy = OLD_m_hEnemy;
	toReceive->m_hTargetEnt = OLD_m_hTargetEnt;
	toReceive->m_pCine = OLD_m_pCine;
	toReceive->m_Activity = OLD_m_Activity;
	toReceive->m_IdealActivity = OLD_m_IdealActivity;
	toReceive->m_movementActivity = OLD_m_movementActivity;
	toReceive->m_movementGoal = OLD_m_movementGoal;
}


