
// Includes.
#include "templatemonster.h"
#include "schedule.h"
#include "activity.h"
#include "util_model.h"
#include "defaultai.h"
#include "soundent.h"
#include "game.h"
#include "decals.h"


EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard)



#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_templatemonster, CTemplateMonster );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( templatemonster, CTemplateMonster );
	
	#if EXTRA_NAMES == 2
		// NOTICE - example extra name! There may be multiple or no extra names too.
		// LINK_ENTITY_TO_CLASS( tempmonster, CTemplateMonster );
	#endif
#endif



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Sequences, in the order they appear in the model. Some sequences have the same display name and so should just
// be referenced by order (numbered index).
enum templateMonster_sequence{  //key: frames, FPS
	SEQ_TEMPLATEMONSTER_XXX,

};


//custom schedule enum
enum{
	SCHED_TEMPLATEMONSTER_XXX = LAST_COMMON_SCHEDULE + 1,
	SCHED_TEMPLATEMONSTER_YYY,
	SCHED_TEMPLATEMONSTER_ZZZ,


};

//custom task enum
enum{
	TASK_TEMPLATEMONSTER_XXX = LAST_COMMON_TASK + 1,
	TASK_TEMPLATEMONSTER_YYY,
	TASK_TEMPLATEMONSTER_ZZZ,
	

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//schedule details here......

Task_t	tlTemplateMonsterXXX[] =
{
	{ TASK_TEMPLATEMONSTER_XXX,			0				},
	{ TASK_TEMPLATEMONSTER_YYY,			0				},
	{ TASK_TEMPLATEMONSTER_ZZZ,			0				},
};

Schedule_t	slTemplateMonsterXXX[] =
{
	{
		tlTemplateMonsterXXX,
		ARRAYSIZE ( tlTemplateMonsterXXX ),
		bits_COND_XXX | bits_COND_YYY | bits_COND_ZZZ,
		bits_SOUND_XXX | bits_SOUND_YYY | bits_SOUND_ZZZ,
		"templateMonsterXXX"
	},
};

// repeat for tl / sl TemplateMonsterYYY and tl / sl TemplateMonsterZZZ.


DEFINE_CUSTOM_SCHEDULES( CTemplateMonster ){
	slTemplateMonsterXXX,
	slTemplateMonsterYYY,
	slTemplateMonsterZZZ,

};
IMPLEMENT_CUSTOM_SCHEDULES( CTemplateMonster, CBaseMonster );






const char* CTemplateMonster::pDeathSounds[] = 
{
	"templatemonster/templatemonster_death.wav",
};
const char* CTemplateMonster::pAlertSounds[] = 
{
	"templatemonster/templatemonster_alert.wav",
};

const char* CTemplateMonster::pIdleSounds[] = 
{
	"templatemonster/templatemonster_idle.wav",
};
const char* CTemplateMonster::pPainSounds[] = 
{
	"templatemonster/templatemonster_pain.wav",
};
const char* CTemplateMonster::pAttackSounds[] = 
{
	"templatemonster/templatemonster_attack.wav",
};
const char* CTemplateMonster::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};
const char* CTemplateMonster::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};






CTemplateMonster::CTemplateMonster(void){


}//END OF CTemplateMonster constructor




/*
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SAVE / RESTORE SCRIPT
// Before uncommenting, must have at least one entry. Yes, C / C++ really hates empty constant arrays.
// See examples of entries in basemonster.cpp. like:
//    DEFINE_FIELD( CBaseMonster, m_flNextAttack, FIELD_TIME ),
// For save type (last parameter), pay attention to some things that go a little beyond data type, like being a time.
// FIELD_TIME factors in the change between game time from loading a different game.
// But for ordinary fields that don't need that adjustment (FIELD_FLOAT), this will royally... mess them up. Subtracting game time from a remembered angle for instance makes no sense.
// See the "_fieldtypes" enumeration of eiface.h for all save types.

TYPEDESCRIPTION	CTemplateMonster::m_SaveData[] = 
{
	
};

// This line does the Save / Restore methods minimally.  Leave it out to implement Save/Restore yourself for
// more customization, but be sure they do what's expected of Save/Restore at a minimum
// (see other examples too).  Save/Restore should handle getting sensitive things between save/load
// that are expected not to be NULL right or else the game would crash.
//IMPLEMENT_SAVERESTORE( CTemplateMonster, CBaseMonster );

// Typically want to do things after the iWriteFieldsResult or iReadFieldsResult. For writing it may not matter,
// but reading may require parent class vars to already be loaded if they are depended on.
// Also beware of depending on this same class's vars during loading, depending on vars not yet loaded is bad.
int CTemplateMonster::Save( CSave &save )
{
	if ( !CBaseMonster::Save(save) )
		return 0;
	int iWriteFieldsResult = save.WriteFields( "CTemplateMonster", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iWriteFieldsResult;
}
int CTemplateMonster::Restore( CRestore &restore )
{
	if ( !CBaseMonster::Restore(restore) )
		return 0;
	int iReadFieldsResult = restore.ReadFields( "CTemplateMonster", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iReadFieldsResult;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/



	
	
void CTemplateMonster::DeathSound( void ){
	int pitch = 95 + RANDOM_LONG(0,9);
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_IDLE, 0, pitch );
}
void CTemplateMonster::AlertSound( void ){
	int pitch = 95 + RANDOM_LONG(0,9);
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1.0, ATTN_NORM, 0, pitch );
}
void CTemplateMonster::IdleSound( void ){
	int pitch = 95 + RANDOM_LONG(0,9);
	// Play a random idle sound
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, ATTN_NORM, 0, pitch );
}
void CTemplateMonster::PainSound( void ){
	int pitch = 95 + RANDOM_LONG(0,9);
	if (RANDOM_LONG(0,5) < 2){
		UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch );
	}
}
void CTemplateMonster::AttackSound( void ){
	int pitch = 95 + RANDOM_LONG(0,9);
	// Play a random attack sound
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, pitch );
}



extern int global_useSentenceSave;
void CTemplateMonster::Precache( void ){
	PRECACHE_MODEL("models/templatemonster.mdl");

	global_useSentenceSave = TRUE;
	
	
	//NOTICE - attempting to precace files that don't exist crashes the game.
	/*
	//PRECACHE_SOUND("templatemonster/templatemonster_XXX.wav");
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	*/



	global_useSentenceSave = FALSE;
}//END OF Precache()



void CTemplateMonster::Spawn( void ){
	Precache( );

	setModel("models/templatemonster.mdl");

	//UTIL_SetOrigin(pev, pev->origin)   //some BSP stuff does this, no idea why.
	UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));

	pev->classname = MAKE_STRING("monster_templatemonster");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	// NOTE - you have to make this exist over in skill.h and handle some other setup (gamerules.cpp, and the CVar in game.cpp)!
	// example: skilldata_t member "scientistHealth" and CVar "sk_scientist_health".
	pev->health			= gSkillData.templatemonsterHealth;

	//NOTICE - don't set "pev->view_ofs" here! Do it through the "SetEyePosition" method instead, which is called by MonsterInit (delayed slightly in real time)
	
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	// If any capabilities need to be forced or added, add them to m_afCapability.
	// Example: To open doors and have melee attack 1 (but note that CAP's for melee and ranged attacks 1 and 2 are automatically added if at least one sequence
	//          was mapped to that ACT_ accordingly. If no sequence is mapped to that ACT, neither naturally in the model nor forced by tryActivitySubstitute / 
	//          lookupActivityHard, a capability may need to be forced this way as it is needed to even allow "CheckRangeAttack1" and similar methods to be
	//          called.):
	//m_afCapability		= bits_CAP_MELEE_ATTACK1 | bits_CAP_DOORS_GROUP;

	//bound to change often from "SetYawSpeed". Likely meaningless here but a default can't hurt.
	pev->yaw_speed		= 100;

	MonsterInit();

	//SetTouch(&CTemplateMonster::CustomTouch );
	//SetTouch( NULL );

}//END OF Spawn();


// Not to be confused with the bounds given by the Spawn method's "UTIL_SetSize".
// That size is used for blocking other entities from occupying the same space.
// This "ObjectCollisionBox" determines what area is able to be hit by linetraces (bullets) or
// other damage checks.  So if that area from SetSize is not enough to include all of the monster,
// namely the head, this needs to be overridden to go beyond and include those.
// Otherwise firing at places just out of bounds but clearlyo n model like below te very tip of the
// head won't register at all, for example.
void CTemplateMonster::SetObjectCollisionBox(void){
	// remove this parent call if using below. Calling the parent is exclusive of setting absmin & max manually.
	CBaseMonster::SetObjectCollisionBox();
	
	/*
	if(pev->deadflag != DEAD_NO){
		//any form of dead.
		CBaseMonster::SetObjectCollisionBox();
	}else{
		pev->absmin = pev->origin + Vector(-X, -X, 0);
		pev->absmax = pev->origin + Vector(X, X, Z);
	}
	*/
}//END OF SetObjectCollisionBox


void CTemplateMonster::SetEyePosition(void){
	// Don't call the parent and instead force view_ofs to use a hardcoded eye position.
	//pev->view_ofs = VEC_VIEW;

	// The default way reads the model for eye position.
	CBaseMonster::SetEyePosition();
}//END OF SetEyePosition


// default, override if necessary.
float CTemplateMonster::getDistTooFar(void){
	return 1024.0;
}
// default, override if necessary.
float CTemplateMonster::getDistLook(void){
	return 2048.0;
}





// based off of GetSchedule for CBaseMonster in schedule.cpp.
Schedule_t* CTemplateMonster::GetSchedule( void ){
	//MODDD - safety.
	if(pev->deadflag != DEAD_NO){
		return GetScheduleOfType( SCHED_DIE );
	}
	SCHEDULE_TYPE baitSched = getHeardBaitSoundSchedule();

	if(baitSched != SCHED_NONE){
		return GetScheduleOfType ( baitSched );
	}

	switch	( m_MonsterState )
	{
	case MONSTERSTATE_PRONE:
		{
			return GetScheduleOfType( SCHED_BARNACLE_VICTIM_GRAB );
			break;
		}
	case MONSTERSTATE_NONE:
		{
			ALERT ( at_aiconsole, "MONSTERSTATE IS NONE!\n" );
			break;
		}
	case MONSTERSTATE_IDLE:
		{
			if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_ALERT_FACE );
			}
			else if ( FRouteClear() )
			{
				// no valid route!
				return GetScheduleOfType( SCHED_IDLE_STAND );
			}
			else
			{
				// valid route. Get moving
				return GetScheduleOfType( SCHED_IDLE_WALK );
			}
			break;
		}
	case MONSTERSTATE_ALERT:
		{
			if ( HasConditions( bits_COND_ENEMY_DEAD ) && LookupActivity( ACT_VICTORY_DANCE ) != ACTIVITY_NOT_AVAILABLE )
			{
				return GetScheduleOfType ( SCHED_VICTORY_DANCE );
			}

			if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE) )
			{
				if ( fabs( FlYawDiff() ) < (1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN );
				}
				else
				{
					return GetScheduleOfType( SCHED_ALERT_SMALL_FLINCH );
				}
			}

			else if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_ALERT_FACE );
			}
			else
			{
				return GetScheduleOfType( SCHED_ALERT_STAND );
			}
			break;
		}
	case MONSTERSTATE_COMBAT:
		{
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// clear the current (dead) enemy and try to find another.
				m_hEnemy = NULL;

				if ( GetEnemy() )
				{
					ClearConditions( bits_COND_ENEMY_DEAD );
					return GetSchedule();
				}
				else
				{
					SetState( MONSTERSTATE_ALERT );
					return GetSchedule();
				}
			}

			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				return GetScheduleOfType ( SCHED_WAKE_ANGRY );
			}
			//MODDD - other condition.  If "noFlinchOnHard" is on and the skill is hard, don't flinch from getting hit.
			else if (HasConditions(bits_COND_LIGHT_DAMAGE) && !HasMemory( bits_MEMORY_FLINCHED) && !(EASY_CVAR_GET_DEBUGONLY(noFlinchOnHard)==1 && g_iSkillLevel==SKILL_HARD)  )
			{
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}
			else if ( !HasConditions(bits_COND_SEE_ENEMY) )
			{
				// we can't see the enemy
				if ( !HasConditions(bits_COND_ENEMY_OCCLUDED) )
				{
					
					if(!FacingIdeal()){
						// enemy is unseen, but not occluded!
						// turn to face enemy
						return GetScheduleOfType(SCHED_COMBAT_FACE);
					}else{
						//We're facing the LKP already. Then we have to go to that point and declare we're stumped there if we still see nothing.
						return GetScheduleOfType(SCHED_CHASE_ENEMY);
					}
				}
				else
				{
					// chase!
					//easyPrintLine("ducks??");
					return GetScheduleOfType( SCHED_CHASE_ENEMY );
				}
			}
			else  
			{

				//easyPrintLine("I say, really now? %d %d", HasConditions(bits_COND_CAN_RANGE_ATTACK1), HasConditions(bits_COND_CAN_RANGE_ATTACK2) );

				// we can see the enemy
				if ( HasConditions(bits_COND_CAN_RANGE_ATTACK1) )
				{
					return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
				}
				if ( HasConditions(bits_COND_CAN_RANGE_ATTACK2) )
				{
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK1) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
				}
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK2) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK2 );
				}
				//MODDD - NOTE - is that intentional?  range1 & melee1,  and not say,  melee1 & melee2???
				if ( !HasConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1) )
				{
					// if we can see enemy but can't use either attack type, we must need to get closer to enemy
					return GetScheduleOfType( SCHED_CHASE_ENEMY );
				}
				else if ( !FacingIdeal() )
				{
					//turn
					return GetScheduleOfType( SCHED_COMBAT_FACE );
				}
				else
				{
					ALERT ( at_aiconsole, "No suitable combat schedule!\n" );
				}
			}
			break;
		}
	case MONSTERSTATE_DEAD:
		{
			return GetScheduleOfType( SCHED_DIE );
			break;
		}
	case MONSTERSTATE_SCRIPT:
		{
			//
			//ASSERT( m_pCine != NULL );

			if(m_pCine == NULL){
				easyPrintLine("WARNING: m_pCine IS NULL!");
			}

			if ( !m_pCine )
			{
				ALERT( at_aiconsole, "Script failed for %s\n", STRING(pev->classname) );
				CineCleanup();
				return GetScheduleOfType( SCHED_IDLE_STAND );
			}

			return GetScheduleOfType( SCHED_AISCRIPT );
		}
	default:
		{
			ALERT ( at_aiconsole, "Invalid State for GetSchedule!\n" );
			break;
		}
	}

	return &slError[ 0 ];
}//END OF GetSchedule()


Schedule_t* CTemplateMonster::GetScheduleOfType( int Type){
	
	switch(Type){
		case SCHED_TEMPLATEMONSTER_XXX:{

		break;}
	}//END OF switch(Type)
	
	return CBaseMonster::GetScheduleOfType(Type);
}//END OF GetScheduleOfType


void CTemplateMonster::ScheduleChange(){

	Schedule_t* endedSchedule = m_pSchedule;

	// Use this to do something when a schedule ends that must happen at the time of interruption.
	// Depending on what schedule that ended typically, whether it was interrupted by some condition or naturally
	// ran out of tasks.
	
	
	
	
	CBaseMonster::ScheduleChange(); //Call the parent.

}//END OF ScheduleChange


Schedule_t* CTemplateMonster::GetStumpedWaitSchedule(){
	return CBaseMonster::GetStumpedWaitSchedule();
}//END OF GetStumpedWaitSchedule




void CTemplateMonster::StartTask( Task_t* pTask ){
	
	switch( pTask->iTask ){
		case TASK_TEMPLATEMONSTER_XXX:{

		break;}
		default:{
			CBaseMonster::StartTask( pTask );
		break;}
	}//END OF switch

}//END OF StartTask

void CTemplateMonster::RunTask( Task_t* pTask ){
	
	//EASY_CVAR_PRINTIF_PRE(templatePrintout, easyPrintLine("RunTask: sched:%s task:%d", this->m_pSchedule->pName, pTask->iTask) );
	
	switch( pTask->iTask ){
		case TASK_TEMPLATEMONSTER_XXX:{

		break;}
		default:{
			CBaseMonster::RunTask(pTask);
		break;}
	}//END OF switch

}//END OF RunTask


//IMPORTANT. Easy to overlook, but this filter can be frustrating if you forget about it.
//           It's required to even do the CheckMelee and CheckRange attack methods below.
//           Classes that have attacks that may not require a direct line of sight to the enemy, like throwing a grenade, override this
//           to remove the bits_COND_SEE_ENEMY part. But be sure to include it back in specific Check methods that still need it.
BOOL CTemplateMonster::FCanCheckAttacks(void){
	if ( HasConditions(bits_COND_SEE_ENEMY) && !HasConditions( bits_COND_ENEMY_TOOFAR ) ){
		return TRUE;
	}

	return FALSE;
}//END OF FCanCheckAttacks

BOOL CTemplateMonster::CheckMeleeAttack1( float flDot, float flDist ){
	return FALSE;
}
BOOL CTemplateMonster::CheckMeleeAttack2( float flDot, float flDist ){
	return FALSE;
}
BOOL CTemplateMonster::CheckRangeAttack1( float flDot, float flDist ){
	return FALSE;
}
BOOL CTemplateMonster::CheckRangeAttack2( float flDot, float flDist ){
	return FALSE;
}



void CTemplateMonster::CustomTouch( CBaseEntity* pOther ){

}//END OF CustomTouch



void CTemplateMonster::MonsterThink(){



	CBaseMonster::MonsterThink();
}//END OF MonsterThink

// PrescheduleThink - this function runs after conditions are collected and before scheduling code is run.
// NOTE - PrescheduleThink is called by RunAI of monsterstate.cpp, which is called from MonsterThink in the parent CBaseMonster class (monsters.cpp).
// The "MonsterThink" below still occurs earlier than PrescheduleThink
void CTemplateMonster::PrescheduleThink (){



	CBaseMonster::PrescheduleThink();
}//END OF PrescheduleThink



int CTemplateMonster::Classify(){
	return CLASS_ALIEN_MONSTER;
}
BOOL CTemplateMonster::isOrganic(){
	return TRUE;
}



// What types of nodes can I take?  Default logic checks for the whether the movetype is flying
// (getNodeTypeAllowed()), and if so, picks AIR or WATER depending on whether the montster
// is underwater at the time.  The check is done at the start of each route generating call.
// Or, make it constnat here.  bits_NODE_LAND, AIR, or WATER.  Or multiple.
int CTemplateMonster::getNodeTypeAllowed(void){
	return -1;
}

// IF a differnet null size is fitting, pick that instead.
// Otherwise it's assumed by fitting into one of the stock sizes in node.cpp's HullIndex method.
// The fallback NODE_HUMAN_HULL may not be very good.
// Choices are: NODE_POINT_HULL, NODE_SMALL_HULL, NODE_HUMAN_HULL, NODE_LARGE_HULL, NODE_FLY_HULL
int CTemplateMonster::getHullIndexForNodes(void){
	return NODE_DEFAULT_HULL;
}
// Variant for flyers that support both LAND and AIR/SWIM (either) types to use when searching
// for groundnodes.  The FLY hull uses the same size as large and may miss several perfectly
// good nodes for smaller flyer hull sizes.  Groundmovers or any other case are fine with a 
// redirect to getHullIndexForNodes.
int CTemplateMonster::getHullIndexForGroundNodes(void){
	return getHullIndexForNodes();
}


int CTemplateMonster::IRelationship( CBaseEntity *pTarget ){
	return CBaseMonster::IRelationship(pTarget);
}


void CTemplateMonster::ReportAIState(){
	// call the parent, and add on to that below if necessary (things specific to this monster that the base class can't get)
	CBaseMonster::ReportAIState();

}//END OF ReportAIState()

//Should this monster be any better or worse at hearing than usual?  Higher meants it hears the same sound from a greater distance.
float CTemplateMonster::HearingSensitivity(void){
	return 1.0f;
}//END OF HearingSensitivity

// To have a schedule interrupted by a schedule (and maybe even picked up by conditions at all?), it has to be part of
// this list at least. And part of the sound interrupt list on a schedule. And the schedule has to be interruptible by sounds,
// period, in the general interrupt conditions part.
// See some other sounds not included by default.
// bits_SOUND_CARCASS, bits_SOUND_MEAT and bits_SOUND_GARBAGE (is that used?) for eating form, but support that properly.
// bits_SOUND_DANGER is placed by the anticipated locations of thrown grenades to tell smart things to flee from them. Maybe
// not alien creatures.
int CTemplateMonster::ISoundMask(void){
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			//MODDD - new
			bits_SOUND_BAIT;
}//END OF ISoundMask




// Whether to do the usual "Look" method for checking for new or existing enemies.
// Needed by archers to be able to call "Look" despite the player never going underwater (causes the PVS check to pass
// at least once to start combat).
BOOL CTemplateMonster::noncombat_Look_ignores_PVS_check(void){
	return FALSE;
}//END OF noncombat_Look_ignores_PVS_check

// You must explicitly allow this monster to be able to do the violent death at all, whether it has one available from
// the model or not (but it should be if allowed).  Use the custom animation system and override what ACT_DIEVIOLENT
// returns if the model doesn't do it right or at all.
BOOL CTemplateMonster::violentDeathAllowed(void){
	return FALSE;
}//END OF hasViolentDeathSequence

// Default case should work fine for most monsters.
// Only allow a violent death animation if the last hit solidly did this much damage.
// Could do checks on m_LastHitGroup too (see method GetDeathActivity of combat.cpp)
BOOL CTemplateMonster::violentDeathDamageRequirement(void){
	return (m_lastDamageAmount >= 20);
}

// This method MUST be overridden to do line traces forwards / backwards to see if there is enough space in whatever
// direction to play the animation to avoid clipping through things.  If this is unnecessary, leave this as it is.
// But "hasViolentDeathSequence" must be overridden to 'return TRUE' above regardless.
BOOL CTemplateMonster::violentDeathClear(void){
	return TRUE;
}//END OF

// Bascially, priority is in order from 1 to 3. 1 is in front of all other DIE act's if conditions are met, even
// at the same time as any other DIE act's. 2 is after GUTSHOT and HEADSHOT. 3 only competes with ACT_DIEBACKWARDS, otherwise never gets a chance.
// Note that animations that appear to throw the model strongly backwards benefit from 3, but flyers just look more dramatic on falling.
// Flyers could use 1 or 2 instead.
int CTemplateMonster::violentDeathPriority(void){
	return 3;
}//END OF violentDeathPriority





GENERATE_TRACEATTACK_IMPLEMENTATION(CTemplateMonster){



	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}

// Overridable part of TRACEATTACK that most monsters call. Can change how much damage different hitgroups do,
// preferrably still involving the skill CVars (extra multipliers are fine).
// Start with a copy from combat.cpp's hitgroupDamage.
float CTemplateMonster::hitgroupDamage(float flDamage, int bitsDamageType, int bitsDamageTypeMod, int iHitgroup) {
	
	return CBaseMonster::hitgroupDamage(bitsDamageType, bitsDamageTypeMod, flDamage, iHitgroup);
}//hitgroupDamage

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CTemplateMonster){



	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}


// Given these features sent over from TakeDamage (or TAKEDAMAGE), should I mark bits_COND_LIGHT_DAMAGE or bits_COND_HEAVY_DAMAGE?
// Override this if those should occur more or less commonly, or even affect the cooldowns for either happening.
// They can be exploitable if spammable to make a monster helplessly interrupted all the time.
// Just copy this same method's content from CBaseMonster if the most flexibility is wanted.
void CTemplateMonster::OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){


	CBaseMonster::OnTakeDamageSetConditions(pevInflictor, pevAttacker, flDamage, bitsDamageType, bitsDamageTypeMod);
}//END OF OnTakeDamageSetConditions




// NOTE - called by CBaseMonster's TakeDamage method. If that isn't called, DeadTakeDamage won't get called naturally.
GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(CTemplateMonster){



	return GENERATE_DEADTAKEDAMAGE_PARENT_CALL(CBaseMonster);
}//END OF DeadTakeDamage




// Parameters: integer named fGibSpawnsDecal
//  The parent method calls GIBMONSTERGIB, then GIBMONSTERSOUND and GIBMONSTEREND, passing along to the latter two whether GIBMONSTERGIB
//  really "gibbed" or not (it must return whether it did or not).
//  This default behavior is ok, change pieces at a time instead in other methods. GIBMONSTERGIB is the most likely one that needs
//  customization, such as only spawning two alien gibs for the ChumToad. The others are good as defaults.
GENERATE_GIBMONSTER_IMPLEMENTATION(CTemplateMonster)
{
	GENERATE_GIBMONSTER_PARENT_CALL(CBaseMonster);
}



// Parameters: BOOL fGibSpawnsDecal
// Returns: BOOL. Did this monster spawn gibs and is safe to stop drawing?
//  If this monster has a special way of spawning gibs or checking whether to spawn gibs, handle that here and remove the parent call.
//  The parent GIBMONSTERGIB method is supposed to handle generic cases like determining whether to spawn human or alien gibs based on
//  Classify(), or robot gibs under german censorship. This implementation can be specific to just this monster instead.
// NOTICE - do NOT do something special here AND call the parent method through GENERATE_GIBMONSTERGIB_PARENT_CALL.
// Anything done here is meant to completely replace how the parent method gibs a monster in general. None of it is required.
GENERATE_GIBMONSTERGIB_IMPLEMENTATION(CTemplateMonster){
	




	return GENERATE_GIBMONSTERGIB_PARENT_CALL(CBaseMonster);
}

//The other related methods, GIBMONSTERSOUND and GIBMONSTEREND, are well suited to the majority of cases.




GENERATE_KILLED_IMPLEMENTATION(CTemplateMonster){



	GENERATE_KILLED_PARENT_CALL(CBaseMonster);
}//END OF Killed


// When this monster is about to be removed from the game (FL_KILLME is set in pev->flags, or REMOVE_ENTITY is about to be used),
// make sure any looping sounds (automatic for short-duration sounds for whatever reason) are told to stop, or else they will go
// on forever. How things are removed by not fitting into map transitions is not understood as well. It is probaby up to the
// engine depending on whether or not something is marked a certain way by method "ChangeList" of triggers.cpp.
// I think looping sounds are reset on initiating transitions, so that shouldn't be an issue at least.
void CTemplateMonster::onDelete(void){
	//UTIL_StopSound(ENT(pev), CHAN_X, "templatemonster/templatemonster_loopingsound.wav");
	//...

}//END OF onDelete


void CTemplateMonster::SetYawSpeed( void ){
	
	// Switch on current activity m_Activity to determine yaw speed and set it?
	switch ( m_Activity ){
		case ACT_IDLE:{
			//pev->yaw_speed = 200;
		break;}
		case ACT_WALK:{
			//pev->yaw_speed = 140;
		break;}
		//case etc.


		default:{
			pev->yaw_speed = 200;
		break;}
	}//END OF switch
	
}//END OF SetYawSpeed





BOOL CTemplateMonster::getMonsterBlockIdleAutoUpdate(void){
	return FALSE;
}
BOOL CTemplateMonster::forceIdleFrameReset(void){
	return FALSE;
}
// Whether this monster can re-pick the same animation before the next frame starts if it anticipates it getting picked.
// This is subtly retail behavior, but may not always play well.
BOOL CTemplateMonster::canPredictActRepeat(void){
	return TRUE;
}
BOOL CTemplateMonster::usesAdvancedAnimSystem(void){
	return TRUE;
}
void CTemplateMonster::SetActivity( Activity NewActivity ){
	CBaseMonster::SetActivity(NewActivity);
}



//IMPORTANT. To get the animation from the model the usual way, you must use "CBaseAnimating::LookupActivity(activity);" to do so,
//           do NOT forget the "CBaseAnimating::" in front and do NOT use any other base class along the way, like CSquadMonster or CBaseMonster.
//           Need to call CBaseAnimating's primitive version because CBaseMonster's LookupActivity may now call LookupActivityHard or tryActivitySubstitute
//           on its own, which would trigger an infinite loop of calls (stack overflow):
//           * This class calling LookupActivityHard, persay, falling back to calling...
//           * Monster's LookupActivity, deciding to call the...
//           * Child class's LookupActiivtyHard for a possible substitution, falling back to calling...
//           * Monster's LookupActivity, deciding to call the...
//           * Child class's LookupActivityHard for a possible substitution, falling back to...
//           Repeat the last two ad infinitum. Crash.
// It is also possible to get an activity from a name through a string like most places:  LookupSequence("runandgun")
// Note that tryActivitySubstitute doesn't usually do anything with a particular sequence given, but verifies that there
// is one at all.  It is not wise to do framerate changesor add anim-queue events for a sequence that may not even get
// picked.
// See LookupActivityHard below for when a sequence to get is most likely to be committed (makes more sense to do the 
// mentioned finer adjustments).
int CTemplateMonster::tryActivitySubstitute(int activity){
	int i = 0;

	// no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_IDLE:{
			return SEQ_TEMPLATEMONSTER_XXX;
		break;}
	}//END OF switch


	// not handled by above? Rely on the model's anim for this activity if there is one.
	return CBaseAnimating::LookupActivity(activity);
}//END OF tryActivitySubstitute

// 
int CTemplateMonster::LookupActivityHard(int activity){
	int i = 0;
	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	// any animation events in progress?  Clear it.
	resetEventQueue();

	//Within an ACTIVITY, pick an animation like this (with whatever logic / random check first):
	//    this->animEventQueuePush(10.0f / 30.0f, 3);  //Sets event #3 to happen at 1/3 of a second
	//    return LookupSequence("die_backwards");      //will play animation die_backwards

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_IDLE:{
			//random chance?
			//return SEQ_TEMPLATEMONSTER_XXX;
		break;}
	}//END OF switch
	
	// not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard



// Handles custom events sent from "LookupActivityHard", which sends events as timed delays along with picking an animation in script.
// So this handles script-provided events, not model ones.
void CTemplateMonster::HandleEventQueueEvent(int arg_eventID){

	switch(arg_eventID){
		case 0:{


		break;}
		case 1:{


		break;}
	}//END OF switch

}//END OF HandleEventQueueEvent


// This handles events built into the model, not custom hard-coded ones (above).
void CTemplateMonster::HandleAnimEvent(MonsterEvent_t* pEvent ){
	
	switch(pEvent->event){
		case 0:{


		break;}
		case 1:{


		break;}
		default:{
			CBaseMonster::HandleAnimEvent( pEvent );
		break;}
	}//END OF switch

}//END OF HandleAnimEvent


