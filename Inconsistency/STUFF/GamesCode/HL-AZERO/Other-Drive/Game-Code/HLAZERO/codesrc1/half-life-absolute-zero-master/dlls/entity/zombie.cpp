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
// Zombie
//=========================================================


//TODO - zombies seem to fly a little more if hit while idle and they don't move.  Turn on autosneaky and see.
//       It's as though they need some natural friction or something to slow down velocity forces from that bullet force CVar.


// UNDONE: Don't flinch every time you get hit

#include "extdll.h"
#include "zombie.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "schedule.h"
#include "soundent.h"
#include "util_debugdraw.h"
#include "defaultai.h"
#include "scripted.h"

EASY_CVAR_EXTERN_DEBUGONLY(zombieBulletResistance);
EASY_CVAR_EXTERN_DEBUGONLY(zombieBulletPushback);


extern DLL_GLOBAL int g_iSkillLevel;
extern DLL_GLOBAL float g_rawDamageCumula;


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ZOMBIE_AE_ATTACK_RIGHT		0x01
#define ZOMBIE_AE_ATTACK_LEFT		0x02
#define ZOMBIE_AE_ATTACK_BOTH		0x03

//MODDD - upped a bit since we have pushback.    maybe.
#define ZOMBIE_FLINCH_DELAY			3		// at most one flinch every n secs




//TODO - a touch event for the corpse to trigger TASK_WAIT_FOR_MOVEMENT_DUMB completion too??

//TODO - should being interrupted by damage under schedule "WaitForScript" also be ablve to move to an uncrouch script?


//what sequence is used for crouching over a corpse?
//a full enum of all sequences is possible but... laziness.
#define ZOMBIE_EATBODY 29


#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_zombie, CZombie );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( zombie, CZombie );
	
	//no extras.

#endif



const char *CZombie::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CZombie::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CZombie::pAttackSounds[] = 
{
	"zombie/zo_attack1.wav",
	"zombie/zo_attack2.wav",
};

const char *CZombie::pIdleSounds[] = 
{
	"zombie/zo_idle1.wav",
	"zombie/zo_idle2.wav",
	"zombie/zo_idle3.wav",
	"zombie/zo_idle4.wav",
};

const char *CZombie::pAlertSounds[] = 
{
	"zombie/zo_alert10.wav",
	"zombie/zo_alert20.wav",
	"zombie/zo_alert30.wav",
};

const char *CZombie::pPainSounds[] = 
{
	"zombie/zo_pain1.wav",
	"zombie/zo_pain2.wav",
};




//MODDD - new.

//custom schedule enum
enum{
	SCHED_ZOMBIE_VICTORY_DANCE = LAST_COMMON_SCHEDULE + 1,
	SCHED_ZOMBIE_VICTORY_DANCE_VALID,
	SCHED_ZOMBIE_SEEK_CORPSE,
	SCHED_ZOMBIE_SEEK_CORPSE_EARLY_FAIL,
	SCHED_ZOMBIE_SEEK_CORPSE_QUICK_FAIL,
};

enum{
	TASK_ZOMBIE_GET_PATH_TO_ENEMY_CORPSE = LAST_COMMON_TASK + 1,
	TASK_ZOMBIE_MOVE_CLOSER_TO_CORPSE,
	TASK_ZOMBIE_VICTORY_DANCE_VALID_CHECK,
	TASK_WAIT_FOR_MOVEMENT_DUMB,
	TASK_ZOMBIE_PLAY_EAT_SEQUENCE,
	TASK_FACE_CORPSE,
	TASK_FORGET_CORPSE,
	TASK_CHECK_GETUP_SEQUENCE,
};

//cloned from slAGruntVictoryDance
Task_t tlZombieMoveToCorpse[] =
{
	{ TASK_STOP_MOVING,						(float)0					},
	//When failing early, wait longer. don't do a distance safety check.
	{ TASK_SET_FAIL_SCHEDULE,				(float)SCHED_ZOMBIE_SEEK_CORPSE_EARLY_FAIL	},
	{ TASK_WAIT,							(float)0.2					},
	{ TASK_ZOMBIE_GET_PATH_TO_ENEMY_CORPSE,	(float)0					},
	{ TASK_WALK_PATH,						(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0					},
	//{ TASK_STOP_MOVING,						(float)0					},
	//{ TASK_FACE_CORPSE,						(float)0					},
	//NEW TASK. Should help get closer once we've finished this route that is fairly close but not close enough.
	{ TASK_SET_FAIL_SCHEDULE,				(float)SCHED_ZOMBIE_VICTORY_DANCE_VALID	},
	{ TASK_ZOMBIE_MOVE_CLOSER_TO_CORPSE,    (float)0					},
	{ TASK_WALK_PATH,						(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT_DUMB,				(float)0					},
	//can play the animation.
	{ TASK_SET_SCHEDULE,    (float)SCHED_ZOMBIE_VICTORY_DANCE			},
};
Schedule_t slZombieMoveToCorpse[] =
{
	{ 
		tlZombieMoveToCorpse,
		ARRAYSIZE ( tlZombieMoveToCorpse ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,

		bits_SOUND_CARCASS,

		"ZombieMoveToCorpse"
	},
};


Task_t tlZombieVictoryDance[] =
{
	//ensure this fail schedule runs.  To uncrouch from an eating animation.
	{ TASK_SET_FAIL_SCHEDULE_HARD,				(float)SCHED_ZOMBIE_SEEK_CORPSE_QUICK_FAIL	},
	{ TASK_STOP_MOVING,						(float)0					},
	{ TASK_FACE_CORPSE,						(float)0					},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_CROUCH			},
	{ TASK_ZOMBIE_PLAY_EAT_SEQUENCE,					(float)0	},
	{ TASK_ZOMBIE_PLAY_EAT_SEQUENCE,					(float)0	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_STAND			},
	//{ TASK_PLAY_SEQUENCE,					(float)ACT_THREAT_DISPLAY	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_IDLE			},   //verify, is that okay?

	{ TASK_PLAY_SEQUENCE,					(float)ACT_CROUCH			},
	{ TASK_ZOMBIE_PLAY_EAT_SEQUENCE,					(float)0	},
	{ TASK_ZOMBIE_PLAY_EAT_SEQUENCE,					(float)0	},
	{ TASK_ZOMBIE_PLAY_EAT_SEQUENCE,					(float)0	},
	{ TASK_ZOMBIE_PLAY_EAT_SEQUENCE,					(float)0	},
	{ TASK_ZOMBIE_PLAY_EAT_SEQUENCE,					(float)0	},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_STAND			},
	{ TASK_FORGET_CORPSE,		0				},
};

Schedule_t slZombieVictoryDance[] =
{
	{ 
		tlZombieVictoryDance,
		ARRAYSIZE ( tlZombieVictoryDance ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"ZombieVictoryDance"
	},
};



Task_t tlZombieVictoryDanceValid[] =
{
	//if this fails, wait longer than usual.
	{ TASK_SET_FAIL_SCHEDULE,				(float)SCHED_ZOMBIE_SEEK_CORPSE_EARLY_FAIL	},
	{ TASK_ZOMBIE_VICTORY_DANCE_VALID_CHECK,	(float)0					},
	{ TASK_FACE_CORPSE,						(float)0					},
	{ TASK_SET_SCHEDULE,    (float)SCHED_ZOMBIE_VICTORY_DANCE			},
	
};


Schedule_t slZombieVictoryDanceValid[] =
{
	{ 
		tlZombieVictoryDanceValid,
		ARRAYSIZE ( tlZombieVictoryDanceValid ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"ZombieVictoryDanceValid"
	},
};


Task_t	tlZombieSeekCorpseEarlyFail[] =
{
	{ TASK_FORGET_CORPSE,		0				},
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)6		},

	//MODDD - probably safe?
	{ TASK_FACE_IDEAL,			(float)0	},

	//{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slZombieSeekCorpseEarlyFail[] =
{
	{
		tlZombieSeekCorpseEarlyFail,
		ARRAYSIZE ( tlZombieSeekCorpseEarlyFail ),
		bits_COND_CAN_ATTACK,
		0,
		"ZombieSeekCorpseEarlyFail"
	},
};


Task_t	tlZombieSeekCorpseQuickFail[] =
{
	{ TASK_FORGET_CORPSE,		0				},
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_CHECK_GETUP_SEQUENCE, 0				},
	{ TASK_WAIT,				(float)0.2		},

	//MODDD - is this a good idea?
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_ENEMY,			(float)0    },
	{ TASK_FACE_IDEAL,			(float)0	},
	//{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slZombieSeekCorpseQuickFail[] =
{
	{
		tlZombieSeekCorpseQuickFail,
		ARRAYSIZE ( tlZombieSeekCorpseQuickFail ),
		0,
		0,
		"ZombieSeekCorpseQuickFail"
	},
};


DEFINE_CUSTOM_SCHEDULES( CZombie )
{
	slZombieMoveToCorpse,
	slZombieVictoryDance,
	slZombieVictoryDanceValid,
	slZombieSeekCorpseEarlyFail,
	slZombieSeekCorpseQuickFail,
};

IMPLEMENT_CUSTOM_SCHEDULES( CZombie, CBaseMonster );



//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CZombie::Classify ( void )
{
	return	CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CZombie::SetYawSpeed ( void )
{
	int ys;

	ys = 120;

#if 0
	switch ( m_Activity )
	{
	}
#endif

	pev->yaw_speed = ys;
}


GENERATE_TRACEATTACK_IMPLEMENTATION(CZombie)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}

void CZombie::TraceAttack_Traceless(entvars_t* pevAttacker, float flDamage, Vector vecDir, int bitsDamageType, int bitsDamageTypeMod) {
	// ... what?
	flPushbackForceDamage += flDamage;
	CBaseMonster::TraceAttack_Traceless(pevAttacker, flDamage, vecDir, bitsDamageType, bitsDamageTypeMod);
}//TraceAttack_Traceless

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CZombie)
{
	// damagetype checks moved to TraceAttack to check each time damage is taken before it is all applied at once (calls TakeDamage here).
	if(
		flPushbackForceDamage > 0 &&
		EASY_CVAR_GET_DEBUGONLY(zombieBulletPushback) != 0 &&
		m_MonsterState != MONSTERSTATE_SCRIPT &&
		pev->movetype != MOVETYPE_FLY &&
		IsAlive_FromAI(NULL)  //counts being into the DEAD_DYING deadflag somewhat
	){

		CBaseEntity* pInflictor = CBaseEntity::Instance(pevInflictor);

		// Keep in mind, pInflictor is whatever is dealing the damage directly.  It can be the player hitting me with bullets,
		// or with a projectile (the crossbowbolt is the inflictor instead).  In either case, pAttacker is still the player,
		// the start of the damage chain (player fired the arrow that hit me).

		// ALSO - require the inflictor to be a monster.
		// This disallows projectiles like crossbowbolts from counting as bullet damage (as they also use DMG_BULLET).
		// Could always make some "IsProjectile" method for entities that anything but grenades/bolts return FALSE to.
		// OR just DMG_PROJECTILE, for any damage dealt by an impacting projectile.
		
		
		//if (pInflictor && pInflictor->GetMonsterPointer()!=NULL ) {
		if(pInflictor != NULL &&  !(m_bitsDamageTypeMod & DMG_PROJECTILE) ){
			Vector vecDir = (Center() - pInflictor->Center()).Normalize();

			// some bonus to give smaller attacks more impact.
			flPushbackForceDamage *= 1.27;


			if (pev->deadflag == DEAD_NO) {

				if (flPushbackForceDamage <= 12) {
					// no change needed.
				}
				else {
					// Amounts over that much become decreasinlgly less
					flPushbackForceDamage = 11 + pow(flPushbackForceDamage - 11.0f, 0.84f);
				}
			}else{
				// And why is this force amplified so much otherwise while in DEAD_DYING?  No idea.
				// Offset that with this.  And remember, the movetype is still MOVETYPE_STEP.
				//    just.     weird.
				flPushbackForceDamage *= 0.006;
			}


			// check skill.cfg (sk_zombie_bulletpushback) for how intense the pushback is per difficulty.
			pev->velocity = pev->velocity + vecDir * (DamageForce(flPushbackForceDamage) * gSkillData.zombieBulletPushback);

			if (pev->flags & FL_ONGROUND) {
				::UTIL_SetOrigin(pev, Vector(pev->origin.x, pev->origin.y, pev->origin.z + 0.4));
				pev->flags &= ~FL_ONGROUND;  //is this ok?
				pev->groundentity = NULL;

				//pev->effects |= EF_NOINTERP;
				//pev->renderfx |= STOPINTR;

				// NOTE - not worth setting the movetype to MOVETYPE_TOSS.  The change does not even arrive in time to stop
				// the weird glitchy 'teleport' effect from bad interpolation in studioModelRenderer.cpp, which has been
				// fixed separately anyway.  Even letting that script refer to MOVETYPE_TOSS as well as MOVETYPE_STEP does not
				// give its benefits for TOSS strangely.  Point is, not in the air long enough to benefit from TOSS and we just
				// lose the interpolation benefit (non-choppy movement between 0.1 second minimum think and client/server response
				// times).
				//pev->movetype = MOVETYPE_TOSS;
			}
		}
	}//END OF bullet pushback check
	
	// applied, maybe. Wipe it.
	flPushbackForceDamage = 0;

	//MODDD - as-is script removed.  But why was it ever here?  CBaseMonster's TakeDamage already calls PainSound.
	// CController also had this line.
	// HACK HACK -- until we fix this.
	//if ( IsAlive() )
	//	PainSound();

	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}



//MODDD - NEW.
// Let's reward head damage a little more.  It is the headcrab itself after all.
// Mod of CBaseMonster::hitgroupDamage, so calling the parent is not necessary.
float CZombie::hitgroupDamage(float flDamage, int bitsDamageType, int bitsDamageTypeMod, int iHitgroup) {

	// Allow bullet damage, but not from the gauss (also has DMG_BULLET), to contribute.
	// CHANGE, we're counting the gauss again.
	//BOOL dmgIsPureBullet = (bitsDamageType & DMG_BULLET && !(bitsDamageTypeMod & DMG_GAUSS));
	BOOL dmgIsPureBullet = (bitsDamageType & DMG_BULLET);
	float finalDamage;

	// ALSO - keep track of the damage dealt before adjusting for hitgroup.
	// And ADD IT UP.  We'll assume it applies the next time TakeDamage is called,
	// otherwise a full shotgun blast only does the pushback of one shell hitting. Not too exciting.


	//---just because the basemonster hitgroupDamage does this now---
	g_rawDamageCumula += flDamage;  //does not care about influence from hitboxes.

	// flPushbackForceDamage is for determining how much damage will affect knockback.
	if (dmgIsPureBullet) {
		// NEW REQUIREMENT.  Pushback damage only comes from non-headshots.   ok.
		if (iHitgroup != HITGROUP_HEAD) {
			flPushbackForceDamage += flDamage;
		}
	}

	if (!(bitsDamageTypeMod & DMG_HITBOX_EQUAL)) {
		// And, have some damage penalties for any groups but the head, BUT not for melee.
		// Melee is hard enough as it is.
		if (bitsDamageType & (DMG_SLASH | DMG_CLUB)) {
			// MELEE
			switch (iHitgroup)
			{
			case HITGROUP_GENERIC:
				finalDamage = flDamage;
			break;
			case HITGROUP_HEAD:
				if (g_iSkillLevel == SKILL_HARD) {
					// less help.
					finalDamage = flDamage * gSkillData.monHead * 1.00;
				}else {
					finalDamage = flDamage * gSkillData.monHead * 1.00;
				}
			break;
			case HITGROUP_CHEST:
				finalDamage = flDamage * gSkillData.monChest * 1.00;
			break;
			case HITGROUP_STOMACH:
				finalDamage = flDamage * gSkillData.monStomach * 1.00;
			break;
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				finalDamage = flDamage * gSkillData.monArm * 1.00;
			break;
			case HITGROUP_LEFTLEG:
			case HITGROUP_RIGHTLEG:
				finalDamage = flDamage * gSkillData.monLeg * 1.00;
			break;
			default:
				finalDamage = flDamage;
			break;
			}

		}
		else {
			// RANGED
			switch (iHitgroup)
			{
			case HITGROUP_GENERIC:
				finalDamage = flDamage;
			break;
			case HITGROUP_HEAD:
				// NOTICE - enhanced headshot damage idea canned.  Already appealing enough with
				// all 'bullet resistance' ignored on headshots now.
				if (g_iSkillLevel == SKILL_HARD) {
					finalDamage = flDamage * gSkillData.monHead * 1.00;
				}else {
					finalDamage = flDamage * gSkillData.monHead * 1.00;
				}
			break;
			case HITGROUP_CHEST:
				finalDamage = flDamage * gSkillData.monChest * 0.97;
			break;
			case HITGROUP_STOMACH:
				finalDamage = flDamage * gSkillData.monStomach * 0.97;
			break;
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				finalDamage = flDamage * gSkillData.monArm * 0.92;
			break;
			case HITGROUP_LEFTLEG:
			case HITGROUP_RIGHTLEG:
				finalDamage = flDamage * gSkillData.monLeg * 0.92;
			break;
			default:
				finalDamage = flDamage;
			break;
			}

		}// END OF bitsDamageType check
	}//END OF bitsDamageTypeMod check
	else {
		// DMG_HITBOX_EQUAL?  No hitbox influence.
		finalDamage = flDamage;
	}


	//MODDD - (comment below found as-is)
	// Take 30% damage from bullets
	if(EASY_CVAR_GET_DEBUGONLY(zombieBulletResistance) == 1 && (dmgIsPureBullet))
	{
		//flDamage *= 0.3;

		// Now, use a skill-related value (difficulty).
		float damageMulti;
		if(iHitgroup != HITGROUP_HEAD) {
			// normal behavior
			damageMulti = (100 - gSkillData.zombieBulletResistance) / 100.0f;
		}
		else {
			// cut the reduction by 24%.  Unloading a full mp5 on the head, even in hard-mode and not killing the zombie,
			// is pretty strange.
			// NOPE, 100% damage from headshots.  No reduction.
			//damageMulti = (100 - gSkillData.zombieBulletResistance * 0.76) / 100.0f;
			damageMulti = 1; //(100 - gSkillData.zombieBulletResistance * 0) / 100.0f;
		}

		finalDamage *= damageMulti;
	}//END OF zombieBulletResistance check


	return finalDamage;
}//hitgroupDamage



GENERATE_KILLED_IMPLEMENTATION(CZombie) {

	GENERATE_KILLED_PARENT_CALL(CBaseMonster);
}//END OF Killed


void CZombie::BecomeDead(void)
{

	/*
	pev->takedamage = DAMAGE_YES;// don't let autoaim aim at corpses.

	// give the corpse half of the monster's original maximum health. 
	pev->health = pev->max_health / 2;
	pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.

	// don't do the MOVETYPE_TOSS change.  Loses velocity from pushback.  Somehow.
	//pev->movetype = MOVETYPE_TOSS;
	*/

	// CHANGE,  copy from CBaseMonster.
	CBaseMonster::BecomeDead();
}



void CZombie::PainSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	if (RANDOM_LONG(0,5) < 2)
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CZombie::AlertSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pAlertSounds[ RANDOM_LONG(0,ARRAYSIZE(pAlertSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CZombie::IdleSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);

	// Play a random idle sound
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}

void CZombie::AttackSound( void )
{
	// Play a random attack sound
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pAttackSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CZombie::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case ZOMBIE_AE_ATTACK_RIGHT:
		{
			// do stuff for this event.
	//		ALERT( at_console, "Slash right!\n" );
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.zombieDmgOneSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
				{
					pHurt->pev->punchangle.z = -18;
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
				}

				// Play a random attack hit sound
				// How about this?
				//UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
				//playStandardMeleeAttackHitSound(pHurt, pAttackHitSounds, ARRAYSIZE(pAttackHitSounds), 1.0, ATTN_NORM, 100 + -5, 100 + 5);
				determineStandardMeleeAttackHitSound(pHurt);
			}
			else{ // Play a random attack miss sound
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			}

			if (RANDOM_LONG(0,1)){
				AttackSound();
			}
		}
		break;

		case ZOMBIE_AE_ATTACK_LEFT:
		{
			// do stuff for this event.
	//		ALERT( at_console, "Slash left!\n" );
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.zombieDmgOneSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
				{
					pHurt->pev->punchangle.z = 18;
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 100;
				}
				//UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
				//playStandardMeleeAttackHitSound(pHurt, pAttackHitSounds, ARRAYSIZE(pAttackHitSounds), 1.0, ATTN_NORM, 100 + -5, 100 + 5);
				determineStandardMeleeAttackHitSound(pHurt);
			}
			else{
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			}

			if (RANDOM_LONG(0,1)){
				AttackSound();
			}
		}
		break;

		case ZOMBIE_AE_ATTACK_BOTH:
		{
			// do stuff for this event.
			//MODDD - add bleeding via the new mask (2nd bitmask arg).
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.zombieDmgBothSlash, DMG_SLASH, DMG_BLEEDING );
			if ( pHurt )
			{
				if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
				{
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100;
				}
				//UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
				//playStandardMeleeAttackHitSound(pHurt, pAttackHitSounds, ARRAYSIZE(pAttackHitSounds), 1.0, ATTN_NORM, 100 + -5, 100 + 5);
				determineStandardMeleeAttackHitSound(pHurt);
			}
			else{
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			}

			if (RANDOM_LONG(0,1)){
				AttackSound();
			}
		}
		break;

		default:{
			CBaseMonster::HandleAnimEvent( pEvent );
		break;}
	}
}


CZombie::CZombie(){
	corpseToSeek = NULL;
	m_hEnemy_CopyRef = NULL;
	lookForCorpseTime = -1;
	nextCorpseCheckTime = -1;
	flPushbackForceDamage = 0;

}


//=========================================================
// Spawn
//=========================================================
void CZombie::Spawn()
{
	Precache( );

	pev->classname = MAKE_STRING("monster_zombie");
	SET_MODEL(ENT(pev), "models/zombie.mdl");
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid			= SOLID_SLIDEBOX;

	// ?????????
	pev->movetype		= MOVETYPE_STEP;
	//pev->movetype = MOVETYPE_TOSS;

	// NOTE - this is ignored now!  See the BloodColor override in zombie.h
	m_bloodColor		= BLOOD_COLOR_GREEN;

	pev->health			= gSkillData.zombieHealth;
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

	MonsterInit();
	
	SetTouch(&CZombie::ZombieTouch );

}

extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CZombie::Precache()
{
	PRECACHE_MODEL("models/zombie.mdl");

	global_useSentenceSave = TRUE;
	
	PRECACHE_SOUND_ARRAY(pAttackHitSounds)
	PRECACHE_SOUND_ARRAY(pAttackMissSounds)
	PRECACHE_SOUND_ARRAY(pAttackSounds)
	PRECACHE_SOUND_ARRAY(pIdleSounds)
	PRECACHE_SOUND_ARRAY(pAlertSounds)
	PRECACHE_SOUND_ARRAY(pPainSounds)
	
	global_useSentenceSave = FALSE;

}	

//=========================================================
// AI Schedules Specific to this monster
//=========================================================


int CZombie::IgnoreConditions ( void )
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	//MODDD - shouldn't that be... or MELEE_ATTACK2?  why 1 twice?
	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK2))
	{
#if 0
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE);
		else
#endif			
		if (m_flNextFlinch >= gpGlobals->time)
			iIgnore |= (bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + ZOMBIE_FLINCH_DELAY;
	}

	return iIgnore;
}


void CZombie::MonsterThink(){

	// Debug feature for serverside printouts.
	// Place a zombie and watch printouts only go to the server console (or player running the server if non-deicated).
	// Uh. Yay.
	/*
	static float tempTime = 0;

	//if (tempTime == 0 || gpGlobals->time >= tempTime) {
	//	tempTime = gpGlobals->time + 2;
	//	easyForcePrint("AIM here SON %.2f %s\n", 4.27f, "abcdefg");
	//	//easyForcePrintServer("BIM here SON %.2f %s\n", 4.27f, "abcdefg");
	//	ClientPrintAll(HUD_PRINTCONSOLE, "aw yea man %s %s end\n", "12.6", "text");
	//}

	if (tempTime == 0 || gpGlobals->time >= tempTime) {
		tempTime = gpGlobals->time + 2;
		easyPrintLineBroadcast("test %d %s", 666, "str");
	}
	*/
	
	
	if(lookForCorpseTime != -1 && gpGlobals->time >= lookForCorpseTime){
		//if we're done looking for a corpse, drop the last copy.
		lookForCorpseTime = -1;
		m_hEnemy_CopyRef = NULL;
	}
	
	if(m_hEnemy != NULL){
		//keep a copy of our enemy. This gets a special preference when it's time to look for corpses.
		//The enemy is lost by the game logic when dead of course, hence the copy to keep track of it. If not gibbed.
		this->m_hEnemy_CopyRef = m_hEnemy;
	}

	CBaseMonster::MonsterThink();
}//END OF MonsterThink




//=========================================================
// GetSchedule 
//=========================================================
Schedule_t* CZombie::GetSchedule( void )
{

	Schedule_t* endedSchedule = m_pSchedule;
	if(endedSchedule != NULL && endedSchedule == slWaitScript && pev->sequence == ZOMBIE_EATBODY && m_MonsterState != MONSTERSTATE_DEAD && m_IdealMonsterState != MONSTERSTATE_DEAD){
		//If this schedule was interrupted / ended and it was ZOMBIE_EATBODY, do a quick getup animation.
		//return GetScheduleOfType(SCHED_ZOMBIE_SEEK_CORPSE_QUICK_FAIL);
		
		return slZombieSeekCorpseQuickFail;
	}

	switch	( m_MonsterState )
	{
	case MONSTERSTATE_ALERT:
		{
			//MODDD TODO - in general, hearing sensitiviy selective per type of thing, like scents and not noise, may be nice too at some point?
			
			//MODDD - zombie eating removal
			/*
			// If a victory dance is possible, look for the corpse to eat for a little bit. Near the enemyLKP.
			if ( HasConditions( bits_COND_ENEMY_DEAD )){
				//return GetScheduleOfType ( SCHED_VICTORY_DANCE );

				corpseToSeek = NULL;  // in case there was one from before.
				lookForCorpseTime = gpGlobals->time + 26;
				// wait a little before moving. Also gives the death anim time to play to then count as dead.
				nextCorpseCheckTime = gpGlobals->time + RANDOM_FLOAT(1.7, 2.8);

			}
			*/

			/*
			if ( HasConditions(bits_COND_SMELL) )
			{
				// there's something stinky. 
				CSound		*pSound;

				pSound = PBestScent();
				if ( pSound )
					return GetScheduleOfType( SCHED_SQUID_WALLOW);
			}
			*/
			break;
		}
	case MONSTERSTATE_COMBAT:
		{

			break;
		}
	}

	return CBaseMonster::GetSchedule();
}//END OF GetSchedule



Schedule_t* CZombie::GetScheduleOfType( int Type){
	
	//MODDD - zombie eating removal
	/*
	switch(Type){	
		//MODDD - zombie eating removal
		case SCHED_ZOMBIE_VICTORY_DANCE:
			//This can only come from being close enough to the corpse and wanting to do the animation.
			return slZombieVictoryDance;
		break;
		case SCHED_ZOMBIE_VICTORY_DANCE_VALID:
			//am I close enough to do the dance now? check is needed if the moveToCorpse fails near the end.
			return slZombieVictoryDanceValid;
		break;
		case SCHED_ZOMBIE_SEEK_CORPSE:
			return &slZombieMoveToCorpse[ 0 ];
		break;
		case SCHED_ZOMBIE_SEEK_CORPSE_EARLY_FAIL:
			return &slZombieSeekCorpseEarlyFail[0];
		break;
		case SCHED_ZOMBIE_SEEK_CORPSE_QUICK_FAIL:
			return &slZombieSeekCorpseQuickFail[0];
		break;
	}//END OF switch(Type)
	*/
	
	return CBaseMonster::GetScheduleOfType(Type);
}//END OF GetScheduleOfType


void CZombie::StartTask(Task_t* pTask){

	switch( pTask->iTask ){
	case TASK_FORGET_CORPSE:{
		corpseToSeek = NULL;
		TaskComplete();
	break;}
	case TASK_CHECK_GETUP_SEQUENCE:{

		//Why is coming from "waitForScript"'s interruption leaving pev->sequence at 0 at this point?  WHO KNOWS.  WTF.
		if(this->m_Activity == ACT_CROUCH || pev->sequence == ZOMBIE_EATBODY){
			//we need to get up! Do a stand animation and complete when it is done (in RunTask)
			this->m_flFramerateSuggestion = 1.3;
			this->SetActivity(ACT_STAND);
		}else{
			//anything else? assume we're standing, just proceed.
			TaskComplete();
		}
	break;}

	case TASK_WAIT_FOR_MOVEMENT_DUMB:{
		if (MovementIsComplete())
		{
			TaskComplete();
			RouteClear();		// Stop moving
		}
	break;}

	//really started as a clone of TASK_GET_PATH_TO_ENEMY_CORPSE.
	//Just touches the distance allowed from the corpse a little to require being closer.
	case TASK_ZOMBIE_GET_PATH_TO_ENEMY_CORPSE:{

		/*
		UTIL_MakeVectors( pev->angles );
		//same dist as agrunt. hm.
		//TODO - snap the m_vecEnemyLKP point to the ground, and put above it slightly first?
		if ( BuildRoute ( m_vecEnemyLKP - gpGlobals->v_forward * 50, bits_MF_TO_LOCATION, NULL ) )
		{
			TaskComplete();
		}
		else
		{
			ALERT ( at_aiconsole, "GetPathToEnemyCorpse failed!!\n" );
			TaskFail();
			m_movementGoal = MOVEGOAL_NONE;
		}
		*/

		/*
		BOOL pass = FALSE;
		CSound* pScent;
		pScent = PBestScent();

		if ( pScent )
		{
			UTIL_MakeVectors( pev->angles );

			if(MoveToLocation( m_movementActivity, 2, pScent->m_vecOrigin - gpGlobals->v_forward * 50 )){
				TaskComplete();
				pass = TRUE;
			}
		}
		*/

		BOOL pass = FALSE;

		if ( corpseToSeek ){
			float distanceToCorpse = (corpseToSeek->pev->origin - this->pev->origin).Length();
			
			if(distanceToCorpse > 28){
				Vector forwardVector;
				
				//UTIL_MakeVectors( pev->angles );
				//forwardVector = gpGlobals->v_forward
				forwardVector = (corpseToSeek->pev->origin - pev->origin).Normalize(); //direction to the corpse is more helpful.

				if(MoveToLocation( m_movementActivity, 2, corpseToSeek->pev->origin - forwardVector * 50 )){
					TaskComplete();
					pass = TRUE;
				}

			}else{
				//closer than 50? just finish.
				TaskComplete();
				pass = TRUE;
			}

		}
		

		if(!pass){
			// no way to get there =(
			ALERT ( at_aiconsole, "TASK_ZOMBIE_GET_PATH_TO_ENEMY_CORPSE failed!!\n" );
				
			TaskFail();
			m_movementGoal = MOVEGOAL_NONE;
		}
	break;}
	case TASK_ZOMBIE_VICTORY_DANCE_VALID_CHECK:{
		//check. If I am close enough to that place, go ahead.
		float distanceToCorpse;

		if(corpseToSeek == NULL){
			TaskFail();
			break;
		}

		distanceToCorpse = (pev->origin - corpseToSeek->pev->origin).Length();  //m_vecEnemyLKP).Length();
		if(distanceToCorpse < 20){
			//close enough for eating anim, do it.  Next task is to do the eating anim (victory dance)
			TaskComplete();
		}else{
			//can't!
			TaskFail();
		}
	break;}
	case TASK_ZOMBIE_MOVE_CLOSER_TO_CORPSE:{
		//Try to get closer to the corpse.
		//But don't end up using the usual pathfinding, it will think the corpse is in the way.
		//Just do a simple trace. Is that path above clear? then we're good.
		TraceResult tr;
		Vector vecStart;
		Vector vecEnd;
		Vector absoluteEnd;
		BOOL pass = FALSE;
		BOOL traceHitCorpse = FALSE;
		Vector forwardVector;

		if(corpseToSeek == NULL){
			TaskFail();
			break;
		}

		
		float distanceToCorpse = (corpseToSeek->pev->origin - this->pev->origin).Length();
		if(distanceToCorpse <= 70){
			//early end check.
			TaskComplete();
			break;
		}

		vecStart = pev->origin + Vector(0, 0, 24);
		//UTIL_MakeVectors( pev->angles );
		//forwardVector = gpGlobals->v_forward
		forwardVector = (corpseToSeek->pev->origin - pev->origin).Normalize(); //direction to the corpse is more helpful.

		absoluteEnd = corpseToSeek->pev->origin - forwardVector * 9;//m_vecEnemyLKP - gpGlobals->v_forward * 8;
		vecEnd = absoluteEnd + Vector(0, 0, 24);
		
		UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, edict(), &tr );

		// !!! ZOMBIE PATH DEBUG
		//DebugLine_Setup(6, vecStart, vecEnd, tr.flFraction);


		if(tr.flFraction < 1 && tr.pHit != NULL){
			CBaseEntity* testHit = CBaseEntity::Instance(tr.pHit);
			if(testHit != NULL && testHit->pev->deadflag != DEAD_NO || testHit->edict() == corpseToSeek->edict() ){
				//what I hit is dead? okay.
				traceHitCorpse = TRUE;
			}else{
				traceHitCorpse = FALSE;
			}
		}
		
		//SLOPPY SLOPPY SLOPPY. do better TODO
		if((traceHitCorpse || (tr.flFraction >= 1 && !tr.fStartSolid && !tr.fAllSolid ) ))
		//if( ((!tr.fStartSolid && !tr.fAllSolid) || traceHitCorpse ) || (traceHitCorpse || tr.flFraction >= 1)
			//(tr.pHit != NULL && CBaseEntity::Instance(tr.pHit)->edict() == corpseToSeek->edict() || CBaseEntity::Instance(tr.pHit)->pev->deadflag != DEAD_NO  )
		//)
		{
			//if(tr.flFraction >= 1){
				pass = TRUE;
			//}else{
				//could still be ok. Is the thing we collide with dead?
				//this is a cheap shot to get around remembering what the last enemy was.
				
				//...moved above, now determines "traceHitCorpse".

			//}
		}else{
			pass = FALSE;
		}

		if(pass){
			//clear path? good.
			this->m_vecMoveGoal = absoluteEnd;
			m_movementGoal = MOVEGOAL_LOCATION;
			TaskComplete();
		}else{
			TaskFail();
			//that did not turn out so well?
		}

	break;}

	case TASK_ZOMBIE_PLAY_EAT_SEQUENCE:{
		SetSequenceByName("eatbody");
	}break;
	case TASK_FACE_CORPSE:{
		if(corpseToSeek != NULL){
			MakeIdealYaw ( corpseToSeek->pev->origin );
			SetTurnActivity(); 
		}else{
			TaskFail();
		}
		break;
	}break;
	


	default:
		CBaseMonster::StartTask( pTask );
	break;

	}//END OF switch


}//END OF StartTask


void CZombie::RunTask(Task_t* pTask){

	//bits_COND_HEAR_SOUND...? careful.
	if(!this->HasConditionsEither(bits_COND_NEW_ENEMY | bits_COND_SEE_HATE | bits_COND_SEE_FEAR | bits_COND_SEE_DISLIKE | bits_COND_SEE_ENEMY | bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE | bits_COND_CAN_ATTACK)){
		if(m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT && pev->deadflag == DEAD_NO){
		
			if(gpGlobals->time <= lookForCorpseTime){

				if(corpseToSeek == NULL && gpGlobals->time >= nextCorpseCheckTime){

					//check every 2 seconds at most.
					nextCorpseCheckTime = gpGlobals->time + 2;

					//allowed to look for the corpse near the LKP.
					CBaseEntity* corpseTest = getNearestDeadBody(m_vecEnemyLKP, 420);

					//if ( HasConditions(bits_COND_SMELL_FOOD) )
					if(corpseTest){
						/*
						CSound* pSound;
						pSound = PBestScent();
				
						//if ( pSound && (!FInViewCone ( &pSound->m_vecOrigin ) || !FVisible ( pSound->m_vecOrigin )) )
						//{
						//	// scent is behind or occluded
						//	return GetScheduleOfType( SCHED_SQUID_SNIFF_AND_EAT );
						//}
						if(pSound){
							DebugLine_SetupPoint(8, pSound->m_vecOrigin, 0, 0, 255);

							// food is right out in the open. Just go get it.
							return GetScheduleOfType( SCHED_ZOMBIE_SEEK_CORPSE );
						}
						*/

						//find it!
						this->corpseToSeek = corpseTest;
				
						//return GetScheduleOfType( SCHED_ZOMBIE_SEEK_CORPSE );
						ChangeSchedule( GetScheduleOfType( SCHED_ZOMBIE_SEEK_CORPSE ) );
						return;
					}

				}
			}//END OF lookForCorpseTime check
		}//END OF state and not-dead checks
	}//END OF conditions checks



	switch( pTask->iTask ){
	case TASK_CHECK_GETUP_SEQUENCE:{
		//This task didn't finish instantly because we are waiting for the stand animation to finish (from crouching, eating, whichever)
		
		if(m_fSequenceFinished){
			TaskComplete();
		}
	break;}
	case TASK_WAIT_FOR_MOVEMENT_DUMB:{
		if (MovementIsComplete())
		{
			TaskComplete();
			RouteClear();		// Stop moving
		}else{
			if(m_Route[m_iRouteIndex].iType & bits_MF_IS_GOAL){
				float distToGoal = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length2D();
				float distToGoal3d = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length();
				//easyForcePrintLine("TASK_WAIT_FOR_MOVEMENT_RANGE: distToGoal:%.2f req:%.2f", distToGoal, pTask->flData);

				//if(corpseToSeek!=NULL){
				//::UTIL_SetSize(corpseToSeek->pev, Vector(0,0,0), Vector(0,0,0));
				//}
				
				// !!! ZOMBIE PATH DEBUG
				//DebugLine_Setup(7, pev->origin, m_Route[ m_iRouteIndex ].vecLocation, 1.0);

				//TODO. make this 30, see if touch works.  maybe even 25.
				if(distToGoal < 11){///pTask->flData){
					//done!
					TaskComplete();
					RouteClear();		// Stop moving
				}
			}
		}
	break;}
	case TASK_ZOMBIE_PLAY_EAT_SEQUENCE:{
		if(this->m_fSequenceFinishedSinceLoop == TRUE){
			//finished? done.
			TaskComplete();
		}
	}break;
	case TASK_FACE_CORPSE:{
		//if(monsterID == 1)easyForcePrintLine("HOO MANN sched:%s ang:%.2f ideal:%.2f", m_pSchedule->pName, UTIL_AngleMod( pev->angles.y ), pev->ideal_yaw );
		
		if(corpseToSeek != NULL){
			MakeIdealYaw( corpseToSeek->pev->origin );
			ChangeYaw( pev->yaw_speed );
			//easyForcePrintLine("TASK_FACE_ENEMY: %s%d WHAT? yawdif:%.2f yaw_spd:%.2f", this->getClassname(), this->monsterID,    FlYawDiff(), pev->yaw_speed);

			if ( FacingIdeal() )
			{
				TaskComplete();
			}
		}else{
			TaskFail();
		}

		break;
	}break;
			

	/*
	case TASK_WHAT:{
		?
	break;}
	*/

	default:
		CBaseMonster::RunTask( pTask );
	break;

	}//END OF switch

}//END OF StartTask



	
BOOL CZombie::getMonsterBlockIdleAutoUpdate(void){
	if(m_pSchedule != slZombieVictoryDance){
		return FALSE;
	}else{
		//let that idle animation finish.
		return TRUE;
	}
}
BOOL CZombie::forceIdleFrameReset(void){
	return FALSE;
}
BOOL CZombie::usesAdvancedAnimSystem(void){
	return TRUE;
}

void CZombie::SetActivity( Activity NewActivity ){
	CBaseMonster::SetActivity(NewActivity);
}



int CZombie::tryActivitySubstitute(int activity){
	int i = 0;

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		//case ACT_VICTORY_DANCE:
		//	return LookupSequence("eatbody");
		//break;
		case ACT_CROUCH:
			return LookupSequence("eatbodystand");
		break;
		case ACT_STAND:
			return LookupSequence("eatbodystand");
		break;
	}//END OF switch

	//not handled by above? Rely on the model's anim for this activity if there is one.
	return CBaseAnimating::LookupActivity(activity);
}//END OF tryActivitySubstitute

int CZombie::LookupActivityHard(int activity){
	int i = 0;
	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	//any animation events in progress?  Clear it.
	resetEventQueue();

	//Within an ACTIVITY, pick an animation like this (with whatever logic / random check first):
	//    this->animEventQueuePush(10.0f / 30.0f, 3);  //Sets event #3 to happen at 1/3 of a second
	//    return LookupSequence("die_backwards");      //will play animation die_backwards

	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		//case ACT_VICTORY_DANCE:
		//	return LookupSequence("eatbody");
		//break;
		case ACT_MELEE_ATTACK1:
			//OVERRIDE.  Have a little better chance of picking the other melee one, will ya?
			if (RANDOM_FLOAT(0, 1) <= 0.57) {
				return LookupSequence("attack1");
			}
			else {
				return LookupSequence("attack2");
			}
		break;
		case ACT_CROUCH:
			m_flFramerateSuggestion = -1;  //crouch to stand anim backwards, that works.
			return LookupSequence("eatbodystand");
		break;
		case ACT_STAND:
			return LookupSequence("eatbodystand");
		break;
	}//END OF switch
	
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard


//Handles custom events sent from "LookupActivityHard", which sends events as timed delays along with picking an animation in script.
//So this handles script-provided events, not model ones.
void CZombie::HandleEventQueueEvent(int arg_eventID){

	switch(arg_eventID){
	case 0:
	{


	break;
	}
	case 1:
	{


	break;
	}
	}//END OF switch

}//END OF HandleEventQueueEvent


int CZombie::CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, BOOL doZCheck, float *pflDist ){

	int mahTask = getTaskNumber();

	if(this->getTaskNumber() == TASK_WAIT_FOR_MOVEMENT_DUMB){
		//you are dumb right now. just say okay.
		return LOCALMOVE_VALID;
	}

	//default behavior.
	return CBaseMonster::CheckLocalMove(vecStart, vecEnd, pTarget, doZCheck, pflDist);
}//END OF CheckLocalMove



//TODO - IIgnore too perhaps for some monsters?
int CZombie::ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_CARCASS	|   //NEW. zombies like these. But require they be fallen human foes.
			//bits_SOUND_MEAT		|
			//bits_SOUND_GARBAGE	|
			bits_SOUND_PLAYER	|
			//MODDD - hears bait.
			bits_SOUND_BAIT;
}

void CZombie::ZombieTouch( CBaseEntity *pOther ){

	if(this->getTaskNumber() == TASK_WAIT_FOR_MOVEMENT_DUMB &&
		pOther != NULL &&
		corpseToSeek != NULL &&
		pOther->edict() == corpseToSeek->edict()
	)
	{
		// complete this task!
		TaskComplete();
	}


	/*
	if (pOther->IsWorld() && pev->movetype == MOVETYPE_TOSS) {
		TraceResult tr;
		// is the ground below me now?
		Vector vecStart = pev->origin + Vector(0, 0, 1);
		Vector vecEnd = pev->origin + Vector(0, 0, -0.5);
		UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, this->edict(), &tr);
		if (tr.flFraction < 1.0) {
			pev->movetype = MOVETYPE_STEP;  //returned to ground
		}
	}
	*/

	/*
	if (pOther->IsWorld() && !(pev->flags & FL_ONGROUND)) {
		TraceResult tr;
		// is the ground below me now?
		Vector vecStart = pev->origin + Vector(0, 0, 1);
		Vector vecEnd = pev->origin + Vector(0, 0, -0.5);
		UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, this->edict(), &tr);
		if (tr.flFraction < 1.0) {
			//pev->effects &= ~EF_NOINTERP;  //returned to ground
			//pev->renderfx &= ~STOPINTR;
		}
	}
	*/



}//END OF ZombieTouch



//Yes, adds arguments not in the original method. Replaces it completely anyways.
CBaseEntity* CZombie::getNearestDeadBody(Vector argSearchOrigin, float argMaxDist){

	CBaseEntity* pEntityScan = NULL;
	CBaseMonster* testMon = NULL;
	float thisDistance;
	CBaseEntity* bestChoiceYet = NULL;
	float leastDistanceYet = argMaxDist; //furthest I go to a dead body.

	//does UTIL_MonstersInSphere work?
	while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, argSearchOrigin, argMaxDist )) != NULL)
	{
		if(pEntityScan->pev == this->pev){
			//is it me? skip it.
			continue;
		}

		testMon = pEntityScan->MyMonsterPointer();
		//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){

		//Don't try to eat leeches, too tiny. Nothing else this small leaves a corpse.
		
		int classifyResult = CLASS_NONE;
		if(testMon != NULL)classifyResult = testMon->Classify();
		
		if(
			testMon != NULL &&
			( (testMon->pev->deadflag == DEAD_DEAD || testMon->pev->deadflag == DEAD_DYING) || ( (testMon->pev->flags & (FL_CLIENT)) && testMon->pev->deadflag == DEAD_RESPAWNABLE && !(testMon->pev->effects &EF_NODRAW) ) ) &&
			testMon->isSizeGiant() == FALSE &&
			testMon->isOrganicLogic() &&
			//!(::FClassnameIs(testMon->pev, "monster_leech") ) && 
			//must be human to want to eat it. Horror trope.
			(classifyResult == CLASS_PLAYER || classifyResult == CLASS_HUMAN_PASSIVE || classifyResult == CLASS_HUMAN_MILITARY || classifyResult == CLASS_PLAYER_ALLY)
		)
		{

			if(m_hEnemy_CopyRef != NULL &&
				testMon->edict() == m_hEnemy_CopyRef->edict() 
			){
				//Special exception. If the thing we picked up is what we were chasing before it died, it gets picked.
				return testMon;
			}

			thisDistance = (testMon->pev->origin - argSearchOrigin).Length();
			
			if(thisDistance < leastDistanceYet){
				//WAIT, one more check. Look nearby for players.

				//UTIL_TraceLine ( node.m_vecOrigin + vecViewOffset, vecLookersOffset, ignore_monsters, ignore_glass,  ENT(pev), &tr );
				bestChoiceYet = testMon;
				leastDistanceYet = thisDistance;
				
			}//END OF minimum distance yet
		}//END OF entity scan null check

	}//END OF while loop through all entities in an area to see which are corpses.
	return bestChoiceYet;
}//END OF getNearestDeadBody


//MODDD - this is the event that runs alongside a schedule about to be changed (re-picking a schedule).
void CZombie::ScheduleChange(void){

	//...nevermind, not the best way to achieve what we wanted.
	CBaseMonster::ScheduleChange();

}//END OF ScheduleChange


void CZombie::OnCineCleanup(CCineMonster* pOldCine){

	/*
	//Generally treat this like base monsters do.  But if we're about to do an uncrouch, don't do anything special in this place.
	//REVOKED.  Do it per map as needed!
	if(pev->sequence == ZOMBIE_EATBODY){
		
	}else{
		CBaseMonster::OnCineCleanup(pOldCine);
	}
	*/

	CBaseMonster::OnCineCleanup(pOldCine);

}//END OF OnCineCleanup


Vector CZombie::BodyTarget(const Vector& posSrc) {
	if (m_Activity == ACT_CROUCH || pev->sequence == ZOMBIE_EATBODY) {
		// we're lower, make other things aim lower.
		// ALSO, do not just casually add "EyePosition"!  It is absolute, not relative.
		// So Center() plus something else absolute, adds the origin twice.  Not good, far from accurate.
		// ALSO, multiplying center by a decimal is a garbage idea.  Half brings it halfway to the origin
		// of the map.       Yeah.  Not sure what I was smoking here.
		//return Center( ) * 0.75 + EyePosition() * 0.25;
		return Center() - Vector(0,0,12);
	}
	else {
		//what the base monster does.
		return CBaseMonster::BodyTarget(posSrc);
	}
};

