
//TODO: stop the horror loop on death (if it was playing) with a chord?


//TODO: should the friendly flee?
//Should the friendly move faster than it is currently (rather slow)?

#include "friendly.h"
#include "schedule.h"
#include "activity.h"
#include "util_model.h"

#include "defaultai.h"
#include "soundent.h"
#include "game.h"
#include "player.h"



EASY_CVAR_EXTERN_DEBUGONLY(friendlyPianoOtherVolume)

EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard)
EASY_CVAR_EXTERN_DEBUGONLY(friendlyPrintout)

extern unsigned short g_sFriendlyVomit;




#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_friendly, CFriendly );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( friendly, CFriendly );
	
	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( mrfriendly, CFriendly );
	#endif
	
#endif




enum friendly_sequence{  //key: frames, FPS
	FRIENDLY_IDLE1,
	FRIENDLY_WALK,
	FRIENDLY_SMALLFLINCH,
	FRIENDLY_SMALLFLINCH2,
	FRIENDLY_ATTACK,
	FRIENDLY_DOUBLEWHIP,
	FRIENDLY_DIESIMPLE,
	FRIENDLY_VOMIT
};





//schedule details here......


//custom schedules
enum
{
	SCHED_FRIENDLY_SEEK_CORPSE = LAST_COMMON_SCHEDULE + 1,
	SCHED_FRIENDLY_SEEK_PLAYER_CORPSE,
	SCHED_FRIENDLY_SEEK_CORPSE_FAIL,
	SCHED_FRIENDLY_TAKE_COVER_FROM_PLAYER,
	SCHED_FRIENDLY_TAKE_COVER_FROM_PLAYER_FAIL,
	SCHED_FRIENDLY_STARE_AT_PLAYER,
};


//custom tasks
enum
{
	TASK_FRIENDLY_SEEK_CORPSE = LAST_COMMON_TASK + 1,
	TASK_FRIENDLY_SEEK_CORPSE_FAIL_WAIT,
	TASK_FRIENDLY_EAT_CORPSE,
	TASK_FRIENDLY_EAT_POST_WAIT,
	TASK_FRIENDLY_GENERIC_WAIT,
	TASK_FRIENDLY_SETUP_STAREBACK,
	TASK_FRIENDLY_TAKE_COVER_FROM_PLAYER,
	TASK_FRIENDLY_TAKE_COVER_FROM_PLAYER_FAIL_WAIT,
	TASK_FRIENDLY_STARE_AT_PLAYER,
	TASK_WAIT_FOR_MOVEMENT_RANGE_CHECK_TARGET,

};



Task_t	tlFriendlySeekCorpse[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_FRIENDLY_SEEK_CORPSE_FAIL	},
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_WAIT,					(float)0.2					},
	{ TASK_FRIENDLY_SEEK_CORPSE,	(float)0					},
	{ TASK_WALK_TO_TARGET,			(float)0					},
	{ TASK_WALK_PATH,	(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT_RANGE_CHECK_TARGET,	(float)40					},
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_FACE_TARGET,				(float)0					},
	{ TASK_FRIENDLY_EAT_CORPSE,		(float)0					},
	//{ TASK_FRIENDLY_SETUP_STAREBACK, (float)0					},
	{ TASK_FRIENDLY_EAT_POST_WAIT,	(float)0					},
};

Schedule_t	slFriendlySeekCorpse[] =
{
	{
		tlFriendlySeekCorpse,
		ARRAYSIZE ( tlFriendlySeekCorpse ),
		//TODO: much more!  sound_  stuff too.
		
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_ATTACK |
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD		|// sound flags
		bits_SOUND_DANGER,
		"friendlySeekCorpse"
	},
};



Task_t	tlFriendlySeekPlayerCorpse[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_FRIENDLY_SEEK_CORPSE_FAIL	},
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_WAIT,					(float)0.2					},
	{ TASK_FRIENDLY_SEEK_CORPSE,	(float)0					},
	{ TASK_WALK_TO_TARGET,			(float)0					},
	{ TASK_WALK_PATH,	(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT_RANGE_CHECK_TARGET,	(float)75					},
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_FACE_TARGET,				(float)0					},
	{ TASK_FRIENDLY_EAT_CORPSE,		(float)0					},
	//{ TASK_FRIENDLY_SETUP_STAREBACK, (float)0					},
	{ TASK_FRIENDLY_EAT_POST_WAIT,	(float)0					},
};
Schedule_t	slFriendlySeekPlayerCorpse[] =
{
	{
		tlFriendlySeekPlayerCorpse,
		ARRAYSIZE ( tlFriendlySeekPlayerCorpse ),
		//TODO: much more!  sound_  stuff too.
		
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_ATTACK |
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD		|// sound flags
		bits_SOUND_DANGER,
		"friendlySeekPlayerCorpse"
	},
};





Task_t	tlFriendlySeekCorpseFail[] =
{
	{ TASK_STOP_MOVING,								0				},
	{ TASK_FRIENDLY_SEEK_CORPSE_FAIL_WAIT,			0				},
};

Schedule_t	slFriendlySeekCorpseFail[] =
{
	{
		tlFriendlySeekCorpseFail,
		ARRAYSIZE ( tlFriendlySeekCorpseFail ),
		
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_ATTACK |
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD		|// sound flags
		bits_SOUND_DANGER,

		"friendlySeekCorpseFail"
	},
};






Task_t	tlFriendlyTakeCoverFromPlayer[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_FRIENDLY_TAKE_COVER_FROM_PLAYER_FAIL	},
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_WAIT,					(float)0.2					},
	{ TASK_FRIENDLY_TAKE_COVER_FROM_PLAYER,	(float)0					},
	{ TASK_WALK_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_STOP_MOVING,				(float)0					},
	//{ TASK_FRIENDLY_SETUP_STAREBACK, (float)0					},
	{ TASK_FRIENDLY_GENERIC_WAIT,	(float)0					},
};

Schedule_t	slFriendlyTakeCoverFromPlayer[] =
{
	{ 
		tlFriendlyTakeCoverFromPlayer,
		ARRAYSIZE ( tlFriendlyTakeCoverFromPlayer ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_ENEMY |
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_ATTACK |
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD		|// sound flags
		bits_SOUND_DANGER,

		"friendlyTakeCoverFromPlayer"
	},
};




Task_t	tlFriendlyTakeCoverFromPlayerFail[] =
{
	{ TASK_STOP_MOVING,								0				},
	{ TASK_FRIENDLY_TAKE_COVER_FROM_PLAYER_FAIL_WAIT,			0				},
};

Schedule_t	slFriendlyTakeCoverFromPlayerFail[] =
{
	{
		tlFriendlyTakeCoverFromPlayerFail,
		ARRAYSIZE ( tlFriendlyTakeCoverFromPlayerFail ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_ATTACK |
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD		|// sound flags
		bits_SOUND_DANGER,
		"friendlyTakeCoverFromPlayerFail"
	},
};



Task_t	tlFriendlyStareAtPlayer[] =
{
	{ TASK_FRIENDLY_STARE_AT_PLAYER,			0				},
};

Schedule_t	slFriendlyStareAtPlayer[] =
{
	{
		tlFriendlyStareAtPlayer,
		ARRAYSIZE ( tlFriendlyStareAtPlayer ),
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_ENEMY |
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_ATTACK |
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD		|// sound flags
		bits_SOUND_DANGER,
		"friendlyStareAtPlayer"
	},
};


DEFINE_CUSTOM_SCHEDULES( CFriendly )
{


	slFriendlySeekCorpse,
	slFriendlySeekPlayerCorpse,
	slFriendlySeekCorpseFail,
	slFriendlyTakeCoverFromPlayer,
	slFriendlyTakeCoverFromPlayerFail,
	slFriendlyStareAtPlayer,
};

IMPLEMENT_CUSTOM_SCHEDULES( CFriendly, CBaseMonster );









const char* CFriendly::pDeathSounds[] = 
{
	"friendly/friendly_death.wav",
};
const char* CFriendly::pAlertSounds[] = 
{
	"friendly/friendly_alert.wav",
};

const char* CFriendly::pIdleSounds[] = 
{
	"friendly/friendly_idle.wav",
};
const char* CFriendly::pPainSounds[] = 
{
	"friendly/friendly_pain.wav",
};
const char* CFriendly::pAttackSounds[] = 
{
	"friendly/friendly_attack.wav",
};

const char* CFriendly::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};
const char* CFriendly::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};
const char* CFriendly::pVomitVoiceSounds[] = 
{
	"scientist/scream01.wav",
	"scientist/scream02.wav",
};
const char* CFriendly::pVomitHitSounds[] = 
{
	"weapons/electro4.wav",    // This is precached by the player's gauss, always available.  Rest are too.
	"weapons/electro5.wav",
	"weapons/electro6.wav",
};
const char* CFriendly::pChewSounds[] = 
{
	"barnacle/bcl_chew1.wav",
	"barnacle/bcl_chew2.wav",
	"barnacle/bcl_chew3.wav",
};

const char* CFriendly::pVomitSounds[] =
{
	"barnacle/bcl_tongue1.wav",
};


TYPEDESCRIPTION	CFriendly::m_SaveData[] = 
{
	DEFINE_FIELD( CFriendly, m_fPissedAtPlayer, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFriendly, m_fPissedAtPlayerAlly, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFriendly, m_fPissedAtHumanMilitary, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFriendly, extraPissedFactor, FIELD_FLOAT),
	
};

//IMPLEMENT_SAVERESTORE( CFriendly, CBaseMonster );
int CFriendly::Save( CSave &save )
{
	if ( !CBaseMonster::Save(save) )
		return 0;
	return save.WriteFields( "CFriendly", this, m_SaveData, ARRAYSIZE(m_SaveData) );
}
int CFriendly::Restore( CRestore &restore )
{
	if ( !CBaseMonster::Restore(restore) )
		return 0;
	int readFieldsResult = restore.ReadFields( "CFriendly", this, m_SaveData, ARRAYSIZE(m_SaveData) );
	

	SetTouch(&CFriendly::CustomTouch );

	PostRestore();
	return readFieldsResult;
}

void CFriendly::PostRestore(void) {
	if (nextNormalThinkTime == 0)nextNormalThinkTime = 0.01;
	if (pev->nextthink == 0)pev->nextthink = 0.01;

}





CFriendly::CFriendly(void){

	
	m_fPissedAtPlayer = FALSE;
	m_fPissedAtPlayerAlly = FALSE;
	m_fPissedAtHumanMilitary = FALSE;

	extraPissedFactor = 0;

	playedVomitSoundYet = FALSE;

	nextVomitHitSoundAllowed = -1;

	horrorPlayTime = -1;
	horrorSelected = FALSE;

	horrorPlayTimePreDelay = -1;

	eatFinishTimer = -1;
	eatFinishPostWaitTimer = -1;
	nextPlayerSightCheck = -1;
	waitTime = -1;
	timeToStare = -1;
	vomitCooldown = -1;
	nextChewSound = -1;

	playerToLookAt = NULL;
	corpseToSeek = NULL;

	rapidVomitCheck = FALSE;
	rapidVomitCheck_ScheduleFinish = FALSE;
	nextNormalThinkTime = 0.01;


}//END OF CFriendly constructor




	
// NOTE - the arrays are placeholders, don't try to use.
void CFriendly::DeathSound( void ){
	/*
	int pitch = 95 + RANDOM_LONG(0,9);
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_IDLE, 0, pitch );
	*/
}
void CFriendly::AlertSound( void ){
	/*
	int pitch = 95 + RANDOM_LONG(0,9);
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1.0, ATTN_NORM, 0, pitch );
	*/
}
void CFriendly::IdleSound( void ){
	/*
	int pitch = 95 + RANDOM_LONG(0,9);
	// Play a random idle sound
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, ATTN_NORM, 0, pitch );
	*/
}
void CFriendly::PainSound( void ){
	/*
	int pitch = 95 + RANDOM_LONG(0,9);
	if (RANDOM_LONG(0,5) < 2){
		UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch );
	}
	*/

	// just play high-pitched scientist pain sentences for now

	int pitch = 40 + RANDOM_LONG(0,8);
	//const char* targetString = UTIL_VarArgs("!scientist_sci_pain%d", RANDOM_LONG(1, 10));
	//UTIL_PlaySound( edict(), CHAN_VOICE, targetString, 1.0, ATTN_NORM, 0, pitch );
	int theChoice = RANDOM_LONG(0, 3);

	//easyForcePrintLine("I PLAID %d", theChoice);

	switch(theChoice){
		case 0:UTIL_PlaySound( edict(), CHAN_VOICE, "!scientist_sci_pain1", 1.0, ATTN_NORM, 0, pitch );break;
		case 1:UTIL_PlaySound(edict(), CHAN_VOICE, "!scientist_sci_pain4", 1.0, ATTN_NORM, 0, pitch); break;
		case 2:UTIL_PlaySound(edict(), CHAN_VOICE, "!scientist_sci_pain5", 1.0, ATTN_NORM, 0, pitch); break;
		// scientist/sci_fear15.wav
		case 3:UTIL_PlaySound(edict(), CHAN_VOICE, "!SC_SCREAM14", 1.0, ATTN_NORM, 0, pitch); break;

	}//switch

	
}
void CFriendly::AttackSound( void ){
	/*
	int pitch = 95 + RANDOM_LONG(0,9);
	// Play a random attack sound
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, pitch );
	*/
}





void CFriendly::VomitSound(void){
	int pitch = 88 + RANDOM_LONG(0,8);
	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pVomitSounds), 1.0, ATTN_NORM - 0.2, 0, pitch );
}
void CFriendly::VomitVoiceSound(void){
	int pitch = 40 + RANDOM_LONG(0,8);
	// Play a random attack sound
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pVomitVoiceSounds), 1.0, ATTN_NORM, 0, pitch );
}
void CFriendly::VomitHitSound(edict_t* pevToPlayAt){	
	int pitch = 80 + RANDOM_LONG(0, 6);
	//UTIL_PlaySound( edict(), CHAN_STATIC, RANDOM_SOUND_ARRAY(pVomitHitSounds), 1.0, ATTN_NORM, 0, pitch );
	
	const int randomSound = RANDOM_LONG(0, ARRAYSIZE(pVomitHitSounds)-1 );
	
	// UTIL_EmitAmbientSound ?
	// no soundsentencesave.  (Ambient supports sentences though)
	UTIL_PlaySound( pevToPlayAt, CHAN_STATIC, pVomitHitSounds[randomSound], 1.0, ATTN_NORM, 0, pitch, FALSE);
	
}//END OF VomitHitSound



extern int global_useSentenceSave;
void CFriendly::Precache( void )
{

	PRECACHE_MODEL("models/friendly.mdl");

	PRECACHE_MODEL( "sprites/hotglow_green.spr" );




	global_useSentenceSave = TRUE;
	
	//PRECACHE_SOUND("friendly/friendly_XXX.wav");

	PRECACHE_SOUND("friendly/friendly_horror.wav");
	PRECACHE_SOUND("friendly/friendly_horror_start.wav");
	PRECACHE_SOUND("friendly/friendly_horror_end.wav");

	PRECACHE_SOUND("barnacle/bcl_bite3.wav");

	//when we have them!
	/*
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	*/
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);



	UTIL_PRECACHESOUND_ARRAY(pVomitVoiceSounds, ARRAYSIZE(pVomitVoiceSounds) );
	UTIL_PRECACHESOUND_ARRAY(pVomitHitSounds, ARRAYSIZE(pVomitHitSounds), TRUE); //don't skip, player precaches these.
	UTIL_PRECACHESOUND_ARRAY(pChewSounds, ARRAYSIZE(pChewSounds));
	UTIL_PRECACHESOUND_ARRAY(pVomitSounds, ARRAYSIZE(pVomitSounds));


	

	global_useSentenceSave = FALSE;

}//END OF Precache()



void CFriendly::Spawn( void )
{
	Precache( );

	SET_MODEL(ENT(pev), "models/friendly.mdl");
	//UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));
	//UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));  //agrunt
	
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	//UTIL_SetSize(pev, Vector(-20, -20, 0), Vector(20, 20, 36));
	
	//UTIL_SetSize(pev, Vector(-18, -18, 0), Vector(18, 18, 40));

	pev->classname = MAKE_STRING("monster_friendly");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	pev->health			= gSkillData.friendlyHealth;
	pev->view_ofs		= Vector ( 0, 0, 20 );// position of the eyes relative to monster's origin.
	pev->yaw_speed		= 5;//!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?

	// VIEW_FIELD_WIDE?  it is -0.7.
	m_flFieldOfView		= -0.15;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();

	
	//m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	//MODDD - no longer necessary, yes?
	/*
	m_afCapability |= bits_CAP_MELEE_ATTACK1;
	m_afCapability |= bits_CAP_MELEE_ATTACK2;
	m_afCapability |= bits_CAP_RANGE_ATTACK1;
	*/

	SetTouch(&CFriendly::CustomTouch );
	//SetTouch( NULL );

}//END OF Spawn();


//based off of GetSchedule for CBaseMonster in schedule.cpp.
Schedule_t* CFriendly::GetSchedule ( void )
{
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
				//Hold up. Maybe we can creep around.


				//try to get the corpse.corpseToSeek
				CBaseEntity* corpseTest = getNearestDeadBody();

				if(corpseTest){
					//find it!
					this->corpseToSeek = corpseTest;

					//EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: Corpse! %s"), STRING(corpseTest->pev->classname) );
					if(corpseTest->pev->flags & (FL_CLIENT)){
					
						easyForcePrintLine("IS THAT A PLAYER?");
						return GetScheduleOfType( SCHED_FRIENDLY_SEEK_PLAYER_CORPSE );	
					}else{
						easyForcePrintLine("IS THAT A MONSTER?");
						return GetScheduleOfType( SCHED_FRIENDLY_SEEK_CORPSE );	
					}
					
					
				}else{
					//wander for out-of-player cover?
					EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: Wander!"));
					return GetScheduleOfType( SCHED_FRIENDLY_TAKE_COVER_FROM_PLAYER );	
				}

				




				// no valid route!
				//return GetScheduleOfType( SCHED_IDLE_STAND );
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
				/*
				if ( fabs( FlYawDiff() ) < (1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN );
				}
				else
				{
					return GetScheduleOfType( SCHED_ALERT_SMALL_FLINCH );
				}
				*/
				//Mr. Friendly will always flinch.
				return GetScheduleOfType( SCHED_ALERT_SMALL_FLINCH );
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
					return GetScheduleOfType( SCHED_CHASE_ENEMY_SMART );
				}
			}
			else  
			{

				//easyPrintLine("I say, what now? %d %d", HasConditions(bits_COND_CAN_RANGE_ATTACK1), HasConditions(bits_COND_CAN_RANGE_ATTACK2) );

				// we can see the enemy
				if ( HasConditions(bits_COND_CAN_RANGE_ATTACK1) )
				{
					vomitCooldown = gpGlobals->time + 6.0;
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
					return GetScheduleOfType( SCHED_CHASE_ENEMY_SMART );
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


Schedule_t* CFriendly::GetScheduleOfType( int Type){
	
	EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("FRIENDLY: GET SCHEDULE OF TYPE %d", Type));

	switch(Type){
		
		case SCHED_FRIENDLY_SEEK_CORPSE:
			return &slFriendlySeekCorpse[0];
		case SCHED_FRIENDLY_SEEK_PLAYER_CORPSE:
			return &slFriendlySeekPlayerCorpse[0];
		case SCHED_FRIENDLY_SEEK_CORPSE_FAIL:
			return &slFriendlySeekCorpseFail[0];
		case SCHED_FRIENDLY_TAKE_COVER_FROM_PLAYER:
			return &slFriendlyTakeCoverFromPlayer[0];
		case SCHED_FRIENDLY_TAKE_COVER_FROM_PLAYER_FAIL:
			return &slFriendlyTakeCoverFromPlayerFail[0];
		case SCHED_FRIENDLY_STARE_AT_PLAYER:
			return &slFriendlyStareAtPlayer[0];

	}//END OF switch(Type)
	
	return CBaseMonster::GetScheduleOfType(Type);
}//END OF GetScheduleOfType(...)



void CFriendly::StartTask( Task_t *pTask ){


	switch( pTask->iTask ){
		case TASK_FRIENDLY_SEEK_CORPSE:
		{

			if(corpseToSeek == NULL){
				m_hTargetEnt = NULL; //just in case.
				TaskFail();
				return;
			}

			m_hTargetEnt = corpseToSeek;
			TaskComplete();


			/*
			CBaseEntity* corpseTest = getNearestDeadBody();
			if(corpseTest != NULL){
				//thing to path towards next.
				m_hTargetEnt = corpseTest;
				TaskComplete();
				//good, proceed.
			}else{
				//just in case?
				m_hTargetEnt = NULL;
				TaskFail();
				return;
			}
			*/
			

		break;
		}
		case TASK_FRIENDLY_EAT_CORPSE:
		{
			//TODO - make it impossible to take the same corpse as another mr. friendly!
			float corpseEatTime;
			Vector corpseBounds;
			float corpseSize;

			if(FNullEnt(m_hTargetEnt)){
				//how did that happen? Just stop.
				TaskFail();
				return;
			}
			
			//play a bite sound?
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_bite3.wav", 1, ATTN_NORM, 0, 80 + RANDOM_LONG(2, 8) );	

			//headcrab size: 13824
			//size of agrunt: about 348160. corpse is 3073280.00
			corpseBounds = (m_hTargetEnt->pev->absmax - m_hTargetEnt->pev->absmin);
			corpseSize = corpseBounds.x * corpseBounds.y * corpseBounds.z;

			//easyForcePrintLine("CORPSE SIZE?! %.2f", corpseSize);

			//NOTICE: corpses often get bigger bounding boxes than when the monster was alive. multiplied 27,000 by 10.
			corpseEatTime = RANDOM_FLOAT(5, 8) + corpseSize/270000 * RANDOM_FLOAT(0.9, 1.1);
			
			//easyForcePrintLine("YOU PSYCHOPATH %.2f", corpseEatTime);
			

			//to show eating for now. closest thing I suppose?
			SetSequenceByIndexForceLoops(FRIENDLY_VOMIT, -1.1f, TRUE, TRUE );

			eatFinishTimer = gpGlobals->time + corpseEatTime;
			if(corpseEatTime < 8){
				eatFinishPostWaitTimer = gpGlobals->time + corpseEatTime + RANDOM_FLOAT(6, 12);
			}else if(corpseEatTime < 999){
				eatFinishPostWaitTimer = gpGlobals->time + corpseEatTime + RANDOM_FLOAT(5, 10);
			}

			this->nextChewSound = gpGlobals->time + 0.8;
			
		break;
		}
		case TASK_FRIENDLY_GENERIC_WAIT:

			//time to check when any players are looking at me (changeSchedule to stare back)
			nextPlayerSightCheck = gpGlobals->time + 1.0;

			//time to just sit and wait for a new schedule, like find another corpse or move to other cover.
			waitTime = gpGlobals->time + RANDOM_FLOAT(14, 23);

		break;
		case TASK_FRIENDLY_EAT_POST_WAIT:
			nextPlayerSightCheck = gpGlobals->time + 1.0;
			//nothing else to do. "eatFinishPostWaitTimer" already tells me how long to wait.
		break;
		case TASK_FRIENDLY_SEEK_CORPSE_FAIL_WAIT:
		case TASK_FRIENDLY_TAKE_COVER_FROM_PLAYER_FAIL_WAIT:

			this->SetActivity(ACT_IDLE);

			waitTime = gpGlobals->time + RANDOM_FLOAT(2, 4.5);
		break;
		case TASK_WAIT_FOR_MOVEMENT_RANGE_CHECK_TARGET:
		{
			if(FNullEnt(m_hTargetEnt)){
				TaskFail();
				return;
			}
			//same thing as the parent class.
			Task_t tempTask;
			tempTask.iTask = TASK_WAIT_FOR_MOVEMENT_RANGE;
			tempTask.flData = pTask->flData;
			CBaseMonster::RunTask(&tempTask);
		break;
		}
		case TASK_FRIENDLY_TAKE_COVER_FROM_PLAYER:
		{

			float leastDistanceYet = 1200;
			entvars_t* pevPlayerToHideFrom = NULL;
			float tempDist = 0;
			//Try to scan the local area for a player?
			CBaseEntity* pPlayerEntityScan = NULL;
			while( (pPlayerEntityScan = UTIL_FindEntityByClassname(pPlayerEntityScan, "player")) != NULL){
				
				if(pPlayerEntityScan->pev->deadflag != DEAD_NO || entityHidden(pPlayerEntityScan) ){
					continue; //doesn't count
				}
				/*
				//unnecessary?
				if( (pPlayerEntityScan->pev->origin - pev->origin).Length() > 1200){
					//too far away to care. disregard.
					continue;
				}
				*/

				tempDist = (pPlayerEntityScan->pev->origin - pev->origin).Length();
				if( tempDist < leastDistanceYet ){
					pevPlayerToHideFrom = pPlayerEntityScan->pev;
					leastDistanceYet = tempDist;
				}


				//For now, the only criteria is closeness. Hide from the closest player, distance-wise.



				/*
				//TODO - in case there are multiple players, pick just one to hide from. If able to look at us and indeed looking at us (dot product check), that should put him at a higher weight.
				//       e.g., a player 1000 units away who is looking straight at me may weight more heaviliy for hiding priority than a player 700 units away that has no line of sight to me.
				TraceResult trSeeCheck;
				UTIL_TraceLine(this->pev->origin + Vector(0, 0, 10), pPlayerEntityScan->EyePosition(), dont_ignore_monsters, ignore_glass, pPlayerEntityScan->edict(), &trSeeCheck);

				//if our fraction is not 1.0 (miss? and the thing hit (if anything was hit) is not pEntiyScan (the corpse), we hit something blocking the view.
				//Good.
				if(trSeeCheck.flFraction != 1.0 && CBaseEntity::Instance(trSeeCheck.pHit)->pev != this->pev ){
					//this player's vision to us is blocked.

				}else{
					//this player is looking at us, or is capable of!

					//failure!
					//playerLooking = TRUE;
					//break;
				}
				*/

			}//END OF player scan check


			//if(newPathDelay == -1){
				//findCoverTaskDataMem = pTask->flData;
			//Hide from the nearest (or whatever was picked) player. If none was found or close enough, this will just randomly move to another spot from itself.
			friendly_findCoverFromPlayer(pevPlayerToHideFrom, 0);
				//newPathDelay = gpGlobals->time + newPathDelayDuration;
				//waitingForNewPath = FALSE;
			//}else{
			//	waitingForNewPath = TRUE;
			//}

			break;
		}
		case TASK_FRIENDLY_STARE_AT_PLAYER:
		{
			timeToStare = gpGlobals->time + RANDOM_FLOAT(6, 8.5);
		break;
		}
		default:
			CBaseMonster::StartTask( pTask );
		break;
	}//END OF switch(...)

}//END OF StartTask(...)

void CFriendly::RunTask( Task_t *pTask ){
	
	//EASY_CVAR_PRINTIF_PRE(templatePrintout, easyPrintLine("RunTask: sched:%s task:%d", this->m_pSchedule->pName, pTask->iTask) );
	
	switch( pTask->iTask ){
		case TASK_FRIENDLY_SEEK_CORPSE:

		break;
		case TASK_FRIENDLY_EAT_CORPSE:
		{
			
			if(FNullEnt(m_hTargetEnt)){
				//how did that happen? Just stop.
				TaskFail();
				return;
			}
			//easyForcePrintLine("TELL ME?! %.2f %.2f", gpGlobals->time, eatFinishTimer);
			if(gpGlobals->time >= eatFinishTimer){
				//done eating. Gib the corpse?

				//TODO - make it so these spawned gibs can't make decals? something about editing the sticky gibs perhaps?
				m_hTargetEnt->pev->health = -50;
				m_hTargetEnt->Killed( this->pev, this->pev, GIB_ALWAYS_NODECAL );
				//m_hTargetEnt->GetMonsterPointer()->GibMonster(FALSE, TRUE);
				eatFinishTimer = -1;
				TaskComplete();

				SetActivity(ACT_IDLE);
				return;
			}

			if(gpGlobals->time >= nextChewSound){
				nextChewSound = gpGlobals->time + 1.0;
				UTIL_PlaySound( ENT(pev), CHAN_STATIC, RANDOM_SOUND_ARRAY(pChewSounds), 1.0, ATTN_NORM, 0, 90 + RANDOM_LONG(2, 8) );
			}



		break;
		}
		case TASK_FRIENDLY_EAT_POST_WAIT:
		case TASK_FRIENDLY_GENERIC_WAIT:
		{
			// waiting in cover.
			//If the player looks at me, look back for a little perhaps? interrupt to have a "stare at" schedule and then wait a little more, uninterruptably?


			
			if(gpGlobals->time >= nextPlayerSightCheck){
				nextPlayerSightCheck = gpGlobals->time + 0.7;
				

				float leastDistanceYet = 1600;
				CBaseEntity* closestLookingPlayer = NULL;

				CBaseEntity* pPlayerEntityScan = NULL;
				while( (pPlayerEntityScan = UTIL_FindEntityByClassname(pPlayerEntityScan, "player")) != NULL){
					
					if(pPlayerEntityScan->pev->deadflag != DEAD_NO || entityHidden(pPlayerEntityScan) ){
						continue; //doesn't count
					}

					float distToPlayer = (pPlayerEntityScan->pev->origin - pev->origin).Length();

					//unnecessary?
					/*
					if(distToPlayer > 1600){
						continue;
					}
					*/
					
					TraceResult trSeeCheck;
					UTIL_TraceLine(pPlayerEntityScan->EyePosition(), this->pev->origin + Vector(0, 0, 10), dont_ignore_monsters, ignore_glass, pPlayerEntityScan->edict(), &trSeeCheck);

					
					BOOL daMatchaa = FALSE;
					CBaseEntity* tempEnt = CBaseEntity::Instance(trSeeCheck.pHit);
					if(tempEnt != NULL){
						daMatchaa = (tempEnt->pev == this->pev);
					}
					
					
					EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: Player looking at me? trace:%d, dist:%.2f, looking:%d", daMatchaa, distToPlayer, UTIL_IsFacing(pPlayerEntityScan->pev, pev->origin, 0.3)));

					
					

					////there is a straight line from them to me, good.
					if(daMatchaa ){
						//not only is looking possible (unobstructed straight line from player to me), they also must BE looking.
						if(UTIL_IsFacing(pPlayerEntityScan->pev, pev->origin, 0.3)){
							//now the leastDist check.
							if(distToPlayer < leastDistanceYet){
								closestLookingPlayer = pPlayerEntityScan;
								leastDistanceYet = distToPlayer;
							}

						}
					}

				}//END OF player while loop


				if(closestLookingPlayer != NULL){
					playerToLookAt = closestLookingPlayer;
					ChangeSchedule(slFriendlyStareAtPlayer);
					return;
				}

			}//END OF nextPlayerSightCheck check



			if(pTask->iTask == TASK_FRIENDLY_EAT_POST_WAIT){
				if(gpGlobals->time >= eatFinishPostWaitTimer){
					eatFinishPostWaitTimer = -1;
					TaskComplete();
				}
			}else if(pTask->iTask == TASK_FRIENDLY_GENERIC_WAIT){
				
				if(gpGlobals->time >= waitTime){
					waitTime = -1;
					TaskComplete(); //that's it.
				}
			}


		break;
		}
		case TASK_FRIENDLY_SEEK_CORPSE_FAIL_WAIT:
		case TASK_FRIENDLY_TAKE_COVER_FROM_PLAYER_FAIL_WAIT:
			if(gpGlobals->time >= waitTime){
				waitTime = -1;
				TaskComplete(); //that's it.
			}
		break;
		case TASK_WAIT_FOR_MOVEMENT_RANGE_CHECK_TARGET:
		{
			if(FNullEnt(m_hTargetEnt)){
				TaskFail();
				return;
			}
			
			//same thing as the parent class.
			Task_t tempTask;
			tempTask.iTask = TASK_WAIT_FOR_MOVEMENT_RANGE;
			tempTask.flData = pTask->flData;
			CBaseMonster::RunTask(&tempTask);
		break;
		}
		case TASK_FRIENDLY_STARE_AT_PLAYER:
		{
			if(this->playerToLookAt == NULL){
				//the schedule this comes in has only this task. This effectively ends this schedule.
				TaskComplete();
				return;
			}

			if(gpGlobals->time >= timeToStare){
				//done starting. Enough time passed.
				TaskComplete();
				return;
			}


			//for now, no line check. just stare at the player through the wall.
			
			MakeIdealYaw ( playerToLookAt->pev->origin );

				
			if ( FacingIdeal() ){
				//TaskComplete();
				//no, just keep looking. Do we need to reset the turning activity?
				if(m_IdealActivity == ACT_TURN_LEFT || m_IdealActivity == ACT_TURN_RIGHT){
					SetActivity(ACT_IDLE);
				}
			}else{
				//let's get closer to looking at the player then.
				ChangeYaw( pev->yaw_speed );

				//NOTE - should be ok to continually (frame-by-frame) call this animation while turning. Setting the ideal activity to the same thing over and over isn't an issue.
				SetTurnActivity(); 
			}

		break;
		}
		default:
			CBaseMonster::RunTask(pTask);
		break;
	}//END OF switch(...)

}//END OF RunTask(...)



//short-ranged swipe.
BOOL CFriendly::CheckMeleeAttack1( float flDot, float flDist ){
	//return CBaseMonster::CheckMeleeAttack1(flDot, flDist);
	if ( flDist <= 80 && flDot >= 0.7 && !HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) ) //&& FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) )
	{
		return TRUE;
	}
	return FALSE;
}

//big double-sweep to bring the enemy closer to me.
BOOL CFriendly::CheckMeleeAttack2( float flDot, float flDist ){
	if ( flDist > 80 && flDist <= 130 && flDot >= 0.7 && !HasConditions( bits_COND_CAN_RANGE_ATTACK1) )
	{
		return TRUE;
	}
	return FALSE;
}
//TODO: require the enemy to be so close to the frinedly for so long (half a second? longer?) before puking? Otherwise this will happen as soon as the player is close enough.
BOOL CFriendly::CheckRangeAttack1( float flDot, float flDist ){

	if(vomitCooldown == -1 || gpGlobals->time >= vomitCooldown ){
		//proceed!
	}else{
		//definitely not ready.
		return FALSE;
	}

	if ( flDist <= 110 && flDot >= 0.74 )
	{
		return TRUE;
	}
	return FALSE;
}
BOOL CFriendly::CheckRangeAttack2( float flDot, float flDist ){
	return FALSE;
}



//TODO - we could also do a check like how breakables do for telling if something is on top or not.
//But the absmin - absmax comparison doesn't look too bad either.  Perhaps the chumtoad could also benefit from that?  eh, options.
void CFriendly::CustomTouch( CBaseEntity *pOther ){
	//easyForcePrintLine("TOUCH REGISTERED WITH %s", ::FClassname(pOther));

	//HOW ABOUT THIS? OTHER: 108.41 71.41 145.41 ME: 0.03 -0.97 73.03
	
	CBaseMonster* tempMonsterOther = pOther->GetMonsterPointer();
	
	//The point of this is, if a monster is on top of me that I hate (relationship-wise), throw it off in a random direction.  No free rides for you.
	if ( pOther->pev->flags & (FL_MONSTER|FL_CLIENT) && tempMonsterOther!=NULL ){

		if(IRelationship(pOther) > R_NO){
			//easyForcePrintLine("HOW ABOUT THIS? OTHER: %.2f %.2f %.2f ME: %.2f %.2f %.2f", pOther->pev->origin.z, pOther->pev->absmin.z, pOther->pev->absmax.z, pev->origin.z, pev->absmin.z, pev->absmax.z);
			if(pOther->pev->absmin.z >= this->pev->absmax.z - 8 &&
				pOther->pev->velocity.Length() < 150 && (tempMonsterOther->fApplyTempVelocity==FALSE || tempMonsterOther->velocityApplyTemp.Length() < 150)
			){
				//get off of me!
				//launch this hostile off.
				float flRandomDirection = RANDOM_FLOAT(0, 2*M_PI);
				float vecX = cos(flRandomDirection) * 160;
				float vecY = sin(flRandomDirection) * 160;
			
				//easyForcePrintLine("GET OFF!!!! %s %.2f %.2f %.2f", FClassname(pOther), flRandomDirection, vecX, vecY);
				
				//UTIL_MoveToOrigin ( ENT(pev), pev->origin + Vector(0, 0, 41), 41, MOVE_STRAFE );
				//pOther->pev->origin.z += 25;
				//UTIL_MoveToOrigin ( ENT(pOther->pev), pOther->pev->origin + Vector(0, 0, 45), 45, MOVE_STRAFE );
				//pOther->pev->velocity = pOther->pev->velocity + Vector(vecX, vecY, 1800);
				//pOther->pev->velocity = Vector(vecX, vecY, 1800);
				
				/*
				if (tempMonsterOther->IsPlayer())
					UTIL_MakeVectors( pev->angles );
				else
					UTIL_MakeAimVectors( pev->angles );
				pOther->pev->velocity = pOther->pev->velocity - gpGlobals->v_forward * 480 + gpGlobals->v_up * 170 + gpGlobals->v_right * 46;
				*/

				tempMonsterOther->fApplyTempVelocity = TRUE;
				tempMonsterOther->velocityApplyTemp = pOther->pev->velocity + Vector(vecX, vecY, 70);

			}//END OF bound check
		}//END Of hate check

	}//END OF monster check


}//END OF CustomTouch(...)





void CFriendly::MonsterThink( void ){
	//easyForcePrintLine("AM I A %d", HasConditions(bits_COND_SEE_ENEMY));

	static int cumulativeThing = 0;

	/*
	//horrorSelected means, has the player picked me to make noise yet?
	if(m_hEnemy && m_hEnemy->IsPlayer() && horrorSelected && (m_hEnemy->pev->origin - pev->origin).Length() < 3000){
		//if our enemy is the player, add us to the player's queue of Mr. Friendlies.
		//CBasePlayer* tempPlayer = static_cast<CBasePlayer*>(CBaseEntity::Instance(m_hEnemy) );
		//...no, too complicated. Just go ahead and play.
	}else{
		horrorSelected = FALSE;
	}



	if(horrorPlayTimePreDelay != -1 && horrorPlayTimePreDelay < gpGlobals->time){
		horrorPlayTimePreDelay = -1;
	}

	if(horrorPlayTimePreDelay == -1 && m_pSchedule == slChaseEnemy){

		if(horrorSelected && horrorPlayTime == -1){
			horrorPlayTime = 0;
		}

		if(horrorSelected && horrorPlayTime != -1 && horrorPlayTime < gpGlobals->time){
			//CHECK: only play if I'm the closest to the player? plus a general distance check too?
			horrorPlayTime = gpGlobals->time + 0.558;
			//ATTN_STATIC ?



			//TODO: make the volume a factor of distance?
			float tempVol = ( ((-(m_hEnemy->pev->origin - this->pev->origin).Length()) / 3000) + 1.1);

			if(tempVol < 0){tempVol = 0;}
			if(tempVol > 1){tempVol = 1;}

			easyForcePrintLine("WHERE ELSE SUCKAH %.2f", tempVol);

			//UTIL_PlaySound( edict(), CHAN_VOICE, "friendly/friendly_horror.wav", 1.0, 1.8, 0, 100, TRUE );
			UTIL_PlaySound( m_hEnemy->edict(), CHAN_STATIC, "friendly/friendly_horror.wav", tempVol, 1.8, 0, 100, TRUE );
		}
	}
	*/

	BOOL okayForNormalThink = (!rapidVomitCheck || gpGlobals->time >= nextNormalThinkTime);

	
	if(pev->deadflag == DEAD_NO ){
		if(
		pev->frame >= (28.1f/47.0f)*255 && 
		pev->frame <= (43.6f/47.0f)*255 &&
		this->m_pSchedule == slRangeAttack1 //only if attacking. Other uses of the anim don't count.
		){
			//during these frames, continually check for shield damage

			if(rapidVomitCheck == FALSE){
				rapidVomitCheck = TRUE;
				rapidVomitCheck_ScheduleFinish = FALSE;
				nextNormalThinkTime = 0.01;  //do think logic this time, as it was scheduled this frame. We weren't in rapidVomitCheck mode before.
			}

			//copy of CheckTraceHullAttack for here.
			//CBaseEntity *pHurt = CheckTraceHullAttack( 60, gSkillData.zombieDmgOneSlash, DMG_SLASH );
		
			int iDamage = 0; //DMG_SLASH DMG_POISON ?;
			float flDist = 120;
			int iDmgType = 0;
			int iDmgTypeMod = 0;

			TraceResult tr;

			if (IsPlayer())
				UTIL_MakeVectors( pev->angles );
			else
				UTIL_MakeAimVectors( pev->angles );

			Vector vecStart = pev->origin;
			vecStart.z += pev->size.z * 0.5;
			Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist );

			UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );

			if ( tr.pHit )
			{
				CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

				//if ( iDamage > 0 )
				{
					//not the usual way.
					//pEntity->TakeDamage( pev, pev, iDamage, iDmgType, iDmgTypeMod );

					//MODDD - draw blood.
					//UTIL_fromToBlood(this, pEntity, (float)iDamage, flDist, &tr.vecEndPos, &vecStart, &vecEnd);
					//nope.

					//instead, just kill some shields.


					if(pEntity != NULL && pEntity->pev->flags & (FL_MONSTER|FL_CLIENT)){
						//What is wrong with this? Who knows.
						
						cumulativeThing++;
						//easyForcePrintLine("TIMES LOOKED: %d", cumulativeThing);
						if(g_iSkillLevel <= SKILL_EASY){
							pEntity->pev->armorvalue = max(pEntity->pev->armorvalue - 3.6, 0);
						}else if(g_iSkillLevel == SKILL_MEDIUM){
							pEntity->pev->armorvalue = max(pEntity->pev->armorvalue - 4.7, 0);
						}else if(g_iSkillLevel >= SKILL_HARD){
							pEntity->pev->armorvalue = max(pEntity->pev->armorvalue - 5.9, 0);
						}

						//attemptAddToShieldSapList();
						if(gpGlobals->time >= nextVomitHitSoundAllowed){
							//delay required so it doesn't spam.
							VomitHitSound( ENT( pEntity->pev ) );
							nextVomitHitSoundAllowed = gpGlobals->time + 0.2;
						}
					}//END OF player check. Should we play another sound for the vomit hitting something else without touching shields?
					else{

						if(gpGlobals->time >= nextVomitHitSoundAllowed){
							//???
						}
					}
				}//no damage requirement, whatever.

			}//END OF if other thing hit

		}//END OF vomit frame check
		else{
			//IMPORTANT - delink any player taking damage.
			//unlinkShieldSapList();
			if(rapidVomitCheck && !rapidVomitCheck_ScheduleFinish){
				rapidVomitCheck_ScheduleFinish = TRUE;
			}
		}

	}//END OF pev->deadflag
	else{
		//dead? Stop the rapid logic.
		if(rapidVomitCheck && !rapidVomitCheck_ScheduleFinish){
			rapidVomitCheck_ScheduleFinish = TRUE;
		}
	}

	if (okayForNormalThink) {
		
		if (pev->sequence == FRIENDLY_WALK && m_Activity == ACT_RUN) {
			// apply change to all framerate variables
			m_flFramerateSuggestion = getRunActFramerate();
			pev->framerate = m_flFramerateSuggestion;
			// Not this one!  That's for deeper model animation logic, like what speed makes sense for the given model to give a framerate of "1"
			//m_flFrameRate = m_flFramerateSuggestion;
		}

		if (extraPissedFactor > 0) {
			if (m_MonsterState == MONSTERSTATE_COMBAT) {
				// I'm peeved!  go down more slowly.
				extraPissedFactor -= 0.2;
			}
			else {
				// No target?  drop off faster.
				extraPissedFactor -= 0.35;
			}
		}

		if(pev->deadflag == DEAD_NO ){

			if(m_pSchedule == slFriendlySeekCorpse || m_pSchedule == slFriendlySeekPlayerCorpse){
				//If our target goes nonexistent or somehow comes back to life (player adrenaline?), stop this schedule.


				if(m_hTargetEnt == NULL ||
					//is not dead? drop it.
					m_hTargetEnt->pev->deadflag < DEAD_DEAD ||
					//is a dead respawnable player that's no longer drawn (gibbed)? Also stop.
					(( (m_hTargetEnt->pev->flags & (FL_CLIENT)) && m_hTargetEnt->pev->deadflag == DEAD_RESPAWNABLE && (m_hTargetEnt->pev->effects & EF_NODRAW)  )) )
				{
					TaskFail();
				}
			}

			if(pev->sequence == FRIENDLY_VOMIT){

				if(
				pev->frame >= (25.3f/47.0f)*255 && 
				pev->frame <= (45.2f/47.0f)*255 &&
				this->m_pSchedule == slRangeAttack1 //only if attacking. Other uses of the anim don't count.
				){
					if(!playedVomitSoundYet){
						playedVomitSoundYet = TRUE;
						VomitSound();
					}

					//::UTIL_MakeVectors(pev->angles);
					Vector vecG;
					Vector vecF;
					Vector vecU;
					UTIL_MakeVectorsPrivate(pev->angles, vecF, vecG, vecU);

					const Vector position = pev->origin + vecF*27 + vecU*25;
					int ballsToSpawn = 1;
					if(RANDOM_LONG(0, 2) == 0){
						ballsToSpawn = 2;
					}
					PLAYBACK_EVENT_FULL (FEV_GLOBAL, NULL, g_sFriendlyVomit, 0.0, (float *)&position, (float *)&vecF, 0.0, 0.0, ballsToSpawn, 0, FALSE, FALSE);

				}//END OF less strict check for the vomit effect


			}//END OF vomit anim check
			else{
				//unlinkShieldSapList();
				if(rapidVomitCheck && !rapidVomitCheck_ScheduleFinish){
					rapidVomitCheck_ScheduleFinish = TRUE;
				}
			}

		}//END OF pev->deadflag check (not dead)
	
		
		CBaseMonster::MonsterThink();

		if(rapidVomitCheck){
			//When were we supposed to have a typical think frame? This avoids doing all the usual AI more often than usual which
			//would just waste resources.
			nextNormalThinkTime = gpGlobals->time + 0.1;
		}
	}//END OF if(okayForNormalThink)


	//easyForcePrintLine("MY SEQUENCE s:%d f:%.2f fl?:%d custoseq:%d", pev->sequence, pev->frame, m_fSequenceLoops, usingCustomSequence);

	//nextnormalthinktime:64.2
	//time: 64.233333
	//nextnormalthinktime:64.33333
	if(rapidVomitCheck_ScheduleFinish){
		rapidVomitCheck = FALSE;
		rapidVomitCheck_ScheduleFinish = FALSE;
		pev->nextthink = nextNormalThinkTime;
		cumulativeThing = 0;
	}

	if(rapidVomitCheck){
		pev->nextthink = gpGlobals->time + 0.025;
	}
	

	// Why would this happen??  Don't report it when we're being deleted (supposed to happen)
	if (pev->nextthink <= 0 && !(pev->flags & FL_KILLME) ) {
		easyForcePrintLine("!!!WARNING!  Friendly had a pev->nextthink at or below 0. This can kill the AI!");
		// save it.
		pev->nextthink = gpGlobals->time + 0.1;
	}


}//END OF MonsterThink


void CFriendly::stopHorrorSound(void){
	//UTIL_StopSound( edict(), CHAN_VOICE, "friendly/friendly_horror.wav", TRUE );
	if(m_hEnemy!=NULL && m_hEnemy->IsPlayer())UTIL_StopSound( m_hEnemy->edict(), CHAN_STATIC, "friendly/friendly_horror.wav", TRUE );
}



int CFriendly::Classify( void ){
	return CLASS_ALIEN_MONSTER;
}
BOOL CFriendly::isOrganic(void){
	return TRUE;
}

int CFriendly::IRelationship( CBaseEntity *pTarget ){
	
	if(!UTIL_IsAliveEntity(pTarget)){
		return R_NO;
	}
	

	BOOL* pHateVarRef = NULL;
	if(pTarget->IsPlayer() ){pHateVarRef = &m_fPissedAtPlayer;}
	else if(pTarget->Classify() == CLASS_PLAYER_ALLY){pHateVarRef = &m_fPissedAtPlayerAlly;}   //NOTE: scientists are CLASS_HUMAN_PASSIVE
	else if(pTarget->Classify() == CLASS_HUMAN_MILITARY){pHateVarRef = &m_fPissedAtHumanMilitary;}   //NOTE: scientists are CLASS_HUMAN_PASSIVE
	//else if()

	//If someone from this faction has ever attacked me, I hate them.
	if(pHateVarRef != NULL && *pHateVarRef==TRUE)
		return R_HT;
		//if ( m_afMemory & bits_MEMORY_PROVOKED )


	return R_NO;  //unless provoked, Mr. Friendly doesn't have a care in the world.
	//return CBaseMonster::IRelationship(pTarget);
}//END OF IRelationship(...)


void CFriendly::ReportAIState(void){
	//call the parent, and add on to that.
	CBaseMonster::ReportAIState();
	//print anything special with easyForcePrintLine
}//END OF ReportAIState()




GENERATE_TRACEATTACK_IMPLEMENTATION(CFriendly)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}


GENERATE_TAKEDAMAGE_IMPLEMENTATION(CFriendly)
{
	BOOL* pHateVarRef = NULL;

	//no NULL check.  Are you daft man?
	if(pevAttacker != NULL){
		CBaseEntity* pEntAttacker = CBaseEntity::Instance(pevAttacker);
		
		if(pEntAttacker != NULL){
			extraPissedFactor += flDamage * 5;
			if (extraPissedFactor > 40) extraPissedFactor = 40;  //cap it.

			if(pEntAttacker->IsPlayer() ){pHateVarRef = &m_fPissedAtPlayer;}
			else if(pEntAttacker->Classify() == CLASS_PLAYER_ALLY){pHateVarRef = &m_fPissedAtPlayerAlly;}   //NOTE: scientists are CLASS_HUMAN_PASSIVE
			else if(pEntAttacker->Classify() == CLASS_HUMAN_MILITARY){pHateVarRef = &m_fPissedAtHumanMilitary;}   //NOTE: scientists are CLASS_HUMAN_PASSIVE
		}
	}


	if(pHateVarRef != NULL){
		if(*pHateVarRef == FALSE){
			//I now hate what attacked me.
			*pHateVarRef = TRUE;

			//MODDD SUGGESTION: make whoever attacked me my enemy if I currently have no enemy?
			if(m_hEnemy == NULL && pevAttacker != NULL){
				CBaseEntity* entTest = CBaseEntity::Instance(pevAttacker);
				if(entTest != NULL){
					m_hEnemy = entTest;
				}
			}

			if(pHateVarRef == &m_fPissedAtPlayer){
				//horror
				//horrorPlayTime = gpGlobals->time; //play now!.. not this way.
				if(m_hEnemy != NULL){
					if(EASY_CVAR_GET_DEBUGONLY(friendlyPianoOtherVolume) > 0){
						UTIL_PlaySound( m_hEnemy->edict(), CHAN_STATIC, "friendly/friendly_horror_start.wav", EASY_CVAR_GET_DEBUGONLY(friendlyPianoOtherVolume), 1.8, 0, 100, TRUE );
					}

					CBaseEntity* entTest = CBaseEntity::Instance(m_hEnemy.Get() );
					if(entTest != NULL && entTest->IsPlayer()){
						// paranoia paranoia
						CBasePlayer* tempPlayer = static_cast<CBasePlayer*>( entTest);
						//horrorPlayTimePreDelay = gpGlobals->time + 0.531;
						tempPlayer->horrorPlayTimePreDelay = gpGlobals->time + 0.531;
					}
				}//END OF enemy null check

				
			}

		}
	}
	//???
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}


GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(CFriendly){
	
	return GENERATE_DEADTAKEDAMAGE_PARENT_CALL(CBaseMonster);
}


GENERATE_KILLED_IMPLEMENTATION(CFriendly){

	if(pev->deadflag == DEAD_NO){

		unlinkShieldSapList();

		horrorSelected = FALSE;
		horrorPlayTime = -1;

		if(m_hEnemy != NULL && m_hEnemy->IsPlayer()){
			
			CBaseEntity* entTest = CBaseEntity::Instance(m_hEnemy.Get() );
			if(entTest != NULL){
				// paranoia paranoia
				CBasePlayer* tempPlayer = static_cast<CBasePlayer*>( entTest);
				
				//if the player's closest mr. friendly was me, disassociate me with the player since I'm about to be deleted.
				if(tempPlayer->closestFriendlyMem == this){
					tempPlayer->horrorPlayTime = -1;
					//stop!
					stopHorrorSound();
					tempPlayer->closestFriendlyMem = NULL;
					tempPlayer->closestFriendlyMemEHANDLE = NULL;
				}

				//easyForcePrintLine("WELL?!! %d : %d", m_hEnemy!=NULL, (m_hEnemy!=NULL)?m_hEnemy->IsPlayer():-1 );
				
				if(EASY_CVAR_GET_DEBUGONLY(friendlyPianoOtherVolume) > 0){
					UTIL_PlaySound( m_hEnemy->edict(), CHAN_STATIC, "friendly/friendly_horror_end.wav", EASY_CVAR_GET_DEBUGONLY(friendlyPianoOtherVolume), 1.8, 0, 100, TRUE );
				}
			}//END OF entTest (player) null check
		}//END OF enemy and isPlayer check

	}


	GENERATE_KILLED_PARENT_CALL(CBaseMonster);
}//END OF Killed


void CFriendly::OnDelete(void){
	unlinkShieldSapList();
	stopHorrorSound();
}


void CFriendly::ForgetEnemy(void) {
	unlinkShieldSapList();
	stopHorrorSound();

	m_fPissedAtPlayer = FALSE;
	m_fPissedAtPlayerAlly = FALSE;
	m_fPissedAtHumanMilitary = FALSE;

	extraPissedFactor = 0;

	CBaseMonster::ForgetEnemy();

}//END OF ForgetEnemy



void CFriendly::SetYawSpeed( void ){
	int ys;

	if(this->m_MonsterState == MONSTERSTATE_COMBAT){
		ys = 210;
	}else if(this->m_MonsterState == MONSTERSTATE_ALERT){
		ys = 150;
	}else{
		ys = 110;
	}
	
	pev->yaw_speed = ys;
	return;
}//END OF SetYawSpeed(...)




BOOL CFriendly::getMonsterBlockIdleAutoUpdate(void){
	return FALSE;
}
BOOL CFriendly::forceIdleFrameReset(void){
	return FALSE;
}
BOOL CFriendly::usesAdvancedAnimSystem(void){
	return TRUE;
}

void CFriendly::SetActivity( Activity NewActivity ){
	CBaseMonster::SetActivity(NewActivity);
}

int CFriendly::LookupActivityHard(int activity){
	int i = 0;
	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	//any animation events in progress?  Clear it.
	resetEventQueue();

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_WALK:
			//just report that we have it.
			//easyForcePrintLine("Friendly: ACT_WALK?");
			this->m_flFramerateSuggestion = 1.87;
			return FRIENDLY_WALK;
		break;
		case ACT_RUN:
			//easyForcePrintLine("Friendly: ACT_RUN?");
			//also report that we have it.

			m_flFramerateSuggestion = getRunActFramerate();

			return FRIENDLY_WALK;
		break;
		case ACT_SMALL_FLINCH:
			//don't give the act. Let the model handle which anim is selected, does not matter. (#1 or #2)
			this->m_flFramerateSuggestion = 2.1;
		break;
		case ACT_MELEE_ATTACK1:
			this->m_flFramerateSuggestion = 1.57;
			if(::g_iSkillLevel == SKILL_HARD) this->m_flFramerateSuggestion *=1.06;
			this->animEventQueuePush(35.9f / 51.0f, 0);
			return FRIENDLY_ATTACK;
		break;
		case ACT_MELEE_ATTACK2:
			this->m_flFramerateSuggestion = 1.34;
			if(::g_iSkillLevel == SKILL_HARD) this->m_flFramerateSuggestion *=1.06;
			this->animEventQueuePush(9.6f / 41.0f, 1);
			this->animEventQueuePush(27.0f / 41.0f, 2);
			return FRIENDLY_DOUBLEWHIP;
		break;
		case ACT_RANGE_ATTACK1:
			this->m_flFramerateSuggestion = 1.09;
			//this->animFrameStartSuggestion = 0;
			//this->animFrameCutoffSuggestion = 47;
			this->animationFPSSuggestion = 30;
			this->animEventQueuePush(27.0f / 47.0f, 3);
			//does this even do anything?
			//this->animationFramesSuggestion = 76;

			this->playedVomitSoundYet = FALSE; //will soon

			return FRIENDLY_VOMIT;
		break;
	}//END OF switch(...)
	
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard(...)


int CFriendly::tryActivitySubstitute(int activity){
	int i = 0;

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_WALK:
			//just report that we have it.
			return FRIENDLY_WALK;
		break;
		case ACT_RUN:
			//also report that we have it.
			return FRIENDLY_WALK;
		break;
		case ACT_SMALL_FLINCH:
			//doesn't matter, just say we have an act at all.
			return FRIENDLY_SMALLFLINCH2;
		break;
		case ACT_MELEE_ATTACK1:
			return FRIENDLY_ATTACK;
		break;
		case ACT_MELEE_ATTACK2:
			return FRIENDLY_DOUBLEWHIP;
		break;
		case ACT_RANGE_ATTACK1:
			return FRIENDLY_VOMIT;
		break;
	}//END OF switch(...)


	//not handled by above? We're not using the script to determine animation then. Rely on the model's anim for this activity if there is one.
	//CBaseAnimating::
	return CBaseAnimating::LookupActivity(activity);
}//END OF tryActivitySubstitute(...)


//NOTICE - "UTIL_MakeVectors" is called in "CheckTraceHullAttack" and seems to work fine there.
//         Anywhere after can be trusted to have the gpGlobals->v_DIRECTION's set correctly.
void CFriendly::HandleEventQueueEvent(int arg_eventID){
	

	switch(arg_eventID){
	case 0:{
		// close range melee
		
		//TODO - custom damage per attacks.
		CBaseEntity *pHurt = CheckTraceHullAttack( 88, gSkillData.zombieDmgBothSlash, DMG_SLASH );
		if ( pHurt )
		{
			if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 7;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 40 + gpGlobals->v_up * 23 - gpGlobals->v_right * 15;
			}
			// Play a random attack hit sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
		}
		else // Play a random attack miss sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

		if (RANDOM_LONG(0,1))
			AttackSound();
	break;
	}
	case 1:{
		//"double whip" (long range) melee, 1st attack
		
		//TAGGG - CRITICAL.   The third coord of that first vector is supposed to be 0, right??
		// Same for the 'CheckTraceHullAttack' further below.
		CBaseEntity *pHurt = CheckTraceHullAttack( Vector(0, 0, 0 ), 137, gSkillData.zombieDmgBothSlash, DMG_SLASH );
		if ( pHurt )
		{
			if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact())
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = -6;
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * 480 + gpGlobals->v_up * 170 + gpGlobals->v_right * 46;
			}
			// Play a random attack hit sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
		}
		else // Play a random attack miss sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

		if (RANDOM_LONG(0,1))
			AttackSound();
	break;
	}
	case 2:{
		//"double whip" 2nd attack
		CBaseEntity *pHurt = CheckTraceHullAttack( Vector(0, 0, 0 ), 120, gSkillData.zombieDmgBothSlash, DMG_SLASH );
		if ( pHurt )
		{
			if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 6;
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * 420 + gpGlobals->v_up * 120 - gpGlobals->v_right * 45;
			}
			// Play a random attack hit sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
		}
		else // Play a random attack miss sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

		if (RANDOM_LONG(0,1))
			AttackSound();

	break;
	}
	case 3:{
		//vomit

		//Drains all player shields more rapidly by upping the think method's rate instead, and doing the usual think logic at its usual interval.
		//Little math to see how many shorter think ticks add up to a typical think tick (0.1 seconds).  That's done in MonsterThink in this class.

		VomitVoiceSound();

	break;
	}
	}//END OF switch(...)


}//END OF HandleEventQueueEvent(...)



void CFriendly::HandleAnimEvent(MonsterEvent_t *pEvent ){
	
	switch( pEvent->event ){
		/*
	case 0:
	{

	break;
	}
	*/
	default:
		CBaseMonster::HandleAnimEvent( pEvent );
	break;
	}//END OF switch(...)
}


// Framerate to use for the running animation.  Bit of logic repeated for the moment run is picked
// and in realtime as "extraPissedFactor" drops from time since taking damage.  (higher = faster).
float CFriendly::getRunActFramerate(void) {
	float targetFramerate;
	if (extraPissedFactor <= 0) {
		targetFramerate = 5.5;
	}
	else {
		// don't let it go too high.
		targetFramerate = min(5.5 + (extraPissedFactor / 10), 8);
	}

	if (::g_iSkillLevel == SKILL_HARD) targetFramerate *= 1.07;

	return targetFramerate;
}




//Version that also checks to see if the player is looking at a corpse. reduces desirability.
CBaseEntity* CFriendly::getNearestDeadBody(void){

	CBaseEntity* pEntityScan = NULL;
	CBaseMonster* testMon = NULL;
	float thisDistance;
	CBaseEntity* bestChoiceYet = NULL;
	float leastDistanceYet = 3000; //furthest I go to a dead body.

	//does UTIL_MonstersInSphere work?
	while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, pev->origin, 800 )) != NULL)
	{
		if(pEntityScan->pev == this->pev){
			//is it me? skip it.
			continue;
		}

		testMon = pEntityScan->MyMonsterPointer();
		//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){

		if(testMon != NULL){
			EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: is corpse ok? classname:%s df:%d, isgi:%d, nl:%d", FClassname(testMon), testMon->pev->deadflag, testMon->isSizeGiant(), FClassnameIs(testMon->pev, "monster_leech") ) );
		}else{
			//EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: is corpse ok? DUCK NO NOT A MONSTER: classname:%s", FClassname(pEntityScan) ) );
		}
		

		//Don't try to eat leeches, too tiny. Nothing else this small leaves a corpse.
		if(testMon != NULL && (testMon->pev->deadflag == DEAD_DEAD || ( (testMon->pev->flags & (FL_CLIENT)) && testMon->pev->deadflag == DEAD_RESPAWNABLE && !(testMon->pev->effects &EF_NODRAW) ) ) && testMon->isSizeGiant() == FALSE && (testMon->isOrganicLogic() ) && !(::FClassnameIs(testMon->pev, "monster_leech") ) ){
			thisDistance = (testMon->pev->origin - pev->origin).Length();
			EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: corpsecheck2: dist? %.2f", thisDistance ));
	
			if(thisDistance < leastDistanceYet){
				//WAIT, one more check. Look nearby for players.



				BOOL playerLooking = FALSE; //must remain false for all players (possibly player friendlys / human military too?) to be a valid, secluded corpse spot.
				CBaseEntity* pPlayerEntityScan = NULL;
				while( (pPlayerEntityScan = UTIL_FindEntityByClassname(pPlayerEntityScan, "player")) != NULL){


					if(pPlayerEntityScan->pev->deadflag != DEAD_NO || entityHidden(pPlayerEntityScan)){
						continue; //doesn't count
					}

					TraceResult trSeeCheck;
					UTIL_TraceLine(pPlayerEntityScan->EyePosition(), testMon->pev->origin + Vector(0, 0, 10), dont_ignore_monsters, dont_ignore_glass, pPlayerEntityScan->edict(), &trSeeCheck);


					CBasePlayer* thePlaya = static_cast<CBasePlayer*>(pPlayerEntityScan);

					
					//if our fraction is not 1.0 (miss? and the thing hit (if anything was hit) is not pEntiyScan (the corpse), we hit something blocking the view.
					//Good.
					
					/*
					thePlaya->debugVect1Success = (trSeeCheck.flFraction >= 1.0 && !trSeeCheck.fStartSolid && !trSeeCheck.fAllSolid);
					thePlaya->debugVect1Draw = TRUE;
					thePlaya->debugVect1Start = pPlayerEntityScan->EyePosition();
					thePlaya->debugVect1End = thePlaya->debugVect1Start + (testMon->pev->origin + Vector(0, 0, 10) - thePlaya->debugVect1Start) * trSeeCheck.flFraction   ;
					*/

					//easyForcePrintLine("ARE YOU daft man %.2f:%s", trSeeCheck.flFraction, FClassname(CBaseEntity::Instance(trSeeCheck.pHit)) );
					
					BOOL tempSucc = FALSE;
					if(trSeeCheck.flFraction != 1.0){
						CBaseEntity* hitTest = CBaseEntity::Instance(trSeeCheck.pHit);
						if(hitTest != NULL){
							tempSucc = (hitTest->pev != pEntityScan->pev);
						}
					}
					
					if(tempSucc ){
						//valid spot.
						EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("YA goodboy %.2f", thisDistance ));
					}else{
						//failure!
						EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("YA baddy %.2f", thisDistance ));

						playerLooking = TRUE;
						break;
					}

				}//END OF player scan check

				//UTIL_TraceLine ( node.m_vecOrigin + vecViewOffset, vecLookersOffset, ignore_monsters, ignore_glass,  ENT(pev), &tr );
				
				//if close enough, forget the player check.
				if(!playerLooking || thisDistance < 200){
					bestChoiceYet = testMon;
					leastDistanceYet = thisDistance;
				}//END OF playerLooking check
	
			}//END OF minimum distance yet
		}//END OF entity scan null check

	}//END OF while loop through all entities in an area to see which are corpses.
	return bestChoiceYet;

}//END OF getNearestDeadBody



void CFriendly::friendly_findCoverFromPlayer( entvars_t* pevPlayerToHideFrom, float flMoveWaitFinishedDelay ){
	
	entvars_t *pevCover;

	float minDistance = 0;

	if ( pevPlayerToHideFrom == NULL )
	{
		// Find cover from self if no player to hide from is available
		pevCover = pev;
		minDistance = 600;    //move at least a fair distance away.
//				TaskFail();
//				return;
	}
	else
		pevCover = pevPlayerToHideFrom;
			
	/*
	if ( FindLateralCover( pevCover->origin, pevCover->view_ofs ) )
	{
		// try lateral first
		m_flMoveWaitFinished = gpGlobals->time + flMoveWaitFinishedDelay;
		TaskComplete();
	}
	else
	
	*/
	if ( FindCover( pevCover->origin, pevCover->view_ofs, minDistance, 1500))//CoverRadius() ) )
	{
		// then try for plain ole cover
		m_flMoveWaitFinished = gpGlobals->time + flMoveWaitFinishedDelay;
		TaskComplete();


		//if that didn't work, try again with lower standards?
	}else if(minDistance!=0 && FindCover( pevCover->origin, pevCover->view_ofs, 400, 1500))//CoverRadius() ) )
	{
		m_flMoveWaitFinished = gpGlobals->time + flMoveWaitFinishedDelay;
		TaskComplete();
	}
	else
	{
		// no coverwhatsoever.	
		RouteClear();
		TaskFail();

	}

}



void CFriendly::attemptAddToShieldSapList(CBaseEntity* argEnt){



}//END OF attemptAddToShieldSapList

void CFriendly::unlinkShieldSapList(void){


}//END OF unlinkShieldSapList


BOOL CFriendly::isProvokable(void){
	return TRUE;
}
BOOL CFriendly::isProvoked(void){
	//return (m_afMemory & bits_MEMORY_PROVOKED);

	//If pissed at the player or allies, he's the Barney's problem.
	return (m_fPissedAtPlayer || m_fPissedAtPlayerAlly);
}



void CFriendly::ScheduleChange(){


	//MODDD - probably unnecessary, but can't hurt. Right?
	if(rapidVomitCheck){
		//Not doing a rapid vomit check anymore then.
		rapidVomitCheck = FALSE;
		rapidVomitCheck_ScheduleFinish = FALSE;
		pev->nextthink = nextNormalThinkTime;
		//cumulativeThing = 0;
	}


	CBaseMonster::ScheduleChange();
	
}//END OF ScheduleChange



int CFriendly::getHullIndexForNodes(void){
    return NODE_LARGE_HULL;  //safe?
}


BOOL CFriendly::needsMovementBoundFix(void){
	return TRUE;
}


