

#ifndef MONSTER_SAVE_STATE
#define MONSTER_SAVE_STATE

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
//#include "common/vector.h"

// !!! Not to be confused with saving/restoring games!  This is for saving/restoring several stats
// about a monster between different points of time, but primarily for CBarney so far (see description
// below)
// ALSO not to be confused with the 'monsterstate.h' file, which is for methods strongly related to
// m_MonsterState and the ideal one, still methods of CBaseMonster.  SimpleMonsterSaveState is an
// independent class.

// Simple data class for remembering important stats about the Barney before he goes into cap-busting mode
// pre-disaster from getting spammed with 'use' 's.
// When forgiven (enough time passed), the state gets insta-restored to what it was before the 30th use-
// spam to work like he's supposed to.
// Note that saving the game quickly restores the state to pre-pissed before saving (forgiven state), and
// puts it back before resuming.  This means, in the same game (no exit/load), the barney's still pissed,
// but loading the game still starts unpissed.  Not worth saving all this state-info in the game data, so
// a loaded game always sees the barnies unpissed when picked.

class SimpleMonsterSaveState {
public:
	int OLD_m_afMemory;
	Schedule_t* OLD_m_pSchedule;
	int OLD_m_iScheduleIndex;
	int OLD_m_iTaskStatus;
	int OLD_m_afConditions;
	int OLD_m_afConditionsNextFrame;
	int OLD_m_afConditionsMod;
	int OLD_m_afConditionsModNextFrame;
	MONSTERSTATE OLD_m_MonsterState;
	MONSTERSTATE OLD_m_IdealMonsterState;
	int OLD_sequence;
	int OLD_body;
	Vector OLD_origin;
	Vector OLD_angles;
	EHANDLE OLD_m_hEnemy;
	EHANDLE OLD_m_hTargetEnt;
	CCineMonster* OLD_m_pCine;
	Activity OLD_m_Activity;
	Activity OLD_m_IdealActivity;
	Activity OLD_m_movementActivity;
	int OLD_m_movementGoal;



	SimpleMonsterSaveState(void);
	void Save(CBaseMonster* toRead);
	void Restore(CBaseMonster* toReceive);


};



#endif //MONSTER_SAVE_STATE
