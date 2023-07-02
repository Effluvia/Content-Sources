/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Houndeye - spooky sonic dog.
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "schedule.h"
#include "util_model.h"
#include "nodes.h"
#include "squadmonster.h"
#include "soundent.h"
#include "game.h"


EASY_CVAR_EXTERN_DEBUGONLY(houndeyeAttackMode)
EASY_CVAR_EXTERN_DEBUGONLY(houndeyePrintout)
EASY_CVAR_EXTERN_DEBUGONLY(houndeye_attack_canGib)



// How long after doing a "leaderlook", can I do a leaderlook again?
// Includes the duration of the leaderlook anim since it begins on playing leaderlook.
// Also, still a floaty randon range when after this amount the leaderlook will happen.
#define LEADERLOOK_COOLDOWN_SETTING 14



// houndeye does 20 points of damage spread over a sphere 384 units in diameter, and each additional
// squad member increases the BASE damage by 110%, per the spec.
#define HOUNDEYE_MAX_SQUAD_SIZE			4
#define HOUNDEYE_MAX_ATTACK_RADIUS		384
#define HOUNDEYE_SQUAD_BONUS			(float)1.1

//Changed to use instance var "numberOfEyeSkins" instead.
//#define HOUNDEYE_EYE_FRAMES 4 // how many different switchable maps for the eye

#define HOUNDEYE_SOUND_STARTLE_VOLUME	128 // how loud a sound has to be to badly scare a sleeping houndeye



//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define HOUND_AE_WARN			1
#define HOUND_AE_STARTATTACK	2
#define HOUND_AE_THUMP			3
#define HOUND_AE_ANGERSOUND1	4
#define HOUND_AE_ANGERSOUND2	5
#define HOUND_AE_HOPBACK		6
#define HOUND_AE_CLOSE_EYE		7



extern CGraph WorldGraph;




//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_HOUND_CLOSE_EYE = LAST_COMMON_TASK + 1,
	TASK_HOUND_OPEN_EYE,
	TASK_HOUND_THREAT_DISPLAY,
	TASK_HOUND_FALL_ASLEEP,
	TASK_HOUND_WAKE_UP,
	TASK_HOUND_HOP_BACK,

	//MODDD
	TASK_HOUND_LEADERLOOK
};

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_HOUND_AGITATED = LAST_COMMON_SCHEDULE + 1,
	SCHED_HOUND_HOP_RETREAT,
	SCHED_HOUND_FAIL,
};





class CHoundeye : public CSquadMonster
{
public:
	CUSTOM_SCHEDULES;

	BOOL firstSpecialAttackFrame;
	BOOL canResetSound;

	int m_iSpriteTexture;
	BOOL m_fAsleep;// some houndeyes sleep in idle mode if this is set, the houndeye is lying down
	BOOL m_fDontBlink;// don't try to open/close eye if this bit is set!
	Vector	m_vecPackCenter; // the center of the pack. The leader maintains this by averaging the origins of all pack members.

	static int numberOfEyeSkins;
	float leaderLookCooldown;

	CHoundeye(void);

	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	//MODDD
	void setModel(void);
	void setModel(const char* m);
	void Activate();

	void Spawn( void );
	void Precache( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void SetYawSpeed ( void );
	void WarmUpSound ( void );
	void AlertSound( void );
	void DeathSound( void );
	void WarnSound( void );
	void PainSound( void );
	void IdleSound( void );
	void StartTask( Task_t *pTask );
	void RunTask ( Task_t *pTask );

	
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE
	GENERATE_KILLED_PROTOTYPE
	void BecomeDead(void);
	GENERATE_DEADTAKEDAMAGE_PROTOTYPE
	void DeathAnimationStart(void);
	void DeathAnimationEnd(void);
	void StartReanimation(void);

	//MODDD - new
	void SonicAttack( void );
	void SonicAttack( BOOL useAlt);

	void PrescheduleThink( void );
	void SetActivity ( Activity NewActivity );

	//MODDD - new
	void WriteBeamColor ( void );
	void WriteBeamColor( BOOL useAlt);
	void MonsterThink(void);
	void ChangeSchedule ( Schedule_t *pNewSchedule );


	BOOL CheckRangeAttack1 ( float flDot, float flDist );


	void SetObjectCollisionBox( void )
	{
		if(pev->deadflag != DEAD_NO){
			pev->absmin = pev->origin + Vector(-56, -56, 0);
			pev->absmax = pev->origin + Vector(56, 56, 36);
		}else{
			CBaseMonster::SetObjectCollisionBox();
		}
	}

	BOOL FValidateHintType ( short sHint );
	BOOL FCanActiveIdle ( void );
	Schedule_t *GetScheduleOfType ( int Type );
	Schedule_t *CHoundeye::GetSchedule( void );


	//MODDD - new. Enter the 21st century!
	BOOL getMonsterBlockIdleAutoUpdate(void);
	BOOL forceIdleFrameReset(void);
	BOOL canPredictActRepeat(void);
	BOOL usesAdvancedAnimSystem(void);

	int tryActivitySubstitute(int activity);
	int LookupActivityHard(int activity);

};


//???
int CHoundeye::numberOfEyeSkins = -1;




#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_houndeye, CHoundeye );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( houndeye, CHoundeye );

	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( hound, CHoundeye );
		LINK_ENTITY_TO_CLASS( monster_hound, CHoundeye );
	#endif
#endif

TYPEDESCRIPTION	CHoundeye::m_SaveData[] =
{
	DEFINE_FIELD( CHoundeye, m_iSpriteTexture, FIELD_INTEGER ),
	DEFINE_FIELD( CHoundeye, m_fAsleep, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHoundeye, m_fDontBlink, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHoundeye, m_vecPackCenter, FIELD_POSITION_VECTOR ),
};

IMPLEMENT_SAVERESTORE( CHoundeye, CSquadMonster );









//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//MODDD - new
Task_t	tlLeaderLook1[] =
{
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,		(float)ACT_LEADERLOOK },
	{ TASK_HOUND_LEADERLOOK,		0 },
	// why a WAIT after leaderlook?  This is just time we can't turn to listen to a new sound.
	//{ TASK_WAIT,				(float)5		},// repick IDLESTAND every five seconds. gives us a chance to pick an active idle, fidget, etc.
};

Schedule_t	slHoundLeaderLook[] =
{
	{
		tlLeaderLook1,
		ARRAYSIZE ( tlLeaderLook1 ),
		//NOTE: conditions altered.  See "Schedule_t	slIdleStand[]" in "defaultai.cpp" for the original.

		//MODDD - Why on Earth do most houndeye schedules lack this??
		bits_COND_SEE_ENEMY |
		// BOB SAGET'S LEFT NUT, WHY WAS THIS MISSING?!
		bits_COND_NEW_ENEMY |

		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_PROVOKED,
		// likely already heard a sound to do the leaderlook, don't get interrupted in the middle of a leaderlook to do "leaderlook".
		// bits_COND_HEAR_SOUND,

		// sounds probably don't mean anything with bits_COND_HEAR_SOUND commented out.
		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER/*		|

		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE
		*/
		,

		"LeaderLook"  //???
	},
};

/////////////////////////////////////////////////////////////////




Task_t	tlHoundGuardPack[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_GUARD,				(float)0		},
};

Schedule_t	slHoundGuardPack[] =
{
	{
		tlHoundGuardPack,
		ARRAYSIZE ( tlHoundGuardPack ),
		bits_COND_SEE_HATE		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_PROVOKED		|
		bits_COND_HEAR_SOUND,

		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_MEAT			|
		bits_SOUND_PLAYER,
		"GuardPack"
	},
};

// primary range attack
Task_t	tlHoundYell1[] =
{
	{ TASK_STOP_MOVING,			(float)0					},
	{ TASK_FACE_IDEAL,			(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_SET_SCHEDULE,		(float)SCHED_HOUND_AGITATED	},
};

Task_t	tlHoundYell2[] =
{
	{ TASK_STOP_MOVING,			(float)0					},
	{ TASK_FACE_IDEAL,			(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
};

Schedule_t	slHoundRangeAttack[] =
{
	{
		tlHoundYell1,
		ARRAYSIZE ( tlHoundYell1 ),
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"HoundRangeAttack1"
	},
	{
		tlHoundYell2,
		ARRAYSIZE ( tlHoundYell2 ),
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"HoundRangeAttack2"
	},
};

// lie down and fall asleep
Task_t	tlHoundSleep[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE			},
	{ TASK_WAIT_RANDOM,			(float)5				},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_CROUCH		},
	{ TASK_SET_ACTIVITY,		(float)ACT_CROUCHIDLE	},
	{ TASK_HOUND_FALL_ASLEEP,	(float)0				},
	{ TASK_WAIT_RANDOM,			(float)25				},
	{ TASK_HOUND_CLOSE_EYE,		(float)0				},
	//{ TASK_WAIT,				(float)10				},
	//{ TASK_WAIT_RANDOM,			(float)10				},
};

Schedule_t	slHoundSleep[] =
{
	{
		tlHoundSleep,
		ARRAYSIZE ( tlHoundSleep ),
		bits_COND_HEAR_SOUND	|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_NEW_ENEMY,

		bits_SOUND_COMBAT		|
		bits_SOUND_PLAYER		|
		bits_SOUND_WORLD,
		"Hound Sleep"
	},
};

// wake and stand up lazily
Task_t	tlHoundWakeLazy[] =
{
	{ TASK_STOP_MOVING,			(float)0			},
	{ TASK_HOUND_OPEN_EYE,		(float)0			},
	{ TASK_WAIT_RANDOM,			(float)2.5			},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_STAND	},
	{ TASK_HOUND_WAKE_UP,		(float)0			},
};

Schedule_t	slHoundWakeLazy[] =
{
	{
		tlHoundWakeLazy,
		ARRAYSIZE ( tlHoundWakeLazy ),
		0,
		0,
		"WakeLazy"
	},
};

// wake and stand up with great urgency!
Task_t	tlHoundWakeUrgent[] =
{
	{ TASK_HOUND_OPEN_EYE,		(float)0			},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_HOP		},
	{ TASK_FACE_IDEAL,			(float)0			},
	{ TASK_HOUND_WAKE_UP,		(float)0			},
};

Schedule_t	slHoundWakeUrgent[] =
{
	{
		tlHoundWakeUrgent,
		ARRAYSIZE ( tlHoundWakeUrgent ),
		0,
		0,
		"WakeUrgent"
	},
};


Task_t	tlHoundSpecialAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_SPECIAL_ATTACK1,		(float)0		},
	//MODDD - removed?? - NEVERMIND, it is okay.
	{ TASK_PLAY_SEQUENCE,		(float)ACT_IDLE_ANGRY },
};

Schedule_t	slHoundSpecialAttack1[] =
{
	{
		tlHoundSpecialAttack1,
		ARRAYSIZE ( tlHoundSpecialAttack1 ),
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,
		//MODDD - removed...  had no effect before at some point anyways?
		//bits_COND_ENEMY_OCCLUDED,
		
		0,
		"Hound Special Attack1"
	},
};

Task_t	tlHoundAgitated[] =
{
	{ TASK_STOP_MOVING,				0		},
	{ TASK_HOUND_THREAT_DISPLAY,	0		},
};

Schedule_t	slHoundAgitated[] =
{
	{
		tlHoundAgitated,
		ARRAYSIZE ( tlHoundAgitated ),
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,
		0,
		"Hound Agitated"
	},
};

Task_t	tlHoundHopRetreat[] =
{
	{ TASK_STOP_MOVING,				0											},
	{ TASK_HOUND_HOP_BACK,			0											},
	{ TASK_SET_SCHEDULE,			(float)SCHED_TAKE_COVER_FROM_ENEMY	},
};

Schedule_t	slHoundHopRetreat[] =
{
	{
		tlHoundHopRetreat,
		ARRAYSIZE ( tlHoundHopRetreat ),
		0,
		0,
		"Hound Hop Retreat"
	},
};

// hound fails in combat with client in the PVS
Task_t	tlHoundCombatFailPVS[] =
{
	{ TASK_STOP_MOVING,				0			},
	{ TASK_HOUND_THREAT_DISPLAY,	0			},
	{ TASK_WAIT_FACE_ENEMY,			(float)1	},
};

Schedule_t	slHoundCombatFailPVS[] =
{
	{
		tlHoundCombatFailPVS,
		ARRAYSIZE ( tlHoundCombatFailPVS ),
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,
		0,
		"HoundCombatFailPVS"
	},
};

// hound fails in combat with no client in the PVS. Don't keep peeping!
Task_t	tlHoundCombatFailNoPVS[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_HOUND_THREAT_DISPLAY,	0				},
	{ TASK_WAIT_FACE_ENEMY,			(float)2		},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	{ TASK_WAIT_PVS,				0				},
};

Schedule_t	slHoundCombatFailNoPVS[] =
{
	{
		tlHoundCombatFailNoPVS,
		ARRAYSIZE ( tlHoundCombatFailNoPVS ),
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,
		0,
		"HoundCombatFailNoPVS"
	},
};

DEFINE_CUSTOM_SCHEDULES( CHoundeye )
{
	slHoundGuardPack,
	slHoundRangeAttack,
	&slHoundRangeAttack[ 1 ],
	slHoundSleep,
	slHoundWakeLazy,
	slHoundWakeUrgent,
	slHoundSpecialAttack1,
	slHoundAgitated,
	slHoundHopRetreat,
	slHoundCombatFailPVS,
	slHoundCombatFailNoPVS,

	//MODDD - new.
	slHoundLeaderLook,

};

IMPLEMENT_CUSTOM_SCHEDULES( CHoundeye, CSquadMonster );




//=========================================================
// Classify - indicates this monster's place in the
// relationship table.
//=========================================================
int CHoundeye::Classify ( void )
{
	return	CLASS_ALIEN_MONSTER;
}


//=========================================================
//  FValidateHintType
//=========================================================
BOOL CHoundeye::FValidateHintType ( short sHint )
{
	int i;

	static short sHoundHints[] =
	{
		HINT_WORLD_MACHINERY,
		HINT_WORLD_BLINKING_LIGHT,
		HINT_WORLD_HUMAN_BLOOD,
		HINT_WORLD_ALIEN_BLOOD,
	};

	for ( i = 0 ; i < ARRAYSIZE ( sHoundHints ) ; i++ )
	{
		if ( sHoundHints[ i ] == sHint )
		{
			return TRUE;
		}
	}

	ALERT ( at_aiconsole, "Couldn't validate hint type" );
	return FALSE;
}


//=========================================================
// FCanActiveIdle
//=========================================================
BOOL CHoundeye::FCanActiveIdle ( void )
{
	if ( InSquad() )
	{
		CSquadMonster *pSquadLeader = MySquadLeader();

		for (int i = 0; i < MAX_SQUAD_MEMBERS;i++)
		{
			CSquadMonster *pMember = pSquadLeader->MySquadMember(i);

			if ( pMember != NULL && pMember != this && pMember->m_iHintNode != NO_NODE )
			{
				// someone else in the group is active idling right now!
				return FALSE;
			}
		}

		return TRUE;
	}

	return TRUE;
}


//=========================================================
// CheckRangeAttack1 - overridden for houndeyes so that they
// try to get within half of their max attack radius before
// attacking, so as to increase their chances of doing damage.
//=========================================================
BOOL CHoundeye::CheckRangeAttack1 ( float flDot, float flDist )
{
	if ( flDist <= ( HOUNDEYE_MAX_ATTACK_RADIUS * 0.5 ) && flDot >= 0.3 )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CHoundeye::SetYawSpeed ( void )
{
	int ys;

	ys = 90;

	switch ( m_Activity )
	{
	case ACT_CROUCHIDLE://sleeping!
		ys = 0;
		break;
	case ACT_IDLE:
		ys = 60;
		break;
	case ACT_WALK:
		ys = 90;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
	
	if(this->crazyPrintout){
		easyForcePrintLine("YAWSPEED: %.2f  FROM ACT: %d", ys, m_Activity);
	}
}

//=========================================================
// SetActivity
//=========================================================
void CHoundeye::SetActivity ( Activity NewActivity )
{
	int iSequence;

	if ( NewActivity == m_Activity )
		return;

	if ( m_MonsterState == MONSTERSTATE_COMBAT && NewActivity == ACT_IDLE && RANDOM_LONG(0,1) )
	{
		// play pissed idle.


		if(canSetAnim == TRUE){
			iSequence = LookupSequence( "madidle" );
		}

		m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

		// In case someone calls this with something other than the ideal activity
		m_IdealActivity = m_Activity;

		// Set to the desired anim, or default anim if the desired is not present
		if (canSetAnim == TRUE && iSequence > ACTIVITY_NOT_AVAILABLE )
		{
			pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
			pev->frame			= 0;		// FIX: frame counter shouldn't be reset when its the same activity as before
			ResetSequenceInfo();
			SetYawSpeed();
		}

	}
	else
	{

		if(NewActivity == ACT_SPECIAL_ATTACK1 && EASY_CVAR_GET_DEBUGONLY(houndeyeAttackMode) == 1){
			//if this is 1, we want the retail-charge animation to apply instead.  This is why.
			m_Activity = NewActivity;

			/*
			iSequence = LookupSequence( "attack" );
			canSetAnim = TRUE;
			if (canSetAnim == TRUE && iSequence > ACTIVITY_NOT_AVAILABLE )
			{
				pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
				pev->frame			= 0;		// FIX: frame counter shouldn't be reset when its the same activity as before
				ResetSequenceInfo();
				SetYawSpeed();
			}
			*/
			setAnimation("attack", TRUE);


			//MODDD IMPORTANT:  THIS USED TO BE canSetAnim == FALSE, which did nothing.
			//In case that was SOMEHOW better, not doing anything, comment out this line!
			canSetAnim = FALSE;
			//CSquadMonster::SetActivity ( NewActivity );


		}else{
			//MODDD - this was alone here before.  The rest in the greater "else" block is new.
			CSquadMonster::SetActivity ( NewActivity );
		}


	}
	canSetAnim = TRUE;  //can next time?
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CHoundeye::HandleAnimEvent( MonsterEvent_t *pEvent )
{

	switch ( pEvent->event )
	{
		case HOUND_AE_WARN:
			// do stuff for this event.
			WarnSound();
			break;

		case HOUND_AE_STARTATTACK:
			WarmUpSound();
			break;

		case HOUND_AE_HOPBACK:
			{
				float flGravity = g_psv_gravity->value;

				pev->flags &= ~FL_ONGROUND;

				pev->velocity = gpGlobals->v_forward * -200;
				pev->velocity.z += (0.6 * flGravity) * 0.5;

				break;
			}

		case HOUND_AE_THUMP:
			// emit the shockwaves
			//!!!
			if(EASY_CVAR_GET_DEBUGONLY(houndeyeAttackMode) == 0){
				//why?  if tempp == 1, we ONLY play the shock in a different place.
				//NOTE: at 2, this never even seems to happen.  Ah well, no loss.
				SonicAttack();
			}
			break;

		case HOUND_AE_ANGERSOUND1:
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "houndeye/he_pain3.wav", 1, ATTN_NORM);
			break;

		case HOUND_AE_ANGERSOUND2:
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "houndeye/he_pain1.wav", 1, ATTN_NORM);
			break;

		case HOUND_AE_CLOSE_EYE:
			if ( !m_fDontBlink )
			{
				//MODDD - frame #3 shows the hound eye open now?
				pev->skin = max(numberOfEyeSkins - 1, 0);
				//pev->skin = 2;

			}
			break;

		default:
			CSquadMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//MODDD
CHoundeye::CHoundeye(void){

	firstSpecialAttackFrame = TRUE;
	canResetSound = TRUE;
	leaderLookCooldown = -1;

}


void CHoundeye::setModel(void){
	CHoundeye::setModel(NULL);
}
void CHoundeye::setModel(const char* m){
	CBaseMonster::setModel(m);

	//After setting the model, can count the number of skins.
	if(numberOfEyeSkins == -1){
		//never loaded numberOfEyeSkins? Do so.
		numberOfEyeSkins = getNumberOfSkins();
		//easyForcePrintLine("HOUNDEYE RAW SKIN COUNT A: %d", getNumberOfSkins());
		//EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "HOUND: SKINCOUNTPOST1: %d %d", numberOfEyeSkins, getnumberOfEyeSkins( ) );
		//EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "HOUND: BODYCOUNTPOST1: %d", this->getNumberOfBodyParts( ) );
		
		if(numberOfEyeSkins == 0){
			EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "WARNING: Houndeye skin count is 0, error! Check houndeye.mdl for multiple skins. If it has them, please report this.  Forcing default of 4..."));
			numberOfEyeSkins = 4;
		}else if(numberOfEyeSkins != 4){
			EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "WARNING: Houndeye skin count is %d, not 4. If houndeye.mdl does have 4 skins, please report this.", numberOfEyeSkins));
			if(numberOfEyeSkins < 1) numberOfEyeSkins = 1; //safety.
		}
	}

}//END OF setModel


//MODDD - doesn't "restore" make more sense?
// this is ok. But see if the scientist should change.
void CHoundeye::Activate(){
	
	//easyForcePrintLine("HOUNDEYE RAW SKIN COUNT B: %d", getNumberOfSkins());
	CSquadMonster::Activate();
}

//=========================================================
// Spawn
//=========================================================
void CHoundeye::Spawn()
{
	Precache( );
	
	easyForcePrintLine("HOUNDEYE RAW SKIN COUNT B: %d", getNumberOfSkins());

	setModel("models/houndeye.mdl");
	UTIL_SetSize(pev, Vector ( -16, -16, 0 ), Vector ( 16, 16, 36 ) );

	pev->classname = MAKE_STRING("monster_houndeye");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_YELLOW;
	pev->effects		= 0;
	pev->health			= gSkillData.houndeyeHealth;
	pev->yaw_speed		= 5;//!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_fAsleep			= FALSE; // everyone spawns awake
	m_fDontBlink		= FALSE;
	m_afCapability		|= bits_CAP_SQUAD;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================


extern int global_useSentenceSave;

void CHoundeye::Precache()
{

	PRECACHE_MODEL("models/houndeye.mdl");

	global_useSentenceSave = TRUE;

	PRECACHE_SOUND("houndeye/he_alert1.wav");
	PRECACHE_SOUND("houndeye/he_alert2.wav");
	PRECACHE_SOUND("houndeye/he_alert3.wav");

	PRECACHE_SOUND("houndeye/he_die1.wav");
	PRECACHE_SOUND("houndeye/he_die2.wav");
	PRECACHE_SOUND("houndeye/he_die3.wav");

	PRECACHE_SOUND("houndeye/he_idle1.wav");
	PRECACHE_SOUND("houndeye/he_idle2.wav");
	PRECACHE_SOUND("houndeye/he_idle3.wav");



	PRECACHE_SOUND("houndeye/he_hunt1.wav");
	PRECACHE_SOUND("houndeye/he_hunt2.wav");
	PRECACHE_SOUND("houndeye/he_hunt3.wav");

	//MODDD - this is an addition.
	PRECACHE_SOUND("houndeye/he_idle4.wav");

	//MODDD - this is an addition.  Meant to do this, right?
	PRECACHE_SOUND("houndeye/he_hunt4.wav");


	PRECACHE_SOUND("houndeye/he_pain1.wav");

	//MODDD - this is an addition.
	PRECACHE_SOUND("houndeye/he_pain2.wav");

	PRECACHE_SOUND("houndeye/he_pain3.wav");
	PRECACHE_SOUND("houndeye/he_pain4.wav");
	PRECACHE_SOUND("houndeye/he_pain5.wav");

	PRECACHE_SOUND("houndeye/he_attack1.wav");

	//MODDD - this is an addition.
	PRECACHE_SOUND("houndeye/he_attack2.wav");

	PRECACHE_SOUND("houndeye/he_attack3.wav");

	PRECACHE_SOUND("houndeye/he_blast1.wav");
	PRECACHE_SOUND("houndeye/he_blast2.wav");
	PRECACHE_SOUND("houndeye/he_blast3.wav");

	global_useSentenceSave = FALSE;

	m_iSpriteTexture = PRECACHE_MODEL( "sprites/shockwave.spr" );




}

//=========================================================
// IdleSound
//=========================================================
void CHoundeye::IdleSound ( void )
{
	//MODDD - now uses he_idle4.
	switch ( RANDOM_LONG(0,3) )
	{
	case 0:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_idle1.wav", 1, ATTN_NORM );
		break;
	case 1:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_idle2.wav", 1, ATTN_NORM );
		break;
	case 2:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_idle3.wav", 1, ATTN_NORM );
		break;
	case 3:
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "houndeye/he_idle4.wav", 1, ATTN_NORM);
	}
}

//=========================================================
// IdleSound
//=========================================================
void CHoundeye::WarmUpSound ( void )
{
	switch ( RANDOM_LONG(0,2) )
	{
	case 0:
		UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "houndeye/he_attack1.wav", 0.7, ATTN_NORM );
		break;
	case 1:
		UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "houndeye/he_attack2.wav", 0.7, ATTN_NORM );
		break;
	case 2:
		UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "houndeye/he_attack3.wav", 0.7, ATTN_NORM );
		break;
	}
}

//=========================================================
// WarnSound
//=========================================================
void CHoundeye::WarnSound ( void )
{
	switch ( RANDOM_LONG(0,3) )
	{
	case 0:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_hunt1.wav", 1, ATTN_NORM );
		break;
	case 1:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_hunt2.wav", 1, ATTN_NORM );
		break;
	case 2:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_hunt3.wav", 1, ATTN_NORM );
		break;
	case 3:
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "houndeye/he_hunt4.wav", 1, ATTN_NORM );
		break;
	}
}

//=========================================================
// AlertSound
//=========================================================
void CHoundeye::AlertSound ( void )
{

	if ( InSquad() && !IsLeader() )
	{
		return; // only leader makes ALERT sound.
	}

	switch ( RANDOM_LONG(0,2) )
	{
	case 0:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_alert1.wav", 1, ATTN_NORM );
		break;
	case 1:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_alert2.wav", 1, ATTN_NORM );
		break;
	case 2:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_alert3.wav", 1, ATTN_NORM );
		break;
	}
}

//=========================================================
// DeathSound
//=========================================================
void CHoundeye::DeathSound ( void )
{
	switch ( RANDOM_LONG(0,2) )
	{
	case 0:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_die1.wav", 1, ATTN_NORM );
		break;
	case 1:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_die2.wav", 1, ATTN_NORM );
		break;
	case 2:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_die3.wav", 1, ATTN_NORM );
		break;
	}
}

//=========================================================
// PainSound
//=========================================================
void CHoundeye::PainSound ( void )
{
	switch ( RANDOM_LONG(0,4) )
	{
	case 0:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_pain3.wav", 1, ATTN_NORM );
		break;
	case 1:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_pain4.wav", 1, ATTN_NORM );
		break;
	case 2:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "houndeye/he_pain5.wav", 1, ATTN_NORM );
		break;
	case 3:
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "houndeye/he_pain1.wav", 1, ATTN_NORM );
		break;
	case 4:
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "houndeye/he_pain2.wav", 1, ATTN_NORM );
		break;
	}
}


//MODDD - new version.
void CHoundeye::WriteBeamColor ( ){

	WriteBeamColor(FALSE);

}

//=========================================================
// WriteBeamColor - writes a color vector to the network
// based on the size of the group.
//=========================================================
void CHoundeye::WriteBeamColor ( BOOL useAltcolor )
{
	BYTE	bRed, bGreen, bBlue;


	if(!useAltcolor){

		if ( InSquad() )
		{
			switch ( SquadCount() )
			{
			case 2:
				// no case for 0 or 1, cause those are impossible for monsters in Squads.
				bRed	= 101;
				bGreen	= 133;
				bBlue	= 221;
				break;
			case 3:
				bRed	= 67;
				bGreen	= 85;
				bBlue	= 255;
				break;
			case 4:
				bRed	= 62;
				bGreen	= 33;
				bBlue	= 211;
				break;
			default:
				ALERT ( at_aiconsole, "Unsupported Houndeye SquadSize!\n" );
				bRed	= 188;
				bGreen	= 220;
				bBlue	= 255;
				break;
			}
		}
		else
		{
			// solo houndeye - weakest beam
			bRed	= 188;
			bGreen	= 220;
			bBlue	= 255;
		}

	}//END OF if(!useAltcolor)
	else{
		bRed	= 248;
		bGreen	= 248;
		bBlue	= 42;


	}


	WRITE_BYTE( bRed   );
	WRITE_BYTE( bGreen );
	WRITE_BYTE( bBlue  );
}


//MODDD - new version below.  The parameterless (default) now just sends "FALSE".
void CHoundeye::SonicAttack(){

	SonicAttack(FALSE);
}

//=========================================================
// SonicAttack
//=========================================================
//MODDD - now accepts a bool.
void CHoundeye::SonicAttack (BOOL useAlt )
{
	float	flAdjustedDamage;
	float	flDist;

	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "houndeye/he_blast1.wav", 1, ATTN_NORM);break;
	case 1:	UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "houndeye/he_blast2.wav", 1, ATTN_NORM);break;
	case 2:	UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "houndeye/he_blast3.wav", 1, ATTN_NORM);break;
	}

	// blast circles
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_BEAMCYLINDER );
		WRITE_COORD( pev->origin.x);
		WRITE_COORD( pev->origin.y);
		WRITE_COORD( pev->origin.z + 16);
		WRITE_COORD( pev->origin.x);
		WRITE_COORD( pev->origin.y);
		WRITE_COORD( pev->origin.z + 16 + HOUNDEYE_MAX_ATTACK_RADIUS / .2); // reach damage radius over .3 seconds
		WRITE_SHORT( m_iSpriteTexture );
		WRITE_BYTE( 0 ); // startframe
		WRITE_BYTE( 0 ); // framerate
		WRITE_BYTE( 2 ); // life
		WRITE_BYTE( 16 );  // width
		WRITE_BYTE( 0 );   // noise

		WriteBeamColor(useAlt);

		WRITE_BYTE( 255 ); //brightness
		WRITE_BYTE( 0 );		// speed
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_BEAMCYLINDER );
		WRITE_COORD( pev->origin.x);
		WRITE_COORD( pev->origin.y);
		WRITE_COORD( pev->origin.z + 16);
		WRITE_COORD( pev->origin.x);
		WRITE_COORD( pev->origin.y);
		WRITE_COORD( pev->origin.z + 16 + ( HOUNDEYE_MAX_ATTACK_RADIUS / 2 ) / .2); // reach damage radius over .3 seconds
		WRITE_SHORT( m_iSpriteTexture );
		WRITE_BYTE( 0 ); // startframe
		WRITE_BYTE( 0 ); // framerate
		WRITE_BYTE( 2 ); // life
		WRITE_BYTE( 16 );  // width
		WRITE_BYTE( 0 );   // noise

		WriteBeamColor(useAlt);

		WRITE_BYTE( 255 ); //brightness
		WRITE_BYTE( 0 );		// speed
	MESSAGE_END();


	CBaseEntity *pEntity = NULL;
	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, HOUNDEYE_MAX_ATTACK_RADIUS )) != NULL)
	{
		if ( pEntity->pev->takedamage != DAMAGE_NO )
		{
			if ( !FClassnameIs(pEntity->pev, "monster_houndeye") )
			{// houndeyes don't hurt other houndeyes with their attack

				//EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "What did I hurt? %s", STRING(pEntity->pev->classname)));

				// houndeyes do FULL damage if the ent in question is visible. Half damage otherwise.
				// This means that you must get out of the houndeye's attack range entirely to avoid damage.
				// Calculate full damage first

				if ( SquadCount() > 1 )
				{
					// squad gets attack bonus.
					flAdjustedDamage = gSkillData.houndeyeDmgBlast + gSkillData.houndeyeDmgBlast * ( HOUNDEYE_SQUAD_BONUS * ( SquadCount() - 1 ) );
				}
				else
				{
					// solo
					flAdjustedDamage = gSkillData.houndeyeDmgBlast;
				}

				flDist = (pEntity->Center() - pev->origin).Length();

				flAdjustedDamage -= ( flDist / HOUNDEYE_MAX_ATTACK_RADIUS ) * flAdjustedDamage;

				if ( !FVisible( pEntity ) )
				{
					if ( pEntity->IsPlayer() )
					{
						// if this entity is a client, and is not in full view, inflict half damage. We do this so that players still
						// take the residual damage if they don't totally leave the houndeye's effective radius. We restrict it to clients
						// so that monsters in other parts of the level don't take the damage and get pissed.
						flAdjustedDamage *= 0.5;
					}
					else if ( !FClassnameIs( pEntity->pev, "func_breakable" ) && !FClassnameIs( pEntity->pev, "func_pushable" ) )
					{
						// do not hurt nonclients through walls, but allow damage to be done to breakables
						flAdjustedDamage = 0;
					}
				}

				//ALERT ( at_aiconsole, "Damage: %f\n", flAdjustedDamage );

				if (flAdjustedDamage > 0 )
				{
					int damageType = DMG_SONIC;
					if(EASY_CVAR_GET_DEBUGONLY(houndeye_attack_canGib)){
						damageType |= DMG_ALWAYSGIB;
					}

					pEntity->TakeDamage ( pev, pev, flAdjustedDamage, damageType, DMG_HITBOX_EQUAL );
				}
			}
		}
	}
}

//=========================================================
// start task
//=========================================================
void CHoundeye::StartTask ( Task_t *pTask )
{
	
	switch ( pTask->iTask )
	{
	//MODDD - from schedule.cpp
	case TASK_FACE_IDEAL:
	{
		
		if(this->crazyPrintout){
			/////easyForcePrintLine("START TASK:  FACE IDEAL.");
		}
		SetTurnActivity();

		if(pev->yaw_speed == 0){
			/////easyForcePrintLine("HOUNDEYE ISSUE: WHAT YOU DOIN WILLIS");
			pev->yaw_speed = 90;
		}

		break;
	}
	case TASK_HOUND_FALL_ASLEEP:
		{
			m_fAsleep = TRUE; // signal that hound is lying down (must stand again before doing anything else!)
			TaskComplete();
			break;
		}
	case TASK_HOUND_WAKE_UP:
		{
			m_fAsleep = FALSE; // signal that hound is standing again
			TaskComplete();
			break;
		}
	case TASK_HOUND_OPEN_EYE:
		{
			m_fDontBlink = FALSE; // turn blinking back on and that code will automatically open the eye
			TaskComplete();
			break;
		}
	case TASK_HOUND_CLOSE_EYE:
		{
			//MODDD - no, pev->skin 2 is closed.
			//pev->skin = 0;
			//HOUNDEYE_EYE_FRAMES - 1 is fine.
			pev->skin = max(numberOfEyeSkins - 1, 0);

			m_fDontBlink = TRUE; // tell blink code to leave the eye alone.
			break;
		}
	case TASK_HOUND_THREAT_DISPLAY:
		{
			m_IdealActivity = ACT_IDLE_ANGRY;
			break;
		}
	case TASK_HOUND_HOP_BACK:
		{
			m_IdealActivity = ACT_LEAP;
			break;
		}
	case TASK_RANGE_ATTACK1:
		{
			m_IdealActivity = ACT_RANGE_ATTACK1;

			//this is exactly what the base monster (in schedule.cpp) says to do, we're fine.
			//CSquadMonster::StartTask(pTask);

			//MODDD - this used to be commented out!
			if ( InSquad() )
			{
				//MODDD - lots of edits

				//NOTICE: attempted improvement.  Still hard to understand what "bits_SLOT_HOUND_BATTERY" is referring to.
				/*
				CSquadMonster *pSquadLeader = MySquadLeader();

				for (int i = 0; i < MAX_SQUAD_MEMBERS;i++)
				{
					CSquadMonster *pMember = pSquadLeader->MySquadMember(i);

					if ( pMember != NULL && pMember != this && pMember->m_iHintNode != NO_NODE && pMember->m_iMySlot == bits_SLOT_HOUND_BATTERY )
					{

						MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
							WRITE_BYTE( TE_BEAMENTS );
							WRITE_SHORT( ENTINDEX( this->edict() ) );
							WRITE_SHORT( ENTINDEX( pMember->edict() ) );
							WRITE_SHORT( m_iSpriteTexture );
							WRITE_BYTE( 0 ); // framestart
							WRITE_BYTE( 0 ); // framerate
							WRITE_BYTE( 10 ); // life
							WRITE_BYTE( 40 );  // width
							WRITE_BYTE( 10 );   // noise
							WRITE_BYTE( 0  );   // r, g, b
							WRITE_BYTE( 50 );   // r, g, b
							WRITE_BYTE( 250);   // r, g, b
							WRITE_BYTE( 255 );	// brightness
							WRITE_BYTE( 30 );		// speed
						MESSAGE_END();
						break;
					}
				}
				*/

				/*
				// see if there is a battery to connect to.
				//CSquadMonster *pSquad = m_hSquadLeader;
				CSquadMonster *pSquad = (CSquadMonster *)((CBaseEntity *)m_hSquadLeader);
				while ( pSquad )
				{
					if ( pSquad->m_iMySlot == bits_SLOT_HOUND_BATTERY )
					{
						// draw a beam.
						MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
							WRITE_BYTE( TE_BEAMENTS );
							WRITE_SHORT( ENTINDEX( this->edict() ) );
							WRITE_SHORT( ENTINDEX( pSquad->edict() ) );
							WRITE_SHORT( m_iSpriteTexture );
							WRITE_BYTE( 0 ); // framestart
							WRITE_BYTE( 0 ); // framerate
							WRITE_BYTE( 10 ); // life
							WRITE_BYTE( 40 );  // width
							WRITE_BYTE( 10 );   // noise
							WRITE_BYTE( 0  );   // r, g, b
							WRITE_BYTE( 50 );   // r, g, b
							WRITE_BYTE( 250);   // r, g, b
							WRITE_BYTE( 255 );	// brightness
							WRITE_BYTE( 30 );		// speed
						MESSAGE_END();
						break;
					}

					pSquad = pSquad->m_hSquadNext;
				}
				*/
			}
			break;
		}


	case TASK_SPECIAL_ATTACK1:
		{

			//MODDD - not needed.
			/*
			WarmUpSound();
			pev->frame = 0;

			pev->sequence = 10;
			canSetAnim = FALSE;

				//UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM); ??
				//firstSpecialAttackFrame = TRUE;
			*/

			if( EASY_CVAR_GET_DEBUGONLY(houndeyeAttackMode) == 2){
				//provide the warm-up sound, the usual source isn't there.
				WarmUpSound();
			}


			m_IdealActivity = ACT_SPECIAL_ATTACK1;
			break;
		}
	case TASK_GUARD:
		{
			m_IdealActivity = ACT_GUARD;
			break;
		}
		//MODDD - added
	case TASK_HOUND_LEADERLOOK:
		{
			//length is 121/30
			// ??? why?
			//leaderlookTimeMax = gpGlobals->time + (120.5f/30.0f);
			//setAnimation("leaderlook", TRUE);
			//???

			// Dont' set leaderlook again too soon
			leaderLookCooldown = gpGlobals->time + LEADERLOOK_COOLDOWN_SETTING;
			this->SetSequenceByName("leaderlook");
		}
	default:
		{
			CSquadMonster::StartTask(pTask);
			break;
		}
	}

	if(pTask->iTask != TASK_SPECIAL_ATTACK1){
		firstSpecialAttackFrame = TRUE;
	}

}



//=========================================================
// RunTask
//=========================================================
void CHoundeye::RunTask ( Task_t *pTask )
{

	if(pTask->iTask != TASK_SPECIAL_ATTACK1){
		if(canResetSound){
			UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);
		}
		canResetSound = FALSE;
	}

	//EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "messg %d", pTask->iTask));
	switch ( pTask->iTask )
	{
	//MODDD - from schedule.cpp
	case TASK_FACE_IDEAL:
	//case TASK_FACE_ROUTE:
	{

		if(pev->yaw_speed == 0){
			/////easyForcePrintLine("HOUND ISSUE: YOU GOTTA BE");
			pev->yaw_speed = 90;
		}

		ChangeYaw( pev->yaw_speed );

		
		if(this->crazyPrintout){
			easyForcePrintLine("RUNTASK, TASK_FACE_IDEAL. yawspd: %.2f FacingIdeal? %d, ydelta: %.2f, yIdeal: %.2f", pev->yaw_speed, FacingIdeal(), FlYawDiff(), pev->ideal_yaw   );
		
		
			if(crazyPrintout){
				///easyForcePrintLine("FLYawDiff:::current: %.2f ideal: %.2f", flCurrentYaw, pev->ideal_yaw);
		
				Vector vecStart = this->pev->origin + Vector(0, 0, 12);

				//angle = pevTest->v_angle;
				Vector angle = pev->angles;
				Vector forward;
				UTIL_MakeVectorsPrivate( angle, forward, NULL, NULL );
				UTIL_drawLineFrame(vecStart, vecStart + forward * 50, 8, 255, 0, 0);

				/*
				angle = pev->v_angle;
				forward;
				UTIL_MakeVectorsPrivate( angle, forward, NULL, NULL );
				UTIL_drawLineFrame(vecStart, vecStart + forward * 50, 8, 0, 255, 0);
				*/

				float arc = pev->ideal_yaw * (M_PI / 180.0) ;

				Vector vecIdealForward = Vector(cos(arc), sin(arc), 0  );
				UTIL_drawLineFrame(vecStart, vecStart + vecIdealForward * 50, 8, 0, 0, 255);

			}

		}


		if ( FacingIdeal() )
		{
			TaskComplete();
		}
		break;
	}
	case TASK_HOUND_THREAT_DISPLAY:
		{
			MakeIdealYaw ( m_vecEnemyLKP );
			ChangeYaw ( pev->yaw_speed );

			if ( m_fSequenceFinished )
			{
				TaskComplete();
			}

			break;
		}
	case TASK_HOUND_CLOSE_EYE:
		{
			//MODDD
			/*
			if ( pev->skin < HOUNDEYE_EYE_FRAMES - 1 )
			{
				pev->skin++;
			}
			*/
			//no, HOUNDEYE_FRAMES - 1 (3) is still open.   - 2, however, works.
			//HM, -1 is best, that is the greatest frame (closed).
			if ( pev->skin < numberOfEyeSkins - 1 )
			{
				pev->skin++;
			}

			break;
		}
	case TASK_HOUND_HOP_BACK:
		{
			if ( m_fSequenceFinished )
			{
				TaskComplete();
			}
			break;
		}
	case TASK_SPECIAL_ATTACK1:
		{

			//MODDD TODO - uh, were we supposed to make this flicker randomly between all values every frame while charging the wave attack?
			pev->skin = RANDOM_LONG(0, max(numberOfEyeSkins - 1, 0) );
			//pev->skin = 3;


			//pev->skin values
			//0 = wide open,
			//1 = partly closed
			//2 = completely closed.
			//3 = wide open again?


			//MODDD - not needed
			/*
			if(firstSpecialAttackFrame){
				firstSpecialAttackFrame = FALSE;
				WarmUpSound();
				pev->frame = 0;
				pev->sequence = 10;
				canResetSound = TRUE;
				//ResetSequenceInfo();
				//SetYawSpeed();
			}
			WarmUpSound();
			pev->frame = 0;
			pev->sequence = 10;

			//canResetSound
			//UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM); ??
			*/

			MakeIdealYaw ( m_vecEnemyLKP );
			ChangeYaw ( pev->yaw_speed );

			float life;
			life = ((255 - pev->frame) / (pev->framerate * m_flFrameRate));
			if (life < 0.1) life = 0.1;


			MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
				WRITE_BYTE(  TE_IMPLOSION);
				WRITE_COORD( pev->origin.x);
				WRITE_COORD( pev->origin.y);
				WRITE_COORD( pev->origin.z + 16);
				WRITE_BYTE( 50 * life + 100);  // radius
				WRITE_BYTE( pev->frame / 25.0 ); // count
				WRITE_BYTE( life * 10 ); // life
			MESSAGE_END();



			if ( m_fSequenceFinished )
			{
				//MODDD - also not needed.
				/*
				canResetSound = FALSE;
				*/
				//???
				SonicAttack(TRUE);
				TaskComplete();
			}

			break;
		}
	//MODDD - addition
	case TASK_HOUND_LEADERLOOK:

		//EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "LEADeRLOOK??????? sf:%d t:%.2f tm:%.2f", m_fSequenceFinished, gpGlobals->time, leaderlookTimeMax));
		if ( m_fSequenceFinished )
		{
			//if this ends mark the sequence as over.
			usingCustomSequence = FALSE;
			TaskComplete();
		}


	break;
	case TASK_DIE:

		if(pev->framerate > 0 && pev->frame > 170) {
			// close further each frame then.
			if (pev->skin < numberOfEyeSkins-1) {
				pev->skin++;
			}
		}

		CSquadMonster::RunTask(pTask);
	break;

	default:
		{
			CSquadMonster::RunTask(pTask);
			break;
		}
	}
}


GENERATE_TRACEATTACK_IMPLEMENTATION(CHoundeye){
	GENERATE_TRACEATTACK_PARENT_CALL(CSquadMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CHoundeye){
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CSquadMonster);
}


GENERATE_KILLED_IMPLEMENTATION(CHoundeye) {
	// don't let random blinking logic work now.
	// Start with the eye open.
	// Near the end of TASK_DIE, it will close.
	m_fDontBlink = TRUE;
	pev->skin = 0;

	GENERATE_KILLED_PARENT_CALL(CSquadMonster);
}

void CHoundeye::BecomeDead(void)
{
	CSquadMonster::BecomeDead();
}

GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(CHoundeye){
	return GENERATE_DEADTAKEDAMAGE_PARENT_CALL(CBaseMonster);
}
void CHoundeye::DeathAnimationStart(){
	CSquadMonster::DeathAnimationStart();
}
void CHoundeye::DeathAnimationEnd(){

	// In case for some strange reason we didn't close in time.
	pev->skin = numberOfEyeSkins - 1;

	CSquadMonster::DeathAnimationEnd();
}


void CHoundeye::StartReanimation(void) {
	// open.  don't blink during that time (Spawn being called again reset m_fDontBlink)
	// ...nevermind, just allow blinking.  It opens the eye up just fine.
	m_fDontBlink = FALSE;
	//pev->skin = 0;
	CSquadMonster::StartReanimation();
}


//MODDD - NOTE.  See "PrescheduleThink" further down, that rarely shows up.
void CHoundeye::MonsterThink(void){
	if(m_pSchedule != NULL){
		EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "HOUNDEYE REPORT: sched: %s task: %d state: %d idealstate: %d act: %d idealact: %d see-en: %d interruptable: %d enemy: %s target: %s", getScheduleName(), getTaskNumber(), this->m_MonsterState, this->m_IdealMonsterState, this->m_Activity, this->m_IdealActivity, HasConditions(bits_COND_SEE_ENEMY), m_pSchedule->iInterruptMask, FClassname(m_hEnemy), FClassname(m_hTargetEnt)  ));
	}else{
		EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "IM A ineffective houndeye"));
	}
	
	CSquadMonster::MonsterThink();
}

void CHoundeye::ChangeSchedule( Schedule_t *pNewSchedule ){
	EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "HOUNDEYE: SCHED CHANGED!!!!!!!!!!!!!! %s", getScheduleName()));
	CSquadMonster::ChangeSchedule(pNewSchedule);
}


//=========================================================
// PrescheduleThink
//=========================================================
void CHoundeye::PrescheduleThink ( void )
{

	//EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "MY MAX SKIN: %d", numberOfEyeSkins));

	// if the hound is mad and is running, make hunt noises.
	if ( m_MonsterState == MONSTERSTATE_COMBAT && m_Activity == ACT_RUN && RANDOM_FLOAT( 0, 1 ) < 0.2 )
	{
		WarnSound();
	}

	// at random, initiate a blink if not already blinking or sleeping
	if ( !m_fDontBlink )
	{
		if ( ( pev->skin == 0 ) && RANDOM_LONG(0,0x7F) == 0 )
		{// start blinking!

			//MODDD - no, HOUNDEYE_EYE_FRAMES - 1 (3) just gives an open eye again.  - 2 is good.
			pev->skin = max(numberOfEyeSkins - 1, 0);
			//pev->skin = 2;
		}
		else if ( pev->skin > 0 )
		{// already blinking
			pev->skin--;
		}
	}

	// if you are the leader, average the origins of each pack member to get an approximate center.
	if ( IsLeader() )
	{
		CSquadMonster *pSquadMember;
		int iSquadCount = 0;

		for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
		{
			pSquadMember = MySquadMember(i);

			if (pSquadMember)
			{
				iSquadCount++;
				m_vecPackCenter = m_vecPackCenter + pSquadMember->pev->origin;
			}
		}

		m_vecPackCenter = m_vecPackCenter / iSquadCount;
	}
}


//=========================================================
// GetScheduleOfType
//=========================================================
Schedule_t* CHoundeye::GetScheduleOfType ( int Type )
{
	EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "Hound:GETSCHEDOFTYPE: %d", Type));
	
	if ( m_fAsleep )
	{
		// if the hound is sleeping, must wake and stand!
		if ( HasConditions( bits_COND_HEAR_SOUND ) )
		{
			CSound *pWakeSound;

			pWakeSound = PBestSound();
			ASSERT( pWakeSound != NULL );
			if ( pWakeSound )
			{
				MakeIdealYaw ( pWakeSound->m_vecOrigin );

				if ( FLSoundVolume ( pWakeSound ) >= HOUNDEYE_SOUND_STARTLE_VOLUME )
				{
					// awakened by a loud sound
					return &slHoundWakeUrgent[ 0 ];
				}
			}
			// sound was not loud enough to scare the bejesus out of houndeye
			return &slHoundWakeLazy[ 0 ];
		}
		else if ( HasConditions( bits_COND_NEW_ENEMY ) )
		{
			// get up fast, to fight.
			return &slHoundWakeUrgent[ 0 ];
		}

		else
		{
			// hound is waking up on its own
			return &slHoundWakeLazy[ 0 ];
		}
	}
	switch	( Type )
	{
	case SCHED_IDLE_STAND:
		{
			// we may want to sleep instead of stand!
			if ( InSquad() && !IsLeader() && !m_fAsleep && RANDOM_LONG(0,29) < 1 )
			{
				return &slHoundSleep[ 0 ];
			}else
			//MODDD - insertion.  If I am the leader, may play "leaderLook" instead.
			// And chance reduced a tad since the leaderlook schedule no longer forces
			// a wait of 5 seconds after.
			if ( InSquad() && IsLeader() && !m_fAsleep && gpGlobals->time >= leaderLookCooldown && RANDOM_LONG(0,2) < 1 )
			{
				return &slHoundLeaderLook[ 0 ];
			}
			//MODDD - resume after insertion.
			else
			{
				return CSquadMonster::GetScheduleOfType( Type );
			}
		}
	case SCHED_RANGE_ATTACK1:
		{
			return &slHoundRangeAttack[ 0 ];
/*
			if ( InSquad() )
			{
				return &slHoundRangeAttack[ RANDOM_LONG( 0, 1 ) ];
			}

			return &slHoundRangeAttack[ 1 ];
*/
		}
	case SCHED_SPECIAL_ATTACK1:
		{
			return &slHoundSpecialAttack1[ 0 ];
		}
	case SCHED_GUARD:
		{
			return &slHoundGuardPack[ 0 ];
		}
	case SCHED_HOUND_AGITATED:
		{
			return &slHoundAgitated[ 0 ];
		}
	case SCHED_HOUND_HOP_RETREAT:
		{
			return &slHoundHopRetreat[ 0 ];
		}
	case SCHED_FAIL:
		{
			if ( m_MonsterState == MONSTERSTATE_COMBAT )
			{
				if ( !FNullEnt( FIND_CLIENT_IN_PVS( edict() ) ) )
				{
					// client in PVS
					return &slHoundCombatFailPVS[ 0 ];
				}
				else
				{
					// client has taken off!
					return &slHoundCombatFailNoPVS[ 0 ];
				}
			}
			else
			{
				return CSquadMonster::GetScheduleOfType ( Type );
			}
		}
	default:
		{
			return CSquadMonster::GetScheduleOfType ( Type );
		}
	}
}

//=========================================================
// GetSchedule
//=========================================================
Schedule_t *CHoundeye::GetSchedule( void )
{
	//no need for extra bait script, defaults should carry over.

	switch	( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{
// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster::GetSchedule();
			}

			//MODDD - heavy damage must flinch.
			if(HasConditions(bits_COND_HEAVY_DAMAGE)){
				return GetScheduleOfType(SCHED_BIG_FLINCH);
			}

			if ( HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE ) )
			{
				if ( RANDOM_FLOAT( 0 , 1 ) <= 0.4 )
				{
					TraceResult tr;
					UTIL_MakeVectors( pev->angles );
					UTIL_TraceHull( pev->origin, pev->origin + gpGlobals->v_forward * -128, dont_ignore_monsters, head_hull, ENT( pev ), &tr );

					if ( tr.flFraction == 1.0 )
					{
						// it's clear behind, so the hound will jump
						return GetScheduleOfType ( SCHED_HOUND_HOP_RETREAT );
					}
				}

				return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
			}

			if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				if ( OccupySlot ( bits_SLOTS_HOUND_ATTACK ) )
				{
					//???
					if(EASY_CVAR_GET_DEBUGONLY(houndeyeAttackMode) == 1 || EASY_CVAR_GET_DEBUGONLY(houndeyeAttackMode) == 2 || EASY_CVAR_GET_DEBUGONLY(houndeyeAttackMode) == 3){
						return GetScheduleOfType ( SCHED_SPECIAL_ATTACK1 );
					}else{
						return GetScheduleOfType ( SCHED_RANGE_ATTACK1 );
					}
					//
				}

				return GetScheduleOfType ( SCHED_HOUND_AGITATED );
			}
			break;
		}
	}

	//MODDD - just seeing what was gathered from the parent class.
	Schedule_t* choice = CSquadMonster::GetSchedule();;
	EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "Houndeye: SCHED OUTSOURCED: %s", choice->pName));
	
	return choice;
}


BOOL CHoundeye::getMonsterBlockIdleAutoUpdate(void){
	return FALSE;
}
BOOL CHoundeye::forceIdleFrameReset(void){
	return FALSE;
}
//Whether this monster can re-pick the same animation before the next frame starts if it anticipates it getting picked.
//This is subtly retail behavior, but may not always play well.
BOOL CHoundeye::canPredictActRepeat(void){
	return TRUE;
}
BOOL CHoundeye::usesAdvancedAnimSystem(void){
	return TRUE;
}


int CHoundeye::tryActivitySubstitute(int activity){
	int i = 0;

	//no need for default, just falls back to the normal activity lookup.
	/*
	switch(activity){
		case ACT_IDLE:{
			//return SEQ_TEMPLATEMONSTER_XXX;
		break;}
	}//END OF switch
	*/

	//not handled by above? Rely on the model's anim for this activity if there is one.
	return CBaseAnimating::LookupActivity(activity);
}//END OF tryActivitySubstitute

int CHoundeye::LookupActivityHard(int activity){
	int i = 0;
	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	//any animation events in progress?  Clear it.
	resetEventQueue();

	//Within an ACTIVITY, pick an animation like this (with whatever logic / random check first):
	//    this->animEventQueuePush(10.0f / 30.0f, 3);  //Sets event #3 to happen at 1/3 of a second
	//    return LookupSequence("die_backwards");      //will play animation die_backwards

	if(monsterID == 6){
		int x = 45;
	}

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_IDLE:{
			if(InSquad() && IsLeader() && !m_fAsleep && gpGlobals->time >= leaderLookCooldown){
				//random chance: 30%?
				if(RANDOM_FLOAT(0, 1) <= 0.18){
					//this->SetSequenceByName("leaderlook");
					leaderLookCooldown = gpGlobals->time + LEADERLOOK_COOLDOWN_SETTING;
					return LookupSequence("leaderlook");
				}
			}
		break;}
	}//END OF switch
	
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard


