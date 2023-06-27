
#include "kingpin.h"
#include "kingpin_ball.h"
#include "hornet_kingpin.h"
#include "schedule.h"
#include "activity.h"
#include "util_model.h"
#include "defaultai.h"
#include "soundent.h"
#include "game.h"
//MODDD - why not?
#include "weapons.h"
#include "util_debugdraw.h"


// TELEPORT:  a kingpin can spawn some sprite similar to the mage-charge one that expands rapidly and teleports the
// kingpin when its mage anim ends and the sprite's reached a fair size.
// Is there an alternate green sprite used on retail maps?  That can be used for a healing aura the kingpin
// uses while in cover (interrupted by taking much damage while healing).




/*
IDEA:  kingpin-fired hornets should see if a straight-line path to the target is obscurred by something in the way and, if so, maybe adjust the route
to go past the thing in the way crudely?  Like a kingpin firing from a low point (by a ramp toward them), so you only see the top of the kingpin.  The hornets should see the ramp in the way and angle upwards somewhat.

If the kingpin takes damage while the enemy is very close behind, end the attack early and face instantly for melee!
*/



// TODO!  How about a big psionic shield?  After being interrupted from taking a lot of damage, puts up a temporary big blue shield
// that takes up twice it's normal health, and blocks enemies that touch it + a push a direction away and shock damage, but goes away
// with a single explosion (can't block projectiles while it's up, that is passive ability that doesn't work only while the big shield's up).



#define KINGPIN_SHOCKER_RADIUS 260
//Max range to search for powerup-able monsters. Bigger than the APPLY range below. If the closest monster is outside of APPLY but within SEARCH, this Kingpin may walk closer to get in APPLY range.
#define KINGPIN_POWERUP_SEARCH_RANGE 2100
//Max range this Kingpin may use the Power Up ability. May also walk towards monsters and then use it if they are not too far away (within SEARCH range at least).
#define KINGPIN_POWERUP_APPLY_RANGE 1600





EASY_CVAR_EXTERN_DEBUGONLY(animationKilledBoundsRemoval)
EASY_CVAR_EXTERN_DEBUGONLY(thoroughHitBoxUpdates)

EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard)

EASY_CVAR_EXTERN_DEBUGONLY(houndeye_attack_canGib)
EASY_CVAR_EXTERN_DEBUGONLY(kingpinDebug)
//EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar)

//???
extern short g_sBallForceFieldSprite;




#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_kingpin, CKingpin );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( kingpin, CKingpin );
	
	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( king, CKingpin );
	#endif
	
#endif





//TODO - incorporate the idea of a minimum amount of time the graphic appears.
//If not be a constant 0.3 seconds itself.  Go on/off in that amount of time frozen in whatever place was picked to reflect the projectile at.


//TODO - slightly slower mage loop anim for charging the electirc laser?
//different pulse noise for teleporting preparation?
//the freezy one for doing the teleport? a few particles fly of after doing it?

//a scale-in fade effect before homing balls are made during its charge??





//TODO MAJOR CONCERN - why don't ISLave beams (and thus Kingpin beams / sprites) seem to set SF_BEAM_TEMPORARY / SF_SPRITE_TEMPORARY for them?
//                     And are there possible issues on linked beams / sprites getting deleted betwween level transitions from the default
//                     ~FCAP_ACROSS_TRANSITION (lack of that capability)?  Should any links to beams / sprites thus be EDICT's instead of 
//                     general class references?


//TODO - reflect projectiles and with a neat blue glowbright or whatever they're called sprites, clientside event or here as a sprite. probably that.
// and the charge effect sprite (glowing flare thing above the mage_loop'ing kingpin) should probably also fade into existance like the fade out instead of instantly appearing
// at the start of a CHARGE task.
// ...I think this works now?



#define CHARGE_POINT_UP 94
#define CHARGE_POINT_FORWARD 48

#define KINPIN_ELECTRIC_LASER_CHARGETIME 1.6f

#define KINGPIN_VOICE_ATTENUATION (ATTN_NORM - 0.34f)

// was 0.3f?
#define REFLECT_BASE_DELAY 0.12f

//sequences in the anim, in the order they appear in the anim. Some anims have the same display name and so should just be referenced by order
//(numbered index), named well after purpose and based on display names for clarity. Safer this way.


/*
enum kingpin_sequence {  //key: frames, FPS
	KINGPIN_ATTACK_BOTH,  //21, 30
	KINGPIN_ATTACK_LEFT,  //21, 30
	KINGPIN_ATTACK_RIGHT, //21, 30
	KINGPIN_DIE_FORWARD,  //41, 30
	KINGPIN_DIE_HEADSHOT,  //47, 30
	KINGPIN_DIE_SIMPLE,  //71, 30
	KINGPIN_MAGE_START,  //16, 30
	KINGPIN_MAGE_LOOP,  //16, 30
	KINGPIN_MAGE_END,  //15, 30
	KINGPIN_SMALL_FLINCH1,  //16, 30
	KINGPIN_SMALL_FLINCH2,  //16, 30
	KINGPIN_IDLE1,  //121, 22
	KINGPIN_WALK,  //41, 30
	KINGPIN_RUN  //20, 30

};
*/


enum kingpin_sequence {  //key: frames, FPS
	KINGPIN_IDLE1,  //121, 22
	KINGPIN_WALK,  //41, 30
	KINGPIN_RUN,  //20, 30
	KINGPIN_SMALL_FLINCH1,  //16, 30
	KINGPIN_SMALL_FLINCH2,  //16, 30
	KINGPIN_ATTACK_BOTH,  //21, 30
	KINGPIN_ATTACK_LEFT,  //21, 30
	KINGPIN_ATTACK_RIGHT, //21, 30
	KINGPIN_MAGE_START,  //16, 30
	KINGPIN_MAGE_LOOP,  //16, 30
	KINGPIN_MAGE_END,  //15, 30
	KINGPIN_DIE_FORWARD,  //41, 30
	KINGPIN_DIE_HEADSHOT,  //47, 30
	KINGPIN_DIE_SIMPLE,  //71, 30


};






/*
enum kingpin_sequence {  //key: frames, FPS
	KINGPIN_IDLE,
	KINGPIN_WALK,
	KINGPIN_RUN,

};
*/


//ANIMATION COMMENT - don't know if the "run" animation puts the kingpin forwards a little much? When it stops to do "attack_" anything
//                    it appears to be pushed back noticably, or on changing from "idle1" to "run" it pushes forwards.

//                    Also is the point while standing overall a little pushed to the back, maybe the run's position is correct?
//                    move a little to the left or right while it tries to melee, it pushes itself left or right in a circular fashion instead of rotating on its own center.



//MODDD - the islave should prefer walking a lot too.
//        TODO QUESTION - Should thrown projectiles like grenades be reflected? how about chumtoads, snarks?  Already sure crossbow bolts / RPG rounds should be reflected.








//placeholders for now.
/*
#define KINGPIN_SCYTHE 0
#define KINGPIN_PSIONIC_CHARGE 1
#define KINGPIN_PSIONIC_LAUNCH 0
#define KINGPIN_DIE 0
#define KINGPIN_POWERUP 0
*/
//TODO - should there be an anim for deflecting projectiles or is that passive roughly in the direction it's looking? or all directions all the time?




	

//custom schedules
enum
{
	//SCHED_KINGPIN_POWERUP = LAST_COMMON_SCHEDULE + 1,
	//SCHED_KINGPIN_MOVE_TO_POWERUP,
	//SCHED_KINGPIN_ZZZ,


	SCHED_KINGPIN_SHOCKER = LAST_COMMON_SCHEDULE + 1,
	SCHED_KINGPIN_ELECTRIC_BARRAGE,
	SCHED_KINGPIN_SPEED_MISSILE,
	SCHED_KINGPIN_ELECTRIC_LASER,
	SCHED_KINGPIN_SUPERBALL,

	SCHED_CHASE_ENEMY_SMART_STOP_SIGHT_SHOCKER_FOLLOWUP,

	SCHED_KINGPIN_ELECTRIC_BARRAGE_CHARGE_FAIL,
	SCHED_KINGPIN_ELECTRIC_LASER_CHARGE_FAIL,

	SCHED_KINGPIN_GENERIC_RANGE_FAIL,

};

//custom tasks
enum
{
	//TASK_KINGPIN_PSIONIC_CHARGE = LAST_COMMON_TASK + 1,
	//TASK_KINGPIN_PSIONIC_LAUNCH,
	
	TASK_KINGPIN_ELECTRIC_BARRAGE_START = LAST_COMMON_TASK + 1,
	TASK_KINGPIN_ELECTRIC_BARRAGE_CHARGE,
	TASK_KINGPIN_ELECTRIC_BARRAGE_LOOP,
	TASK_KINGPIN_ELECTRIC_BARRAGE_END,

	TASK_KINGPIN_SPEED_MISSILE_START,
	TASK_KINGPIN_SPEED_MISSILE_FIRE,
	TASK_KINGPIN_SPEED_MISSILE_END,

	TASK_KINGPIN_ELECTRIC_LASER_START,
	TASK_KINGPIN_ELECTRIC_LASER_CHARGE,
	TASK_KINGPIN_ELECTRIC_LASER_FIRE,
	TASK_KINGPIN_ELECTRIC_LASER_END,

	TASK_KINGPIN_SUPERBALL_START,
	TASK_KINGPIN_SUPERBALL_FIRE,
	TASK_KINGPIN_SUPERBALL_END,


	TASK_KINGPIN_ELECTRIC_BARRAGE_CHARGE_INTERRUPTED,
	TASK_KINGPIN_ELECTRIC_LASER_CHARGE_INTERRUPTED,
	TASK_KINGPIN_GENERIC_RANGE_FAIL,

	TASK_KINGPIN_SHOCKER_ADMINISTER,
	TASK_GATE_KINGPIN_DISTANCE_MINIMUM,

};



///////////////////////////////////////////////////////////////////////////////////



const char *CKingpin::pDeathSounds[] = 
{
	"controller/con_die1.wav",
	"controller/con_die2.wav",
};

const char *CKingpin::pAlertSounds[] = 
{
	"controller/con_alert1.wav",
	"controller/con_alert2.wav",
	"controller/con_alert3.wav",
};


const char *CKingpin::pIdleSounds[] = 
{
	"controller/con_idle1.wav",
	"controller/con_idle2.wav",
	"controller/con_idle3.wav",
	"controller/con_idle4.wav",
	"controller/con_idle5.wav",
	"ambience/alien_chatter.wav",
};
const char *CKingpin::pPainSounds[] = 
{
	"controller/con_pain1.wav",
	"controller/con_pain2.wav",
	"controller/con_pain3.wav",
};


const char *CKingpin::pAttackSounds[] = 
{
	"controller/con_attack1.wav",
	"controller/con_attack2.wav",
	"controller/con_attack3.wav",
};




//NEW!!!
const char* CKingpin::pElectricBarrageHitSounds[] =
{
	"debris/zap1.wav",
	"debris/zap3.wav",
	"debris/zap8.wav",
};

const char* CKingpin::pElectricBarrageFireSounds[] =
{
	"debris/beamstart4.wav",
	"debris/beamstart6.wav",
	"debris/beamstart10.wav",
	"debris/beamstart11.wav",
};

const char* CKingpin::pElectricBarrageEndSounds[] =
{
	
	//stop powerup sound
	"debris/zap3.wav",
	"debris/zap4.wav",
	"debris/zap6.wav",
};

const char* CKingpin::pShockerFireSounds[] =
{
	"houndeye/he_blast1.wav",
	"houndeye/he_blast2.wav",
	"houndeye/he_blast3.wav",
};



/*
const char* CKingpin::pDeathSounds[] = 
{
	"kingpin/kingpin_death.wav",
};
const char* CKingpin::pAlertSounds[] = 
{
	"kingpin/kingpin_alert.wav",
};

const char* CKingpin::pIdleSounds[] = 
{
	"kingpin/kingpin_idle.wav",
};
const char* CKingpin::pPainSounds[] = 
{
	"kingpin/kingpin_pain.wav",
};
const char* CKingpin::pAttackSounds[] = 
{
	"kingpin/kingpin_attack.wav",
};
*/
const char* CKingpin::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};
const char* CKingpin::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};




TYPEDESCRIPTION	CKingpin::m_SaveData[] = 
{
	// why does the houndeye save spritetexture? who knows just do it too.
	DEFINE_FIELD(CKingpin, m_iSpriteTexture, FIELD_INTEGER),
	DEFINE_FIELD( CKingpin, m_voicePitch, FIELD_INTEGER),

	DEFINE_FIELD( CKingpin, chargeEffect, FIELD_CLASSPTR),
	
	DEFINE_ARRAY( CKingpin, m_pBeam, FIELD_CLASSPTR, KINGPIN_MAX_BEAMS ),
	DEFINE_ARRAY( CKingpin, m_flBeamExpireTime, FIELD_TIME, KINGPIN_MAX_BEAMS ),
	DEFINE_FIELD( CKingpin, m_iBeams, FIELD_INTEGER ),
	
	
	DEFINE_ARRAY( CKingpin, m_pEntityToReflect, FIELD_EHANDLE, KINGPIN_MAX_REFLECTEFFECT ),
	DEFINE_ARRAY( CKingpin, m_pReflectEffect, FIELD_CLASSPTR, KINGPIN_MAX_REFLECTEFFECT ),
	DEFINE_ARRAY( CKingpin, m_flReflectEffectApplyTime, FIELD_TIME, KINGPIN_MAX_REFLECTEFFECT ),
	DEFINE_ARRAY( CKingpin, m_flReflectEffectExpireTime, FIELD_TIME, KINGPIN_MAX_REFLECTEFFECT ),
	DEFINE_ARRAY( CKingpin, m_flReflectEffect_EndDelayFactor, FIELD_FLOAT, KINGPIN_MAX_REFLECTEFFECT),
	DEFINE_FIELD( CKingpin, m_iReflectEffect, FIELD_INTEGER ),

	
	
	DEFINE_FIELD( CKingpin, electricBarrageShotsFired, FIELD_INTEGER ),

	
	DEFINE_FIELD( CKingpin, chargeFinishTime, FIELD_TIME ),
	DEFINE_FIELD( CKingpin, electricBarrageNextFireTime, FIELD_TIME ),
	DEFINE_FIELD( CKingpin, electricBarrageStopTime, FIELD_TIME ),
	DEFINE_FIELD( CKingpin, electricBarrageIdleEndTime, FIELD_TIME ),
	DEFINE_FIELD( CKingpin, administerShockerTime, FIELD_TIME ),
	
	DEFINE_FIELD( CKingpin, primaryAttackCooldownTime, FIELD_TIME ),
	DEFINE_FIELD( CKingpin, enemyHiddenResponseTime, FIELD_TIME ),
	DEFINE_FIELD( CKingpin, enemyHiddenChaseTime, FIELD_TIME ),
	DEFINE_FIELD( CKingpin, giveUpChaseTime, FIELD_TIME ),
	
	DEFINE_FIELD( CKingpin, accumulatedDamageTaken, FIELD_FLOAT ),
	DEFINE_FIELD(CKingpin, shockerCooldown, FIELD_FLOAT),
	
	
};

//IMPLEMENT_SAVERESTORE( CKingpin, CBaseMonster );
int CKingpin::Save( CSave &save )
{
	if ( !CBaseMonster::Save(save) )
		return 0;
	int iWriteFieldsResult = save.WriteFields( "CKingpin", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iWriteFieldsResult;
}
int CKingpin::Restore( CRestore &restore )
{
	if ( !CBaseMonster::Restore(restore) )
		return 0;
	int iReadFieldsResult = restore.ReadFields( "CKingpin", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return iReadFieldsResult;
}


void CKingpin::PostRestore() {
	if (nextNormalThinkTime == 0)nextNormalThinkTime = 0.01;
	if (pev->nextthink == 0)pev->nextthink = 0.01;
}


CKingpin::CKingpin(void){

	SetupBeams();
	SetupReflectEffects();
	
	powerUpNearbyMonstersCooldown = -1;
	forceEnemyOnPoweredUpMonstersCooldown = -1;
	forceEnemyOnPoweredUpMonstersHardCooldown = -1;
	forgetRecentInflictingMonsterCooldown = -1;
	recentInflictingMonster = NULL;

	
	electricBarrageShotsFired = 0;

	chargeFinishTime = 0;
	electricBarrageNextFireTime = 0;
	electricBarrageStopTime = 0;
	electricBarrageIdleEndTime = 0;
	administerShockerTime = 0;
	
	primaryAttackCooldownTime = 0;
	enemyHiddenResponseTime = 0;
	enemyHiddenChaseTime = 0;
	giveUpChaseTime = 0;

	accumulatedDamageTaken = 0;

	chargeEffect = NULL;

	shockerCooldown = -1;
	blockAttackShockerCooldown = FALSE;

	//don't need to save this one.  Not much of an influence and re-picking up on it is no big deal.
	enemyNullTimeSet = FALSE;

	nextNormalThinkTime = -1;
	usingFastThink = FALSE;
			

}//END OF CKingpin constructor




//schedule details here......



/*
Task_t	tlKingpinRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	//{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_KINGPIN_PSIONIC_CHARGE, (float)0},
	{ TASK_KINGPIN_PSIONIC_LAUNCH, (float)0},
};

Schedule_t	slKingpinRangeAttack1[] =
{
	{ 
		tlKingpinRangeAttack1,
		ARRAYSIZE ( tlKingpinRangeAttack1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Kingpin Range Attack1"
	},
};
*/



//Ripped from slPrimaryMeleeAttack1 of defaultai.cpp.
Task_t	tlKingpinMeleeAttack[] =
{
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,        (float)ACT_IDLE },  //safety
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK1,		(float)0		},
};

Schedule_t	slKingpinMeleeAttack[] =
{
	{ 
		tlKingpinMeleeAttack,
		ARRAYSIZE ( tlKingpinMeleeAttack ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		
		//MODDD - restoring heavy damage as interruptable.
		//bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|

		bits_COND_ENEMY_OCCLUDED,
		0,
		"Kingpin Melee Attack"
	},
};


	
Task_t	tlKingpinShocker[] =
{
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_KINGPIN_GENERIC_RANGE_FAIL},
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,        (float)ACT_IDLE },  //safety
	{ TASK_FACE_ENEMY,			(float)0		},
	
	//give them the shocker.
	{ TASK_KINGPIN_SHOCKER_ADMINISTER, (float)0},

	{ TASK_GATE_KINGPIN_DISTANCE_MINIMUM, (float)0},

	//then switch to chasing them in melee and cut their ass up.
	{TASK_SET_SCHEDULE, (float)SCHED_CHASE_ENEMY_SMART_STOP_SIGHT_SHOCKER_FOLLOWUP},

};

Schedule_t	slKingpinShocker[] =
{
	{ 
		tlKingpinShocker,
		ARRAYSIZE ( tlKingpinShocker ), 
		//bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		//bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,
		//bits_COND_HEAR_SOUND,
		

		0,
		//bits_SOUND_DANGER,

		"Kingpin Shocker"
	},
};




//TODO - missile attack?  do a mage start & end and send 3 projectiles, maybe hornets speeding at the enemy, non-homing?


Task_t	tlKingpinElectricBarrage[] =
{
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_KINGPIN_GENERIC_RANGE_FAIL},
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,        (float)ACT_IDLE },  //safety
	{ TASK_FACE_ENEMY,			(float)0		},
	//{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_KINGPIN_ELECTRIC_BARRAGE_START, (float)0},
	{ TASK_SET_FAIL_SCHEDULE_HARD,		(float)SCHED_KINGPIN_ELECTRIC_BARRAGE_CHARGE_FAIL	},
	{ TASK_KINGPIN_ELECTRIC_BARRAGE_CHARGE, (float)0},
	{ TASK_KINGPIN_ELECTRIC_BARRAGE_LOOP, (float)0},
	{ TASK_KINGPIN_ELECTRIC_BARRAGE_END, (float)0},
};

Schedule_t	slKingpinElectricBarrage[] =
{
	{ 
		tlKingpinElectricBarrage,
		ARRAYSIZE ( tlKingpinElectricBarrage ), 
		//bits_COND_NEW_ENEMY			|
		//bits_COND_ENEMY_DEAD		|
		//bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,
		//bits_COND_HEAR_SOUND,
		

		0,
		//bits_SOUND_DANGER,

		"Kingpin Electric Barrage"
	},
};



Task_t	tlKingpinSpeedMissile[] =
{
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_KINGPIN_GENERIC_RANGE_FAIL},
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,        (float)ACT_IDLE },  //safety
	{ TASK_FACE_ENEMY,			(float)0		},
	//{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_KINGPIN_SPEED_MISSILE_START, (float)0},
	{ TASK_KINGPIN_SPEED_MISSILE_FIRE, (float)0},
	{ TASK_KINGPIN_SPEED_MISSILE_END, (float)0},
};

Schedule_t	slKingpinSpeedMissile[] =
{
	{ 
		tlKingpinSpeedMissile,
		ARRAYSIZE ( tlKingpinSpeedMissile ), 
		//bits_COND_NEW_ENEMY			|
		//bits_COND_ENEMY_DEAD		|
		//bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,
		//bits_COND_HEAR_SOUND,
		

		0,
		//bits_SOUND_DANGER,

		"Kingpin Speed Missile"
	},
};


Task_t	tlKingpinElectricLaser[] =
{
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_KINGPIN_GENERIC_RANGE_FAIL},
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	//{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_KINGPIN_ELECTRIC_LASER_START, (float)0},
	{ TASK_SET_FAIL_SCHEDULE_HARD,		(float)SCHED_KINGPIN_ELECTRIC_LASER_CHARGE_FAIL	},
	{ TASK_KINGPIN_ELECTRIC_LASER_CHARGE, (float)0},
	{ TASK_KINGPIN_ELECTRIC_LASER_FIRE, (float)0},
	{ TASK_KINGPIN_ELECTRIC_LASER_END, (float)0},
};

Schedule_t	slKingpinElectricLaser[] =
{
	{ 
		tlKingpinElectricLaser,
		ARRAYSIZE ( tlKingpinElectricLaser ), 
		//bits_COND_NEW_ENEMY			|
		//bits_COND_ENEMY_DEAD		|
		//bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_HEAR_SOUND,
		

		0,
		//bits_SOUND_DANGER,

		"Kingpin Electric Laser"
	},
};



Task_t	tlKingpinSuperBall[] =
{
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_KINGPIN_GENERIC_RANGE_FAIL},
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,        (float)ACT_IDLE },  //safety
	{ TASK_FACE_ENEMY,			(float)0		},
	//{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_KINGPIN_SUPERBALL_START, (float)0},
	{ TASK_KINGPIN_SUPERBALL_FIRE, (float)0},
	{ TASK_KINGPIN_SUPERBALL_END, (float)0},
};

Schedule_t	slKingpinSuperBall[] =
{
	{ 
		tlKingpinSuperBall,
		ARRAYSIZE ( tlKingpinSuperBall ), 
		//bits_COND_NEW_ENEMY			|
		//bits_COND_ENEMY_DEAD		|
		//bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_HEAR_SOUND,
		

		0,
		//bits_SOUND_DANGER,

		"Kingpin Super Ball"
	},
};




Task_t	tlKingpinElectricBarrageChargeFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_KINGPIN_ELECTRIC_BARRAGE_CHARGE_INTERRUPTED, 0				},
	//{ TASK_WAIT,				(float)0.2		},
	//{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slKingpinElectricBarrageChargeFail[] =
{
	{
		tlKingpinElectricBarrageChargeFail,
		ARRAYSIZE ( tlKingpinElectricBarrageChargeFail ),
		0,
		0,
		"Kingpin Electric Barrage Charge Fail"
	},
};



Task_t	tlKingpinElectricLaserChargeFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_KINGPIN_ELECTRIC_LASER_CHARGE_INTERRUPTED, 0				},
	//{ TASK_WAIT,				(float)0.2		},
	//{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slKingpinElectricLaserChargeFail[] =
{
	{
		tlKingpinElectricLaserChargeFail,
		ARRAYSIZE ( tlKingpinElectricLaserChargeFail ),
		0,
		0,
		"Kingpin Electric Laser Charge Fail"
	},
};


Task_t	tlKingpinGenericRangeFail[] =
{
	{TASK_KINGPIN_GENERIC_RANGE_FAIL, (float)0   },
};

Schedule_t	slKingpinGenericRangeFail[] =
{
	{
		tlKingpinGenericRangeFail,
		ARRAYSIZE ( tlKingpinGenericRangeFail ),
		0,
		0,
		"Kingpin Generic Range Fail"
	},
};







//MODDD - Same as slChaseEnemySmart_StopSight from defaultai.cpp, but changes the FAIL_SCHEDULE from SCHED_CHASE_ENEMY_FAILED to do a ranged attack instead.
Task_t tlChaseEnemySmart_StopSight_ShockerFollowup[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_KINGPIN_ELECTRIC_BARRAGE	},
	//{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	//{ TASK_RUN_PATH,			(float)0		},
	{ TASK_MOVE_TO_ENEMY_RANGE,(float)0		},
	{ TASK_CHECK_STUMPED,(float)0			},
};

Schedule_t slChaseEnemySmart_ShockerFollowup[] =
{
	{ 
		tlChaseEnemySmart_StopSight_ShockerFollowup,
		ARRAYSIZE ( tlChaseEnemySmart_StopSight_ShockerFollowup ),
		bits_COND_NEW_ENEMY			|
		//MODDD - added, the bullsquid counts this.  Why doesn't everything?
		bits_COND_ENEMY_DEAD |

		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_TASK_FAILED		|
		bits_COND_HEAR_SOUND |
		//bits_COND_SEE_ENEMY |
		//bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		
		bits_SOUND_DANGER,
		"Kingpin Chase Enemy Shocker Followup"
	},
};








/*
Task_t	tlKingpinXXX[] =
{
	{ TASK_KINGPIN_XXX,			0				},
	{ TASK_KINGPIN_YYY,			0				},
	{ TASK_KINGPIN_ZZZ,			0				},
};

Schedule_t	slKingpinXXX[] =
{
	{
		tlKingpinXXX,
		ARRAYSIZE ( tlKingpinXXX ),
		bits_COND_XXX | bits_COND_YYY | bits_COND_ZZZ,
		bits_SOUND_XXX | bits_SOUND_YYY | bits_SOUND_ZZZ,
		"kingpinXXX"
	},
};

//repeat for tl / sl KingpinYYY and tl / sl KingpinZZZ.


*/



DEFINE_CUSTOM_SCHEDULES( CKingpin )
{
	slKingpinMeleeAttack,
	slKingpinShocker,
	slKingpinElectricBarrage,
	slKingpinSpeedMissile,
	slKingpinElectricLaser,
	slKingpinSuperBall,
	slKingpinElectricBarrageChargeFail,
	slKingpinElectricLaserChargeFail,
	slChaseEnemySmart_ShockerFollowup,

};
IMPLEMENT_CUSTOM_SCHEDULES( CKingpin, CBaseMonster );




//MODDD - sound calls dummied out until they really exist.
void CKingpin::DeathSound( void ){
	int pitch = m_voicePitch + RANDOM_LONG(0,4);
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, KINGPIN_VOICE_ATTENUATION, 0, pitch );
}
void CKingpin::AlertSound( void ){
	int pitch = m_voicePitch + RANDOM_LONG(0,4);
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1.0, KINGPIN_VOICE_ATTENUATION, 0, pitch );
}
void CKingpin::IdleSound( void ){
	if(RANDOM_LONG(0, 2) <= 1){  //2/3
		int pitch = m_voicePitch + RANDOM_LONG(0,4);

		// Play a random idle sound
		UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, KINGPIN_VOICE_ATTENUATION, 0, pitch );
	}else{  //1/3
		int pitch = 99 + RANDOM_LONG(0, 6);
		//the special one?
		UTIL_PlaySound( edict(), CHAN_VOICE, "ambience/alien_chatter.wav", 0.73f, KINGPIN_VOICE_ATTENUATION, 0, pitch );
	}
}
void CKingpin::PainSound( void ){
	int pitch = m_voicePitch + RANDOM_LONG(0,4);
	if (RANDOM_LONG(0,5) < 2){
		UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, KINGPIN_VOICE_ATTENUATION, 0, pitch );
	}
}
void CKingpin::AttackSound( void ){
	int pitch = m_voicePitch + RANDOM_LONG(0,4);
	// Play a random attack sound
	UTIL_PlaySound( edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, KINGPIN_VOICE_ATTENUATION, 0, pitch );
}


extern int global_useSentenceSave;
void CKingpin::Precache( void )
{
	PRECACHE_MODEL("models/kingpin.mdl");
	PRECACHE_MODEL("sprites/lgtning.spr");

	global_useSentenceSave = TRUE;
	
	//NOTICE - attempting to precace files that don't exist crashes the game.
	
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	
	PRECACHE_SOUND_ARRAY(pElectricBarrageHitSounds);
	PRECACHE_SOUND_ARRAY(pElectricBarrageFireSounds);
	PRECACHE_SOUND_ARRAY(pElectricBarrageEndSounds);
	PRECACHE_SOUND_ARRAY(pShockerFireSounds);
	
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);


	// for now...
	PRECACHE_SOUND("houndeye/he_blast1.wav");
	PRECACHE_SOUND("houndeye/he_blast2.wav");
	PRECACHE_SOUND("houndeye/he_blast3.wav");
	
	PRECACHE_SOUND("ambience/zapmachine.wav");
	PRECACHE_SOUND("ambience/particle_suck1.wav");
	
	
	//PRECACHE_SOUND("weapons/electro4.wav", TRUE);  //no sentence equivalent... wait below does it already.
	PRECACHE_SOUND("weapons/electro5.wav", TRUE);  //YOU DOOFUS all the weapons/electro's were precached by the player!
	PRECACHE_SOUND("weapons/electro6.wav", TRUE);

	PRECACHE_SOUND("weapons/gauss2.wav", TRUE); //precached by player.
	PRECACHE_SOUND("weapons/mine_charge.wav", TRUE); //precached by player.

	
	PRECACHE_SOUND("garg/gar_stomp1.wav");
	
	PRECACHE_SOUND("x/x_shoot1.wav");
	//PRECACHE_SOUND("debris/beamstart4.wav");
	


	PRECACHE_SOUND("debris/beamstart8.wav");
	PRECACHE_SOUND("debris/beamstart15.wav");
	

	// NEW SOUNDS ABOVE HERE
	////////////////////////////////////////////////////////////////





	PRECACHE_MODEL ("sprites/hotglow_ff.spr");


	// For the KingpinBall.
	/////////////////////////////////////////////////////////
	PRECACHE_MODEL("sprites/xspark1.spr");
	PRECACHE_MODEL("sprites/xspark4.spr");
	
	global_useSentenceSave = TRUE;
	PRECACHE_SOUND("weapons/electro4.wav", TRUE);//don't skip. This is precached by the player gauss, just keep it.
	PRECACHE_SOUND("debris/zap4.wav");
	global_useSentenceSave = FALSE;
	/////////////////////////////////////////////////////////


	// For the charge ball effect.  or looping electric barrage?
	// whatever, just have a bunch of stuff nihilanth uses.
	/////////////////////////////////////////////////////////
	PRECACHE_MODEL("sprites/xspark4.spr");
	PRECACHE_MODEL("sprites/xspark1.spr");
	PRECACHE_MODEL("sprites/flare6.spr");
	PRECACHE_MODEL("sprites/nhth1.spr");
	PRECACHE_MODEL("sprites/exit1.spr");
	PRECACHE_MODEL("sprites/tele1.spr");
	PRECACHE_MODEL("sprites/animglow01.spr");
	PRECACHE_MODEL("sprites/muzzleflash3.spr");

	
	// and from the houndeye.
	m_iSpriteTexture = PRECACHE_MODEL( "sprites/shockwave.spr" );


	/////////////////////////////////////////////////////////

	UTIL_PrecacheOther( "hornet_kingpin" );
	UTIL_PrecacheOther( "kingpin_ball" );


	// precaching the hornet or a subclass of it grabs these
	/*
	PRECACHE_SOUND( "agrunt/ag_fire1.wav" );
	PRECACHE_SOUND( "agrunt/ag_fire2.wav" );
	PRECACHE_SOUND( "agrunt/ag_fire3.wav" );
	*/
}//END OF Precache()




void CKingpin::Spawn( void )
{
	Precache( );
	
	//model seems kinda off center. may be ok? unsure.
	SET_MODEL(ENT(pev), "models/kingpin.mdl");
	UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 100));

	pev->classname = MAKE_STRING("monster_kingpin");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	pev->health			= gSkillData.kingpinHealth;
	pev->yaw_speed		= 5;

	m_flFieldOfView		= VIEW_FIELD_FULL;// indicates the width of this monster's forward view cone ( as a dotproduct result )

	//Kingpin was described as having 360 degree vision from having all kinds of eyes or ways to get that kind of sensory information.
	//But should still be stunnable to open up the back for vulnerability.
	//Perhaps shots to the head, even from the front, should still get some bonus anyways?


	m_MonsterState		= MONSTERSTATE_NONE;

	m_afCapability		= bits_CAP_MELEE_ATTACK1 | bits_CAP_RANGE_ATTACK1 | bits_CAP_RANGE_ATTACK2; //| bits_CAP_DOORS_GROUP;

	MonsterInit();


	//MODDD - CVar me?
	//m_voicePitch = randomValueInt((int)EASY_CVAR_GET_DEBUGONLY(hassaultVoicePitchMin), (int)EASY_CVAR_GET_DEBUGONLY(hassaultVoicePitchMax) );

	m_voicePitch = randomValueInt(66, 76);


	//SetTouch(&CKingpin::CustomTouch );
	//SetTouch( NULL );

}//END OF Spawn();




void CKingpin::SetEyePosition(void){
	//Don't call the parent and instead force view_ofs to use a hardcoded eye position.
	//pev->view_ofs = VEC_VIEW;

	//The default way reads the model for eye position.
	CBaseMonster::SetEyePosition();
}//END OF SetEyePosition



//based off of GetSchedule for CBaseMonster in schedule.cpp.
Schedule_t* CKingpin::GetSchedule ( void )
{

	// WHyyyy is this so freakin' persistent.
	stopElectricBarrageLoopSound();

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

			//Check. Are there agrunts nearby to power up? If none are close enough to power up but aren't much further, route to them and
			//then power themn up.
			//NO - it is determined the powerup & focus on enemy will be passive instead. Don't happen just because this monster has
			//nothing else to do or require an active interaction like an anim.
			/*
			CBaseMonster* closestPowerupableMonster = findClosestPowerupableMonster();

			float distanceToClosest = (closestPowerupableMonster->pev->origin - this->pev->origin).Length();

			if(closestPowerupableMonster){
				//Assessment. How far away is this monster?
				if(distanceToClosest < KINGPIN_POWERUP_APPLY_RANGE){
					//start the powerup schedule now!
					return GetScheduleOfType(SCHED_KINGPIN_POWERUP);
				}else if(distanceToClosest < KINGPIN_POWERUP_SEARCH_RANGE){
					//get closer to this monster, then power up!
					this->m_hTargetEnt = closestPowerupableMonster;
					return GetScheduleOfType(SCHED_KINGPIN_MOVE_TO_POWERUP);
				}

			}//END OF possible schedule change (power-up) based on how close a powerup-able monster is

			*/


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
						//return GetScheduleOfType(SCHED_CHASE_ENEMY_STOP_SIGHT);
						return pickChaseOrStaySchedule();
					}
				}
				else
				{
					// chase!
					//easyPrintLine("ducks??");

					
					return pickChaseOrStaySchedule();
				}
			}
			else  
			{
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK1) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
				}
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK2) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK2 );
				}



				// because sometimes from interrupting an existing schedule, the conditions get cleared.
				// The check in "pickChaseOrStaySchedule" will probably be the only pracitcal one but whatever.
				if ( HasConditionsEither(bits_COND_CAN_RANGE_ATTACK2) )
				{
					//Come taste my super balls!  ...oh kill me now.
					return slKingpinSuperBall;
				}
				

				if ( HasConditionsEither(bits_COND_CAN_RANGE_ATTACK1) )
				{
					float flDist;
					if(m_hEnemy == NULL){
						//... what?
						return GetScheduleOfType(SCHED_IDLE_STAND);
					}


					//Make a decision.  How close are we to the enemy?
					flDist = (m_hEnemy->pev->origin - pev->origin).Length();



					//pick one regardless of distance if we forced one with KinpginDebug.


					switch( (int)(EASY_CVAR_GET_DEBUGONLY(kingpinDebug)) ){
					case 1:
						return slChaseEnemySmart_StopSight;
					break;
					case 2:
						return slChaseEnemySmart_ShockerFollowup;
					break;
					case 3:
						return slKingpinShocker;
					break;
					case 4:
						return slKingpinElectricBarrage;
					break;
					case 5:
						return slKingpinElectricLaser;
					break;
					case 6:
						return slKingpinSpeedMissile;
					break;
					case 7:
						return slKingpinSuperBall;
					break;


					}//END OF switch of CVar kingpinDebug.


					if(flDist <= 300.0f){

						// 100% chance shocker
						return GetScheduleOfType(SCHED_KINGPIN_SHOCKER);
					}else if(flDist <= 450.0f){
						//80% shocker.
						//20% electric barrage.
						float randomChoice = RANDOM_FLOAT(0.0f, 1.0f);

						if(randomChoice < 0.80f){
							return slKingpinShocker;
						}else{
							return slKingpinElectricBarrage;
						}


					}else if(flDist <= 800.0f){
						//20% shocker.
						//70% electric barrage.
						//10% electric laser.
						float randomChoice = RANDOM_FLOAT(0.0f, 1.0f);

						if(randomChoice < 0.20f){
							return slKingpinShocker;
						}else if(randomChoice < 0.20f + 0.70f){
							return slKingpinElectricBarrage;
						}else{
							return slKingpinElectricLaser;
						}


					}else if(flDist <= 1150.0f){
						//50% electric barrage
						//30% speed missile
						//20% electric laser
						float randomChoice = RANDOM_FLOAT(0.0f, 1.0f);

						if(randomChoice < 0.50f){
							return slKingpinElectricBarrage;
						}else if(randomChoice < 0.50f + 0.30f){
							return slKingpinSpeedMissile;
						}else{
							return slKingpinElectricLaser;
						}

					}else if(flDist <= 2000.0f){
						//50% speed missile
						//40% electric laser
						//10% super ball (bypass)

						float randomChoice = RANDOM_FLOAT(0.0f, 1.0f);

						if(randomChoice < 0.50f){
							return slKingpinSpeedMissile;
						}else if(randomChoice < 0.50f + 0.40f){
							return slKingpinElectricLaser;
						}else{
							return slKingpinSuperBall;
						}
					}else if(flDist <= 2700.0f){
						//40% speed missile
						//50% electric laser
						//10% super ball (bypass)

						float randomChoice = RANDOM_FLOAT(0.0f, 1.0f);

						if(randomChoice < 0.40f){
							return slKingpinSpeedMissile;
						}else if(randomChoice < 0.40f + 0.50f){
							return slKingpinElectricLaser;
						}else{
							return slKingpinSuperBall;
						}
					}else{

						//further? just follow instead.
						//...wait how did this condition even pass then.  Fall below I guess.
						
						//HACK - force it to pick that usual area this way.
						ClearConditions(bits_COND_CAN_RANGE_ATTACK1);
					}




					
					//return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
				}//END OF ranged attack 1 condition check

				




				//MODDD - NOTE - is that intentional?  range1 & melee1,  and not say,  melee1 & melee2???
				if ( !HasConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1) )
				{
					// if we can see enemy but can't use either attack type, we must need to get closer to enemy
					//No, I still prefer to look at my enemy in anticipation of sending a potent ranged attack.

					//if the enemy is too far away we do want to try to get closer though.  Or if the enemy has hidden for way too long.
					
					//return pickChaseOrStaySchedule();

					return GetScheduleOfType(SCHED_CHASE_ENEMY);
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





//MODDD - special method for the kingpin to store some otherwise repeated script.
Schedule_t* CKingpin::pickChaseOrStaySchedule(void){

	//First, if I'm due for RANGED_ATTACK2 (enemy was behind cover for too long) to send a super ball, do so.
	if ( HasConditionsEither(bits_COND_CAN_RANGE_ATTACK2) )
	{
		//Come taste my super balls!  ...oh kill me now.
		return slKingpinSuperBall;
	}



	if(
		m_hEnemy != NULL &&
		(
			((m_hEnemy->pev->origin - pev->origin).Length() > 2600.0f) ||
			(gpGlobals->time >= enemyHiddenChaseTime)
		)
	){
		//If the enemy has moved far enough away, or it is time to chase the enemy, go ahead.
		return GetScheduleOfType( SCHED_CHASE_ENEMY_STOP_SIGHT );
	}else{
		//otherwise I'd rather stay here.
		return GetScheduleOfType( SCHED_COMBAT_LOOK );
	}

}//END OF pickChaseOrStaySchedule













Schedule_t* CKingpin::GetScheduleOfType( int Type){
	
	switch(Type){
		/*
		case SCHED_KINGPIN_POWERUP:
			
			//do this animation.
			this->SetSequenceByIndex(KINGPIN_POWERUP);
			//no schedules yet??
		break;
		case SCHED_KINGPIN_MOVE_TO_POWERUP:
			//... what?
		break;
		*/
		
		case SCHED_CHASE_ENEMY_STOP_SIGHT:
		case SCHED_CHASE_ENEMY_SMART_STOP_SIGHT:
			// also, record when to give up a chase and allow an attack regardless of distance.
			giveUpChaseTime = gpGlobals->time + RANDOM_FLOAT(3.5f, 4.6f);
			
			//!!is this a good idea too?
			enemyHiddenChaseTime = gpGlobals->time + 10.0f;

			return slChaseEnemySmart_StopSight;
		break;
		case SCHED_CHASE_ENEMY_SMART_STOP_SIGHT_SHOCKER_FOLLOWUP:
			// Same as ChaseEnemySmart_StopSight, but jumps to an Electric Barrage if pathfinding fails during it.
			// Ordinary pathfind failure repicks from schedules in an ordinary fashion.  Following a shockwave attack,
			// failure to do any melee from declaring failure to reach the enemy should result in the next best thing:
			// electric barrage.
			// More importantly, this doesn't stop at just seeing the enemy. It keeps going to do melee attacks, presumably
			// while they are disoriented by the short-range intense shockwave that came right before this.
			giveUpChaseTime = gpGlobals->time + RANDOM_FLOAT(3.5f, 4.6f);
			return slChaseEnemySmart_ShockerFollowup;
		break;

		case SCHED_MELEE_ATTACK1:
			return slKingpinMeleeAttack;
		break;
		case SCHED_RANGE_ATTACK1:
			//......wat.  never call this directly, pick something else if the range attack condition  is met.

			//return slKingpinRangeAttack1;
		break;
		case SCHED_KINGPIN_SHOCKER:
			if (gpGlobals->time >= shockerCooldown) {
				shockerCooldown = gpGlobals->time + RANDOM_FLOAT(3.8, 7);
				return slKingpinShocker;
			}
			else {
				// Try chasing then for melee?  SCHED_COMBAT_FACE may work too?
				//return pickChaseOrStaySchedule();
				blockAttackShockerCooldown = TRUE;
				return GetScheduleOfType(SCHED_CHASE_ENEMY);
			}
		break;

		case SCHED_KINGPIN_ELECTRIC_BARRAGE:
			return slKingpinElectricBarrage;
		break;
		case SCHED_KINGPIN_SPEED_MISSILE:
			return slKingpinSpeedMissile;
		break;
		case SCHED_KINGPIN_ELECTRIC_LASER:
			return slKingpinElectricLaser;
		break;
		case SCHED_KINGPIN_SUPERBALL:
			return slKingpinSuperBall;
		break;


		
		case SCHED_KINGPIN_ELECTRIC_BARRAGE_CHARGE_FAIL:
			return slKingpinElectricBarrageChargeFail;
		break;

		case SCHED_KINGPIN_ELECTRIC_LASER_CHARGE_FAIL:
			return slKingpinElectricLaserChargeFail;
		break;
		case SCHED_KINGPIN_GENERIC_RANGE_FAIL:
			return slKingpinGenericRangeFail;
		break;
		
	}//switch(Type)
	
	return CBaseMonster::GetScheduleOfType(Type);
}//END OF GetScheduleOfType(...)


void CKingpin::StartTask( Task_t *pTask ){


	switch( pTask->iTask ){
		
		case TASK_KINGPIN_GENERIC_RANGE_FAIL:
			//just in case this interrupted a ranged attack very early.  returning to it too soon may look a little funny.
			setPrimaryAttackCooldown();

			m_IdealActivity = ACT_IDLE;  //just for safety.

			TaskComplete();
		break;
		
		case TASK_KINGPIN_SHOCKER_ADMINISTER:{

			administerShockerTime = gpGlobals->time + ((7.0f / 30.0f) / pev->framerate);

			//This does not come with the melee attack event, nor should it.  If it were included with the model, we
			//need to be told to ignore that in HandleEvents or whatever handles model events instead of our custom queue ones.
			SetSequenceByIndex(KINGPIN_ATTACK_BOTH);

		break;}
		case TASK_GATE_KINGPIN_DISTANCE_MINIMUM:{
			float distanceToEnemy;
			if(m_hEnemy == NULL){
				//???????????
				TaskFail();
				return;
			}

			distanceToEnemy = (m_hEnemy->pev->origin - pev->origin).Length();

			if(distanceToEnemy <= 500){
				//commence the chase.
				TaskComplete();
				return;
			}else{
				//too far? don't bother, re-pick a different schedule.
				//TaskFail();

				//this is... probably better?
				ChangeSchedule(GetSchedule());
				return;
			}
			
			

		break;}



		case TASK_KINGPIN_ELECTRIC_BARRAGE_CHARGE_INTERRUPTED:{

			//just... call these anyways for safety.
			removeChargeEffect();
			stopElectricBarrageLoopSound();

			if(pev->sequence == KINGPIN_MAGE_LOOP || pev->sequence == KINGPIN_MAGE_START){
				
				if(pev->sequence == KINGPIN_MAGE_LOOP){
					//make a shorting-out sound.
					playElectricBarrageEndSound();
				}


				if(m_IdealMonsterState == MONSTERSTATE_DEAD){
					//No, dying takes precedence.  Let someting else handle this.
					
					//Strangely at the killing blow, the deadflag isn't changed from DEAD_NO yet. 
					//TASK_DIE from a die schedule has to do that (not picked yet at this point, an interrupt fail schedule)
					//But m_IdealMonsterState is set to MONSTERSTATE_DEAD.
					//if(pev->deadflag != DEAD_NO){

					//TaskComplete();  //And don't call TaskComplete in addition to changing the schedule if you're doing that.
					//At least not after doing so, that's terrible horrendous karma.
					//go a step beyond: change to the SCHED_DIE death animation.
					ChangeSchedule(GetScheduleOfType(SCHED_DIE));
					return;
				}


				SetSequenceByIndex(KINGPIN_MAGE_END);
			}else{
				//just proceed.
				TaskComplete();
			}
		break;}
		case TASK_KINGPIN_ELECTRIC_LASER_CHARGE_INTERRUPTED:{
			
			//just... call these anyways for safety.
			removeChargeEffect();
			stopElectricLaserChargeSound();

			if(pev->sequence == KINGPIN_MAGE_LOOP || pev->sequence == KINGPIN_MAGE_START){
				
				if(pev->sequence == KINGPIN_MAGE_LOOP){
					//make a shorting-out sound.
					playElectricBarrageEndSound();
				}


				if(m_IdealMonsterState == MONSTERSTATE_DEAD){
					//No, dying takes precedence.  Let someting else handle this.
					//TaskComplete();
					//go a step beyond: change to the SCHED_DIE death animation.
					ChangeSchedule(GetScheduleOfType(SCHED_DIE));
					return;
				}

				SetSequenceByIndex(KINGPIN_MAGE_END);
			}else{
				//just proceed.
				TaskComplete();
			}
		break;}


		case TASK_KINGPIN_ELECTRIC_BARRAGE_START:{

			m_Activity = ACT_RANGE_ATTACK1;
			m_IdealActivity = ACT_RANGE_ATTACK1;

			SetSequenceByIndex(KINGPIN_MAGE_START);
		break;}
		case TASK_KINGPIN_ELECTRIC_BARRAGE_CHARGE:{
			
			createChargeEffect();

			this->m_iForceLoops = TRUE;
			SetSequenceByIndex(KINGPIN_MAGE_LOOP);
			this->m_iForceLoops = -1;

			playElectricBarrageStartSound();

			accumulatedDamageTaken = 0;  //start keeping track of this.


			chargeFinishTime = gpGlobals->time + 0.9f;

		break;}
		case TASK_KINGPIN_ELECTRIC_BARRAGE_LOOP:{
			//the sequence set last time still loops, let it run.

			playElectricBarrageLoopSound();  //does this auto replay?

			electricBarrageShotsFired = 0;
			electricBarrageNextFireTime = gpGlobals->time + 0.5f;
			electricBarrageStopTime = -1;
			electricBarrageIdleEndTime = gpGlobals->time + 7.0f;

			//If m_hEnemy is lost, and we reset the idle time to end sooner, we need to mark whether that's been done yet.
			enemyNullTimeSet = FALSE;


		break;}
		case TASK_KINGPIN_ELECTRIC_BARRAGE_END:{
			playElectricBarrageEndSound();
			removeChargeEffect();
			stopElectricBarrageLoopSound();
			SetSequenceByIndex(KINGPIN_MAGE_END);
		break;}
		
		
		case TASK_KINGPIN_ELECTRIC_LASER_START:{
			
			m_Activity = ACT_RANGE_ATTACK1;
			m_IdealActivity = ACT_RANGE_ATTACK1;


			SetSequenceByIndex(KINGPIN_MAGE_START);
		break;}
		case TASK_KINGPIN_ELECTRIC_LASER_CHARGE:{
			
			createChargeEffect();
			
			this->m_iForceLoops = TRUE;
			SetSequenceByIndex(KINGPIN_MAGE_LOOP, 0.7f);
			this->m_iForceLoops = -1;

			playElectricLaserChargeSound();

			accumulatedDamageTaken = 0;  //start keeping track of this.  may not use it this time.


			chargeFinishTime = gpGlobals->time + KINPIN_ELECTRIC_LASER_CHARGETIME;
		break;}
		case TASK_KINGPIN_ELECTRIC_LASER_FIRE:{
			Vector vecStart;
			Vector vecForward;
			Vector vecRight;
			Vector vecUp;
			
			UTIL_MakeAimVectorsPrivate(pev->angles, vecForward, vecRight, vecUp);

			//vecStart = pev->origin + vecForward * 18 + Vector(0, 0, 60);
			vecStart = pev->origin + vecUp * CHARGE_POINT_UP + vecForward * CHARGE_POINT_FORWARD;


			Vector firePoint;
			BOOL canFireLaser = FALSE;
			CBaseEntity* fireHitIntention = NULL;

			

			//if we have to?
			stopElectricLaserChargeSound();

			removeChargeEffect();

			

			//If our enemy is NOT null or dead, and we can see them, they are a valid target.
			if( !(m_hEnemy == NULL || !(m_hEnemy->IsAlive_FromAI(this))) && HasConditions(bits_COND_SEE_ENEMY)){
				//clearly our enemy is in sight. Do it
				canFireLaser = TRUE;
				firePoint = m_hEnemy->BodyTargetMod(vecStart);
			}else{

				CBaseEntity* testEntity;
				testEntity = attemptFindTowardsPoint(m_vecEnemyLKP);
				
				if(testEntity != NULL){
					canFireLaser = TRUE;
					firePoint = testEntity->BodyTargetMod(vecStart);
				}


				

				//...could we have also just done GetEnemy(TRUE); and seen if m_hEnemy was null after that?
				//   this does check each memeber of the stack instead though.

				/*
				int i;
				Vector vecForward;
				Vector vecForward2D;
				UTIL_MakeVectorsPrivate ( pev->angles, vecForward, NULL, NULL );
				vecForward2D = vecForward.Make2D();
				
				for(i = 0; i < m_intOldEnemyNextIndex; i++){
					//can we see this enemy instead?
					
					if(m_hOldEnemy[i] != NULL && m_hOldEnemy[i]->IsAlive_FromAI(this) ){
						//thank you CheckAttacks of basemonster.cpp.
						Vector2D vec2LOS;
						float flDot;

						
						vec2LOS = ( m_hOldEnemy[i]->pev->origin - pev->origin ).Make2D();
						vec2LOS = vec2LOS.Normalize();
							
						flDot = DotProduct (vec2LOS , vecForward2D() );


						if(flDot >= 0.7f && (FVisible(m_hOldEnemy[i]) || FVisible(vecStart, m_hOldEnemy[i] )) ){
							//visible from just myself or that point? it's good.
								
							firePoint = m_hOldEnemy[i]->BodyTargetMod(vecStart);
							fireHitIntention = m_hOldEnemy[i];
							canFireLaser = TRUE;
							break;
						}
					}
				}
				*/









				//If our charge effect is present, it can also have a line of sight to the enemy.
				//Otherwise sitting and doing nothing while the enemy (player) peeks at it is a little odd.

				//TraceResult tr;
				//UTIL_TraceLine(this->EyePosition(),
				//wait we already have a method for this.


				/*
				//they are not? Can we pick a different enemy?
				//TODO - this ain't workin.
				BOOL getSuccess = GetEnemy();


				//if(FVisible(m_hEnemy->EyePosition())){
				if(getSuccess){
					//this also counts.
					canFireLaser = TRUE;
				}
				*/

				//GO THROUGH THE ENEMY STACK AND SEE IF ONE OF THEM IS VISIBLE IF m_hENEMY IS NOT NULL AND NOT VISIBLE.
				//ONLY THEN GIVE UP IF NONE IS GOOD.





			}//END OF see enemy check.

			
			if(canFireLaser){

				fireElectricDenseLaser(fireHitIntention, firePoint);
			}else{
				//well that was anticlimactic. Nowhere for the laser to go.
				//Make the same electric fizz out the electric barrage ending does for now.
				playElectricBarrageEndSound();
			}



			TaskComplete();
		break;}
		case TASK_KINGPIN_ELECTRIC_LASER_END:{
			
			SetSequenceByIndex(KINGPIN_MAGE_END);
		break;}



		case TASK_KINGPIN_SUPERBALL_START:{
			m_Activity = ACT_RANGE_ATTACK1;
			m_IdealActivity = ACT_RANGE_ATTACK1;

			SetSequenceByIndex(KINGPIN_MAGE_START);
		break;}
		case TASK_KINGPIN_SUPERBALL_FIRE:{
			playSuperBallFireSound();
			fireSuperBall();

			
			enemyHiddenResponseTime = gpGlobals->time + RANDOM_FLOAT(5.9f, 7.6f);

			TaskComplete();
		break;}
		case TASK_KINGPIN_SUPERBALL_END:{
			
			SetSequenceByIndex(KINGPIN_MAGE_END);
		break;}


		case TASK_KINGPIN_SPEED_MISSILE_START:{
			m_Activity = ACT_RANGE_ATTACK1;
			m_IdealActivity = ACT_RANGE_ATTACK1;

			SetSequenceByIndex(KINGPIN_MAGE_START);
		break;}
		case TASK_KINGPIN_SPEED_MISSILE_FIRE:{
			//playSpeedMissileFireSound();
			fireSpeedMissile();
			TaskComplete();
		break;}
		case TASK_KINGPIN_SPEED_MISSILE_END:{
			
			SetSequenceByIndex(KINGPIN_MAGE_END);
		break;}
		/*
		case TASK_KINGPIN_PSIONIC_CHARGE:
		{
			
			if(m_hEnemy == NULL){
				//stop?
				TaskFail();
				return;
			}


			//set animation..?
			this->SetSequenceByIndex(KINGPIN_PSIONIC_CHARGE, 0.8f);


		break;
		}
		case TASK_KINGPIN_PSIONIC_LAUNCH:
		{

			if(m_hEnemy == NULL){
				//stop?
				TaskFail();
				return;
			}

			m_IdealActivity = ACT_RANGE_ATTACK1;
			signalActivityUpdate = TRUE;

		break;
		}
		*/
		default:
			CBaseMonster::StartTask( pTask );
		break;
	}//END OF switch(...)

}//END OF StartTask(...)

void CKingpin::RunTask( Task_t *pTask ){
	
	//EASY_CVAR_PRINTIF_PRE(templatePrintout, easyPrintLine("RunTask: sched:%s task:%d", this->m_pSchedule->pName, pTask->iTask) );
	
	switch( pTask->iTask ){

		
		case TASK_KINGPIN_SHOCKER_ADMINISTER:{


			if(administerShockerTime != -1 && gpGlobals->time >= administerShockerTime){
				//oh yeah here we go.
				administerShocker();
				administerShockerTime = -1;
			}


			if(m_fSequenceFinishedSinceLoop){
				TaskComplete();
			}
		break;}
		case TASK_KINGPIN_ELECTRIC_BARRAGE_CHARGE_INTERRUPTED:{
			if(m_fSequenceFinishedSinceLoop || pev->frame > 200){
				setPrimaryAttackCooldown();
				TaskComplete();
			}
		break;}
		case TASK_KINGPIN_ELECTRIC_LASER_CHARGE_INTERRUPTED:{
			if(m_fSequenceFinishedSinceLoop || pev->frame > 200){
				setPrimaryAttackCooldown();
				TaskComplete();
			}
		break;}
		case TASK_KINGPIN_ELECTRIC_BARRAGE_START:{
			if(m_fSequenceFinishedSinceLoop || pev->frame > 200){
				TaskComplete();
			}
		break;}
		case TASK_KINGPIN_ELECTRIC_BARRAGE_CHARGE:{

			turnToFaceEnemyLKP();


			if(accumulatedDamageTaken >= 25){
				//taken too much damage? stop.
				TaskFail();
				return;
			}


			if(gpGlobals->time >= chargeFinishTime){
				TaskComplete();
			}
		break;}
		case TASK_KINGPIN_ELECTRIC_BARRAGE_LOOP:{

			if(electricBarrageStopTime == -1){
				float flDot;
				float distanceToEnemy;
						

				
				if(accumulatedDamageTaken >= 30){
					//taken too much damage? stop.
					TaskFail();
					return;
				}


				//NULL or dead?
				if(m_hEnemy == NULL || !(m_hEnemy->IsAlive_FromAI(this)) ){
					m_hEnemy = NULL;  //force it off, dead enemies are no good.

					if(!enemyNullTimeSet){
						//do so.
						enemyNullTimeSet = TRUE;
						electricBarrageIdleEndTime = gpGlobals->time + 4;
					}

					//can we get a different enemy in the meantime?
					GetEnemy(TRUE);

					if(m_hEnemy != NULL){
						//reset the idle time as though we started the schedule again.
						enemyNullTimeSet = FALSE;
						electricBarrageIdleEndTime = gpGlobals->time + 7.0f;
					}

				}else{
					distanceToEnemy = (m_hEnemy->pev->origin - pev->origin).Length();

					if(distanceToEnemy >= 1300){

						//If the enemy is too far away, we can't fire.  or would rather not sometimes.
						//If we can see the enemy at the same time they go too far, make the end time really short.
						if(!enemyNullTimeSet){
							

							if(HasConditions(bits_COND_SEE_ENEMY)){
								enemyNullTimeSet = TRUE;
								electricBarrageIdleEndTime = gpGlobals->time + 1.3f;
							}else{
								//don't act on this at all.  If we can't even see them to notice they're too far away we think they're hiding behind cover that would put them
								//closer if they came out the way they entered.
							}

						}//END OF null time set check


					}else{

						//we have an enemy to focus on.
						enemyNullTimeSet = FALSE;

						turnToFaceEnemyLKP();

						flDot = getDotProductWithEnemyLKP();
				
						//check, not too soon since firing recently? is the enemy in sight?  Am I facing them enough?
						if(gpGlobals->time >= electricBarrageNextFireTime && HasConditions(bits_COND_SEE_ENEMY) && flDot >= 0.7){
						
							if(distanceToEnemy <= 1100){
								//only have the will to extend the idle time if the enemy isn't hugging the boundary.
								electricBarrageIdleEndTime = gpGlobals->time + 7.0f;
							}
							//electricBarrageNextFireTime = gpGlobals->time + 0.38f;
							electricBarrageNextFireTime = gpGlobals->time + 0.1f;
							electricBarrageShotsFired += 1;

							//fire!
							fireElectricBarrageLaser();

						}

					}//END OF good distance check
				
				}//END OF enemy null check
				
				//MODDD TEMP - limitless!
				//if(electricBarrageShotsFired >= 8 || gpGlobals->time >= electricBarrageIdleEndTime){
				if(electricBarrageShotsFired >= 300 || gpGlobals->time >= electricBarrageIdleEndTime){
					//If I fired too many shots or too much time has passed since firing a shot, let this end.
					electricBarrageStopTime = gpGlobals->time + 0.5; //little boundary.
				}

			}else{
				//electricBarrageStopTime was used? Pay attention to it instead
				if(gpGlobals->time >= electricBarrageStopTime){
					this->ClearBeams();
					TaskComplete(); //move on.
				}
			}

		break;}
		case TASK_KINGPIN_ELECTRIC_BARRAGE_END:{
			if(m_fSequenceFinishedSinceLoop){
				setPrimaryAttackCooldown();
				TaskComplete();
			}
		break;}


		case TASK_KINGPIN_ELECTRIC_LASER_START:{
			if(m_fSequenceFinishedSinceLoop || pev->frame > 200){
				TaskComplete();
			}
		break;}
		case TASK_KINGPIN_ELECTRIC_LASER_CHARGE:{

			if(accumulatedDamageTaken >= 25){
				//taken too much damage? stop.
				TaskFail();
				return;
			}
			
			//the implosion effect is done in "updateChargeEffect", on checking for this schedule (electric laser).
			turnToFaceEnemyLKP();
			
			if(gpGlobals->time >= chargeFinishTime){
				TaskComplete();
			}
		break;}
		case TASK_KINGPIN_ELECTRIC_LASER_FIRE:{
			//this should not run, complete in StartTask.
		break;}
		case TASK_KINGPIN_ELECTRIC_LASER_END:{
			
			if(m_fSequenceFinishedSinceLoop){
				setPrimaryAttackCooldown();
				TaskComplete();
			}
		break;}



		case TASK_KINGPIN_SUPERBALL_START:{
			if(m_fSequenceFinishedSinceLoop){
				TaskComplete();
			}
		break;}
		case TASK_KINGPIN_SUPERBALL_FIRE:{
			//this should not run, complete in StartTask.
		break;}
		case TASK_KINGPIN_SUPERBALL_END:{
			if(m_fSequenceFinishedSinceLoop){
				setPrimaryAttackCooldown();
				TaskComplete();
			}
		break;}




		case TASK_KINGPIN_SPEED_MISSILE_START:{
			if(m_fSequenceFinishedSinceLoop){
				TaskComplete();
			}
		break;}
		case TASK_KINGPIN_SPEED_MISSILE_FIRE:{
			//this should not run, complete in StartTask.
		break;}
		case TASK_KINGPIN_SPEED_MISSILE_END:{
			if(m_fSequenceFinishedSinceLoop){
				setPrimaryAttackCooldown();
				TaskComplete();
			}
		break;}










		/*
		case TASK_KINGPIN_PSIONIC_CHARGE:

			//ripped houndeye.
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
		
			if(this->m_fSequenceFinished){
				TaskComplete();
			}
			

		break;
		case TASK_KINGPIN_PSIONIC_LAUNCH:

			if(this->m_fSequenceFinished){
				TaskComplete();
			}

		break;
		*/

		default:
			CBaseMonster::RunTask(pTask);
		break;
	}//END OF switch(...)

}//END OF RunTask(...)



BOOL CKingpin::CheckMeleeAttack1( float flDot, float flDist ){
	

	if ( flDist <= 90 && flDot >= 0.75  ) //&& FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) )
	{
		return TRUE;
	}
	return FALSE;



}//END OF CheckMeleeAttack1

BOOL CKingpin::CheckMeleeAttack2( float flDot, float flDist ){
	return FALSE;
}//END OF CheckMeleeAttack2


BOOL CKingpin::CheckRangeAttack1( float flDot, float flDist ){
	BOOL seesEnemy = FALSE;

	//We're going to use RangeAttack1 for the general straight-forward attack methods
	//electric barrage
	//electric laser
	//speed missile

	//When picking an attack method, make sure the actual range is fitting for a given attack.
	// electricBarrage is relatively short-range for instance.
	
	if (blockAttackShockerCooldown && gpGlobals->time < shockerCooldown && flDist < 400) {
		// shocker cooldown is up?  Only interrupt if out of range or the cooldown expires.
		return FALSE;
	}

	blockAttackShockerCooldown = FALSE;  // assumption?  Or do this in GetSchedule at the very start?

	if(this->m_pSchedule == slChaseEnemySmart_StopSight || this->m_pSchedule == slChaseEnemySmart_ShockerFollowup){
		// HOLD ON.  Allowing a ranged attack  will interrupt this schedule.
		// If the enemy got far away enough that is ok, or if this it taking too long.
		if( (flDist > 520.0f) || gpGlobals->time >= giveUpChaseTime){
			//return TRUE;
			//the check below chould still fail.  If the enemy is too far away or occluded the chase can continue.
		}else{
			return FALSE;  //don't block it.
		}
	}


	//There is a cooldown time between direct attacks.
	if(gpGlobals->time >= primaryAttackCooldownTime){
		//pass.
	}else{
		return FALSE;  //not yet
	}


	if(HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_ENEMY_OCCLUDED)){
		//usual way.
		seesEnemy = TRUE;

	}//else if(this->chargeEffect != NULL){
	// Removing the charge effect being present requirement.  The kingpin is one all-seeing bastard.
	else{
		Vector vecStart;
		Vector vecForward;
		Vector vecRight;
		Vector vecUp;
	
			
		UTIL_MakeAimVectorsPrivate(pev->angles, vecForward, vecRight, vecUp);

		//vecStart = pev->origin + vecForward * 18 + Vector(0, 0, 60);
		vecStart = pev->origin + vecUp * CHARGE_POINT_UP + vecForward * CHARGE_POINT_FORWARD;

		//If our charge effect is present, it can also have a line of sight to the enemy.
		//Otherwise sitting and doing nothing while the enemy (player) peeks at it is a little odd.

		//TraceResult tr;
		//UTIL_TraceLine(this->EyePosition(),
		//wait we already have a method for this.
		if(FVisible(vecStart, m_hEnemy->EyePosition())){
			//this also counts.
			//AND HACK: update our m_vecEnemyLKP to be safe.
			setEnemyLKP(m_hEnemy);

			seesEnemy = TRUE;
		}

	}//END OF sight checks.

	


	//Must be seen and not occluded, at least to initiate.
	//Turn off the flDot check, we'll turn to face them soon enough if they're visible in any direction.
	if( seesEnemy &&  flDist >= 0 && flDist <= 2600.0f) ///&& flDot >= 0.8 )
	{
		//HACK - count this as seeing the enemy in case it's indirectly through the beam.
		//This works because "CheckAttacks" is called in basemonster.cpp by "CheckEnemy", which is called by monsterstate.cpp's "runAI" method.
		//The important part is, this (RunAI -> CheckEnemy -> CheckAttacks -> CheckRangeAttack1) gets called AFTER Look (RunAI -> Look),
		//so that this decision to override the bits_COND_SEE_ENEMY getting cleared by not having a direct line of sight lasts (we force it on but it was off)
		//until the MaintainSchedule which calls StartTask / RunTask with the current schedule in mind.
		//But "SetconditionsFrame" could've worked if that weren't the case.  Or maybe not, there would still be ways around that.
		SetConditions(bits_COND_SEE_ENEMY);
		return TRUE;
	}
	

	return FALSE;
}//END OF CheckRangeAttack1

BOOL CKingpin::CheckRangeAttack2( float flDot, float flDist ){
	
	//This is for the Super Ball homing attack, a ball like the head controller ball that uses air nodes for pathfinding.  But still rougly 
	//follows it to be fairly avoidable like the head controller ball.
	

	if(gpGlobals->time >= enemyHiddenResponseTime){
		//allow.
	}else{
		//too much direct action too soon.
		return FALSE;
	}

	if(m_pSchedule == slChaseEnemy || m_pSchedule == slChaseEnemySmart || m_pSchedule == slChaseEnemySmart_StopSight || m_pSchedule == slChaseEnemySmart_ShockerFollowup || m_pSchedule == ::slPathfindStumped){
		//If we're chasing the enemy or stumped, don't interrupt to do these things in any case.
		return FALSE;
	}


	//If the enemy has been hiding for too long, send this to shake things up.
	if(HasConditions(bits_COND_ENEMY_OCCLUDED)){

		return TRUE;
	}


	return FALSE;
}//END OF CheckRangeAttack2



void CKingpin::CustomTouch( CBaseEntity *pOther ){

}


void CKingpin::SetFastThink(BOOL newVal){

	if(usingFastThink != newVal){
		if(newVal){
			// nextthink will be given to nextNormalThinkTime.
			// Then replace it with whichever is sooner: where it was already going, or current time + 0.025
			nextNormalThinkTime = pev->nextthink;
			pev->nextthink = min(pev->nextthink, gpGlobals->time + 0.025);
		}else{
			// restore nextthink.
			pev->nextthink = nextNormalThinkTime;
		}
		usingFastThink = newVal;
	}
}

void CKingpin::MonsterThink( void ){


	//BOOL okayForNormalThink = (!quickThink || gpGlobals->time >= nextNormalThinkTime);
	
	BOOL doNormalThink;

	if(!usingFastThink){
		// for now, always, for projectile-reflect logic to run more often
		SetFastThink(TRUE);
	}

	
	if(usingFastThink){
		if(gpGlobals->time >= nextNormalThinkTime){
			doNormalThink = TRUE;
			nextNormalThinkTime = gpGlobals->time + 0.1;
			// !!! Doing it this way only works if nextNormalThinkTime is set
			// to gpGlobals->time at the time usingFastThink is turned on
			//nextNormalThinkTime = nextNormalThinkTime + 0.1;
		}else{
			doNormalThink = FALSE;
		}
	}else{
		// always.
		doNormalThink = TRUE;
	}




	// !!! NOT CONFINED BY NORMAL THINK!   Do checks for nearby projectiles much more often
	
	if (m_IdealMonsterState != MONSTERSTATE_DEAD) {
		CBaseEntity* pEntityScan = NULL;
		Vector searchStart = EyePosition();
		//if there is some sort of projectile headed towards me, we will try to reflect it.
		while ((pEntityScan = UTIL_FindEntityInSphere(pEntityScan, searchStart, 450)) != NULL) {

			if (!(pEntityScan->pev->flags & FL_KILLME)) {

				if (FClassnameIs(pEntityScan->pev, "bolt")) {
					int x = 666;
				}

				//If this is not scheduled for deletion
				float rangeFactor;
				float reflectDelayFactor;

				const Vector incomingVelocity = pEntityScan->GetVelocityLogical();
				const float incomingSpeed = incomingVelocity.Length();

				const int otherProjType = pEntityScan->GetProjectileType();

				const float distanceToEnt = (searchStart - pEntityScan->pev->origin).Length();

				if (incomingSpeed < 800) {
					//standard is ok.
					reflectDelayFactor = 1.0;
					rangeFactor = 0.65;
				}
				else {
					float filteredIncomingSpeed = min(incomingSpeed, 2000);  //cap me at 2000 for safety.
					//climbs up to 2000.
					reflectDelayFactor = (1.0 - (((filteredIncomingSpeed - 800) / 1200) * 0.97));
					rangeFactor = 0.65 + ((filteredIncomingSpeed - 800) / 1200) * 0.35;
				}


				/*
				switch(otherProjType){
				case PROJECTILE_NONE:
					//can't work with that?

				break;
				case PROJECTILE_BOLT:

				break;
				case PROJECTILE_GRENADE:

				break;
				case PROJECTILE_ROCKET:

				break;
				case PROJECTILE_ENERGYBALL:

				break;
				case PROJECTILE_ORGANIC_DUMB:

				break;
				case PROJECTILE_ORGANIC_HARMLESS:

				break;
				case PROJECTILE_ORGANIC_HOSTILE:

				break;
				}//END OF switch
				*/

				//reflectDelayFactor
				//rangeFactor

				if (otherProjType > PROJECTILE_NONE && incomingSpeed > 0.1f && distanceToEnt < (rangeFactor * 450)) {
					//also, is it coming towards me? This should also avoid trying to re-reflect projectiles already reflected
					//but still in reflection range for a little bit.
					const char* entityName = pEntityScan->getClassname();


					const float zSpeed = incomingVelocity.z;
					const float distanceZ = fabs((searchStart.z - pEntityScan->pev->origin.z));
					const float distance2D = (searchStart - pEntityScan->pev->origin).Length2D();

					const Vector2D incomingDirection = incomingVelocity.Make2D().Normalize();
					const Vector2D directionTowardsMe = (searchStart - pEntityScan->pev->origin).Make2D().Normalize();
					const float closenessToApproach = DotProduct(incomingDirection, directionTowardsMe);


					//Two cases.  Either it's about to land near me regardless of the direction it's going (unlikely but possible),
					//OR it's headed close enough to towards me.

					if (
						//If the projectile is close enough to moving towards this kingpin,
						closenessToApproach > 0.45f ||
						//or it's bouncing and too close to landing to me...
						((pev->movetype == MOVETYPE_TOSS || pev->movetype == MOVETYPE_BOUNCE) && (distance2D < (400 * rangeFactor) && zSpeed < -80 && distanceZ < 500 * rangeFactor))

					){
						attemptReflectProjectileStart(pEntityScan, reflectDelayFactor);
					}

				}//END OF "is projectile" check and moving check


			}//END OF fl_killme check
		}//END OF while loop through nearby entities
	}//END OF dead check





	if (doNormalThink) {

		if (m_MonsterState == MONSTERSTATE_COMBAT) {
			//Periodically power up monstesr and make them target my enemy.

			if (powerUpNearbyMonstersCooldown == -1 || gpGlobals->time >= powerUpNearbyMonstersCooldown) {
				easyForcePrintLine("KINGPIN: I licked THE COUCH");
				this->powerUpMonsters();
				powerUpNearbyMonstersCooldown = gpGlobals->time + 8;
			}


			if (m_pSchedule == slCombatFaceNoStump || m_pSchedule == slCombatFace || m_pSchedule == slCombatLook) {
				//do a check.  Has enemyHiddenChaseTime been surpassed?
				if (gpGlobals->time >= enemyHiddenChaseTime) {
					//just force it to stop from happening again too soon in any case.
					//enemyHiddenChaseTime = gpGlobals->time + 40.0f;

					//TaskFail();  //re-pick a following schedule most likely.
					//or ChangeSchedule into that?  what is best.
				}
			}

			if (
				//that is, if a monster recently damaged me, or our long-time cooldown is up....
				(recentInflictingMonster != NULL || (m_hEnemy != NULL && (forceEnemyOnPoweredUpMonstersCooldown == -1 || gpGlobals->time >= forceEnemyOnPoweredUpMonstersCooldown))) &&

				//And the hard cooldown (to avoid spamming the logic) passes:
				(forceEnemyOnPoweredUpMonstersHardCooldown == -1 || gpGlobals->time >= forceEnemyOnPoweredUpMonstersHardCooldown))
			{

				CBaseEntity* enemyToForce;
				BOOL forceEnemyPassive;

				if (recentInflictingMonster != NULL) {
					enemyToForce = recentInflictingMonster;
					forceEnemyPassive = FALSE;
				}
				else {
					enemyToForce = m_hEnemy;
					forceEnemyPassive = TRUE;
				}


				this->forceEnemyOnPoweredUpMonsters(enemyToForce, forceEnemyPassive);

				forceEnemyOnPoweredUpMonstersCooldown = gpGlobals->time + 15;
				forceEnemyOnPoweredUpMonstersHardCooldown = gpGlobals->time + 2.5;
			}


			if (forgetRecentInflictingMonsterCooldown != -1 && gpGlobals->time >= forgetRecentInflictingMonsterCooldown) {
				recentInflictingMonster = NULL;
				forgetRecentInflictingMonsterCooldown = -1;
			}

		}//END OF combat state check


		CBaseMonster::MonsterThink();

		//if(this->HasConditionsEither(bits_COND_SEE_ENEMY) && !this->HasConditionsEither(bits_COND_ENEMY_OCCLUDED) ){
		if (this->HasConditionsEither(bits_COND_SEE_ENEMY)) {
			//can see the enemy? not occluded?  Then reset the hidden time.

			enemyHiddenResponseTime = gpGlobals->time + 12.0f;
			enemyHiddenChaseTime = gpGlobals->time + 40.0f;
		}

		UpdateBeams();
		updateChargeEffect();
		UpdateReflectEffects();

	}//END OF doNormalThink



	//easyForcePrintLine("THE TIMES %.2f : %.2f %d", gpGlobals->time, nextNormalThinkTime, doNormalThink);

	if(usingFastThink){
		pev->nextthink = gpGlobals->time + 0.025;
	}
	// Why would this happen??  Don't report it when we're being deleted (supposed to happen)
	if (pev->nextthink <= 0 && !(pev->flags & FL_KILLME) ) {
		easyForcePrintLine("!!!WARNING!  Kingpin had a pev->nextthink at or below 0. This can kill the AI!");
		// save it.
		pev->nextthink = gpGlobals->time + 0.1;
	}
}//MonsterThink



int CKingpin::Classify( void ){
	return CLASS_ALIEN_MONSTER;
}
BOOL CKingpin::isOrganic(void){
	return TRUE;
}

int CKingpin::IRelationship( CBaseEntity *pTarget ){

	return CBaseMonster::IRelationship(pTarget);
}//END OF IRelationship(...)


void CKingpin::ReportAIState(void){
	//call the parent, and add on to that.
	CBaseMonster::ReportAIState();
	//print anything special with easyForcePrintLine
}//END OF ReportAIState()



GENERATE_TRACEATTACK_IMPLEMENTATION(CKingpin){
	easyPrintLine("kingpin ID%d: I WAS JUST HIT AT HITBOX:%d", this->monsterID, ptr->iHitgroup);

	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}


GENERATE_TAKEDAMAGE_IMPLEMENTATION(CKingpin){
	//Alert poweredupmonsters of this occurence in MonsterThink by the "recentlyDamage" feature!
	if(this->pev->deadflag == DEAD_NO && pevInflictor != NULL){
		CBaseEntity* entityTest = CBaseEntity::Instance(pevInflictor);
		//easyForcePrintLine("WELL???? %d %d", (entityTest != NULL), (CBaseEntity::Instance(pevAttacker) != NULL) );
		if(entityTest != NULL){
			CBaseMonster* monsterTest = entityTest->GetMonsterPointer();
			if(monsterTest != NULL && IRelationship(monsterTest) >= R_DL){
				//Require disliking the attacking monster in the first place in case this was just friendly fire.
				recentInflictingMonster = monsterTest;
				forgetRecentInflictingMonsterCooldown = gpGlobals->time + 4;
			}
		}
	}

	//generally fine?
	int taskNumba = getTaskNumber();
	if(taskNumba == TASK_KINGPIN_ELECTRIC_BARRAGE_LOOP || taskNumba == TASK_KINGPIN_ELECTRIC_LASER_CHARGE){

		//non-timed damage only counts for this feature.

		if( !(bitsDamageTypeMod & (DMG_TIMEDEFFECT|DMG_TIMEDEFFECTIGNORE)) ){
			//These are interruptable by //accumulatedDamageTaken
			accumulatedDamageTaken += flDamage;
			//also cut the actual damage received in half while charging.  Our goal was to interrupt the attack.
			flDamage = flDamage / 2.0f;
		
		}//END OF non-timed damage check
	}

	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}


void CKingpin::OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){
	
	//MODDD - intervention. Timed damage might not affect the AI since it could get needlessly distracting.
	if(bitsDamageTypeMod & (DMG_TIMEDEFFECT|DMG_TIMEDEFFECTIGNORE) ){
		//If this is continual timed damage, don't register as any damage condition. Not worth possibly interrupting the AI.
		return;
	}

	//default case from CBaseMonster's TakeDamage.
	//Also count being in a non-combat state to force looking in that direction.  But maybe at least 0 damage should be a requirement too, even in cases where the minimum damage for LIGHT is above 0?
	if (m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT || flDamage > 0 )
	{
		SetConditions(bits_COND_LIGHT_DAMAGE);

		//MODDD NEW - set a timer to forget a flinch-preventing memory bit.
		forgetSmallFlinchTime = gpGlobals->time + 9.0f;
	}

	//MODDD - HEAVY_DAMAGE was unused before.  For using the BIG_FLINCH activity that is (never got communicated)
	//    Stricter requirement:  this attack took 70% of health away.
	//    The agrunt used to use this so that its only flinch was for heavy damage (above 20 in one attack), but that's easy by overriding this OnTakeDamageSetconditions method now.
	//    Keep it to using light damage for that instead.
	//if ( flDamage >= 20 )
	if( gpGlobals->time >= forgetBigFlinchTime && (flDamage >=  pev->max_health * 0.55 || flDamage >= 30)  )
	{
		SetConditions(bits_COND_HEAVY_DAMAGE);
		forgetSmallFlinchTime = gpGlobals->time + 9.0f;
		forgetBigFlinchTime = gpGlobals->time + 15.0f;
	}
}//END OF OnTakeDamageSetConditions


GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(CKingpin){

	return GENERATE_DEADTAKEDAMAGE_PARENT_CALL(CBaseMonster);
}

//Parameters: integer named fGibSpawnsDecal
GENERATE_GIBMONSTER_IMPLEMENTATION(CKingpin)
{
	
	// it's just a good idea
	// Moved this to "OnDelete" for completeness.
	//stopElectricBarrageLoopSound();

	GENERATE_GIBMONSTER_PARENT_CALL(CBaseMonster);
}



//if dead and a poweredup monster's directedEnemyIssuer is THIS, forget its directedEnemy. !!!
GENERATE_KILLED_IMPLEMENTATION(CKingpin){
	
	//BOOL firstCall = FALSE;
	if(!HasMemory( bits_MEMORY_KILLED )){
		//firstCall = TRUE;
		//no need to re-do this check again.
		deTargetMyCommandedMonsters();
	}

	/*
	if(pev->deadflag == DEAD_NO){
		//keep this in mind...
		firstCall = TRUE;
	}
	*/

	//MODDD - is still doing here ok?
	GENERATE_KILLED_PARENT_CALL(CBaseMonster);


	/*
	//if you have the "FL_KILLME" flag, it means this is about to get deleted (gibbed). No point in doing any of this then.
	if(firstCall && !(pev->flags & FL_KILLME) ){
		cheapKilled();
	}//END OF firstCall check
	*/
	
	ClearBeams();
	ClearReflectEffects();
	removeChargeEffect();
	// it's just a good idea
	stopElectricBarrageLoopSound();
	
}//END OF Killed


void CKingpin::onDelete(void){
	ClearBeams();
	removeChargeEffect();

	stopElectricBarrageLoopSound();
}



void CKingpin::SetYawSpeed( void ){
	

	


	switch(pev->sequence){

		case KINGPIN_ATTACK_LEFT:
		case KINGPIN_ATTACK_RIGHT:
		case KINGPIN_ATTACK_BOTH:
			pev->yaw_speed = 110;
		break;


		case KINGPIN_MAGE_START:
			pev->yaw_speed = 12;
		break;
		case KINGPIN_MAGE_LOOP:
			pev->yaw_speed = 48;
		break;
		case KINGPIN_MAGE_END:
			pev->yaw_speed = 12;
		break;


		default:
			pev->yaw_speed = 140;
		break;

	}//END OF switch


	if(IsMoving()){
		//don't allow us to get stuck. Allow at least some yaw speed.
		if(pev->yaw_speed == 0){
			pev->yaw_speed = 48;
		}
	}



	
}//END OF SetYawSpeed(...)





BOOL CKingpin::getMonsterBlockIdleAutoUpdate(void){
	return FALSE;
}
BOOL CKingpin::forceIdleFrameReset(void){
	return FALSE;
}
BOOL CKingpin::usesAdvancedAnimSystem(void){
	return TRUE;
}

void CKingpin::SetActivity( Activity NewActivity ){
	CBaseMonster::SetActivity(NewActivity);
}



int CKingpin::LookupActivityHard(int activity){
	int i = 0;
	m_flFramerateSuggestion = 1;
	m_iForceLoops = -1;
	pev->framerate = 1;
	//any animation events in progress?  Clear it.
	resetEventQueue();

	//Within an ACTIVITY, pick an animation like this (with whatever logic / random check first):
	//    this->animEventQueuePush(10.0f / 30.0f, 3);  //Sets event #3 to happen at 1/3 of a second
	//    return LookupSequence("die_backwards");      //will play animation die_backwards

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){

		case ACT_SMALL_FLINCH:{
			//There are two ACTs for flinching but the first one's broken.
			//Just force the second one.
			this->m_flFramerateSuggestion = 1.5f;
			return KINGPIN_SMALL_FLINCH2;
		break;}


		case ACT_BIG_FLINCH:{
			//unless a unique BIG_FLINCH mapped sequence shows up, I'll  just use the small flinch one(s) slowed down a little
			//in comparison.
			this->m_flFramerateSuggestion = 1.1f;
			return KINGPIN_SMALL_FLINCH2;
		break;}

		case ACT_MELEE_ATTACK1:{
			
			//TODO - events.

			switch(RANDOM_LONG(0, 2)){
				case 0:
					
					this->animEventQueuePush(10.0f/30.0f, 0);

					return KINGPIN_ATTACK_LEFT;
				break;
				case 1:
					
					this->animEventQueuePush(10.0f/30.0f, 1);

					return KINGPIN_ATTACK_RIGHT;
				break;
				case 2:

					
					this->animEventQueuePush(7.0f/30.0f, 2);

					return KINGPIN_ATTACK_BOTH;
				break;
			}//END OF switch

		break;}

		case ACT_WALK:{
			return KINGPIN_WALK;
		break;}
		case ACT_RUN:{
			return KINGPIN_RUN;
		break;}

					 
		//Change these if turn animations are given. These are picking the IDLE1 to say to change it if significant turning is requested.
		case ACT_TURN_LEFT:{
			return KINGPIN_IDLE1;
		break;}
		case ACT_TURN_RIGHT:{
			return KINGPIN_IDLE1;
		break;}


		
		/*
		case ACT_MELEE_ATTACK1:
			//pev->flags &= ~EF_NOINTERP;
			//pev->effects |= EF_NOINTERP;
			m_flFramerateSuggestion = 6;
			this->animEventQueuePush(30.0f/22.0f, 0);
			this->m_iForceLoops = 0;
			//pev->framerate = 6;
			return KINGPIN_SCYTHE;
		break;
		*/
		/*
		case ACT_RANGE_ATTACK1:
			m_flFramerateSuggestion = 3;
			this->animEventQueuePush(60.0f/22.0f, 1);
			return KINGPIN_PSIONIC_LAUNCH;
		break;
		*/

	}//END OF switch(...)
	
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard(...)


int CKingpin::tryActivitySubstitute(int activity){
	int i = 0;

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		

		case ACT_SMALL_FLINCH:{
			//There are two ACTs for flinching but the first one's broken.
			//Just force the second one.
			return KINGPIN_SMALL_FLINCH2;
		break;}

		case ACT_BIG_FLINCH:{
			//There are two ACTs for flinching but the first one's broken.
			//Just force the second one.
			return KINGPIN_SMALL_FLINCH2;
		break;}

		case ACT_MELEE_ATTACK1:{
			//no need for random logic.  Just say we have one.
			return KINGPIN_ATTACK_BOTH;
		break;}
		case ACT_WALK:{
			return KINGPIN_WALK;
		break;}
		case ACT_RUN:{
			return KINGPIN_RUN;
		break;}

		//Change these if turn animations are given. These are picking the IDLE1 to say to change it if significant turning is requested.
		case ACT_TURN_LEFT:{
			return KINGPIN_IDLE1;
		break;}
		case ACT_TURN_RIGHT:{
			return KINGPIN_IDLE1;
		break;}
		
		/*
		case ACT_RANGE_ATTACK1:
			return KINGPIN_PSIONIC_LAUNCH;
		break;
		*/
	}//END OF switch(...)


	//not handled by above? We're not using the script to determine animation then. Rely on the model's anim for this activity if there is one.
	return CBaseAnimating::LookupActivity(activity);
}//END OF tryActivitySubstitute(...)

//Handles custom events sent from "LookupActivityHard", which sends events as timed delays along with picking an animation in script.
//So this handles script-provided events, not model ones.
void CKingpin::HandleEventQueueEvent(int arg_eventID){

	switch(arg_eventID){
	case 0:{
		//scythe attack, left.
		//TODO - custom damage for the kingpin's attacks?
		CBaseEntity *pHurt = CheckTraceHullAttack( 98, gSkillData.zombieDmgOneSlash * 1.4f, DMG_SLASH, DMG_BLEEDING );
		if ( pHurt )
		{
			if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact()  )
			{
				pHurt->pev->punchangle.z = -11;
				pHurt->pev->punchangle.x = -16;
				//pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 40 + gpGlobals->v_up * 23 - gpGlobals->v_right * 15;
			}
			// Play a random attack hit sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
		}
		else // Play a random attack miss sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

		if (RANDOM_LONG(0,1))
			AttackSound();

	break;}
	case 1:{
		//scythe attack, right.
		//TODO - custom damage for the kingpin's attacks?
		CBaseEntity *pHurt = CheckTraceHullAttack( 98, gSkillData.zombieDmgOneSlash * 1.2f, DMG_SLASH, DMG_BLEEDING );
		if ( pHurt )
		{
			if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact()  )
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = -7;
				//pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 40 + gpGlobals->v_up * 23 - gpGlobals->v_right * 15;
			}
			// Play a random attack hit sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
		}
		else // Play a random attack miss sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

		if (RANDOM_LONG(0,1))
			AttackSound();

	break;}
	case 2:{
		//scythe attack, both.
		//TODO - custom damage for the kingpin's attacks?
		CBaseEntity *pHurt = CheckTraceHullAttack( 98, gSkillData.zombieDmgOneSlash * 1.8f, DMG_SLASH, DMG_BLEEDING );
		if ( pHurt )
		{
			if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact()  )
			{
				pHurt->pev->punchangle.z = -27;
				pHurt->pev->punchangle.x = 4;
				//pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 40 + gpGlobals->v_up * 23 - gpGlobals->v_right * 15;
			}
			// Play a random attack hit sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
		}
		else // Play a random attack miss sound
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

		if (RANDOM_LONG(0,1))
			AttackSound();

	break;}



	case 3:{
		
		//launchSuperBall();


	break;}

	case 4:{
		//



	break;}



	}//END OF switch(...)


}//END OF HandleEventQueueEvent(...)


//This handles events built into the model, not custom hard-coded ones (above).
void CKingpin::HandleAnimEvent(MonsterEvent_t *pEvent ){
	switch( pEvent->event ){
	case 0:
	{

	break;
	}

	default:
		CBaseMonster::HandleAnimEvent( pEvent );
	break;
	}//END OF switch(...)
}






CBaseMonster* CKingpin::findClosestPowerupableMonster(void){

	CBaseEntity* pEntityScan = NULL;
	CBaseMonster* testMon = NULL;
	float thisDistance;
	CBaseMonster* bestChoiceYet = NULL;

	const float flMaxSearchDist = KINGPIN_POWERUP_SEARCH_RANGE;

	float leastDistanceYet = flMaxSearchDist; //furthest I go

	//does UTIL_MonstersInSphere work?
	while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, pev->origin, flMaxSearchDist )) != NULL)
	{
		if(pEntityScan->pev == this->pev){
			//is it me? skip it.
			continue;
		}

		testMon = pEntityScan->MyMonsterPointer();
		//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){

		if(testMon != NULL){
			//EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: is corpse ok? classname:%s df:%d, isgi:%d, nl:%d", FClassname(testMon), testMon->pev->deadflag, testMon->isSizeGiant(), FClassnameIs(testMon->pev, "monster_leech") ) );
		}else{
			////EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: is corpse ok? beep NO NOT A MONSTER: classname:%s", FClassname(pEntityScan) ) );
		}
		
		if(testMon != NULL && (testMon->pev->deadflag == DEAD_NO) && (::FClassnameIs(testMon->pev, "monster_alien_grunt") ) ){
			thisDistance = (testMon->pev->origin - pev->origin).Length();
			if(thisDistance < leastDistanceYet){
				bestChoiceYet = testMon;
				leastDistanceYet = thisDistance;
				
			}//END OF minimum distance yet
		}//END OF entity scan null check

	}//END OF while loop through all entities
	return bestChoiceYet;

}//END OF getNearestDeadBody



void CKingpin::powerUpMonsters(void){

	
	CBaseEntity* pEntityScan = NULL;
	CBaseMonster* testMon = NULL;
	float thisDistance;
	CBaseMonster* bestChoiceYet = NULL;

	const float flMaxSearchDist = KINGPIN_POWERUP_APPLY_RANGE;


	//does UTIL_MonstersInSphere work?
	while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, pev->origin, flMaxSearchDist )) != NULL)
	{
		if(pEntityScan->pev == this->pev){
			//is it me? skip it.
			continue;
		}

		testMon = pEntityScan->MyMonsterPointer();
		//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){

		if(testMon != NULL){
			//EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: is corpse ok? classname:%s df:%d, isgi:%d, nl:%d", FClassname(testMon), testMon->pev->deadflag, testMon->isSizeGiant(), FClassnameIs(testMon->pev, "monster_leech") ) );
		}else{
			////EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: is corpse ok? beep NO NOT A MONSTER: classname:%s", FClassname(pEntityScan) ) );
		}


		/*
		if(testMon != NULL){
			easyForcePrintLine("HELLO friendFACE %s", FClassname(testMon));
		
			if(FClassnameIs(testMon->pev, "monster_alien_grunt")){
				easyForcePrintLine("AM I oh???? ", testMon->monsterID);
			}
		}*/
		

		if(testMon != NULL && (testMon->pev->deadflag == DEAD_NO) && (::FClassnameIs(testMon->pev, "monster_alien_grunt") ) ){
			//thisDistance = (testMon->pev->origin - pev->origin).Length();
			//if(thisDistance < KINGPIN_POWERUP_APPLY_RANGE){
			//no duh.

			easyForcePrintLine("ILL probably not %d", testMon->monsterID);
			//This can reset the powerup duration for monsters that are already powered up.
			testMon->setPoweredUpOn(this, 12);


			//}//END OF minimum distance yet



		}//END OF entity scan null check

	}//END OF while loop through all entities



}//END OF powerUpMonsters





void CKingpin::removeFromPoweredUpCommandList(CBaseMonster* argToRemove){

	//yadda yadda. dropped feature.
	

}


//When a Kingpin dies, it needs to tell monsters it gave an attack order to forget it.
void CKingpin::deTargetMyCommandedMonsters(void){

	
	CBaseEntity* pEntityScan = NULL;
	CBaseMonster* testMon = NULL;
	float thisDistance;
	CBaseMonster* bestChoiceYet = NULL;

	const float flMaxSearchDist = KINGPIN_POWERUP_APPLY_RANGE*2;


	//does UTIL_MonstersInSphere work?
	while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, pev->origin, flMaxSearchDist )) != NULL)
	{
		if(pEntityScan->pev == this->pev){
			//is it me? skip it.
			continue;
		}

		testMon = pEntityScan->MyMonsterPointer();
		//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){

		if(testMon != NULL){
			//EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: is corpse ok? classname:%s df:%d, isgi:%d, nl:%d", FClassname(testMon), testMon->pev->deadflag, testMon->isSizeGiant(), FClassnameIs(testMon->pev, "monster_leech") ) );
		}else{
			////EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: is corpse ok? beep NO NOT A MONSTER: classname:%s", FClassname(pEntityScan) ) );
		}
		
		
		if(testMon != NULL && (testMon->pev->deadflag == DEAD_NO) && (::FClassnameIs(testMon->pev, "monster_alien_grunt") ) ){
			testMon->forgetForcedEnemy(this, FALSE);
		}//END OF entity scan null check

	}//END OF while loop through all entities


}//END OF deTargetMyCommandedMonsters


void CKingpin::forceEnemyOnPoweredUpMonsters(CBaseEntity* monsterToForce, BOOL argPassive){
	
	CBaseEntity* pEntityScan = NULL;
	CBaseMonster* testMon = NULL;
	float thisDistance;
	CBaseMonster* bestChoiceYet = NULL;

	const float flMaxSearchDist = KINGPIN_POWERUP_APPLY_RANGE*2;
	
	//does UTIL_MonstersInSphere work?
	while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, pev->origin, flMaxSearchDist )) != NULL)
	{
		if(pEntityScan->pev == this->pev){
			//is it me? skip it.
			continue;
		}

		testMon = pEntityScan->MyMonsterPointer();
		//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){

		if(testMon != NULL){
			//EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: is corpse ok? classname:%s df:%d, isgi:%d, nl:%d", FClassname(testMon), testMon->pev->deadflag, testMon->isSizeGiant(), FClassnameIs(testMon->pev, "monster_leech") ) );
		}else{
			////EASY_CVAR_PRINTIF_PRE(friendlyPrintout, easyPrintLine("Friendly: is corpse ok? beep NO NOT A MONSTER: classname:%s", FClassname(pEntityScan) ) );
		}
		
		
		if(testMon != NULL && (testMon->pev->deadflag == DEAD_NO) && (::FClassnameIs(testMon->pev, "monster_alien_grunt") ) ){
			testMon->forceNewEnemy(this, monsterToForce, FALSE);
		}//END OF entity scan null check

	}//END OF while loop through all entities


}//END OF forceEnemyOnPoweredUpMonsters



// this is being used for something else instead.
void CKingpin::playPsionicLaunchSound(){
	switch(RANDOM_LONG(0, 2)){
	case 0:UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "houndeye/he_blast1.wav", 1, ATTN_NORM + 0.2, 0, 80 + RANDOM_LONG(0, 5));break;
	case 1:UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "houndeye/he_blast2.wav", 1, ATTN_NORM + 0.2, 0, 80 + RANDOM_LONG(0, 5));break;
	case 2:UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "houndeye/he_blast3.wav", 1, ATTN_NORM + 0.2, 0, 80 + RANDOM_LONG(0, 5));break;
	}
}//END OF playPsionicLaunchSound()




void CKingpin::ScheduleChange(void){

	// EHhhhh paranoia, have this check anyway
	Schedule_t* endSchedule = this->m_pSchedule;

	// ?
	// Could this cause the end-sounds to ever play twice?  Maybe some kind of BOOL for
	// having played it recently would be good.

	if(endSchedule == slKingpinElectricBarrage){
		int taskNumba = getTaskNumber();
		stopElectricBarrageLoopSound();
		if(taskNumba == TASK_KINGPIN_ELECTRIC_BARRAGE_START || taskNumba == TASK_KINGPIN_ELECTRIC_BARRAGE_LOOP){
			playElectricBarrageEndSound();
		}
		removeChargeEffect();  //if it is up.  does nothing if not.
	}

	if(endSchedule == slKingpinElectricLaser){
		int taskNumba = getTaskNumber();
		stopElectricLaserChargeSound();
		if(taskNumba == TASK_KINGPIN_ELECTRIC_LASER_FIRE || taskNumba == TASK_KINGPIN_ELECTRIC_LASER_CHARGE){
			playElectricBarrageEndSound();
		}
		removeChargeEffect();
	}

	

	CBaseMonster::ScheduleChange();

}//END OF ScheduleChange




/*
void CKingpin::playSuperBallStartSound(void){
	int pitch = 100;
	
	UTIL_PlaySound( edict(), CHAN_VOICE, "ambience/alien_hollow.wav", 1.0, ATTN_NORM, 0, pitch );
}

*/





void CKingpin::playForceFieldReflectSound(void){
	int pitch = 150 + RANDOM_LONG(-5, 5);
	
	UTIL_PlaySound( edict(), CHAN_VOICE, "debris/beamstart4.wav", 1.0, ATTN_NORM - 0.3f, 0, pitch );
}



void CKingpin::playSuperBallFireSound(void){
	int pitch = 100 + RANDOM_LONG(-5, 5);
	
	UTIL_PlaySound( edict(), CHAN_VOICE, "x/x_shoot1.wav", 1.0, ATTN_NORM - 0.6f, 0, pitch );
}


void CKingpin::playElectricBarrageStartSound(void){
	int pitch = 112;
	UTIL_PlaySound( edict(), CHAN_VOICE, "ambience/particle_suck1.wav", 1.0, ATTN_NORM - 0.6f, 0, pitch );
}//END OF playElectricBarrageStartSound


void CKingpin::playElectricBarrageLoopSound(void){
	int pitch = 110;
	
	//x/x_teleattack1.wav  ???  pitch lower or higher maybe?
	
	UTIL_PlaySound( edict(), CHAN_STATIC, "ambience/zapmachine.wav", 1.0, ATTN_NORM - 0.24f, 0, pitch );
}//END OF playElectricBarrageStartSound

void CKingpin::stopElectricBarrageLoopSound(void){
	UTIL_StopSound( edict(), CHAN_STATIC, "ambience/zapmachine.wav");
}



void CKingpin::playElectricBarrageEndSound(void){
	int pitch = 90 + RANDOM_LONG(0,4);
	UTIL_PlaySound( edict(), CHAN_STATIC, RANDOM_SOUND_ARRAY(pElectricBarrageEndSounds), 1.0, ATTN_NORM - 0.5f, 0, pitch );
	
}//END OF playElectricBarrageEndSound



void CKingpin::playElectricBarrageFireSound(void){
	//maybe don't play a sound for this, it's spammy enough as it is.

	//int pitch = 180;
	//UTIL_PlaySound( edict(), CHAN_WEAPON, "weapons/gauss2.wav", 1.0, ATTN_NORM, 0, pitch, FALSE );
	//pElectricBarrageFireSounds ????
}

void CKingpin::playElectricBarrageHitSound(CBaseEntity* arg_target, const Vector& arg_location){
	int pitch = 158 + RANDOM_LONG(0, 24);
	
	edict_t* toSend;

	if(arg_target != NULL){
		toSend = arg_target->edict();
	}else{
		toSend = edict();
	}
	
	switch(RANDOM_LONG(0, 2)){
	case 0:
		//precached by the client always, so don't use the soundSentenceSave system for this one.
		UTIL_EmitAmbientSound( toSend, arg_location, "weapons/electro4.wav", 0.74f, ATTN_NORM, 0, pitch, FALSE );
	break;
	case 1:
		UTIL_EmitAmbientSound( toSend, arg_location, "weapons/electro5.wav", 0.74f, ATTN_NORM, 0, pitch, FALSE );
	break;
	case 2:
		UTIL_EmitAmbientSound( toSend, arg_location, "weapons/electro6.wav", 0.74f, ATTN_NORM, 0, pitch, FALSE );
	break;
	}//END OF switch

}




void CKingpin::playElectricLaserChargeSound(void){
	int pitch = 89;
	
	UTIL_PlaySound( edict(), CHAN_WEAPON, "weapons/mine_charge.wav", 1.0, ATTN_NORM - 0.6f, 0, pitch, FALSE );

}

void CKingpin::stopElectricLaserChargeSound(void){
	int pitch = 89;
	
	//UTIL_StopSound( ENT(pev), CHAN_WEAPON, "debris/zap4.wav" );
	UTIL_StopSound( edict(), CHAN_WEAPON, "weapons/mine_charge.wav", FALSE);

}







void CKingpin::playElectricLaserFireSound(void){
	int pitch = 88;
	//UTIL_PlaySound( edict(), CHAN_WEAPON, "weapons/gauss2.wav", 1.0, ATTN_NORM - 0.7f, 0, pitch, FALSE );
	UTIL_PlaySound( edict(), CHAN_WEAPON, "weapons/beamstart15.wav", 1.0, ATTN_NORM - 0.7f, 0, pitch, FALSE );
	
	//or maybe one of the other beamstart#.wav's from pElectricBarrageFireSounds since that's no longer used for electric barrage firing?
	
	if (RANDOM_LONG(0,1))
		AttackSound();
}

//That is, at the location hit, regardless of hitting anything organic or not.
void CKingpin::playElectricLaserHitSound(CBaseEntity* arg_target, const Vector& arg_location){
	int pitch = 102;
	//UTIL_PlaySound( edict(), CHAN_VOICE, "garg/gar_stomp1.wav", 1.0, ATTN_NORM, 0, pitch );

	
	edict_t* toSend;

	if(arg_target != NULL){
		toSend = arg_target->edict();
	}else{
		toSend = edict();
	}


	UTIL_EmitAmbientSound( toSend, arg_location, "garg/gar_stomp1.wav", 1.0, ATTN_NORM - 0.18f, 0, pitch, TRUE );

	//precached by the client always, so don't use the soundSentenceSave system for this one.
	switch(RANDOM_LONG(0, 2)){
	case 0:
		UTIL_EmitAmbientSound( toSend, arg_location, "weapons/electro4.wav", 1.0, ATTN_NORM, 0, pitch, FALSE );
	break;
	case 1:
		UTIL_EmitAmbientSound( toSend, arg_location, "weapons/electro5.wav", 1.0, ATTN_NORM, 0, pitch, FALSE );
	break;
	case 2:
		UTIL_EmitAmbientSound( toSend, arg_location, "weapons/electro6.wav", 1.0, ATTN_NORM, 0, pitch, FALSE );
	break;
	}//END OF switch
	
}



CBeam*& CKingpin::getNextBeam(void){
	int returnID;

	if(m_pBeam[m_iBeams] != NULL){
		//remove it first.
		UTIL_Remove( m_pBeam[m_iBeams] );
		m_pBeam[m_iBeams] = NULL;
	}

	returnID = m_iBeams;

	//assuming we're using this now.
	m_flBeamExpireTime[returnID] = gpGlobals->time + 0.2f;

	//anticipate needing the next beam ID.
	m_iBeams++;

	if (m_iBeams >= KINGPIN_MAX_BEAMS){
		m_iBeams = 0;  //start at 0 next time.
	}

	return m_pBeam[returnID];
}




//similar to ClearBeams, but forces all m_pBeam's to NULL for safety.
//Only to be used at startup (CKingpin constructor, anything unused must be NULL,
//garbage memory or references won't be good)
void CKingpin::SetupBeams(void){
	for(int i = 0; i < KINGPIN_MAX_BEAMS; i++){
		m_pBeam[i] = NULL;
		m_flBeamExpireTime[i] = 0;
	}
	m_iBeams = 0;
}//END OF SetupBeams

void CKingpin::ClearBeams(void){
	int i;
	for(i = 0; i < KINGPIN_MAX_BEAMS; i++){
		if(m_pBeam[i] != NULL){
			UTIL_Remove( m_pBeam[i] );
			m_pBeam[i] = NULL;
			m_flBeamExpireTime[i] = 0;
		}
	}//END OF for loop through beams
	m_iBeams = 0;
	//pev->skin = 0;

	//UTIL_StopSound( ENT(pev), CHAN_WEAPON, "debris/zap4.wav", FALSE );
}//END OF ClearBeams

//Call me every frame of think logic.  Do any beams need to be cleaned up?
void CKingpin::UpdateBeams(void){
	int i;
	for(i = 0; i < KINGPIN_MAX_BEAMS; i++){
		if(m_pBeam[i] != NULL){
			//has this been surpassed?
			if(gpGlobals->time >= m_flBeamExpireTime[i]){
				UTIL_Remove( m_pBeam[i] );
				m_pBeam[i] = NULL;
			}//END OF time check
		}//END OF null check
	}//END OF for loop through beams

}//END OF UpdateBeams







/*
	m_pEntityToReflect
	m_pReflectEffect
	m_flReflectEffectExpireTime
	m_iReflectEffect
*/
	

//CSprite** arg_reflectEffect, EHANDLE* arg_entityToReflect
int CKingpin::getNextReflectHandleID(void){

	//TOdo: return an open ID insyread. find one thru all, no overwriting / deleting anythong in progress.
	//return -1 for none avail.
	//or this works too.?

	int returnID = -1;
	int i;
	int i_raw;

	for(i = m_iReflectEffect, i_raw = 0; i_raw < KINGPIN_MAX_REFLECTEFFECT; i++, i_raw++){
		
		if(i >= KINGPIN_MAX_REFLECTEFFECT){
			//LOOP AROUND
			i = 0;
		}

		//if(m_flReflectEffectExpireTime[i] == 0 || gpGlobals->time > m_flReflectEffectExpireTime[i]){
		if(m_pReflectEffect[i] == NULL && m_pEntityToReflect[i] == NULL){
			// if not set or already used, this is ok to grab.
			returnID = i;
			// next to start from.
			m_iReflectEffect = (i + 1) % KINGPIN_MAX_REFLECTEFFECT;
			break;
		}

	}//END OF for

	
	if(returnID != -1){
		//success, return pointers to those items.
		//Leave it up to the caller to set cooldown or whatever.
		return returnID;
	}else{
		//oh well.
		return -1;
	}

	//assuming we're using this now... no
	//m_flReflectEffectExpireTime[returnID] = gpGlobals->time + 0.2f;


}//END OF getNextReflectHandleID




//similar to ClearBeams, but forces all m_pReflectEffect's to NULL for safety.
//Only to be used at startup (CKingpin constructor, anything unused must be NULL,
//garbage memory or references won't be good)
void CKingpin::SetupReflectEffects(void){
	for(int i = 0; i < KINGPIN_MAX_REFLECTEFFECT; i++){
		m_pEntityToReflect[i] = NULL;
		m_pReflectEffect[i] = NULL;
		m_flReflectEffectExpireTime[i] = 0;
		m_flReflectEffectApplyTime[i] = 0;
		m_flReflectEffect_EndDelayFactor[i] = 0;
	}
	m_iReflectEffect = 0;
}//END OF SetupReflectEffects

void CKingpin::ClearReflectEffects(void){
	int i;
	for(i = 0; i < KINGPIN_MAX_REFLECTEFFECT; i++){
		if(m_pReflectEffect[i] != NULL){
			UTIL_Remove( m_pReflectEffect[i] );
			m_pReflectEffect[i] = NULL;
			m_pEntityToReflect[i] = NULL;  //not necessary...? nah go ahead.
			m_flReflectEffectExpireTime[i] = 0;
			m_flReflectEffectApplyTime[i] = 0;
			m_flReflectEffect_EndDelayFactor[i] = 0;
		}
	}//END OF for loop through ref
	m_iReflectEffect = 0;
	//pev->skin = 0;

	//UTIL_StopSound( ENT(pev), CHAN_WEAPON, "debris/zap4.wav", FALSE );
}//END OF ClearReflectEffects

//Call me every frame of think logic.  Do any refs need to be cleaned up?
void CKingpin::UpdateReflectEffects(void){
	int i;
	for(i = 0; i < KINGPIN_MAX_REFLECTEFFECT; i++){
		if(m_pReflectEffect[i] != NULL){
			//has this been surpassed?








			if(m_flReflectEffectApplyTime[i] != -1){



				//Is this a good idea??
				if(gpGlobals->time <= m_flReflectEffectApplyTime[i] && m_pEntityToReflect[i] != NULL){
					Vector anticipatedProjectileOrigin;
					const float timeUntilApply = (m_flReflectEffectApplyTime[i] - gpGlobals->time);

					anticipatedProjectileOrigin = m_pEntityToReflect[i]->pev->origin + m_pEntityToReflect[i]->GetVelocityLogical() * (timeUntilApply);

					if(m_pEntityToReflect[i]->pev->movetype == MOVETYPE_TOSS || m_pEntityToReflect[i]->pev->movetype == MOVETYPE_BOUNCE ){
						//anticipate the effect of gravity.
						int gravityFactor = 0;
						if(m_pEntityToReflect[i]->pev->gravity == 0){
							gravityFactor = 1; //implied?
						}
						anticipatedProjectileOrigin = anticipatedProjectileOrigin + Vector(0, 0, gravityFactor * g_psv_gravity->value * timeUntilApply);
					}


				}

				if(gpGlobals->time >= m_flReflectEffectApplyTime[i]){
					m_flReflectEffectApplyTime[i] = -1;

					//Reflect the entity there sucka!
					//TODO OH SHIT
					if(m_pEntityToReflect[i] != NULL){
						//still valid? reflect!
						const Vector entityCurrentVelocity = m_pEntityToReflect[i]->GetVelocityLogical();
						Vector flatVelocity = Vector(entityCurrentVelocity.x, entityCurrentVelocity.y, 0);

						//m_pEntityToReflect[i]->pev->velocity = flatVelocity * -1 + Vector(0, 0, 100);
						m_pEntityToReflect[i]->SetVelocityLogical(flatVelocity * -1 + Vector(0, 0, 160));
					
						if(m_pEntityToReflect[i]->pev->movetype == MOVETYPE_FLY){
							//I will fall.
							m_pEntityToReflect[i]->pev->movetype = MOVETYPE_TOSS;
						}else if(m_pEntityToReflect[i]->pev->movetype == MOVETYPE_BOUNCEMISSILE){
							m_pEntityToReflect[i]->pev->movetype = MOVETYPE_BOUNCE;
						}
					}



					//OH YAH
					//Expand_TargetTime ??
					//m_pReflectEffect[i]->Expand(0.1, (255 / 0.3f));
					m_pReflectEffect[i]->Expand_TimeTarget(1.4, 0.3 * m_flReflectEffect_EndDelayFactor[i]);

					// YES, we have to do this now.  Either that or give it a callback method to tell us when it runs out of opacity and decides to delete itself.
					// Because it won't tell us when it does, leaving us with a pointer to  deleted memory, the worst kind.
					m_pReflectEffect[i] = NULL;

				}//END OF time passed m_flReflectEffectApplyTime check
				
			}//END OF ReflectEffecTApplyTime check

			
			if(gpGlobals->time >= m_flReflectEffectExpireTime[i]){
				/*
				if(m_pReflectEffect[i] != NULL){
					UTIL_Remove( m_pReflectEffect[i] );
				}
				m_pReflectEffect[i] = NULL;
				*/

				//just force this for safety.
				m_pEntityToReflect[i] = NULL;

			}//END OF time check
			


		}//END OF null check
	}//END OF for loop through ref

}//END OF UpdateReflectEffects


BOOL CKingpin::AlreadyReflectingEntity(CBaseEntity* arg_check){
	int i;

	for(i = 0; i < KINGPIN_MAX_REFLECTEFFECT; i++){
		if(m_pReflectEffect[i] != NULL){
			//check.
			//m_pEntityToReflect[i]
			//if(m_pReflectEffect[i] != NULL && (m_pEntityToReflect[i].Get() == arg_check->edict()) ){
			if(m_pEntityToReflect[i] != NULL && (m_pEntityToReflect[i].Get() == arg_check->edict()) ){
				//already in there? say so.
				return TRUE;
			}
		}
	}//END OF for
	

	//didn't find one? say so.
	return FALSE;
}//END OF AlreadyReflectingEntity





//This is a smaller one for the rapid-fire laser barrage.
//Only one laser in one call though.
void CKingpin::fireElectricBarrageLaser(void){
	const float distToEnemy = (m_vecEnemyLKP - pev->origin).Length();
	const float distToEnemyFraction = UTIL_clamp(distToEnemy / 1300.0f, 0.0f, 1.0f);
	//const float inaccuracyAmount = UTIL_clamp(distToEnemy / 1300.0f, 0.5f, 1.0f);
	const float inaccuracyAmount = UTIL_clamp(distToEnemyFraction, 0.1f, 0.7f);
	//const float damageFraction = UTIL_clamp( 1.0f - distToEnemyFraction, 0.7f, 1.0f);
	float damageFraction;

	if(distToEnemyFraction <= 0.6f){
		//full damage.
		damageFraction = 1.0f;
	}else{
		//steadily decreases.
		damageFraction = 0.6f + (1.0f - distToEnemyFraction);
	}

	Vector vecSrc, vecAim;
	TraceResult tr;
	CBaseEntity *pEntity;

	Vector vecHitLoc;

	playElectricBarrageFireSound();

	
	//this is... a NULL reference pointer to the next available beam.  Just go with it.
	CBeam*& beamRef = getNextBeam();

	//damnit why I gotta do everything for you >_>
	UTIL_MakeAimVectors( pev->angles );

	
	vecSrc = pev->origin + gpGlobals->v_up * CHARGE_POINT_UP + gpGlobals->v_forward * CHARGE_POINT_FORWARD + gpGlobals->v_forward * RANDOM_FLOAT(-9, 9) + gpGlobals->v_right * RANDOM_FLOAT(-9, 9) + gpGlobals->v_up * RANDOM_FLOAT(-9, 9);
	vecAim = ShootAtEnemyMod( vecSrc );

	//is that okay for inaccuracy?
	//float deflection = 0.031;
	//float deflection = 0.018;
	
	//yes, reduce the deflection the further the enemy is. Keep the spread fairly consistent at all ranges,
	//but just steadily higher near the end of the range instead of drastically greater (to the point of being nearly worthless in accuracy)
	float deflection = ((1.0f - (inaccuracyAmount)) * 0.029f) + 0.009f;
	vecAim = vecAim + gpGlobals->v_right * RANDOM_FLOAT( -deflection, deflection ) + gpGlobals->v_up * RANDOM_FLOAT( -deflection, deflection );
	//CHECK THE RANGE HERE. that is whatever vecAim is multipled by.
	UTIL_TraceLine ( vecSrc, vecSrc + vecAim * (distToEnemy + 300.0f), dont_ignore_monsters, ENT( pev ), &tr);
	

	/*
	//leave vecAim the way it is. Make the place hit (inaccuracy factored into that) depend on the distance between me and the enemy.
	//vecHitLoc = vecSrc + vecAim * 1800 + 60*Vector(RANDOM_FLOAT(-inaccuracyAmount, inaccuracyAmount), RANDOM_FLOAT(-inaccuracyAmount, inaccuracyAmount), RANDOM_FLOAT(-inaccuracyAmount, inaccuracyAmount));
	vecHitLoc = vecSrc + vecAim * 1800 + 140*RANDOM_FLOAT(-inaccuracyAmount, inaccuracyAmount)*gpGlobals->v_right + 140*RANDOM_FLOAT(-inaccuracyAmount, inaccuracyAmount)*gpGlobals->v_up;
	UTIL_TraceLine ( vecSrc, vecHitLoc, dont_ignore_monsters, ENT( pev ), &tr);
	*/
	
	//rapid, make it small.
	beamRef = CBeam::BeamCreate( "sprites/lgtning.spr", 14 + (int)( (1.0f - distToEnemyFraction) * 12.0f)  );
	if (!beamRef)
		return;

	//beamRef->PointEntInit( tr.vecEndPos, entindex( ) );
	// uhhh what is my attachment?
	//beamRef->SetEndAttachment( 0 );

	beamRef->PointsInit(vecSrc, tr.vecEndPos);
	//beamRef->SetColor( 180, 255, 96 );

	//TODO - little more random perhaps?
	beamRef->SetColor( 180 + RANDOM_LONG(-70, 70), 70 + RANDOM_LONG(-70, 70), 180 + RANDOM_LONG(-70, 70) );
	//if we have to travel further, get weaker.
	beamRef->SetBrightness( 70 + ((int)(damageFraction * 180)) );
	//the further away, the less noise.
	beamRef->SetNoise( 10 + (1.0f - distToEnemyFraction) * RANDOM_LONG(32, 60) );

	//TODO - with distance the damage decreases?
	pEntity = CBaseEntity::Instance(tr.pHit);
	if (pEntity != NULL && pEntity->pev->takedamage)
	{
		//It will keep accumulating without this... seriously why is there no documentation on how this shit works.
		ClearMultiDamage();

		//TODO - different damage for the Kingping in skills!
		//don't do damage differently to different hitboxes.
		pEntity->TraceAttack( pev, gSkillData.slaveDmgZap * 0.12f, vecAim, &tr, DMG_SHOCK, DMG_HITBOX_EQUAL );
		
		//okay, why not apply this every time damage is dealt..?
		ApplyMultiDamage(pev, pev);
	}

	//::DebugLine_SetupPoint(6, tr.vecEndPos, 255, 0, 0);

	playElectricBarrageHitSound(m_hEnemy, tr.vecEndPos);
}//END OF fireElectricBarrageLaser



// This is the strong loud single-fire one.
// IMPORTANT NOTICE - the "arg_hitIntention" is not used yet, but if it ever is, give it a null check!
// If we deem firint at just an enemyLKP okay, we need to allow arg_hitIntention to be null to say, at nothing in particular. Just the place.
void CKingpin::fireElectricDenseLaser(CBaseEntity* arg_hitIntention, const Vector& arg_hitTargetPoint){

	playElectricLaserFireSound();

	/*
	if(arg_hitInteion == NULL){

	}
	*/


	//??
	//playElectricBarrageFireSound();


	Vector vecSrc, vecAim;
	TraceResult tr;
	CBaseEntity *pEntity;

	
	//this is... a NULL reference pointer to the next available beam.  Just go with it.
	CBeam*& beamRef = getNextBeam();

	//damnit why I gotta do everything for you >_>
	UTIL_MakeAimVectors( pev->angles );

	
	vecSrc = pev->origin + gpGlobals->v_up * CHARGE_POINT_UP + gpGlobals->v_forward * CHARGE_POINT_FORWARD;
	
	//vecAim = ShootAtEnemy( vecSrc );
	vecAim = (arg_hitTargetPoint - vecSrc).Normalize();

	//is that okay for inaccuracy?
	float deflection = 0.0003;

	vecAim = vecAim + gpGlobals->v_right * RANDOM_FLOAT( -deflection, deflection ) + gpGlobals->v_up * RANDOM_FLOAT( -deflection, deflection );

	//CHECK THE RANGE HERE. that is whatever vecAim is multipled by.
	UTIL_TraceLine ( vecSrc, vecSrc + vecAim * 3000, dont_ignore_monsters, ENT( pev ), &tr);

	//rapid, make it small.
	beamRef = CBeam::BeamCreate( "sprites/lgtning.spr", 160 );
	if (!beamRef)
		return;

	//beamRef->PointEntInit( tr.vecEndPos, entindex( ) );
	// uhhh what is my attachment?
	//beamRef->SetEndAttachment( 0 );

	beamRef->PointsInit(vecSrc, tr.vecEndPos);
	//beamRef->SetColor( 180, 255, 96 );

	//TODO - little more random perhaps?
	beamRef->SetColor( 235 + RANDOM_LONG(-20, 20), 80 + RANDOM_LONG(-20, 20), 90 + RANDOM_LONG(-20, 20) );
	beamRef->SetBrightness( 245 );
	beamRef->SetNoise( 9 + RANDOM_LONG(0, 4) );

	//TODO - with distance the damage decreases?
	pEntity = CBaseEntity::Instance(tr.pHit);
	if (pEntity != NULL && pEntity->pev->takedamage)
	{
		//It will keep accumulating without this... seriously why is there no documentation on how this shit works.
		ClearMultiDamage();

		//TODO - different damage for the Kingping in skills!
		//don't do damage differently to different hitboxes.
		pEntity->TraceAttack( pev, gSkillData.slaveDmgZap * 3.0f, vecAim, &tr, DMG_SHOCK, DMG_HITBOX_EQUAL );
		
		//okay, why not apply this every time damage is dealt..?
		ApplyMultiDamage(pev, pev);
	}

	//::DebugLine_SetupPoint(6, tr.vecEndPos, 255, 0, 0);

	playElectricLaserHitSound(m_hEnemy, tr.vecEndPos);

}


// homing ball of doom!
void CKingpin::fireSuperBall(void){
	
	//create the ball
	Vector vecStart, angleGun;
	Vector vecForward;
	Vector vecRight;
	Vector vecUp;


	//super ball.  USe pathfinding to route towards the enemy if there are air nodes.
	//Otherwise just immitate a typical controller ball, that's the best we can do.

	//psionic launch (launch a ball)

		
	if(m_hEnemy == NULL){
		//stop?
		TaskFail();
		return;
	}

	//m_IdealActivity = ACT_RANGE_ATTACK1;


	//GetAttachment( 0, vecStart, angleGun );
			
	UTIL_MakeAimVectorsPrivate(pev->angles, vecForward, vecRight, vecUp);

	//vecStart = pev->origin + vecForward * 18 + Vector(0, 0, 60);
	vecStart = pev->origin + vecUp * CHARGE_POINT_UP + vecForward * CHARGE_POINT_FORWARD;
	


	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_ELIGHT );
		WRITE_SHORT( entindex( ) + 0x1000 );		// entity, attachment
		WRITE_COORD( 0 );		// origin
		WRITE_COORD( 0 );
		WRITE_COORD( 0 );
		WRITE_COORD( 32 );	// radius
		WRITE_BYTE( 0 );	// R
		WRITE_BYTE( 62 );	// G
		WRITE_BYTE( 255 );	// B
		WRITE_BYTE( 10 );	// life * 10
		WRITE_COORD( 32 ); // decay
	MESSAGE_END();

	CBaseMonster *pBall = (CBaseMonster*)CreateManual( "kingpin_ball", vecStart, pev->angles, edict() );

	pBall->pev->velocity = Vector( vecForward.x * 100, vecForward.y * 100, 0 );
	pBall->m_hEnemy = m_hEnemy;

	pBall->Spawn();

	//this->playPsionicLaunchSound();
	//this->SetSequenceByIndex(KINGPIN_PSIONIC_LAUNCH);
}//END OF fireSuperBall



//three hornets that move outwards in a triangular fashion away from the top of the kingpin (spawn) at first, slowly forwards, and then zoom at the LKP without homing.
void CKingpin::fireSpeedMissile(void){

	Vector vecForward;
	Vector vecRight;
	Vector vecUp;
	Vector vecSrc;

	//including the sound here I guess.
	int iPitch = 89 + RANDOM_LONG(0, 5);
	switch(RANDOM_LONG(0, 2)){
		case 0: UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "agrunt/ag_fire1.wav", 1.0, ATTN_NORM - 0.63f, 0, iPitch ); break;
		case 1: UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "agrunt/ag_fire2.wav", 1.0, ATTN_NORM - 0.63f, 0, iPitch ); break;
		case 2: UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "agrunt/ag_fire3.wav", 1.0, ATTN_NORM - 0.63f, 0, iPitch ); break;
	}



	UTIL_MakeAimVectorsPrivate(pev->angles, vecForward, vecRight, vecUp);

	vecSrc = pev->origin + vecUp * CHARGE_POINT_UP + vecForward * (CHARGE_POINT_FORWARD + 60);
	


	createSpeedMissileHornet(vecSrc, (vecForward * 0.2 + vecUp ).Normalize()  );
	createSpeedMissileHornet(vecSrc, (vecForward * 0.2 + -vecUp * 0.5f + vecRight * 0.5f ).Normalize()  );
	createSpeedMissileHornet(vecSrc, (vecForward * 0.2 + -vecUp * 0.5f + -vecRight * 0.5f ).Normalize()  );

	
	if (RANDOM_LONG(0,1))
		AttackSound();


}//END OF fireSpeedMissile


void CKingpin::createSpeedMissileHornet(const Vector& arg_location, const Vector& arg_floatVelocity){
	Vector spawnLocation = arg_location + arg_floatVelocity * 30;
	Vector dirToEnemyLKP = (m_vecEnemyLKP - pev->origin).Normalize();
	CBaseMonster* pHornetMonster;

	CBaseEntity* pHornet = CBaseEntity::Create( "hornet_kingpin", spawnLocation, UTIL_VecToAngles( dirToEnemyLKP ), edict() );
	//UTIL_MakeVectors ( pHornet->pev->angles );
			
	//MODDD - change, explanation above.
	//pHornet->pev->velocity = gpGlobals->v_forward * 300;



	pHornet->pev->velocity = arg_floatVelocity * 25;
	
	//pHornet->pev->angles = UTIL_VecToAngles( pHornet->pev->velocity );

	//pHornet->SetThink( &CHornet::StartSpeedMissile );
	//no need, the new one knows to do that.
			

	pHornetMonster = pHornet->MyMonsterPointer();

	

	if(pHornetMonster){
		pHornetMonster->m_hEnemy = m_hEnemy;

		CHornetKingpin* hornetRef = static_cast<CHornetKingpin*>(pHornet);


		if(m_hEnemy != NULL){
			hornetRef->setup(m_hEnemy.GetEntity(), arg_floatVelocity * 10);
		}else{
			hornetRef->setup(this->m_vecEnemyLKP, arg_floatVelocity * 10);
		}

		hornetRef->pev->owner = this->edict();  //good idea?


		//This will get overridden by havingan established enemy anyways.
		//hornetRef->speedMissileDartTarget = m_vecEnemyLKP;
		//Make sure they know to move a little aways from the direct center of the enemy.  Otherwise two of the three fired hornets may crash into each other
		//as they near each other towards the target, particularly if the target moves backwards to give them a little more distance and likelihood of crashing into one another.
		//hornetRef->speedMissileDartTargetOffset = arg_floatVelocity * 12;
	}

}//END OF createSpeedMissileHornet




//This logic comes up so much it may as well be turned into a simple method.
BOOL CKingpin::turnToFaceEnemyLKP(void){
	
	MakeIdealYaw( m_vecEnemyLKP );
	ChangeYaw( pev->yaw_speed );
	
	return FacingIdeal();  //does the caller care if I'm facing the ideal yet?
}//END OF turnToFaceEnemyLKP


float CKingpin::getDotProductWithEnemyLKP(void){
	Vector myDirection;
	Vector2D vec2LOS;

	//borrowed from basemonster.cpp's CheckAttacks method.  See how close I am to facing the target.
	UTIL_MakeAimVectorsPrivate ( pev->angles, myDirection, NULL, NULL );

	vec2LOS = ( m_vecEnemyLKP - pev->origin ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	
	return DotProduct (vec2LOS , myDirection.Make2D() );
}//END OF getDotProductWithEnemyLKP


void CKingpin::createChargeEffect(void){
	Vector vecSrc;
	if(chargeEffect != NULL){
		//???
		UTIL_Remove(chargeEffect);
		chargeEffect = NULL;
	}
	
	//damnit why I gotta do everything for you >_>
	UTIL_MakeAimVectors( pev->angles );

	vecSrc = pev->origin + gpGlobals->v_up * CHARGE_POINT_UP + gpGlobals->v_forward * CHARGE_POINT_FORWARD;
	
	chargeEffect = CSprite::SpriteCreate( "sprites/tele1.spr", vecSrc, TRUE );
	if (chargeEffect)
	{
		//for during the animation when it begins.
		chargeEffect->pev->framerate = 12.0;
		chargeEffect->AnimationScaleFadeIn(6, 380, 2.5, 0.8f, 170);

		
		/*
		chargeEffect->SetTransparency( kRenderTransAdd, 255, 255, 255, 170, kRenderFxNoDissipation );
		//chargeEffect->SetAttachment( edict(), 1 );
		chargeEffect->SetScale( 0.8f );
		chargeEffect->pev->framerate = 12.0;
		chargeEffect->TurnOn( );
		*/
	}
	
	/*
	//any way I can map a light to this entity? probably some clientside event could.
	if (m_pBall)
	{
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_ELIGHT );
			WRITE_SHORT( entindex( ) + 0x1000 );		// entity, attachment
			WRITE_COORD( pev->origin.x );		// origin
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( 256 );	// radius
			WRITE_BYTE( 255 );	// R
			WRITE_BYTE( 192 );	// G
			WRITE_BYTE( 64 );	// B
			WRITE_BYTE( 200 );	// life * 10
			WRITE_COORD( 0 ); // decay
		MESSAGE_END();
	}
	*/

}//END OF createChargeEffect

void CKingpin::updateChargeEffect(void){

	if(chargeEffect != NULL){
		Vector vecForward;
		Vector vecRight;
		Vector vecUp;
		Vector chargeEffectOrigin;

		UTIL_MakeAimVectorsPrivate(pev->angles, vecForward, vecRight, vecUp);


		chargeEffectOrigin = pev->origin + vecForward * CHARGE_POINT_FORWARD + vecUp * CHARGE_POINT_UP;

		chargeEffect->pev->origin = chargeEffectOrigin;



		if(m_pSchedule == slKingpinElectricLaser && getTaskNumber() == TASK_KINGPIN_ELECTRIC_LASER_CHARGE){
			//electric laser's charge task wants an implode effect.
			
			//figure: how long did I start the charge ago?
			float chargeStartTime = chargeFinishTime - KINPIN_ELECTRIC_LASER_CHARGETIME;

			//how long has passed since then?
			float chargeTimeElapsed = gpGlobals->time - chargeStartTime;

			//what portion of that are we done?
			float chargeTimeFraction = chargeTimeElapsed / KINPIN_ELECTRIC_LASER_CHARGETIME;

			
			float life;
			//life = ((255 - pev->frame) / (pev->framerate * m_flFrameRate));
			life = (1.0f - chargeTimeFraction);
			if (life < 0.05) life = 0.05;


			MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
				WRITE_BYTE(  TE_IMPLOSION);
				WRITE_COORD( chargeEffectOrigin.x);
				WRITE_COORD( chargeEffectOrigin.y);
				WRITE_COORD( chargeEffectOrigin.z);
				WRITE_BYTE( 50 * life + 150);  // radius
				//WRITE_BYTE( pev->frame / 25.0 ); // count
				WRITE_BYTE( ((int)( chargeTimeFraction * 16  )) ); // count
				WRITE_BYTE( life * 10 ); // life
			MESSAGE_END();


		}//END OF electric laser charge check




	}//END OF chargeEffect NULL check

}//END OF updateChargeEffect



void CKingpin::removeChargeEffect(void){

	if(chargeEffect != NULL){
		//set it up to remove itself like how the egon does.
		//That isn't go from scale #1 to #2, that's a scale speed and a fade speed.  Go figure.
		chargeEffect->Expand( 3, 460 );

		chargeEffect = NULL;
	}

}//END OF removeChargeEffect


//Like hgrunts, I can also check attacks while the enemy is behind walls.
BOOL CKingpin::FCanCheckAttacks(void){
	if ( !HasConditions( bits_COND_ENEMY_TOOFAR ) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//default, override if necessary.
float CKingpin::getDistTooFar(void){
	return 1024.0 * 3.0f;
}
//default, override if necessary.
float CKingpin::getDistLook(void){
	return 2048.0 * 1.5f;
}





void CKingpin::setPrimaryAttackCooldown(void){

	switch(g_iSkillLevel){
	case SKILL_EASY:
		primaryAttackCooldownTime = gpGlobals->time + RANDOM_FLOAT(2.6f, 3.5f);
	break;
	case SKILL_MEDIUM:
		primaryAttackCooldownTime = gpGlobals->time + RANDOM_FLOAT(2.0f, 2.7f);
	break;
	case SKILL_HARD:
		primaryAttackCooldownTime = gpGlobals->time + RANDOM_FLOAT(1.4f, 2.1f);
	break;
	default:
		//???
		primaryAttackCooldownTime = gpGlobals->time + RANDOM_FLOAT(2.2f, 3.3f);
	break;
	}//END OF switch
}//END OF setPrimaryAttackCooldown



void CKingpin::playShockerFireSound(CBaseEntity* arg_target, const Vector& arg_location){
	int pitch = 84 + RANDOM_LONG(0, 10);
	//UTIL_PlaySound( edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pShockerFireSounds), 1.0, ATTN_NORM, 0, pitch );
	//arg_location


	edict_t* toSend;

	if(arg_target != NULL){
		toSend = arg_target->edict();
	}else{
		toSend = edict();
	}

	UTIL_EmitAmbientSound( toSend, arg_location, RANDOM_SOUND_ARRAY(pShockerFireSounds), 1.0, ATTN_NORM, 0, pitch, TRUE );
}


//do a houndeye shockwave, a little in the direction we're facing... or alot.
void CKingpin::administerShocker(void){
	BOOL useAlt = TRUE;
	float flAdjustedDamage;
	float flDist;
	float zOff = 24;  //used to be 16.

	Vector vecStart, angleGun;
	Vector vecForward;
	float shockerForwardDistance = 0;

	TraceResult tr;
	Vector vecStartFloorwise;

	//GetAttachment( 0, vecStart, angleGun );
			
	UTIL_MakeAimVectorsPrivate(pev->angles, vecForward, NULL, NULL);

	Vector enemyPoint;
	Vector vecTest;
	float shockerZ;

	if(m_hEnemy != NULL){
		enemyPoint = m_hEnemy->pev->origin;
	}else{
		//next best thing.
		enemyPoint = m_vecEnemyLKP;
	}

	float distanceToEnemy = (enemyPoint - pev->origin).Length();
	if(distanceToEnemy <= 300.0f){
		shockerForwardDistance = distanceToEnemy / 2.0f;
	}else{
		//just X in front of the enemy looking towards me.
		shockerForwardDistance = distanceToEnemy - 150.0f;
	}

	//should there be a cap on distance like up to 580 points away?
	//if(shockerForwardDistance > 580.0f){
	//	shockerForwardDistance = 580.0f - 150.0f;
	//}


	//KINGPIN_SHOCKER_RADIUS
	//This is guaranteed to be on the same level as the kingpin's ground though, not the target's.
	//vecStart = pev->origin + vecForward * shockerForwardDistance + Vector(0, 0, zOff);
	
	//How about we use the kingpin's forward floor-wise (vecForward.x and .y), same shockerForwardDistance, take that point, and snap it to the ground?
	vecStartFloorwise = Vector(pev->origin.x, pev->origin.y, 0) + Vector(vecForward.x, vecForward.y, 0) * shockerForwardDistance;

	//go up or down?

	if(enemyPoint.z >= pev->origin.z){
		//above?
	}

	//::DebugLine_SetupPoint(9, vecStartFloorwise + Vector(0, 0, enemyPoint.z), 255, 0, 0);


	vecTest = Vector(vecStartFloorwise.x, vecStartFloorwise.y, enemyPoint.z + 6);

	UTIL_TraceLine(vecTest, vecTest + Vector(0, 0, -140), ignore_monsters, edict(), &tr);

	// oh.  oops.
	//if(tr.fStartSolid || tr.fStartSolid){
	if(tr.fStartSolid){
		//going through something at the start or entirely? Just use the enemy's Z.
		shockerZ = enemyPoint.z + zOff;
	}else{
		
		//did we hit at all?
		if(tr.flFraction >= 1.0f){
			//no? same thing.
			shockerZ = enemyPoint.z + zOff;
		}else{
			//hit something?  go.
			shockerZ = tr.vecEndPos.z + zOff;
		}

	}




	vecStart = Vector(vecStartFloorwise.x, vecStartFloorwise.y, shockerZ);


	playShockerFireSound(m_hEnemy, vecStart);



	//!!!
	//Rest is a rip from SonicAttack from houndeye.cpp.
	


	// blast circles
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, vecStart );
		WRITE_BYTE( TE_BEAMCYLINDER );
		WRITE_COORD( vecStart.x);
		WRITE_COORD( vecStart.y);
		WRITE_COORD( vecStart.z + zOff);
		WRITE_COORD( vecStart.x);
		WRITE_COORD( vecStart.y);
		WRITE_COORD( vecStart.z + zOff + KINGPIN_SHOCKER_RADIUS / .2); // reach damage radius over .3 seconds
		WRITE_SHORT( m_iSpriteTexture );
		WRITE_BYTE( 0 ); // startframe
		WRITE_BYTE( 0 ); // framerate
		WRITE_BYTE( 2 ); // life
		WRITE_BYTE( zOff );  // width
		WRITE_BYTE( 0 );   // noise
		
		WRITE_BYTE( 245   );
		WRITE_BYTE( 16 );
		WRITE_BYTE( 210  );

		WRITE_BYTE( 255 ); //brightness
		WRITE_BYTE( 0 );		// speed
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, vecStart );
		WRITE_BYTE( TE_BEAMCYLINDER );
		WRITE_COORD( vecStart.x);
		WRITE_COORD( vecStart.y);
		WRITE_COORD( vecStart.z + zOff);
		WRITE_COORD( vecStart.x);
		WRITE_COORD( vecStart.y);
		WRITE_COORD( vecStart.z + zOff + ( KINGPIN_SHOCKER_RADIUS / 2 ) / .2); // reach damage radius over .3 seconds
		WRITE_SHORT( m_iSpriteTexture );
		WRITE_BYTE( 0 ); // startframe
		WRITE_BYTE( 0 ); // framerate
		WRITE_BYTE( 2 ); // life
		WRITE_BYTE( zOff );  // width
		WRITE_BYTE( 0 );   // noise
		
		WRITE_BYTE( 245   );
		WRITE_BYTE( 16 );
		WRITE_BYTE( 210  );

		WRITE_BYTE( 255 ); //brightness
		WRITE_BYTE( 0 );		// speed
	MESSAGE_END();


	CBaseEntity *pEntity = NULL;
	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere( pEntity, vecStart, KINGPIN_SHOCKER_RADIUS )) != NULL)
	{
		if ( pEntity->pev->takedamage != DAMAGE_NO )
		{
			BOOL isBreakable = pEntity->isBreakableOrChild();

			//No, do a check for allies in general.  We is smart.
			//if ( !FClassnameIs(pEntity->pev, "monster_houndeye") )
			if(this->IRelationship(pEntity) > R_NO || isBreakable)
			{
				Vector deltaToShockwave;
				Vector directionToShockwave;
				Vector directionToShockwave2D;
				float damageFactor;

				//EASY_CVAR_PRINTIF_PRE(houndeyePrintout, easyPrintLine( "What did I hurt? %s", STRING(pEntity->pev->classname)));

				// houndeyes do FULL damage if the ent in question is visible. Half damage otherwise.
				// This means that you must get out of the houndeye's attack range entirely to avoid damage.
				// Calculate full damage first


				
				flAdjustedDamage = gSkillData.houndeyeDmgBlast;
				

				//BodyTargetMod(pev->origin)  ?  Center()?  whichever works.
				deltaToShockwave = (pEntity->Center() - vecStart);

				directionToShockwave = (deltaToShockwave).Normalize();
				directionToShockwave2D = Vector(directionToShockwave.x, directionToShockwave.y, 0);

				flDist = (deltaToShockwave).Length();

				//flAdjustedDamage -= ( flDist / KINGPIN_SHOCKER_RADIUS ) * flAdjustedDamage;
				//flAdjustedDamage = flAdjustedDamage - (( flDist / KINGPIN_SHOCKER_RADIUS ) * flAdjustedDamage);

				//If the distance between the  goes above the damage radius, flDist / RAD makes a number above 1.  subtracted from 1, it is negative to signify no damage.
				//But if the distance is less than the radius, lesser and lesser goes towards 0, which subtracted from 1, is less of an influence (detractor).
				damageFactor = max(1.0f - (( flDist / KINGPIN_SHOCKER_RADIUS) ), 0.0f);
				flAdjustedDamage = flAdjustedDamage * (damageFactor);


				//? = 100 - (160 / 200) * 100;
				//20
				//? = 100 * (1 - 160/200)
				//

				if(pEntity->IsPlayer() && !pEntity->blocksImpact() && damageFactor > 0.0f){
					//float randomAngle = RANDOM_FLOAT(0.0f, M_PI*2.0f);
					
					int randomZDir;
					if(RANDOM_LONG(0, 1) == 0){
						randomZDir = -1;
					}else{
						randomZDir = 1;
					}

					//FORCE IT.
					
					//OH that's terrible.  A big positive punchangle.x shows below our view model where it stops being rendered (end of model), not good.
					//randomAngle = (M_PI / 2.0f) * 0.0f;

					//other angles (90 degree tests for boundary testing) look okay.
					//randomAngle = (M_PI / 2.0f) * 3.0f;


					/*
					//shake up the camera like mad!  But don't go too high (positive) on the punchangle.x
					//pEntity->pev->punchangle.x = min( cos(randomAngle) * 60.0f * damageFactor, 8.0f);
					pEntity->pev->punchangle.x = cos(randomAngle) * 60.0f * damageFactor + -52.0f;

					//pEntity->pev->punchangle.y = cos(randomAngle) * 50.0f * damageFactor;

					pEntity->pev->punchangle.z = sin(randomAngle) * 60.0f * damageFactor;
					*/

					pEntity->pev->punchangle.x = RANDOM_FLOAT(-1.0f, -0.7f) * 13.0f * damageFactor;
					pEntity->pev->punchangle.z = randomZDir*RANDOM_FLOAT(-1.0f, -0.7f) * 27.0f * damageFactor;

					pEntity->pev->punchangle.y = RANDOM_FLOAT(-0.2f, 0.2f) * 13.0f * damageFactor;


				}


				if ( !FVisible( pEntity ) )
				{
					if ( pEntity->IsPlayer() )
					{
						// if this entity is a client, and is not in full view, inflict half damage. We do this so that players still
						// take the residual damage if they don't totally leave the houndeye's effective radius. We restrict it to clients
						// so that monsters in other parts of the level don't take the damage and get pissed.
						flAdjustedDamage *= 0.5;
					}
					//else if ( !FClassnameIs( pEntity->pev, "func_breakable" ) && !FClassnameIs( pEntity->pev, "func_pushable" ) )
					else if(!isBreakable)
					{
						//MODDD NOTE 
						// hold on here. This may never get reached becacuse of the relationship block. Put an exception for that above too
						// if that's really wanted.   DONE NOW.
						// do not hurt nonclients through walls, but allow damage to be done to breakables
						flAdjustedDamage = 0;
					}
				}


				//increase the damage a little to compensate for being off (location-wise) a bit.
				flAdjustedDamage *= 1.20f;


				//ALERT ( at_aiconsole, "Damage: %f\n", flAdjustedDamage );

				if (flAdjustedDamage > 0 )
				{
					int damageType = DMG_SONIC;

					//sure, let this CVar filter apply to the kingpin shocker I guess.
					if(EASY_CVAR_GET_DEBUGONLY(houndeye_attack_canGib)){
						damageType |= DMG_ALWAYSGIB;
					}

					pEntity->TakeDamage ( pev, pev, flAdjustedDamage, damageType, DMG_HITBOX_EQUAL );


					//Another thing. Do some physical knockback.
					//pEntity->pev->velocity = pEntity->pev->velocity - gpGlobals->v_forward * 420 + gpGlobals->v_up * 120 - gpGlobals->v_right * 45;

					//HACK - if the directionToShockwave's Z is sufficiently small (absolute-value wise, distance from 0 positive or negative), force it to be a little higher positive.

					if(fabs(directionToShockwave.z) < 0.3f){
						directionToShockwave.z = 0.8f;
					}

					//make it closer to 1 for this, since it is a decimal.
					damageFactor = pow(damageFactor, (1.0f/3.0f) );

					pEntity->pev->velocity = pEntity->pev->velocity + directionToShockwave2D * 560 * damageFactor + Vector(0, 0, directionToShockwave.z) * 380 * damageFactor;
					//is this a good idea?
					if(pEntity->pev->flags & FL_ONGROUND){
						pEntity->pev->flags &= ~FL_ONGROUND;
						UTIL_MoveToOrigin(pEntity->edict(), pEntity->pev->origin + Vector(0, 0, 2), 1, MOVE_STRAFE);
					}

				}

			}//END OF relationship or breakable checks
		}//END OF if other entity takes damage
	}//END OF while thru entities in range list


	
	if (RANDOM_LONG(0,1)){
		AttackSound();
	}

}//END OF administerShocker





BOOL CKingpin::needsMovementBoundFix(void){

	return TRUE;
}//END OF needsMovementBoundFix



//MODDD - started as a clone of CBaseMonster's BestVisibleEnemy method (from the as-is SDK).
// Modified to take a point instead, and see which entity is closest to the kingpin,
// but still roughly in the same direction it is facing (towards this point).  A laser that fires some 140 or 180 degrees behind looks weird.
CBaseEntity* CKingpin::attemptFindTowardsPoint(const Vector& arg_searchPoint){
	CBaseEntity	*pReturn;
	CBaseEntity	*pNextEnt;
	float		flNearest;
	float		flDist;

	Vector2D vecDirToSearchPoint = (arg_searchPoint - this->EyePosition()).Make2D().Normalize();

	flNearest = 3000;
	pNextEnt = m_pLink;
	pReturn = NULL;

	while ( pNextEnt != NULL )
	{
		if ( pNextEnt->IsAlive_FromAI(this) )
		{
			// First a check. Is this near a straight line from this Kingpin to arg_searchPoint?
			Vector2D vecTowardsEnemy = (pNextEnt->EyePosition() - this->EyePosition()).Make2D().Normalize();
			float flDot = DotProduct(vecDirToSearchPoint, vecTowardsEnemy);


			// based off of VIEW_FIELD_ULTRA_NARROW, which was +-25 degrees for a value of 0.9.
			// greater values towards 1.0 are more strict (1.0 being impossibly exact... decimals match
			// exactly once in a thousand blue moons), 0 (+- 90 degrees), and -1 (+- 180 degrees, or
			// effectively any angle away from me is satisfactory, since that wraps around all 360 
			// degrees).
			if(flDot >= 0.82){
				//in the direction enough.
				flDist = ( pNextEnt->pev->origin - pev->origin ).Length();
				
				if ( flDist <= flNearest )
				{
					flNearest = flDist;
					pReturn = pNextEnt;
				}
			}
		}

		pNextEnt = pNextEnt->m_pLink;
	}

	return pReturn;
}//END OF attemptFindTowardsPoint


int CKingpin::getHullIndexForNodes(void){
    return NODE_LARGE_HULL;  //safe?
}



void CKingpin::attemptReflectProjectileStart(CBaseEntity* arg_toReflect, float arg_delayFactor){
	
	int emptyReflectHandleID;
	Vector anticipatedProjectileOrigin;

	// Is this entity already being reflected?
	if(AlreadyReflectingEntity(arg_toReflect)){
		// stop.  No need to set a slot for this.
		return;
	}

	emptyReflectHandleID = this->getNextReflectHandleID();

	if(emptyReflectHandleID == -1){
		// also stop.  No slots available.
		return;
	}

	// Going through with this?  Let the projectile know it's getting deflected. Or reflected. Whatever.
	// Stop hornets and rockets from being persistent.
	arg_toReflect->OnDeflected(this);

	// Make me the owner to avoid touching me.  Is this ok?
	// Things that have a separate owner instance/member variable (m_hOwner usually, like Snarks) need to handle
	// that themselves in their own "OnDeflected" versions.
	arg_toReflect->pev->owner = this->edict();


	
	anticipatedProjectileOrigin = arg_toReflect->pev->origin + arg_toReflect->GetVelocityLogical() * (REFLECT_BASE_DELAY * arg_delayFactor);

	if(arg_toReflect->pev->movetype == MOVETYPE_TOSS || arg_toReflect->pev->movetype == MOVETYPE_BOUNCE ){
		// anticipate the effect of gravity.
		int gravityFactor = 0;
		if(arg_toReflect->pev->gravity == 0){
			gravityFactor = 1; //implied?
		}
		anticipatedProjectileOrigin = anticipatedProjectileOrigin + Vector(0, 0, gravityFactor * g_psv_gravity->value * REFLECT_BASE_DELAY * arg_delayFactor);
	}


	m_pReflectEffect[emptyReflectHandleID] = CSprite::SpriteCreate( "sprites/hotglow_ff.spr", anticipatedProjectileOrigin, TRUE );

	
	//SAFETY. Did I create the effect?
	if(m_pReflectEffect[emptyReflectHandleID]){
		m_pReflectEffect[emptyReflectHandleID]->AnimationScaleFadeIn_TimeTarget(1.1, 0.8f, 170, REFLECT_BASE_DELAY * arg_delayFactor);

		//was arg_delayFactor.
		m_flReflectEffect_EndDelayFactor[emptyReflectHandleID] = 1.0f;  // always take the same amount of time to end.  We need to see it.

		//Good, now set this slot up to reflect this entity soon with an effect.
		m_pEntityToReflect[emptyReflectHandleID] = arg_toReflect;

		m_flReflectEffectApplyTime[emptyReflectHandleID] = gpGlobals->time + REFLECT_BASE_DELAY * arg_delayFactor;
		//m_flReflectEffectExpireTime[emptyReflectHandleID] = gpGlobals->time + 0.6 * arg_delayFactor;
		m_flReflectEffectExpireTime[emptyReflectHandleID] = m_flReflectEffectApplyTime[emptyReflectHandleID] + REFLECT_BASE_DELAY * 1.0f; //* arg_delayFactor;
		
	}//END OF ReflectEffect creation check
	else{
		
	}

}//END OF attemptReflectProjectileStart



