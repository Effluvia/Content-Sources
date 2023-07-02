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
// Gargantua
//=========================================================
#include "gargantua.h"
#include "customentity.h"
#include "weapons.h"
#include "soundent.h"
#include "decals.h"
#include "explode.h"
#include "func_break.h"
#include "scripted.h"
#include "gib.h"
#include "util_debugdraw.h"
#include "defaultai.h"


//MODDD - extern
EASY_CVAR_EXTERN_DEBUGONLY(drawCollisionBoundsAtDeath)
EASY_CVAR_EXTERN_DEBUGONLY(drawHitBoundsAtDeath)
EASY_CVAR_EXTERN_DEBUGONLY(thoroughHitBoxUpdates)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)
EASY_CVAR_EXTERN_DEBUGONLY(gargantuaPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(gargantuaCorpseDeath)
EASY_CVAR_EXTERN_DEBUGONLY(gargantuaFallSound)
EASY_CVAR_EXTERN_DEBUGONLY(gargantuaBleeds)
EASY_CVAR_EXTERN_DEBUGONLY(animationKilledBoundsRemoval)
EASY_CVAR_EXTERN_DEBUGONLY(gargantuaKilledBoundsAssist)
EASY_CVAR_EXTERN_DEBUGONLY(animationFramerateMulti)
EASY_CVAR_EXTERN_DEBUGONLY(sv_gargantua_throwattack)


//MODDD - all attacksound and missound pitches increased, were 50 and 50 base, now 60 and 75.


// no need for this now
//#define bits_COND_GARG_FLAMETHROWER_PREATTACK_EXPIRED	( bits_COND_SPECIAL1 )


// Garg animation events
#define GARG_AE_SLASH_LEFT			1
//#define GARG_AE_BEAM_ATTACK_RIGHT	2		// No longer used
#define GARG_AE_LEFT_FOOT			3
#define GARG_AE_RIGHT_FOOT			4
#define GARG_AE_STOMP				5
#define GARG_AE_BREATHE				6


// Gargantua is immune to any damage but this
#define GARG_DAMAGE					(DMG_ENERGYBEAM|DMG_CRUSH|DMG_MORTAR|DMG_BLAST)
#define GARG_EYE_SPRITE_NAME		"sprites/gargeye1.spr"
#define GARG_BEAM_SPRITE_NAME		"sprites/xbeam3.spr"
#define GARG_BEAM_SPRITE2			"sprites/xbeam3.spr"
#define GARG_STOMP_SPRITE_NAME		"sprites/gargeye1.spr"
#define GARG_STOMP_BUZZ_SOUND		"weapons/mine_charge.wav"
#define GARG_GIB_MODEL				"models/metalplategibs.mdl"

//MODDD - was just ATTN_NORM?   Really?    Let my sounds carry further.
//#define ATTN_GARG					(ATTN_NORM)
#define ATTN_GARG					(0.64)

#define STOMP_SPRITE_COUNT			10


#define SPIRAL_INTERVAL		0.1  //025

//MODDD - twice as long for less sprites, too many sprite effects can crash.
//#define STOMP_INTERVAL	0.025
#define STOMP_INTERVAL		0.050

//MODDD - changed, was 80
const float GARG_MELEEATTACKDIST = 115.0;

//MODDD - constant split into three constants: how long it is visually, to what extent it deals damage,
// and how close the enemy must be to start the attack (added delays can make starting exactly at max range
// easy to abuse)
// ORIGINAL WAS 330 FOR ALL THESE
#define GARG_FLAME_LENGTH_EFFECT	400
#define GARG_FLAME_LENGTH_LOGIC		370
#define GARG_FLAME_LENGTH_AI		275

// Was 400.  If the monster is this far away while using the flamethrower, count the duration of the flamethrower
// down faster to try another attack or chase sooner.
#define GARG_FLAME_CANCEL_DIST 350

// Was 64.  If the thing hit by the flamethrower is under this distance from the center of the flame effect (?),
// it will take maximum damage.  Takes reduced the further out it is.
#define GARG_FLAME_MAX_DAMAGE_DIST 90


// If you wanna get technical these should be static vars of CGargantua buuuuuuuut it doesn't make a difference,
// so long as these names aren't collided with elsewhere.
int gStompSprite = 0;
int gGargGibModel = 0;

int g_gargantua_shootflames1_sequenceID = -1;
int g_gargantua_walk_sequenceID = -1;
int g_gargantua_run_sequenceID = -1;
int g_gargantua_throwbody_sequenceID = -1;


float g_gargantua_rawDamageCumula = 0;


//=========================================================
// AI Schedules Specific to this monster
//=========================================================

enum
{
	SCHED_GARG_FLAMETHROWER_FAIL = LAST_COMMON_SCHEDULE + 1,
};


enum
{
	TASK_SOUND_ATTACK = LAST_COMMON_TASK + 1,
	TASK_FLAME_SWEEP,
	TASK_WAIT_FOR_FLAMETHROWER_PREDELAY,
	TASK_PLAY_PRE_FLAMETHROWER_SEQUENCE,
};



Task_t	tlGargFlame[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	// { TASK_PLAY_SEQUENCE,		(float)ACT_SIGNAL1	},
	// IF the player goes out of range while playing the pre-flamethrower sequence,
	// force back to IDLE.  Pick a new sequence unconditionally.
	{ TASK_SET_FAIL_SCHEDULE,   (float)SCHED_GARG_FLAMETHROWER_FAIL},
	//MODDD - NEW.  Again, not to be confused for the 'preAttackDelay'.
	// This attack schedule gets called after the preAttackDelay finishes
	{ TASK_PLAY_PRE_FLAMETHROWER_SEQUENCE, (float)0	},
	{ TASK_SOUND_ATTACK,		(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_MELEE_ATTACK2 },
	//MODDD - changed, was 4.5.  Flamethrower max time active reduced.
	{ TASK_FLAME_SWEEP,			(float)3.7		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slGargFlame[] =
{
	{ 
		tlGargFlame,
		ARRAYSIZE ( tlGargFlame ),
		//MODDD - interruptable by heavy damage
		bits_COND_HEAVY_DAMAGE,
		0,
		"GargFlame"
	},
};




//MODDD - NEW.  For standing during the flamethrower pre-attack delay.
// Not interruptable by MELEE_ATTACK2 or else it would keep infinitely picked over and over
// while waiting for the pre-delay to expire.
// NOTICE - does not involve the 'shootflames1' animation, the stand-to-arms-in-position one.
// That comes automatically on detecting that the previous animation wasn't flame-related.
Task_t	tlGargFlamePreAttack[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	//{ TASK_WAIT_INDEFINITE,		(float)0		},
	{ TASK_WAIT_FOR_FLAMETHROWER_PREDELAY, (float)0 }
};

Schedule_t	slGargFlamePreAttack[] =
{
	{
		tlGargFlamePreAttack,
		ARRAYSIZE ( tlGargFlamePreAttack ), 
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_CAN_ATTACK |
		bits_COND_CAN_MELEE_ATTACK1,
		0,
		"GargFlamePreAttack"
	},
};


// Same as slFail from defaultai.cpp, but forces the act-change to pick a new sequence.
// Failing while getting the flamethrower out can be tricked into thinking there is no need for a 
// new sequence ( the pre-flamethrower anim that puts the arms in place still leaves the current act at IDLE).
// Also wait for less time (was 2), skip the PVS wait.
Task_t	tlGargFlamethrowerFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY_FORCE,	(float)ACT_IDLE },
	{ TASK_WAIT,				(float)0.6		},
	//{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slGargFlamethrowerFail[] =
{
	{
		tlGargFlamethrowerFail,
		ARRAYSIZE ( tlGargFlamethrowerFail ),
		bits_COND_CAN_ATTACK |
		//MODDD - new?  Retrying methods to get to an enemy when pathfinding fails despite a better enemy being closer isn't great.
		bits_COND_NEW_ENEMY
		,
		0,
		"GargFlamethrowerFail"
	},
};





Task_t	tlGargGenericFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },

	// how about this instead
	//{ TASK_WAIT,				(float)0.5		},
	{ TASK_WAIT_FACE_ENEMY,				(float)0.5		},
	
	//{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slGargGenericFail[] =
{
	{
		tlGargGenericFail,
		ARRAYSIZE ( tlGargGenericFail ),
		bits_COND_CAN_ATTACK |
		//MODDD - new?  Retrying methods to get to an enemy when pathfinding fails despite a better enemy being closer isn't great.
		bits_COND_NEW_ENEMY
		,
		0,
		"slGargGenericFail"
	},
};






// primary melee attack
Task_t	tlGargSwipe[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK1,		(float)0		},
};

Schedule_t	slGargSwipe[] =
{
	{ 
		tlGargSwipe,
		ARRAYSIZE ( tlGargSwipe ), 
		
		//MODDD - wait, why was this ever interruptable by MELEE_ATTACK2 (flamethrower)?
		// If this attack started it may as well finish, higher framerates now anyway.
		//bits_COND_CAN_MELEE_ATTACK2 ||
		//MODDD - interruptable by heavy damage
		bits_COND_HEAVY_DAMAGE,
		0,
		"GargSwipe"
	},
};




//MODDD - NEW.  Turned the stomp attack into a custom schedule for more control.
// Was re-using the standard slRangeAttack1 schedule (defaultai.cpp).
// Also interruptable by less things like most other garg schedules.
Task_t	tlGargStompAttack[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};


Schedule_t	slGargStompAttack[] =
{
	{ 
		tlGargStompAttack,
		ARRAYSIZE ( tlGargStompAttack ), 
		//bits_COND_NEW_ENEMY		|
		//bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,
		
		0,
		"GargStompAttack"
	},
};





//MODDD - new.  Clone of ChaseEnemySmart (defaultai.cpp).
// Oh.  Nothing different about this.  Well.  Oops.
Task_t tlGargChaseEnemySmart[] =
{
	//{ TASK_SET_ACTIVITY, (float)ACT_IDLE },   //MODDD is this okay?
	{ TASK_STOP_MOVING, 0},

	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_CHASE_ENEMY_FAILED	},
	//{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	//{ TASK_RUN_PATH,			(float)0		},
	{ TASK_MOVE_TO_ENEMY_RANGE,	(float)100		},
	{ TASK_CHECK_STUMPED,		(float)0		},
	{ TASK_STOP_MOVING, 0},
};

Schedule_t slGargChaseEnemySmart[] =
{
	{
		tlGargChaseEnemySmart,
		ARRAYSIZE ( tlGargChaseEnemySmart ),
		bits_COND_NEW_ENEMY			|
		//MODDD - added, the bullsquid counts this.  Why doesn't everything?
		bits_COND_ENEMY_DEAD |
		
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_TASK_FAILED		|
		bits_COND_HEAR_SOUND |
		//bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		
		bits_SOUND_DANGER,
		"GargChaseEnemySmart"
	},
};





DEFINE_CUSTOM_SCHEDULES( CGargantua )
{
	slGargFlame,
	slGargFlamePreAttack,
	slGargFlamethrowerFail,
	slGargSwipe,
	slGargStompAttack,
	slGargChaseEnemySmart,
	slGargGenericFail,
};

IMPLEMENT_CUSTOM_SCHEDULES( CGargantua, CBaseMonster );






//----------------------------------------------------------------------------------
// Effects/attack-related classes

// Spiral Effect
class CSpiral : public CBaseEntity
{
public:
	void Spawn( void );
	void Think( void );
	int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
	static CSpiral *Create( const Vector &origin, float height, float radius, float duration );
};
LINK_ENTITY_TO_CLASS( streak_spiral, CSpiral );


class CStomp : public CBaseEntity
{
public:
	// now you need save/restore?
	float maxSpeed;

	
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];


	CStomp(void);
	BOOL usesSoundSentenceSave(void);
	void Spawn( void );
	void Think( void );
	static CStomp *StompCreate( const Vector &origin, const Vector &dir, float theDist, float speed, float maxSpeed, edict_t* theCreator  );

private:
// UNDONE: re-use this sprite list instead of creating new ones all the time
//	CSprite		*m_pSprites[ STOMP_SPRITE_COUNT ];
};

LINK_ENTITY_TO_CLASS( garg_stomp, CStomp );







class CSmoker : public CBaseEntity
{
public:
	void Spawn( void );
	void Think( void );
};

LINK_ENTITY_TO_CLASS( env_smoker, CSmoker );





// local method (to this file)
void StreakSplash( const Vector &origin, const Vector &direction, int color, int count, int speed, int velocityRange )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, origin );
	WRITE_BYTE( TE_STREAK_SPLASH );
	WRITE_COORD( origin.x );		// origin
	WRITE_COORD( origin.y );
	WRITE_COORD( origin.z );
	WRITE_COORD( direction.x );	// direction
	WRITE_COORD( direction.y );
	WRITE_COORD( direction.z );
	WRITE_BYTE( color );	// Streak color 6
	WRITE_SHORT( count );	// count
	WRITE_SHORT( speed );
	WRITE_SHORT( velocityRange );	// Random velocity modifier
	MESSAGE_END();
}









void CSpiral::Spawn( void )
{
	pev->movetype = MOVETYPE_NONE;
	pev->nextthink = gpGlobals->time;
	pev->solid = SOLID_NOT;
	UTIL_SetSize(pev, g_vecZero, g_vecZero );
	pev->effects |= EF_NODRAW;
	pev->angles = g_vecZero;
}


CSpiral* CSpiral::Create(const Vector& origin, float height, float radius, float duration)
{
	if (duration <= 0)
		return NULL;

	CSpiral* pSpiral = GetClassPtr((CSpiral*)NULL);
	pSpiral->Spawn();
	pSpiral->pev->dmgtime = pSpiral->pev->nextthink;
	pSpiral->pev->origin = origin;
	pSpiral->pev->scale = radius;
	pSpiral->pev->dmg = height;
	pSpiral->pev->speed = duration;
	pSpiral->pev->health = 0;
	pSpiral->pev->angles = g_vecZero;

	return pSpiral;
}


void CSpiral::Think(void)
{
	float time = gpGlobals->time - pev->dmgtime;

	while (time > SPIRAL_INTERVAL)
	{
		Vector position = pev->origin;
		Vector direction = Vector(0, 0, 1);

		float fraction = 1.0 / pev->speed;

		float radius = (pev->scale * pev->health) * fraction;

		position.z += (pev->health * pev->dmg) * fraction;
		pev->angles.y = (pev->health * 360 * 8) * fraction;
		UTIL_MakeAimVectors(pev->angles);
		position = position + gpGlobals->v_forward * radius;
		direction = (direction + gpGlobals->v_forward).Normalize();

		StreakSplash(position, Vector(0, 0, 1), RANDOM_LONG(8, 11), 20, RANDOM_LONG(50, 150), 400);

		// Jeez, how many counters should this take ? :)
		pev->dmgtime += SPIRAL_INTERVAL;
		pev->health += SPIRAL_INTERVAL;
		time -= SPIRAL_INTERVAL;
	}

	pev->nextthink = gpGlobals->time;

	if (pev->health >= pev->speed)
		UTIL_Remove(this);
}




//MODDD - requires the ent (gargantua) that created this.
// Also takes the 'dir' (expected normalized) and length instead of the end vector to generate the dir from.  And takes a maxSpeed as well.
CStomp *CStomp::StompCreate( const Vector &origin, const Vector &dir, float theDist, float speed, float maxSpeed, edict_t* theCreator )
{
	CStomp *pStomp = GetClassPtr( (CStomp *)NULL );
	
	pStomp->pev->origin = origin;
	//MODDD - scale is the 'life' of the stomp.  May as well go somewhat further.
	pStomp->pev->scale = theDist * 1.8;
	pStomp->pev->movedir = dir;
	pStomp->pev->speed = speed;
	pStomp->maxSpeed = maxSpeed;

	//MODDD - and why did as-is never set the owner to know not to damage the garg that made this stomp?  (rare but still)
	pStomp->pev->owner = theCreator;

	pStomp->Spawn();
	
	return pStomp;
}


TYPEDESCRIPTION	CStomp::m_SaveData[] = 
{
	DEFINE_FIELD( CStomp, maxSpeed, FIELD_FLOAT ),
};
IMPLEMENT_SAVERESTORE( CStomp, CBaseEntity );



CStomp::CStomp(void){
	
}
BOOL CStomp::usesSoundSentenceSave(void){
	return TRUE;
}


void CStomp::Spawn( void )
{
	pev->nextthink = gpGlobals->time;
	pev->classname = MAKE_STRING("garg_stomp");
	pev->dmgtime = gpGlobals->time;

	pev->framerate = 30;
	pev->model = MAKE_STRING(GARG_STOMP_SPRITE_NAME);
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 0;


	//MODDD - uhhh.  why not?
	// nope, blocks proper incline/wall-skipping.  because of course it does.
	//pev->movetype = MOVETYPE_STEP;
	//pev->solid = SOLID_NOT;
	//UTIL_SetSize(pev, g_vecZero, g_vecZero );

	float attn;
	float vol;
	if(g_iSkillLevel == SKILL_HARD){
		// carries less far, since there's six of these things
		attn = ATTN_NORM - 0.08;
		vol = 0.94;
	}else{
		// normal sound stats
		attn = ATTN_NORM - 0.20;
		vol = 1;
	}

	UTIL_PlaySound( edict(), CHAN_BODY, GARG_STOMP_BUZZ_SOUND, vol, attn, 0, PITCH_NORM * 0.55, FALSE);
}




void CStomp::Think( void )
{
	//MODDD - NOTE.  BEWARE.  gpGlobals->frametime is not a reliable measure of the time since the previous think-frame.
	// In fact it's a terrible one.  It's better for seeing the difference between render frames.  Which... kinda doesn't
	// have a use in much of serverside in dealing with most think-related things.
	// Assume a difference of 0.1 since that's standard for think times anyway, definitely here too.
	// (for retail, replace 0.1 with gpGlobals->frametime anyway)
	float betweenThinkTime = 0.1;


	TraceResult tr;

	pev->nextthink = gpGlobals->time + 0.1;

	// Do damage for this frame
	Vector vecStart = pev->origin;
	vecStart.z += 30;
	Vector vecEnd = vecStart + (pev->movedir * pev->speed * betweenThinkTime);

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
	
	if ( tr.pHit && tr.pHit != pev->owner )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		entvars_t *pevOwner = pev;
		if ( pev->owner ){
			pevOwner = VARS(pev->owner);
		}

		if ( pEntity ){
			//MODDD - cutting stomp damage in half, being near-enough to insta kill from standing dead-center
			// in the way seems like a bit much, this would still be bad.
			pEntity->TakeDamage( pev, pevOwner, gSkillData.gargantuaDmgStomp * 0.4, DMG_SONIC, 0 );
		}
	}
	
	// Accelerate the effect
	pev->speed = pev->speed + (betweenThinkTime) * pev->framerate;

	// maxSpeed
	if(pev->speed > maxSpeed){
		// cap it
		pev->speed = maxSpeed;
	}


	pev->framerate = pev->framerate + (betweenThinkTime) * 1500;
	

	// This... isn't working out so great, nevermind.
	/*
	//float myYaw = UTIL_VecToYaw ( pev->movedir );
	float myYaw = UTIL_VecToYawRadians(pev->movedir);

	// record the origin before/after this move.  Effects will be planted between these points.
	Vector prevOrigin = pev->origin;
	BOOL walkSuccess = WALK_MOVE(ENT(pev), myYaw * (180.0f/M_PI), pev->speed * betweenThinkTime, WALKMOVE_NORMAL);

	
	//if(!walkSuccess){
	//	// oh dear?
	//	UTIL_Remove(this);
	//	UTIL_StopSound( edict(), CHAN_BODY, GARG_STOMP_BUZZ_SOUND, FALSE );
	//	return;
	//}

	Vector newOrigin = pev->origin;

	Vector originDelta = newOrigin - prevOrigin;
	Vector moveDir = originDelta.Normalize();
	float originDist = originDelta.Length();

	// and how many times does STOMP_INTERVAL fit into the dmgtime difference from gpGlobals->time?
	float timez = (gpGlobals->time - pev->dmgtime) / STOMP_INTERVAL;
	float eachTimeOriginShift = originDist / timez;
	
	// shift this instead?  Starts at the origin before move, gets moved into the newOrigin place.
	Vector imaginaryOrigin = prevOrigin;
	*/

	// Move and spawn trails
	while ( gpGlobals->time - pev->dmgtime > STOMP_INTERVAL )
	{
		//MODDD - replaced, maybe?
		// (if using imaginaryOrigin instead, replace any mentions of this->pev->origin with that)
		pev->origin = pev->origin + pev->movedir * pev->speed * STOMP_INTERVAL;

		// inch closer to the current origin.
		//imaginaryOrigin = imaginaryOrigin + pev->movedir * eachTimeOriginShift;


		//MODDD - make fewer of these, it gets crashy.  One is plenty this iteration.
		//for ( int i = 0; i < 2; i++ )
		for ( int i = 0; i < 1; i++ )
		{
			CSprite *pSprite = CSprite::SpriteCreate( GARG_STOMP_SPRITE_NAME, this->pev->origin, TRUE );
			if ( pSprite )
			{
				UTIL_TraceLine( this->pev->origin, this->pev->origin - Vector(0,0,500), ignore_monsters, edict(), &tr );
				pSprite->pev->origin = tr.vecEndPos;
				pSprite->pev->velocity = Vector(RANDOM_FLOAT(-200,200),RANDOM_FLOAT(-200,200),175);
				// pSprite->AnimateAndDie( RANDOM_FLOAT( 8.0, 12.0 ) );
				pSprite->pev->nextthink = gpGlobals->time + 0.3;
				pSprite->SetThink( &CBaseEntity::SUB_Remove );
				pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxFadeFast );
			}
		}
		pev->dmgtime += STOMP_INTERVAL;
		// Scale has the "life" of this effect
		pev->scale -= STOMP_INTERVAL * pev->speed;
		if ( pev->scale <= 0 )
		{
			// Life has run out
			UTIL_Remove(this);
			UTIL_StopSound( edict(), CHAN_BODY, GARG_STOMP_BUZZ_SOUND, FALSE );
			//MODDDD - about to be removed?  Why bother with the rest of this loop then?
			break;
		}
	}//while loop through stomp interval


}//Think



void CSmoker::Spawn( void )
{
	pev->movetype = MOVETYPE_NONE;
	pev->nextthink = gpGlobals->time;
	pev->solid = SOLID_NOT;
	UTIL_SetSize(pev, g_vecZero, g_vecZero );
	pev->effects |= EF_NODRAW;
	pev->angles = g_vecZero;
}

void CSmoker::Think( void )
{
	// lots of smoke
	UTIL_Smoke(MSG_PVS, pev->origin, NULL, pev->origin, RANDOM_FLOAT( -pev->dmg, pev->dmg ), RANDOM_FLOAT( -pev->dmg, pev->dmg ), 0, g_sModelIndexSmoke, RANDOM_LONG(pev->scale, pev->scale * 1.1), RANDOM_LONG(8,14));

	pev->health--;
	if ( pev->health > 0 )
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.2);
	else
		UTIL_Remove( this );
}





//----------------------------------------------------------------------------------
// GARGANTUA IMPLEMENTATIONS
//----------------------------------------------------------------------------------

// class CGargantua moved to gargantua.h

#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_gargantua, CGargantua );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( gargantua, CGargantua );
	
	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( alien_gargantua, CGargantua );
		LINK_ENTITY_TO_CLASS( monster_alien_gargantua, CGargantua );
		LINK_ENTITY_TO_CLASS( agargantua, CGargantua );
	#endif

#endif

TYPEDESCRIPTION	CGargantua::m_SaveData[] = 
{
	DEFINE_FIELD( CGargantua, m_pEyeGlow, FIELD_CLASSPTR ),
	DEFINE_FIELD( CGargantua, m_eyeBrightness, FIELD_INTEGER ),
	DEFINE_FIELD( CGargantua, m_seeTime, FIELD_TIME ),
	DEFINE_FIELD( CGargantua, m_flameTime, FIELD_TIME ),
	DEFINE_FIELD( CGargantua, m_streakTime, FIELD_TIME ),
	DEFINE_FIELD( CGargantua, m_painSoundTime, FIELD_TIME ),
	DEFINE_ARRAY( CGargantua, m_pFlame, FIELD_CLASSPTR, 4 ),
	DEFINE_FIELD( CGargantua, m_flameX, FIELD_FLOAT ),
	DEFINE_FIELD( CGargantua, m_flameY, FIELD_FLOAT ),
	DEFINE_FIELD( CGargantua, pissedRunTime, FIELD_TIME ),
};
//IMPLEMENT_SAVERESTORE( CGargantua, CBaseMonster );

int CGargantua::Save( CSave &save )
{
	if ( !CBaseMonster::Save(save) )
		return 0;
	int writeFieldsResult = save.WriteFields( "CGargantua", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return writeFieldsResult;
}
int CGargantua::Restore( CRestore &restore )
{
	if ( !CBaseMonster::Restore(restore) )
		return 0;
	int readFieldsResult = restore.ReadFields( "CGargantua", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	PostRestore();
	return readFieldsResult;
}

void CGargantua::PostRestore() {
	if (nextNormalThinkTime == 0)nextNormalThinkTime = 0.01;
	if (pev->nextthink == 0)pev->nextthink = 0.01;

}


const char *CGargantua::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CGargantua::pBeamAttackSounds[] = 
{
	"garg/gar_flameoff1.wav",
	"garg/gar_flameon1.wav",
	"garg/gar_flamerun1.wav",
};


const char *CGargantua::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};


//now unused.
/*
const char *CGargantua::pRicSounds[] = 
{
#if 0
	"weapons/ric1.wav",
	"weapons/ric2.wav",
	"weapons/ric3.wav",
	"weapons/ric4.wav",
	"weapons/ric5.wav",
#else
	"debris/metal4.wav",
	"debris/metal6.wav",
	"weapons/ric4.wav",
	"weapons/ric5.wav",
#endif
};
*/

const char *CGargantua::pFootSounds[] = 
{
	"garg/gar_step1.wav",
	"garg/gar_step2.wav",
};


const char *CGargantua::pIdleSounds[] = 
{
	"garg/gar_idle1.wav",
	"garg/gar_idle2.wav",
	"garg/gar_idle3.wav",
	"garg/gar_idle4.wav",
	"garg/gar_idle5.wav",
};


const char *CGargantua::pAttackSounds[] = 
{
	"garg/gar_attack1.wav",
	"garg/gar_attack2.wav",
	"garg/gar_attack3.wav",
};

const char *CGargantua::pAlertSounds[] = 
{
	"garg/gar_alert1.wav",
	"garg/gar_alert2.wav",
	"garg/gar_alert3.wav",
};

const char *CGargantua::pPainSounds[] = 
{
	"garg/gar_pain1.wav",
	"garg/gar_pain2.wav",
	"garg/gar_pain3.wav",
};

const char *CGargantua::pStompSounds[] = 
{
	"garg/gar_stomp1.wav",
};

const char *CGargantua::pBreatheSounds[] = 
{
	"garg/gar_breathe1.wav",
	"garg/gar_breathe2.wav",
	"garg/gar_breathe3.wav",
};





CGargantua::CGargantua(void){

	gargDeadBoundChangeYet = FALSE;
	fallShakeTime = -1;

	flameThrowerPreAttackDelay = -1;
	resetFlameThrowerPreAttackDelay = -1;
	flameThrowerPreAttackInterrupted = FALSE;
	pissedRunTime = -1;

	consecutiveStomps = 0;
	grabbedEnt = NULL;

	usingFastThink = FALSE;
	nextNormalThinkTime = 0;

}



void CGargantua::setModel(void){
	CGargantua::setModel(NULL);
}
void CGargantua::setModel(const char* m){
	CBaseMonster::setModel(m);

	if (g_gargantua_shootflames1_sequenceID == -1) {
		g_gargantua_shootflames1_sequenceID = LookupSequence("shootflames1");
		g_gargantua_walk_sequenceID = LookupSequence("walk");
		g_gargantua_run_sequenceID = LookupSequence("run");
		g_gargantua_throwbody_sequenceID = LookupSequence("throwbody");
	}
}

BOOL CGargantua::getMonsterBlockIdleAutoUpdate(void){
	if(m_pSchedule == slGargFlame){
		// don't allow idle-reset.
		return TRUE;
	}
	return FALSE;
}



void CGargantua::DeathSound(void){

	switch(RANDOM_LONG(0, 1)){
		case 0:
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "garg/gar_die1.wav", 1.0, ATTN_NORM - 0.18, 0, PITCH_NORM + RANDOM_LONG(-10,10) );
		break;
		case 1:
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "garg/gar_die2.wav", 1.0, ATTN_NORM - 0.18, 0, PITCH_NORM + RANDOM_LONG(-10,10) );
		break;
	}

}


int CGargantua::IRelationship( CBaseEntity *pTarget )
{
	
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1){
		return R_NO;
	}

	if ( (pTarget->IsPlayer()) )
		if ( (pev->spawnflags & SF_MONSTER_WAIT_UNTIL_PROVOKED ) && ! (m_afMemory & bits_MEMORY_PROVOKED ))
			return R_NO;
	return CBaseMonster::IRelationship( pTarget );
}



void CGargantua::SetFastThink(BOOL newVal){

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

void CGargantua::MonsterThink(void){


	//g_gargantua_throwbody_sequenceID
	/*
	Vector posGun, angleGun;
	//int attach = i%2;
	int attach = 0;
	// attachment is 0 based in GetAttachment
	GetAttachment( attach+1, posGun, angleGun );

	if(m_hEnemy != NULL){
		m_hEnemy->pev->origin = posGun;
	}
	*/

	BOOL doNormalThink;
	
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



	if(pev->sequence == g_gargantua_throwbody_sequenceID){

		if(grabbedEnt == NULL){
			// nevermind, check is handled by a queue event now.  Interrupts the melee schedule if there is no grabbed ent.
			//if(pev->frame >= (10.0f/27.0f) * 255.0f){
			//	// no grabbedEnt, yet this far into the animation?  Give up.
			//	TaskFail();
			//}
		}else{

			/*
			if(grabbedEnt->pev->deadflag != DEAD_NO){
				// should I drop dead things instantly? unsure
				releaseGrabbedEnt();
			}
			*/

			// force it to the arm
			Vector posGun, angleGun;
			//int attach = i%2;
			int attach = 0;
			// attachment is 0 based in GetAttachment
			GetAttachment( attach+0, posGun, angleGun );

			Vector finalEntPos;

			if(grabbedEnt->IsPlayer()){
				// bend things a little so that the player camera isn't looking through the garg
				// (want to make a custom var that adds an offset in cl_dlls/view.cpp?  Me neither.
				// Although the barnacle does that already, see 'global2PSEUDO_grabbedByBarancle'
				// and how it's filled.  Then again this is all pretty cheezy anyway.)
				// Player is centered on pev->origin so no need to adjust that.

				UTIL_MakeAimVectors(this->pev->angles);

				finalEntPos = posGun + gpGlobals->v_forward * 18 + -gpGlobals->v_right * 6 + Vector(0, 0, -32);


			}else{
				// Any other entity?  Assmue there is some difference in z that needs to be adjusted.
				finalEntPos = posGun;
				// multiple of 0 to 0.5 is from on top of the hand to mid-way,
				// multiple of 0.5 to 1 is from mid-way to below the hand. 
				finalEntPos.z += -(grabbedEnt->pev->maxs.z + grabbedEnt->pev->mins.z) * 0.67;
			}

			grabbedEnt->pev->origin = finalEntPos;
		}

	}//g_gargantua_throwbody_sequenceID check



	if(doNormalThink){
		if(pev->sequence != g_gargantua_throwbody_sequenceID && pev->yaw_speed == 0){
			easyPrintLine("WARNING!!! Garg sequence is not throwbody (%d) yet yaw_speed was 0, corrected!", pev->sequence);
			pev->yaw_speed = 60;
		}

		if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1 && this->m_fSequenceFinished){
			switch(RANDOM_LONG(0, 45)){
				case 0:
					this->SetSequenceByName("bust");
				break;
				case 1:
					this->SetSequenceByName("bust");
				break;
				case 2:
					this->SetSequenceByName("bust");
				break;
				case 3:
					this->SetSequenceByName("pushcar");
				break;
				case 4:
					this->SetSequenceByName("kickcar");
				break;
				case 5:
					this->SetSequenceByName("kickcar");
				break;
				case 6:
					this->SetSequenceByName("kickcar");
				break;
				case 7:
					this->SetSequenceByName("kickcar");
				break;
				case 8:
					this->SetSequenceByName("rollcar");
				break;
				case 9:
					this->SetSequenceByName("rollcar");
				break;
				case 10:
					this->SetSequenceByName("smash");
				break;
				case 11:
					this->SetSequenceByName("smash");
				break;
				case 12:
					this->SetSequenceByName("smash");
				break;
				case 13:
					this->SetSequenceByName("smash");
				break;
				case 14:
					this->SetSequenceByName("throwbody");
				break;
				case 15:
					this->SetSequenceByName("throwbody");
				break;
				case 16:
					this->SetSequenceByName("bitehead");
				break;
				case 17:
					this->SetSequenceByName("bitehead");
				break;
				case 18:
					this->SetSequenceByName("bitehead");
				break;
				case 19:
					this->SetSequenceByName("Flinchheavy");
				break;
				case 20:
					this->SetSequenceByName("Flinchheavy");
				break;
				case 21:
					this->SetSequenceByName("flinchlight");
				break;
				case 22:
					this->SetSequenceByName("flinchlight");
				break;
				case 23:
					this->SetSequenceByName("flinchlight");
				break;
				case 24:
					this->SetSequenceByName("flinchlight");
				break;
				case 25:
					this->SetSequenceByName("flinchlight");
				break;
				case 26:
					this->SetSequenceByName("flinchlight");
				break;
				case 27:
					this->SetSequenceByName("180right");
				break;
				case 28:
					this->SetSequenceByName("180right");
				break;
				case 29:
					this->SetSequenceByName("180left");
				break;
				case 30:
					this->SetSequenceByName("180left");
				break;
				case 31:
					this->SetSequenceByName("stomp");
				break;
				case 32:
					this->SetSequenceByName("Attack");
				break;
				case 33:
					this->SetSequenceByName("Attack");
				break;
				case 34:
					this->SetSequenceByName("Attack");
				break;
				case 35:
					this->SetSequenceByName("shootflames2");
				break;
				case 36:
					this->SetSequenceByName("shootflames2");
				break;
				case 37:
					this->SetSequenceByName("shootflames2");
				break;
				case 38:
					this->SetSequenceByName("shootflames1");
				break;
				case 39:
					this->SetSequenceByName("shootflames1");
				break;
				case 40:
					this->SetSequenceByName("shootflames1");
				break;
				case 41:
					this->SetSequenceByName("idle1");
				break;
				case 42:
					this->SetSequenceByName("idle1");
				break;
				case 43:
					this->SetSequenceByName("idle1");
				break;
				case 44:
					this->SetSequenceByName("idle2");
				break;
				case 45:
					this->SetSequenceByName("idle4");
				break;


			}

		}

		CBaseMonster::MonsterThink();
		//////////////////////////////////////////////////////////////
		// END OF normal think space
	}else{
		//
	}


	if(usingFastThink){
		// override the slower think speed that MonsterThink would set this to
		// (and this would never be done otherwise in frames that don't call MonsterThink)
		pev->nextthink = gpGlobals->time + 0.025;
	}
	// Why would this happen??  Don't report it when we're being deleted (supposed to happen)
	if (pev->nextthink <= 0 && !(pev->flags & FL_KILLME) ) {
		easyForcePrintLine("!!!WARNING!  Gargantua had a pev->nextthink at or below 0. This can kill the AI!");
		// save it.
		pev->nextthink = gpGlobals->time + 0.1;
	}

}//MonsterThink




void CGargantua::EyeOn( int level )
{
	m_eyeBrightness = level;	
}


void CGargantua::EyeOff( void )
{
	m_eyeBrightness = 0;
}


void CGargantua::EyeUpdate( void )
{
	if ( m_pEyeGlow )
	{
		m_pEyeGlow->pev->renderamt = UTIL_Approach( m_eyeBrightness, m_pEyeGlow->pev->renderamt, 26 );
		if ( m_pEyeGlow->pev->renderamt == 0 )
			m_pEyeGlow->pev->effects |= EF_NODRAW;
		else
			m_pEyeGlow->pev->effects &= ~EF_NODRAW;
		UTIL_SetOrigin( m_pEyeGlow->pev, pev->origin );
	}
}


void CGargantua::StompAttack( void )
{
	TraceResult trace;

	UTIL_MakeAimVectors( pev->angles );
	Vector vecStart = pev->origin + Vector(0,0,60) + 35 * gpGlobals->v_forward;

	// vecAim is fine for a direction, at least floor-wise, but why use it to see how far to place
	// the stomp-effect?  If some strong incline separates the garg from the enemy, a trace from
	// the garg to the enemy will run into the ramp and give a lot shorter distance than there actually
	// is to the enemy.  And clearly to fire in this direction he sees them anyway.
	// Don't need the trace, for this at least.
	//Vector vecAim = ShootAtEnemy( vecStart );
	//Vector vecEnd = (vecAim * 1024) + vecStart;
	//UTIL_TraceLine( vecStart, vecEnd, ignore_monsters, edict(), &trace );

	//Vector pointDelta = (trace.vecEndPos - vecStart);
	Vector pointDelta = (m_vecEnemyLKP - vecStart);

	// floor-wise only, no Z difference.
	// NOPE!  Do that and the effect doesn't travel over inclines or walls as well.
	// YYYYYYYyyyyyyyyyyyyyeeeeeeeeaaaaaaaaahhhhhhhhhh.
	//pointDelta.z = 0;

	Vector theDir = pointDelta.Normalize();
	float theDist = pointDelta.Length();

	// retail was a flat 0
	int stompStartSpeed;
	// NEW
	int stompMaxSpeed;

	if(g_iSkillLevel == SKILL_HARD){
		stompStartSpeed = 26;
		stompMaxSpeed = 300;
	}else if(g_iSkillLevel == SKILL_MEDIUM){
		stompStartSpeed = 20;
		stompMaxSpeed = 230;
	}else{
		stompStartSpeed = 12;
		stompMaxSpeed = 140;
	}


	// also, stompStartSpeed will be creater if the enemy is further away.
	if(theDist <= 200){
		// no change
	}else if(theDist <= 1300){
		// more distance, more start speed and max speed (but not as much)
		stompStartSpeed *= 1 + (theDist - 200) / (1300 - 200) * (3 - 1);
		stompMaxSpeed *= 1 + (theDist - 200) / (1300 - 200) * (1.8 - 1);
	}else{
		stompStartSpeed *= 3;
		stompMaxSpeed *= 1.8;
	}


	CStomp::StompCreate( vecStart, theDir, theDist, stompStartSpeed, stompMaxSpeed, this->edict() );
	//DebugLine_Setup(0, vecStart, vecStart + theDir * theDist, 0, 255, 0);

	if(g_iSkillLevel == SKILL_HARD){
		// spawn two more X degrees apart left/right!  ...  uh-oh, MATH
		Vector currentForward;  // one choice at a time
		Vector theAngles = UTIL_VecToAngles(theDir);

		// The angle left/right floor-wise will be determined by the distance to the target.
		// Closer is a wider angle, further is more narrow.
		float angleShift;
		if(theDist <= 200){
			angleShift = 40;
		}else if(theDist <= 1300){
			// shrink the angle the more the distance
			angleShift = 40 - (theDist - 200) / (1300 - 200) * (40 - 1.5);
		}else{
			angleShift = 1.5;
		}

		//TEST?
		//angleShift = 9;
		
		Vector theAngLeft(theAngles.x, fmod(theAngles.y - angleShift, 360), theAngles.z);
		Vector theAngRight(theAngles.x, fmod(theAngles.y + angleShift, 360), theAngles.z);

		UTIL_MakeAimVectorsPrivate( theAngLeft, currentForward, NULL, NULL );
		CStomp::StompCreate( vecStart, currentForward, theDist, stompStartSpeed, stompMaxSpeed, this->edict() );
		//DebugLine_Setup(1, vecStart, vecStart + currentForward * theDist, 255, 0, 0);

		UTIL_MakeAimVectorsPrivate( theAngRight, currentForward, NULL, NULL );
		CStomp::StompCreate( vecStart, currentForward, theDist, stompStartSpeed, stompMaxSpeed, this->edict() );
		//DebugLine_Setup(2, vecStart, vecStart + currentForward * theDist, 0, 0, 255);

	}//g_iSkillLevel check

	//MODDD - greater radius, was 1000
	UTIL_ScreenShake( pev->origin, 12.0, 100.0, 2.0, 1500 );

	//MODDD - same for you, lower attenuation.  Was 'ATTN_GARG'
	UTIL_PlaySound( edict(), CHAN_WEAPON, pStompSounds[ RANDOM_LONG(0,ARRAYSIZE(pStompSounds)-1) ], 1.0, ATTN_NORM - 0.23, 0, PITCH_NORM + RANDOM_LONG(-7,7) );


	//MODDD - you can do better
	//UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,20), ignore_monsters, edict(), &trace );

	Vector decalTraceStart = pev->origin + gpGlobals->v_forward * 78 + -gpGlobals->v_right * 28 + Vector(0, 0, 20);
	UTIL_TraceLine( decalTraceStart, decalTraceStart - Vector(0,0,25), ignore_monsters, edict(), &trace );
	if ( trace.flFraction < 1.0 ){
		UTIL_DecalTrace( &trace, DECAL_GARGSTOMP1 );
	}
}


void CGargantua::FlameCreate( void )
{
	int		i;
	Vector		posGun, angleGun;
	TraceResult trace;

	UTIL_MakeVectors( pev->angles );
	
	for ( i = 0; i < 4; i++ )
	{
		if ( i < 2 )
			m_pFlame[i] = CBeam::BeamCreate( GARG_BEAM_SPRITE_NAME, 240 );
		else
			m_pFlame[i] = CBeam::BeamCreate( GARG_BEAM_SPRITE2, 140 );
		if ( m_pFlame[i] )
		{
			int attach = i%2;
			// attachment is 0 based in GetAttachment
			GetAttachment( attach+1, posGun, angleGun );

			Vector vecEnd = (gpGlobals->v_forward * GARG_FLAME_LENGTH_EFFECT) + posGun;
			UTIL_TraceLine( posGun, vecEnd, dont_ignore_monsters, edict(), &trace );

			m_pFlame[i]->PointEntInit( trace.vecEndPos, entindex() );
			if ( i < 2 )
				m_pFlame[i]->SetColor( 255, 130, 90 );
			else
				m_pFlame[i]->SetColor( 0, 120, 255 );
			m_pFlame[i]->SetBrightness( 190 );
			m_pFlame[i]->SetFlags( BEAM_FSHADEIN );
			m_pFlame[i]->SetScrollRate( 20 );
			// attachment is 1 based in SetEndAttachment
			m_pFlame[i]->SetEndAttachment( attach + 2 );
			CSoundEnt::InsertSound( bits_SOUND_COMBAT, posGun, 384, 0.3 );
		}
	}
	UTIL_PlaySound( edict(), CHAN_BODY, pBeamAttackSounds[ 1 ], 1.0, ATTN_NORM - 0.1, 0, PITCH_NORM );
	UTIL_PlaySound( edict(), CHAN_WEAPON, pBeamAttackSounds[ 2 ], 1.0, ATTN_NORM - 0.1, 0, PITCH_NORM );
}


void CGargantua::FlameControls( float angleX, float angleY )
{
	if ( angleY < -180 )
		angleY += 360;
	else if ( angleY > 180 )
		angleY -= 360;

	if ( angleY < -45 )
		angleY = -45;
	else if ( angleY > 45 )
		angleY = 45;

	m_flameX = UTIL_ApproachAngle( angleX, m_flameX, 4 );
	m_flameY = UTIL_ApproachAngle( angleY, m_flameY, 8 );
	SetBoneController( 0, m_flameY );
	SetBoneController( 1, m_flameX );
}


void CGargantua::FlameUpdate( void )
{
	int			i;
	static float offset[2] = { 60, -60 };
	TraceResult		trace;
	Vector			vecStart, angleGun;
	BOOL			streaks = FALSE;

	for ( i = 0; i < 2; i++ )
	{
		if ( m_pFlame[i] )
		{
			Vector vecAim = pev->angles;
			vecAim.x += m_flameX;
			vecAim.y += m_flameY;

			//MODDD - anything based in pev->angles that involves pitch should use MakeAimVectors!
			UTIL_MakeAimVectors( vecAim );

			GetAttachment( i+1, vecStart, angleGun );
			Vector vecEnd = vecStart + (gpGlobals->v_forward * GARG_FLAME_LENGTH_LOGIC); //  - offset[i] * gpGlobals->v_right;

			UTIL_TraceLine( vecStart, vecEnd, dont_ignore_monsters, edict(), &trace );

			m_pFlame[i]->SetStartPos( trace.vecEndPos );
			m_pFlame[i+2]->SetStartPos( (vecStart * 0.6) + (trace.vecEndPos * 0.4) );

			if ( trace.flFraction != 1.0 && gpGlobals->time > m_streakTime )
			{
				StreakSplash( trace.vecEndPos, trace.vecPlaneNormal, 6, 20, 50, 400 );
				streaks = TRUE;
				UTIL_DecalTrace( &trace, DECAL_SMALLSCORCH1 + RANDOM_LONG(0,2) );
			}
			// RadiusDamageAutoRadius( trace.vecEndPos, pev, pev, gSkillData.gargantuaDmgFire, CLASS_ALIEN_MONSTER, DMG_BURN );
			FlameDamage( vecStart, trace.vecEndPos, pev, pev, gSkillData.gargantuaDmgFire * 0.7, CLASS_ALIEN_MONSTER, DMG_BURN );

			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_ELIGHT );
				WRITE_SHORT( entindex( ) + 0x1000 * (i + 2) );		// entity, attachment
				WRITE_COORD( vecStart.x );		// origin
				WRITE_COORD( vecStart.y );
				WRITE_COORD( vecStart.z );
				WRITE_COORD( RANDOM_FLOAT( 32, 48 ) );	// radius
				WRITE_BYTE( 255 );	// R
				WRITE_BYTE( 255 );	// G
				WRITE_BYTE( 255 );	// B
				WRITE_BYTE( 2 );	// life * 10
				WRITE_COORD( 0 ); // decay
			MESSAGE_END();
		}
	}
	if ( streaks )
		m_streakTime = gpGlobals->time;
}



void CGargantua::FlameDamage( Vector vecStart, Vector vecEnd, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	CBaseEntity *pEntity = NULL;
	TraceResult	tr;
	float	flAdjustedDamage;
	Vector		vecSpot;

	Vector vecMid = (vecStart + vecEnd) * 0.5;

	float searchRadius = (vecStart - vecMid).Length();

	Vector vecAim = (vecEnd - vecStart).Normalize( );

	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere( pEntity, vecMid, searchRadius )) != NULL)
	{
		if ( pEntity->pev->takedamage != DAMAGE_NO )
		{
			// UNDONE: this should check a damage mask, not an ignore
			if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
			{// houndeyes don't hurt other houndeyes with their attack
				continue;
			}
			
			vecSpot = pEntity->BodyTarget( vecMid );
		
			float dist = DotProduct( vecAim, vecSpot - vecMid );
			if (dist > searchRadius)
				dist = searchRadius;
			else if (dist < -searchRadius)
				dist = searchRadius;
			
			Vector vecSrc = vecMid + dist * vecAim;

			UTIL_TraceLine ( vecSrc, vecSpot, dont_ignore_monsters, ENT(pev), &tr );

			if(tr.fStartSolid || tr.fAllSolid){
				int x = 45; // ???
			}

			if ( tr.flFraction == 1.0 || tr.pHit == pEntity->edict() )
			{// the explosion can 'see' this entity, so hurt them!
				// decrease damage for an ent that's farther from the flame.
				dist = ( vecSrc - tr.vecEndPos ).Length();

				//DebugLine_Setup(0, vecSrc, tr.vecEndPos, tr.flFraction);

				if (dist > GARG_FLAME_MAX_DAMAGE_DIST)
				{
					flAdjustedDamage = flDamage - (dist - GARG_FLAME_MAX_DAMAGE_DIST) * 0.4;
					if (flAdjustedDamage <= 0)
						continue;
				}
				else
				{
					flAdjustedDamage = flDamage;
				}

				// ALERT( at_console, "hit %s\n", STRING( pEntity->pev->classname ) );
				if (tr.flFraction != 1.0)
				{
					ClearMultiDamage( );
					//MODDD - don't do different damage to different hitboxes.
					pEntity->TraceAttack( pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize( ), &tr, bitsDamageType, DMG_HITBOX_EQUAL );
					ApplyMultiDamage( pevInflictor, pevAttacker );
				}
				else
				{
					pEntity->TakeDamage ( pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType, DMG_HITBOX_EQUAL );
				}
			}
		}
	}
}


//MODDD - added 'playOffSound' parameter.  Might not want to do that on gibbing/deletion.
void CGargantua::FlameDestroy(BOOL playOffSound)
{
	int i;

	if (playOffSound) {
		UTIL_PlaySound(edict(), CHAN_WEAPON, pBeamAttackSounds[0], 1.0, ATTN_NORM - 0.05, 0, PITCH_NORM);
	}
	else {
		// still need to stop the sounds here.
		UTIL_StopSound(edict(), CHAN_WEAPON, pBeamAttackSounds[0]);
		UTIL_StopSound(edict(), CHAN_WEAPON, pBeamAttackSounds[1]);
		UTIL_StopSound(edict(), CHAN_WEAPON, pBeamAttackSounds[2]);
	}

	for ( i = 0; i < 4; i++ )
	{
		if ( m_pFlame[i] )
		{
			UTIL_Remove( m_pFlame[i] );
			m_pFlame[i] = NULL;
		}
	}
}


void CGargantua::PrescheduleThink( void )
{
	if ( !HasConditions( bits_COND_SEE_ENEMY ) )
	{
		m_seeTime = gpGlobals->time + 5;
		EyeOff();
	}
	else{
		EyeOn( 200 );
	}
	
	EyeUpdate();
}


//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CGargantua::Classify ( void )
{
	return	CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
// MODDD - turn speeds overall slightly reduced, especially during flamethrower, faster during stomp
void CGargantua::SetYawSpeed ( void )
{
	int ys;


	if(pev->sequence == g_gargantua_throwbody_sequenceID){
		// no turnin'.  Can't even rely on m_Activity, because likely only m_IdealActivity has been
		// set at this point.  YYYYYYEEEEEEEEEAAAAAAAAAAHHHHHHHHHHHhhhhhhhhhh.
		pev->yaw_speed = 0;
		return;
	}

	switch ( m_Activity )
	{
	case ACT_IDLE:
		ys = 60;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 180;
		break;
	case ACT_WALK:
	case ACT_RUN:
		ys = 60;
		break;

	case ACT_MELEE_ATTACK1:
		// melee
		ys = 80;
	break;
	case ACT_MELEE_ATTACK2:
		// flamethrower
		ys = 45;
	break;
	case ACT_RANGE_ATTACK1:
		// stomp
		ys = 70;
	break;

	default:
		ys = 60;
		break;
	}

	if(g_iSkillLevel == SKILL_HARD){
		ys *= 0.95;
	}else if(g_iSkillLevel == SKILL_MEDIUM){
		ys *= 0.88;
	}else{
		ys *= 0.81;
	}

	pev->yaw_speed = ys;
}




//=========================================================
// Spawn
//=========================================================
void CGargantua::Spawn()
{
	Precache( );

	setModel("models/garg.mdl");

	// bounds reduced slightly floor-wise for more flexibility.
	//UTIL_SetSize( pev, Vector( -32, -32, 0 ), Vector( 32, 32, 64 ) );
	UTIL_SetSize( pev, Vector( -30, -30, 0 ), Vector( 30, 30, 66 ) );


	pev->classname = MAKE_STRING("monster_gargantua");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;

	m_bloodColor		= BLOOD_COLOR_GREEN;

	pev->health			= gSkillData.gargantuaHealth;
	//pev->view_ofs		= Vector ( 0, 0, 96 );// taken from mdl file

	//MODDD - more FieldOfView (was -0.2)
	m_flFieldOfView		= -0.33;// width of forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();

	m_pEyeGlow = CSprite::SpriteCreate( GARG_EYE_SPRITE_NAME, pev->origin, FALSE );
	m_pEyeGlow->SetTransparency( kRenderGlow, 255, 255, 255, 0, kRenderFxNoDissipation );
	m_pEyeGlow->SetAttachment( edict(), 1 );
	EyeOff();
	m_seeTime = gpGlobals->time + 5;
	m_flameTime = gpGlobals->time + 2;

}

extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CGargantua::Precache()
{
	int i;

	PRECACHE_MODEL("models/garg.mdl");
	PRECACHE_MODEL( GARG_EYE_SPRITE_NAME );
	PRECACHE_MODEL( GARG_BEAM_SPRITE_NAME );
	PRECACHE_MODEL( GARG_BEAM_SPRITE2 );
	gStompSprite = PRECACHE_MODEL( GARG_STOMP_SPRITE_NAME );
	gGargGibModel = PRECACHE_MODEL( GARG_GIB_MODEL );


	global_useSentenceSave = TRUE;
	PRECACHE_SOUND( GARG_STOMP_BUZZ_SOUND, TRUE ); //precached by the player, can't skip.
	
	PRECACHE_SOUND("debris/metal6.wav", TRUE);
	
	PRECACHE_SOUND_ARRAY(pAttackHitSounds)
	PRECACHE_SOUND_ARRAY(pBeamAttackSounds)
	PRECACHE_SOUND_ARRAY(pAttackMissSounds)
	//PRECACHE_SOUND_ARRAY(pRicSounds)
	PRECACHE_SOUND_ARRAY(pFootSounds)
	PRECACHE_SOUND_ARRAY(pIdleSounds)
	PRECACHE_SOUND_ARRAY(pAlertSounds)
	PRECACHE_SOUND_ARRAY(pPainSounds)
	PRECACHE_SOUND_ARRAY(pAttackSounds)
	PRECACHE_SOUND_ARRAY(pStompSounds)
	PRECACHE_SOUND_ARRAY(pBreatheSounds)
	
	PRECACHE_SOUND("garg/gar_die1.wav");
	PRECACHE_SOUND("garg/gar_die2.wav");


	global_useSentenceSave = FALSE;
}	




GENERATE_TRACEATTACK_IMPLEMENTATION(CGargantua){
	ALERT( at_aiconsole, "CGargantua::TraceAttack\n");
	BOOL isAliveVar = IsAlive();
	//MODDD - if hit by the gauss, and it is charged enough, do damage.
	BOOL gaussPass = ((bitsDamageTypeMod & (DMG_GAUSS)) && flDamage > 80);


	// HOLD UP.  Before any reduction, save to our own g_gargantua_rawDamageCumula.
	// Can't trust the usual one because it takes damage after it's been reduced by here.
	g_gargantua_rawDamageCumula += flDamage;


	if(g_gargantua_rawDamageCumula > 28){
		// That's enough to sting, I hate ya.
		// (using non-reduced damage lets shotgun blasts piss me off, no way to deal damage but still loud and hard to ignore)
		pissedRunTime = gpGlobals->time + 14;
		if(pev->sequence == g_gargantua_walk_sequenceID){
			// change it
			//pev->sequence = g_gargantua_run_sequenceID;
			SetActivity(ACT_RUN);
		}
	}



	// But cut it a bit.  Thick armor after all.
	if (gaussPass) {
		flDamage *= 0.70;
	}

	//MODDD - nah, this method is fine, even if dead.
	/*
	if ( !IsAlive() )
	{
		CBaseMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
		return;
	}
	*/

	BOOL painSoundPass = FALSE;
	if(pev->deadflag == DEAD_NO){
		painSoundPass = TRUE;
	}else if(pev->deadflag == DEAD_DYING){

		if( (EASY_CVAR_GET_DEBUGONLY(gargantuaCorpseDeath) == 2 || EASY_CVAR_GET_DEBUGONLY(gargantuaCorpseDeath) == 5)){
			painSoundPass = TRUE;
		}

	}else{
		//leave false.
	}

	//pev->deadflag == DEAD_NO
	//MODDD - condition also depends on "gargantuaCorpseDeath" (or being alive, neither dead nor in dying animation).
	// UNDONE: Hit group specific damage?   (comment found here, oh well)
	if ( painSoundPass && ( (bitsDamageType & GARG_DAMAGE) || gaussPass )    )
	{
		if ( m_painSoundTime < gpGlobals->time )
		{
			UTIL_PlaySound( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_GARG, 0, PITCH_NORM );
			m_painSoundTime = gpGlobals->time + RANDOM_FLOAT( 2.5, 4 );
		}
	}

	if(pev->deadflag != DEAD_DEAD){
		bitsDamageType &= GARG_DAMAGE;
	}else{
		// if dead, let "DMG_CLUB" pass...
		bitsDamageType &= (GARG_DAMAGE|DMG_CLUB);
	}

	//MODDD - this might look a little weird. Note that, above, bitsDamageType gets all damage types filtered out except for those in GARG_DAMAGE (or also DMG_CLUB if dead so the crowbar is alloowed).
	//        GARG_DAMAGE is made of a list of forms of damage that count against the gargantua.
	//        If not a single damage type from GARG_DAMAGE is present in the provided bitsDamageType, then stripping bitsDamageType of all but GARG_DAMAGE leaves nothing (0).
	//        So the 0 check here doesn't ask, "Did the sender give us a 0 bitsDamageType?". It's saying, "After stripping out all tpes but those in GARG_DAMAGE from bitsDamageType, does
	//        anything remain? If not...". Also this ends up setting the damage dealt to 0 if the gargantua is alive.
	if ( bitsDamageType == 0 && !gaussPass)
	{
		if ( pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0,100) < 20) )
		{
			float gargantuaBleedsVar = EASY_CVAR_GET_DEBUGONLY(gargantuaBleeds);

			//MODDD - options for hit effect.
			if(gargantuaBleedsVar == 0){
				// no sound or effect.
				if(useBulletHitSound)*useBulletHitSound=FALSE;
			}else if(gargantuaBleedsVar == 1){
				//UTIL_playFleshHitSound(pev);
				// just don't block.
				Vector vecBloodOrigin = ptr->vecEndPos - vecDir * 4;
				SpawnBlood(vecBloodOrigin, flDamage);// a little surface blood.

			}else if(gargantuaBleedsVar == 2){
				if(useBulletHitSound)*useBulletHitSound=FALSE;

				Vector vecBloodOrigin = ptr->vecEndPos - vecDir * 4;
				SpawnBlood(vecBloodOrigin, flDamage);// a little surface blood.

			}else if(gargantuaBleedsVar == 3){
				if(useBulletHitSound)*useBulletHitSound=FALSE;
				UTIL_Ricochet( ptr->vecEndPos, RANDOM_FLOAT(0.5,1.5) );
			}
			
			pev->dmgtime = gpGlobals->time;

//			if ( RANDOM_LONG(0,100) < 25 )
//				UTIL_PlaySound( ENT(pev), CHAN_BODY, pRicSounds[ RANDOM_LONG(0,ARRAYSIZE(pRicSounds)-1) ], 1.0, ATTN_NORM, 0, PITCH_NORM );
		}

		if(isAliveVar){
			// Jeez!  That's a little harsh.  Even times 0.05 is insignificant.
			//flDamage = 0;
			flDamage = flDamage * 0.05;
		}

		//send "FALSE" for "useBloodEffect", as the hit effect to use was handled above already.
		useBloodEffect = FALSE;
		
	}//END OF no normal damage check


	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}


//MODDD - cloned the TraceAttack logic above,  works without a trace or effect-related info passed along.
void CGargantua::TraceAttack_Traceless(entvars_t* pevAttacker, float flDamage, Vector vecDir, int bitsDamageType, int bitsDamageTypeMod) {

	BOOL isAliveVar = IsAlive();
	//MODDD - if hit by the gauss, and it is charged enough, do damage.
	BOOL gaussPass = ((bitsDamageTypeMod & (DMG_GAUSS)) && flDamage > 80);

	// But cut it a bit.  Thick armor after all.
	if (gaussPass) {
		flDamage *= 0.70;
	}

	BOOL painSoundPass = FALSE;
	if(pev->deadflag == DEAD_NO){
		painSoundPass = TRUE;
	}else if(pev->deadflag == DEAD_DYING){

		if( (EASY_CVAR_GET_DEBUGONLY(gargantuaCorpseDeath) == 2 || EASY_CVAR_GET_DEBUGONLY(gargantuaCorpseDeath) == 5)){
			painSoundPass = TRUE;
		}

	}else{
		//leave false.
	}

	if ( painSoundPass && ( (bitsDamageType & GARG_DAMAGE) || gaussPass )    )
	{
		if ( m_painSoundTime < gpGlobals->time )
		{
			UTIL_PlaySound( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_GARG, 0, PITCH_NORM );
			m_painSoundTime = gpGlobals->time + RANDOM_FLOAT( 2.5, 4 );
		}
	}

	if(pev->deadflag != DEAD_DEAD){
		bitsDamageType &= GARG_DAMAGE;
	}else{
		// if dead, let "DMG_CLUB" pass...
		bitsDamageType &= (GARG_DAMAGE|DMG_CLUB);
	}

	if ( bitsDamageType == 0 && !gaussPass)
	{
		if(isAliveVar){
			flDamage = flDamage * 0.05;
		}
	}//END OF no normal damage check


	CBaseMonster::TraceAttack_Traceless(pevAttacker, flDamage, vecDir, bitsDamageType, bitsDamageTypeMod);
}//TraceAttack_Traceless



//definitely.
BOOL CGargantua::isSizeGiant(void){
	return TRUE;
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CGargantua)
{
	if (m_MonsterState == MONSTERSTATE_SCRIPT && (m_pCine && !m_pCine->CanInterrupt())) {
		// in script, just let the parent method run only
		int ret = GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
		g_gargantua_rawDamageCumula = 0;
		return ret;
	}


	ALERT( at_aiconsole, "CGargantua::TakeDamage\n");
	BOOL gaussPass = ((bitsDamageTypeMod & (DMG_GAUSS)) && flDamage > 80);   //again??

	EASY_CVAR_PRINTIF_PRE(gargantuaPrintout, easyPrintLine( "GARG DAMAGE %.2f", pev->health ));
	//easyForcePrintLine("????????????? %d %d", IsAlive(), pev->deadflag);
	if ( IsAlive() )
	{
		// usual checks.
		// Wait, what's the point of this?  TraceAttack already does the damage reduction
		/*
		if (!(bitsDamageType & GARG_DAMAGE) && !gaussPass) {
			//MODDD - penalty of 0.01 is a little much, 0.05 is plenty.
			flDamage *= 0.05;
		}*/


		if ((bitsDamageType & DMG_BLAST) || gaussPass) {
			SetConditions(bits_COND_LIGHT_DAMAGE);
		}


	}else{
		//MODDD - possible intervention.  If the player (FL_CLIENT) uses the crowbar on the corpse (DEAD_DEAD), do at least a little.
		//
		//pevAttacker!=NULL&&FBitSet ( pevAttacker->flags, FL_CLIENT )&&
		//easyForcePrintLine("YOU CRAZY LITTLE ffeee %d", bitsDamageType);
		if ( pev->deadflag == DEAD_DEAD ){

			if(bitsDamageType & DMG_CLUB){
				flDamage = 70;
			}else if(bitsDamageType & (DMG_BLAST | DMG_MORTAR) ){
				flDamage *= 8;
			}
			//pev->health --;
		}

	}

	float valueOf = EASY_CVAR_GET_DEBUGONLY(gargantuaCorpseDeath);

	if( (valueOf == 1 || valueOf == 4) && pev->deadflag == DEAD_DYING){
		EASY_CVAR_PRINTIF_PRE(gargantuaPrintout, easyPrintLine( "DAMAGE BLOCKED!"));
		//when in the dying anim and "gargantuaCorpseDeath" is 1 or 4, do not take damage yet.
		g_gargantua_rawDamageCumula = 0;
		return 1;
	}

	g_gargantua_rawDamageCumula = 0;
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}



void CGargantua::DeathEffect( void )
{
	int i;
	UTIL_MakeVectors(pev->angles);
	Vector deathPos = pev->origin + gpGlobals->v_forward * 100;

	// Create a spiral of streaks
	CSpiral::Create( deathPos, (pev->absmax.z - pev->absmin.z) * 0.6, 125, 1.5 );

	Vector position = pev->origin;
	position.z += 32;
	for ( i = 0; i < 7; i+=2 )
	{
		// SpawnExplosion( position, 70, (i * 0.3), 60 + (i*20) );
		//SpawnExplosion(Vector center, float randomRange, float time, int magnitude);
		//ExplosionCreate(position, NULL, 70, (i * 0.3), 60 + (i * 20), 0)

		Vector thisExpPos = position;
		thisExpPos.x += RANDOM_FLOAT(-70, 70);
		thisExpPos.y += RANDOM_FLOAT(-70, 70);
		thisExpPos.z += RANDOM_FLOAT(6, 18);   //new

		ExplosionCreate(thisExpPos, g_vecZero, NULL, 60 + (i * 20), FALSE, (i * 0.3));

		//MODDD - upped from 15.
		position.z += 20;
	}

	CBaseEntity *pSmoker = CBaseEntity::Create( "env_smoker", pev->origin, g_vecZero, NULL );
	pSmoker->pev->health = 1;	// 1 smoke balls
	pSmoker->pev->scale = 46;	// 4.6X normal size
	pSmoker->pev->dmg = 0;		// 0 radial distribution
	pSmoker->pev->nextthink = gpGlobals->time + 2.5;	// Start in 2.5 seconds
}



GENERATE_KILLED_IMPLEMENTATION(CGargantua)
{
	
	if(grabbedEnt){
		// release, toss a short distance in front
		UTIL_MakeVectors( pev->angles );
		grabbedEnt->pev->velocity = gpGlobals->v_forward * 300 + Vector(0, 0, 80);

		releaseGrabbedEnt();
	}

	//MODDD - not resetting these??  WHY
	SetBoneController( 0, 0 );
	SetBoneController( 1, 0 );

	EyeOff();
	if (m_pEyeGlow) {
		UTIL_Remove(m_pEyeGlow);
		m_pEyeGlow = NULL;
	}

	//MODDD - WHY IS THIS NOT HERE???!!!
	if(FlameIsOn())
		FlameDestroy(TRUE);

	int gibFlag = 0;
	float valueOf = EASY_CVAR_GET_DEBUGONLY(gargantuaCorpseDeath);

	EASY_CVAR_PRINTIF_PRE(gargantuaPrintout, easyPrintLine( "DEAD FLAG: %d", pev->deadflag));

	if(valueOf == 0 || valueOf == 3 || valueOf == 6){
		gibFlag = GIB_NEVER;  //handled by death effect.  Or, at 3, just never gibbable.
	}if(valueOf == 1 || valueOf == 4){

		if(pev->deadflag == DEAD_NO){
			gibFlag = GIB_NEVER;
		}else if(pev->deadflag == DEAD_DYING){
			gibFlag = GIB_NEVER;
		}else if(pev->deadflag == DEAD_DEAD){
			gibFlag = GIB_ALWAYS;
		}

	}else if(valueOf == 2 || valueOf == 5){
		if(pev->deadflag == DEAD_NO){
			gibFlag = GIB_NEVER;
		}else if(pev->deadflag == DEAD_DYING){
			gibFlag = GIB_NORMAL;
		}else if(pev->deadflag == DEAD_DEAD){
			gibFlag = GIB_ALWAYS;
		}

	}

	// NOTE - there is also HasMemory(bits_MEMORY_KILLED)
	BOOL justDied = (pev->deadflag == DEAD_NO);
	


	//MODDD 
	//EASY_CVAR_PRINTIF_PRE(gargantuaPrintout, easyPrintLine( "GARG: GIB I CAME WITH: %d", iGib));
	//if(iGib == GIB_NORMAL){

	//if we're getting gibbed (died the "2nd" time), mute the channels.  Nothing to play sound here anymore.
	if(!justDied){
		//stop sound.
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "common/null.wav", 1, ATTN_NORM);


		//MODDD - not enough?  Try this for all pain sounds I guess.
		//UTIL_StopSound( ENT(pev), CHAN_STATIC, "apache/ap_rotor4.wav" );
	}

	if(valueOf == 4 || valueOf == 5){
		if(justDied){
			//about to be?
			EASY_CVAR_PRINTIF_PRE(gargantuaPrintout, easyPrintLine( "HEALTH FORCED TO 30"));
			//pev->health = 30;
		}
	}
	
	if(pev->deadflag == DEAD_NO){
		//The first blow can never gib this creature.  It's too big to be killed in one hit,
		//no matter how many before that, and just disappear into flying gibs.
		CBaseMonster::Killed( pevInflictor, pevAttacker, GIB_NEVER );
	}else{

		if(valueOf == 0 || valueOf == 6){
			//Retail explosion transform effect mid-death anim.
			//Still never gibbable. The death anim ends in gibbing itself so let it finish.
			CBaseMonster::Killed( pevInflictor, pevAttacker, GIB_NEVER );
		}else{
			//Going to leave a corpse.  Let it be gibbable this time.
			//DYING or DEAD?  eh, gibbable. This is possible now.
			GENERATE_KILLED_PARENT_CALL(CBaseMonster);
		}
	}
}//END OF killed


void CGargantua::onDelete(void) {
	EyeOff();
	if (m_pEyeGlow) {
		UTIL_Remove(m_pEyeGlow);
		m_pEyeGlow = NULL;
	}
	FlameDestroy(FALSE);

	if(grabbedEnt){
		// release, simply drop
		releaseGrabbedEnt();
	}
}






//=========================================================
// CheckMeleeAttack1
// Garg swipe attack
// 
//=========================================================
BOOL CGargantua::CheckMeleeAttack1( float flDot, float flDist )
{
//	ALERT(at_aiconsole, "CheckMelee(%f, %f)\n", flDot, flDist);

	
	if (flDist <= GARG_MELEEATTACKDIST){
		if (flDot >= 0.7)
		{
			return TRUE;
		}else{
			// face them then
			SetConditions(bits_COND_COULD_MELEE_ATTACK1);
		}
	}
	
	return FALSE;
}


// Flame thrower madness!
BOOL CGargantua::CheckMeleeAttack2( float flDot, float flDist )
{
//	ALERT(at_aiconsole, "CheckMelee(%f, %f)\n", flDot, flDist);


	if ( gpGlobals->time > m_flameTime )
	{

		//MODDD - if everything but the dotproduct is right, allow us to turn to face to start the attack.
		/*
		if (flDot >= 0.8 && flDist > GARG_MELEEATTACKDIST)
		{
			if ( flDist <= GARG_FLAME_LENGTH )
				return TRUE;
		}
		*/

		if (flDist > GARG_MELEEATTACKDIST)
		{
			// Am I close enough to want to start the attack?  I don't want my target to get away from moving just an inch.
			if ( flDist <= GARG_FLAME_LENGTH_AI ){

				if(flDot >= 0.8){
					// okay!  Also, if waiting for the pre-delay, let an activity update happen now.
					/*
					if(flameThrowerPreAttackDelay != -1 && gpGlobals->time >= flameThrowerPreAttackDelay){
						flameThrowerPreAttackDelay = -1;
						signalActivityUpdate = TRUE;
						TaskFail();  // re-pick the schedule now
					}
					*/
					return TRUE;
				}else{
					// going to fail.  At least let us know we could have turned to face the right way
					SetConditionsMod(bits_COND_COULD_MELEE_ATTACK2);
				}

			}
		}


	}
	return FALSE;
}


//=========================================================
// CheckRangeAttack1
// flDot is the cos of the angle of the cone within which
// the attack can occur.
//=========================================================
//
// Stomp attack
//
//=========================================================
BOOL CGargantua::CheckRangeAttack1( float flDot, float flDist )
{
	//TEST
	//return FALSE;

	if ( gpGlobals->time > m_seeTime )
	{
		if(flDist > GARG_MELEEATTACKDIST){
			if (flDot >= 0.7){
				return TRUE;
			}else{
				// going to fail.  At least let us know we could have turned to face the right way
				SetConditionsMod(bits_COND_COULD_RANGE_ATTACK1);
			}
		}
	}
	return FALSE;
}




//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CGargantua::HandleAnimEvent(MonsterEvent_t *pEvent)
{

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1){
		return;
	}

	switch( pEvent->event )
	{
	case GARG_AE_SLASH_LEFT:
		{
			// HACKHACK!!!
			//MODDD - inflicts bleeding.
			CBaseEntity *pHurt = GargantuaCheckTraceHullAttack( GARG_MELEEATTACKDIST + 15.0, gSkillData.gargantuaDmgSlash, DMG_SLASH, DMG_BLEEDING );
			if (pHurt)
			{
				if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
				{
					pHurt->pev->punchangle.x = -30; // pitch
					pHurt->pev->punchangle.y = -30;	// yaw
					pHurt->pev->punchangle.z = 30;	// roll
					//UTIL_MakeVectors(pev->angles);	// called by CheckTraceHullAttack
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
				}
				UTIL_PlaySound( edict(), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM - 0.05, 0, 60 + RANDOM_LONG(0,15) );
			}
			else{ // Play a random attack miss sound
				UTIL_PlaySound( edict(), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM - 0.05, 0, 75 + RANDOM_LONG(0,15) );
			}

			// ??????? for what purpose, as-is?
			//Vector forward;
			//UTIL_MakeVectorsPrivate( pev->angles, forward, NULL, NULL );
		}
		break;

	case GARG_AE_RIGHT_FOOT:
	case GARG_AE_LEFT_FOOT:
		UTIL_ScreenShake( pev->origin, 4.0, 3.0, 1.0, 750 );
		UTIL_PlaySound( edict(), CHAN_BODY, pFootSounds[ RANDOM_LONG(0,ARRAYSIZE(pFootSounds)-1) ], 1.0, ATTN_GARG, 0, PITCH_NORM + RANDOM_LONG(-5,5) );
		break;

	case GARG_AE_STOMP:
		StompAttack();
		// retail was 11
		if(g_iSkillLevel == SKILL_HARD){
			m_seeTime = gpGlobals->time + 11.5;
		}else if(g_iSkillLevel == SKILL_MEDIUM){
			m_seeTime = gpGlobals->time + 13;
		}else{
			m_seeTime = gpGlobals->time + 14.5;
		}
		break;

	case GARG_AE_BREATHE:
		UTIL_PlaySound( edict(), CHAN_VOICE, pBreatheSounds[ RANDOM_LONG(0,ARRAYSIZE(pBreatheSounds)-1) ], 1.0, ATTN_GARG, 0, PITCH_NORM + RANDOM_LONG(-7,7) );
		break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}


//=========================================================
// CheckTraceHullAttack - expects a length to trace, amount 
// of damage to do, and damage type. Returns a pointer to
// the damaged entity in case the monster wishes to do
// other stuff to the victim (punchangle, etc)
// Used for many contact-range melee attacks. Bites, claws, etc.

// Overridden for Gargantua because his swing starts lower as
// a percentage of his height (otherwise he swings over the
// players head)
//=========================================================

//MODDD - can also work with the new damage mask in the same way as "CheckTraceHullAttack".
CBaseEntity* CGargantua::GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
	return CGargantua::GargantuaCheckTraceHullAttack(flDist, iDamage, iDmgType, 0);

}


CBaseEntity* CGargantua::GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType, int iDmgTypeMod)
{
	TraceResult tr;

	UTIL_MakeVectors( pev->angles );
	Vector vecStart = pev->origin;
	vecStart.z += 64;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist) - (gpGlobals->v_up * flDist * 0.3);

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
	
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		if (pEntity != NULL && iDamage > 0 )
		{
			pEntity->TakeDamage( pev, pev, iDamage, iDmgType );
			//MODDD - here too.
			//MODDD - draw blood.
			UTIL_fromToBlood(this, pEntity, (float)iDamage, flDist, &tr.vecEndPos, &vecStart, &vecEnd);

		}

		return pEntity;
	}

	return NULL;
}



//MODDD - NEW.  Like CheckTraceHullAttack, but doen't deal damage, just 'gets' the entity that would've been
// fetched by the call (such as verifying the entity grabbed by CheckTraceHullAttack would have certain stats
// before changing animations).
CBaseEntity* CGargantua::GargantuaCheckTraceHull(float flDist)
{
	TraceResult tr;

	UTIL_MakeVectors( pev->angles );
	Vector vecStart = pev->origin;
	vecStart.z += 64;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist) - (gpGlobals->v_up * flDist * 0.3);

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );

	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		// that's it
		return pEntity;
	}

	return NULL;
}







BOOL CGargantua::needsMovementBoundFix(void) {
	return TRUE;
}



Schedule_t *CGargantua::GetScheduleOfType( int Type )
{
	// HACKHACK - turn off the flames if they are on and garg goes scripted / dead
	if ( FlameIsOn() ){
		FlameDestroy(TRUE);
	}


	if(Type == SCHED_MELEE_ATTACK1){
		int x = 4;
	}

	// The point of 'resetFlameThrowerPreAttackDelay' is to force the flameThrowerPreAttackDelay to
	// start over if too much time passes since stopping to wait for the preDelay to finish without
	// using the flamethrower.
	// This is better than resetting flameThrowerPreAttackDelay anytime the garg has to move to catch
	// up with the player (the player could take advantage of the pre-attack delay by resetting it by
	// moving an inch just before it finishes, making the gargantua follow for a long time without
	// attacking if done right).  Now, only completely evading the gargantua for 3 seconds requires
	// the delay to start again
	if(resetFlameThrowerPreAttackDelay != -1 && gpGlobals->time >= resetFlameThrowerPreAttackDelay){
		// start over the flameThrowerPreAttackDelay next time.
		flameThrowerPreAttackDelay = -1;
		resetFlameThrowerPreAttackDelay = -1;
		flameThrowerPreAttackInterrupted = FALSE;
	}

	/*
	// no, no need for this
	//if(activity != ACT_MELEE_ATTACK2 && activity != ACT_MELEE_ATTACK1){
	if(
		Type != SCHED_MELEE_ATTACK1 &&
		Type != SCHED_MELEE_ATTACK2 &&
		Type != SCHED_SMALL_FLINCH &&
		Type != SCHED_BIG_FLINCH &&
		Type != SCHED_RANGE_ATTACK1
	){
		// Reset the delay that resets flameThrowerPreAttackDelay.
		// (yes, really)
		resetFlameThrowerPreAttackDelay = gpGlobals->time + 3;
	}
	*/

	switch( Type )
	{
		case SCHED_CHASE_ENEMY:
			// Not much different, but in case of failure to build a route, make a stomp more likely to occur.
			// (that will happen in SCHED_CHAST_ENEMY_FAILED instead)
			return slGargChaseEnemySmart;
		break;
		case SCHED_CHASE_ENEMY_FAILED:{
			// NOTICE - copy of what defaultai.cpp does, but also decrease m_seeTime.
			// Failing to pathfind while looking at a montser decreases the time until the next stomp (All we can do besides
			// stare at em' like a dumbass)

			// WAIT!  What if the reason we failed is because the player moved into some trigger (FL_MONSTERCLIP) that blocks us?
			// Moving only 3 feet and then stopping because the player darted back into a forbidden area isn't terribly smart, at least
			// get close to see if a flamethrower attack would work, block em' off, yadda yadda.
			// NO NEED.  Corrected other logic, now the garg properly uses 'BuildNearestRoute' even with FL_MONSTERCLIP to get closer
			// when it can.


			// Only decrease the time if there's over 6 seconds left though.
			if(m_seeTime - gpGlobals->time > 6){
				m_seeTime -= 3;
			}

			if(m_hEnemy != NULL){
				setEnemyLKP(m_hEnemy);
			}


			//return &slFail[ 0 ];
			return &slGargGenericFail[ 0 ];
		}
		break;
		case SCHED_RANGE_ATTACK1:
			// stomp attack, use the custom schedule now
			return slGargStompAttack;
		break;
		case SCHED_MELEE_ATTACK2:
			// INTERVENTION.  If the delay hasn't been started yet, do that.
			if(flameThrowerPreAttackDelay == -1){
				if(g_iSkillLevel == SKILL_HARD){
					flameThrowerPreAttackDelay = gpGlobals->time + 0.12;
				}else if(g_iSkillLevel == SKILL_MEDIUM){
					flameThrowerPreAttackDelay = gpGlobals->time + 0.25;
				}else{
					flameThrowerPreAttackDelay = gpGlobals->time + 0.5;
				}
				// In X seconds, reset the flameThrowerPreAttackDelay if no 
				resetFlameThrowerPreAttackDelay = gpGlobals->time + 3;
			}

			if(gpGlobals->time >= flameThrowerPreAttackDelay){
				// -1 or expired delay?  Do it
				return slGargFlame;
			}else{
				// standin' around
				return slGargFlamePreAttack;
			}
		case SCHED_MELEE_ATTACK1:
			return slGargSwipe;
		break;
		case SCHED_GARG_FLAMETHROWER_FAIL:{
			return slGargFlamethrowerFail;
		}
		case SCHED_FAIL:{
			return slGargGenericFail;
		}

	}

	return CBaseMonster::GetScheduleOfType( Type );
}


void CGargantua::StartTask( Task_t *pTask )
{

	float valueOf;
	float time = -1;

	switch ( pTask->iTask )
	{
	case TASK_FACE_ENEMY:

		if(pev->sequence == g_gargantua_throwbody_sequenceID){
			// Let me turn then, this sequence is left-over from a previous schedule
			easyPrintLine("GARG: Debug flag 234");
			pev->yaw_speed = 60;
			//this->signalActivityUpdate = TRUE;
		}

		CBaseMonster::RunTask(pTask);
	break;
	case TASK_PLAY_PRE_FLAMETHROWER_SEQUENCE:

		// If the current activity is not yet melee, do the pre-attack sequence first.
		if(g_iSkillLevel == SKILL_HARD){
			m_flFramerateSuggestion = 1.7;
		}else if(g_iSkillLevel == SKILL_MEDIUM){
			m_flFramerateSuggestion = 1.35;
		}else{
			m_flFramerateSuggestion = 1.15;
		}

		if(flameThrowerPreAttackInterrupted){
			// If flameThrowerPreAttackDelay has been interrupted before, do this faster.
			// Don't let the player kite me so easily.
			m_flFramerateSuggestion = m_flFramerateSuggestion * 1.45;
		}

		SetSequenceByIndex(g_gargantua_shootflames1_sequenceID, m_flFramerateSuggestion, FALSE);
		usingCustomSequence = FALSE;  // don't switch over to idle after this

	break;
	case TASK_FLAME_SWEEP:

		//MODDD - reset these delays too.
		flameThrowerPreAttackDelay = -1;
		resetFlameThrowerPreAttackDelay = -1;
		flameThrowerPreAttackInterrupted = FALSE;

		FlameCreate();
		m_flWaitFinished = gpGlobals->time + pTask->flData;
		//MODDD - this gets changed too.  Was 6.
		m_flameTime = gpGlobals->time + 7.5;
		m_flameX = 0;
		m_flameY = 0;
	break;

	case TASK_SOUND_ATTACK:
		if ( RANDOM_LONG(0,100) < 30 ){
			UTIL_PlaySound( ENT(pev), CHAN_VOICE, pAttackSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackSounds)-1) ], 1.0, ATTN_GARG, 0, PITCH_NORM );
		}
		TaskComplete();
	break;
	case TASK_RANGE_ATTACK1:
		// stomp attack.  Nothing special for startup.
		// oh, except that
		consecutiveStomps = 0;

		CBaseMonster::StartTask( pTask );
	break;
	case TASK_DIE:
		//MODDD - time changed.
		

		//Actually, no.
		valueOf = EASY_CVAR_GET_DEBUGONLY(gargantuaCorpseDeath);
		time = -1;
		if(valueOf == 0){
			time = 1.6;
		}else if(valueOf == 6){
			//full anim
			time = (76.0 / 15.0);
		}


		if(time != -1){
			m_flWaitFinished = gpGlobals->time + time;
			DeathEffect();
		}

		if(valueOf > 0){
			//any value but "0", the death anim will make it to the fall thud (if not gibbed before).
			fallShakeTime = gpGlobals->time + (64.0 / 15.0);
		}


		
		//CBaseMonster::StartTask(pTask);
		//NOTE: copy of StartTask's TASK_DIE fragment for the edit.
		RouteClear();
		m_IdealActivity = GetDeathActivity();
		pev->deadflag = DEAD_DYING;
		//easyPrintLine("ARE YOU SOME KIND OF insecure person??? %.2f %d", EASY_CVAR_GET_DEBUGONLY(thoroughHitBoxUpdates), pev->deadflag );
		//MODDD
		if(EASY_CVAR_GET_DEBUGONLY(thoroughHitBoxUpdates) == 1){
			//update the collision box now,
			this->SetObjectCollisionBox();
		}
		if(EASY_CVAR_GET_DEBUGONLY(gargantuaKilledBoundsAssist) == 0 &&EASY_CVAR_GET_DEBUGONLY(animationKilledBoundsRemoval) == 1){
			setPhysicalHitboxForDeath();
		}






		// FALL THROUGH... NO THAT IS TERRIBLE.
		break;
	case TASK_MELEE_ATTACK1:
		// SPECIAL:  If I'm unable to attack between the time it took to face my target (got too far away from me),
		// don't do the swipe.  It looks kinda dumb.

		if(!HasConditions(bits_COND_CAN_MELEE_ATTACK1)){
			TaskFail();
		}else{
			CBaseMonster::StartTask( pTask );
		}
	break;

	default: 
		CBaseMonster::StartTask( pTask );
		break;
	}
}

//=========================================================
// RunTask
//=========================================================
void CGargantua::RunTask( Task_t *pTask )
{
	float valueOf;
	float valueOf2;

	switch ( pTask->iTask )
	{
	case TASK_PLAY_PRE_FLAMETHROWER_SEQUENCE:{
		float flInterval = 0.1;  //mock interval, resembles think times.
		float recentFrameAdvancePrediction = flInterval * m_flFrameRate * pev->framerate * EASY_CVAR_GET_DEBUGONLY(animationFramerateMulti);

		if(!HasConditions(bits_COND_CAN_MELEE_ATTACK2) && pev->frame < 255*0.4){
			// Enemy got out of sight while waiting to start the flamethrower?
			// Interrupt, mark this.
			// (also don't interrupt after being 40% of the way through the animation, it's fine to be a miss at that point)
			flameThrowerPreAttackInterrupted = TRUE;
			TaskFail();
		}else if(m_fSequenceFinished || pev->frame + pev->framerate * recentFrameAdvancePrediction >= 250){
			// Finished, or likely will be soon?  OK
			TaskComplete();
		}
	}break;
	case TASK_WAIT_FOR_FLAMETHROWER_PREDELAY:
		// while waiting, still look at our enemy.

		//SetConditions(bits_COND_GARG_FLAMETHROWER_PREATTACK_EXPIRED);
		
		if(!HasConditions(bits_COND_CAN_MELEE_ATTACK2)){
			// Enemy got out of sight while waiting to start the flamethrower?
			// Mark this.
			flameThrowerPreAttackInterrupted = TRUE;
			TaskComplete();
		}else if(gpGlobals->time >= flameThrowerPreAttackDelay ){
			// Delay expired normally?  ok
			TaskComplete();
		}

	break;
	case TASK_FACE_ENEMY:

		CBaseMonster::RunTask(pTask);
	break;
	case TASK_STOP_MOVING:
		
		CBaseMonster::RunTask(pTask);
	break;
	case TASK_MELEE_ATTACK1:

		CBaseMonster::RunTask(pTask);
	break;
	case TASK_RANGE_ATTACK1:
		// stomp attack.  On hard difficulty, have a 2nd stomp when the first finishes!
		// And yes, this is a clone of TASK_RANGE_ATTACK1.
		
		lookAtEnemyLKP();


		if ( m_fSequenceFinished ){
			//MODDD NOTE - BEWARE. This is likely to pick the same range attack activity again if the ideal activity remains that way.
 			//m_Activity = ACT_RESET;

			if(consecutiveStomps == 0 && g_iSkillLevel == SKILL_HARD){
				//signalActivityUpdate = TRUE;   no need?
				SetActivity(ACT_RANGE_ATTACK1);
				pev->frame = (11.0f/24.0f) * 255.0f;

				consecutiveStomps++;
			}else{
				if(canPredictActRepeat()){
					switch( pTask->iTask ){
						case TASK_RANGE_ATTACK1:{predictActRepeat(bits_COND_CAN_RANGE_ATTACK1); break;}
						case TASK_RANGE_ATTACK2:{predictActRepeat(bits_COND_CAN_RANGE_ATTACK2); break;}
						case TASK_MELEE_ATTACK1:{predictActRepeat(bits_COND_CAN_MELEE_ATTACK1); break;}
						case TASK_MELEE_ATTACK2:{predictActRepeat(bits_COND_CAN_MELEE_ATTACK2); break;}
						case TASK_SPECIAL_ATTACK1:{predictActRepeat(bits_COND_SPECIAL1); break;}
						case TASK_SPECIAL_ATTACK2:{predictActRepeat(bits_COND_SPECIAL2); break;}
					}//END OF inner switch
				}
			
				/*
				if(m_Activity != ACT_RESET){
					//HACKY MC HACKERSAXXX
					// it doesn't look like this is doing anything.
					//...Now calling this and then "MaintainSchedule"?  That would be truly dastardly.
					ChangeSchedule(GetSchedule());
					return.
				}
				*/
				TaskComplete();
			}//consecutiveStomps/hard check
		}//m_fSequenceFinished check


	break;
	case TASK_DIE:
		//MODDD - This is the transform-effect.  Removed, fall and become a gib-able corpse like the rest.
		valueOf = EASY_CVAR_GET_DEBUGONLY(gargantuaCorpseDeath);
		valueOf2 = EASY_CVAR_GET_DEBUGONLY(gargantuaFallSound);
		
		

		if(fallShakeTime != -1 && valueOf > 0 && gpGlobals->time > fallShakeTime){
			UTIL_ScreenShake( pev->origin, 12.0, 100.0, 2.0, 1000 );
			fallShakeTime = -1;

			if(valueOf2 == 1){
				UTIL_PlaySound( edict(), CHAN_BODY, "!gargFallSnd", 1, ATTN_NORM - 0.1, 0, PITCH_NORM, FALSE); //* 0.55);
			}else if(valueOf2 == 2){
				UTIL_PlaySound( edict(), CHAN_BODY, "debris/metal6.wav", 1, ATTN_NORM - 0.1, 0, PITCH_NORM, FALSE);
			}

		}

		if ( (valueOf == 0 || valueOf == 6) && gpGlobals->time > m_flWaitFinished )
		{
			//MODDD - notes
			//  This is for the gargantua's gib explosion at death following the transform stretch effect,
			//  in dramatic video game fashion for something big and scary dying.
		    
			pev->renderfx = kRenderFxExplode;
			pev->rendercolor.x = 255;
			pev->rendercolor.y = 0;
			pev->rendercolor.z = 0;
			StopAnimation();
			pev->nextthink = gpGlobals->time + 0.15;
			SetThink( &CBaseEntity::SUB_Remove );
			
			//MODDD - NOTE. custom gib generation?  ok, suppose that's fine.
			// The way most places would do this is CGib::SpawnRandomGibs(...).  Might be worth looking at other examples
			// to see if this can be fitted to that easily.
			int i;
			int parts = MODEL_FRAMES( gGargGibModel );
			for ( i = 0; i < 10; i++ )
			{
				CGib *pGib = GetClassPtr( (CGib *)NULL );

				pGib->Spawn( GARG_GIB_MODEL );
				
				int bodyPart = 0;
				if ( parts > 1 )
					bodyPart = RANDOM_LONG( 0, pev->body-1 );

				pGib->pev->body = bodyPart;

				//MODDD - blood from YELLOW to GREEN, and they are now different.
				pGib->m_bloodColor = BLOOD_COLOR_GREEN;

				pGib->m_material = matNone;
				pGib->pev->origin = pev->origin;
				pGib->pev->velocity = UTIL_RandomBloodVector() * RANDOM_FLOAT( 300, 500 );
				pGib->pev->nextthink = gpGlobals->time + 1.25;
				pGib->SetThink( &CBaseEntity::SUB_FadeOut );
			}
			
			
			MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
				WRITE_BYTE( TE_BREAKMODEL);

				// position
				WRITE_COORD( pev->origin.x );
				WRITE_COORD( pev->origin.y );
				WRITE_COORD( pev->origin.z );

				// size
				WRITE_COORD( 200 );
				WRITE_COORD( 200 );
				WRITE_COORD( 128 );

				// velocity
				WRITE_COORD( 0 ); 
				WRITE_COORD( 0 );
				WRITE_COORD( 0 );

				// randomization
				WRITE_BYTE( 200 ); 

				// Model
				WRITE_SHORT( gGargGibModel );	//model id#

				// # of shards
				WRITE_BYTE( 50 );

				// duration
				WRITE_BYTE( 20 );// 3.0 seconds

				// flags

				WRITE_BYTE( BREAK_FLESH );
			MESSAGE_END();

			return;
		}
		else{
			//MODDD - notes
			//  This is the rest of the Gargantua's falling anim that is cutoff in retail.
			//  Includes the gargantua falling and potentially crushing whatever is unfortunate enough
			//  to be in front of him when he hits the ground, provided gargantuaKilledBoundsAssist is 1.
			
				//includes the usual way... no.
				//CBaseMonster::RunTask(pTask);
				
			if(!gargDeadBoundChangeYet && EASY_CVAR_GET_DEBUGONLY(gargantuaKilledBoundsAssist)==1 && pev->frame >= ((60.0/75.0)*255)){
				gargDeadBoundChangeYet = TRUE;


			/*
				float radz = this->pev->angles[1];
				*/
				Vector vecForward = UTIL_VecGetForward2D(pev->angles);

				//UTIL_printLineVector("EDDEDEDE", vecForward);
				//always bias slightly to the (relative) left (see yourself as facing the yaw)...

				Vector vecRight = CrossProduct(vecForward, Vector(0,0,1));

				Vector boundCenter = pev->origin + vecForward * 200 + vecRight * -17;


				//Perhaps we can slighlty adjust for being slightly longer than wide at floor-wise rotations?
				float theYaw = UTIL_VecToYawRadians(pev->angles);

				//apply the warp.  Base: sideways facing right.  Box is:

				//-80, -60, 0
				//80, 60, 0

				//...just go for it!

				//? = 60 + function(angle) * 20
				//OR
				//? = 60 + coordMod(angle)
				//coordMod = function(angle) * 20


				//range: 0.6 - 1.  At most no change... this steadily reduces the total size as it gets more square (remove that nasty corner space on a 45 degree rotated monster)
						

				//that is, M_PI / 4.0f    (90 degrees, in radians).
				//float halfRight = 0.7853981634f;

				//float totalModScalar = ((fabs(cos(theYaw)) - 0.70710678f )) ;
				float totalModScalar =  (fabs(cos(theYaw)) - fabs(sin(theYaw)) )*0.45 + 0.65;



				float xMod = fabs(cos(theYaw)) * 44 * totalModScalar;
				float yMod = fabs(sin(theYaw)) * 44 * totalModScalar;
						
				float finalX = 49 + xMod;
				float finalY = 49 + yMod;


				// we have an idea of the place to fall now.
				
				CBaseEntity *pList[64];
				int count = UTIL_EntitiesInBox(
					pList, 64,
					boundCenter + Vector(-finalX - 8, -finalY - 8, 0),
					boundCenter + Vector(finalX + 8, finalY + 8, 86),
					//MODDD - why (FL_CLIENT|FL_MONSTER) for required type?   That excludes other things that can take damage like breakables
					0
				);

				for(int i = 0; i < count; i++){
					if(pList[i]->pev != this->pev ){
						//Crushed by this guy? Insta-death.
						pList[i]->TakeDamage(this->pev, this->pev, 99999, DMG_ALWAYSGIB, 0);
					}

				}


				UTIL_SetSize(pev,
					(boundCenter + Vector(-finalX, -finalY, 0)) - pev->origin,
					(boundCenter + Vector(finalX, finalY, 86)) - pev->origin
				);

				/*
				//UTIL_drawBox(pev->origin  + Vector(0,0,0), pev->origin +  Vector(40, 40, 40));
				UTIL_drawBox(
					(boundCenter + Vector(-finalX, -finalY, 0)),
					(boundCenter + Vector(finalX, finalY, 86))
				);
				*/
				this->SetObjectCollisionBox();



			}else if ( m_fSequenceFinished && pev->frame >= 255 )
			{
				pev->health = 180;

				//MODDD - FOR DEBUGGING
				if(EASY_CVAR_GET_DEBUGONLY(drawCollisionBoundsAtDeath) == 1){
					UTIL_drawBox(pev->origin + pev->mins, pev->origin + pev->maxs);
				}
				if(EASY_CVAR_GET_DEBUGONLY(drawHitBoundsAtDeath) == 1){
					UTIL_drawBox(pev->absmin, pev->absmax);
				}
					

				pev->deadflag = DEAD_DEAD;
				StopAnimation();
				//easyPrintLine("DEAD: boxFlat?   %d", (BBoxFlat()));
				if(EASY_CVAR_GET_DEBUGONLY(gargantuaKilledBoundsAssist) == 0){
						
					if(EASY_CVAR_GET_DEBUGONLY(animationKilledBoundsRemoval) == 2){
						setPhysicalHitboxForDeath();
					}
					
				}else{

					
				}

				//MODDD - bound-altering script for death moved to "setPhyiscalHitboxForDeath" for better through by a CVar.
				if ( ShouldFadeOnDeath() ){
					// this monster was created by a monstermaker... fade the corpse out.
					SUB_StartFadeOut();
				}
				else{
					// body is gonna be around for a while, so have it stink for a bit.
					//...no, this is too big and tough for something to just start eating. No need.
					//CSoundEnt::InsertSound ( bits_SOUND_CARCASS, pev->origin, 384, 30 );
				}
				//an event.
				onDeathAnimationEnd();
			}


			//}else{

			//MODDDD - scrapped.
			/*
				if ( m_fSequenceFinished && pev->frame >= 255 )
				{
					EASY_CVAR_PRINTIF_PRE(gargantuaPrintout, easyPrintLine( "GARG DEAD!"));
					//set the size differently now.
					
					//MODDD - DEATH COLLISION BOX IDEA SCRAPPED.  Not possible with all the differnet rotations, as the main "hitbox" box itself cannot be rotated between 90 degree increments.
					
					//Vector forward;
					//UTIL_MakeVectorsPrivate( pev->angles, forward, NULL, NULL );
					//
					//
					//

					//UTIL_printLineVector("what is it", forward);

					////UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
					////UTIL_SetSize(pev, Vector(80, -80, 0), Vector(350, 80, 200));


					//Vector pnt1 = UTIL_rotateShift( Vector(80, -80, 0), forward );
					//Vector pnt2 = UTIL_rotateShift( Vector(350, 80, 200), forward );

					//UTIL_SetSize(pev, pnt1, pnt2);
					//UTIL_drawBox(pev->origin + pev->mins, pev->origin + pev->maxs);
					////EASY_CVAR_PRINTIF_PRE(gargantuaPrintout, easyPrintLine( "ahhh %.2f %.2f %.2f, %.2f %.2f %.2f, %.2f %.2f %.2f", pev->origin.x, pev->origin.y, pev->origin.z, pev->mins.x, pev->mins.y, pev->mins.z, pev->maxs.x, pev->maxs.y, pev->maxs.z ));

					//UTIL_drawLine(pev->origin, pev->origin + Vector(0, 0, 40));
					//
					//UTIL_drawBox(pev->absmin, pev->absmax);
					//pev->absmax = pev->origin + Vector(60, 60, 40);
					//pev->absmin = pev->origin + Vector(40, 40, 0);
					//UTIL_drawBox(pev->absmin, pev->absmax);
					//
				}
			*/


			//}//END OF else(...)
			
		}
		break;

	case TASK_FLAME_SWEEP:
		if ( gpGlobals->time > m_flWaitFinished )
		{
			FlameDestroy(TRUE);
			TaskComplete();
			FlameControls( 0, 0 );
			SetBoneController( 0, 0 );
			SetBoneController( 1, 0 );
		}
		else
		{
			BOOL cancel = FALSE;
			//BOOL tooClose = (Distance(pev->origin, m_vecEnemyLKP) < GARG_MELEEATTACKDIST - 20);
			BOOL tooClose = FALSE;

			Vector angles = g_vecZero;

			FlameUpdate();
			CBaseEntity *pEnemy = m_hEnemy;
			if ( pEnemy )
			{
				float distToEnemy;
				Vector org = pev->origin;
				org.z += 64;
				Vector dir = pEnemy->BodyTarget(org) - org;
				distToEnemy = dir.Length();
				angles = UTIL_VecToAngles( dir );
				angles.x = -angles.x;
				angles.y -= pev->angles.y;

				tooClose = (distToEnemy < GARG_MELEEATTACKDIST - 20);

				//MODDD - Also, breaking sight with the player counts as dropping the attack faster.
				if ( distToEnemy > GARG_FLAME_CANCEL_DIST || tooClose || !HasConditions(bits_COND_SEE_ENEMY)){
					cancel = TRUE;
				}
			}
			if ( fabs(angles.y) > 60 )
				cancel = TRUE;
			
			if ( cancel )
			{
				//MODDD - times changed, was 0.5 for each
				// ALSO, if I see the enemy, I realize they're out of range faster. More slowly otherwise (evaded behind cover admist the flames).
				if(HasConditions(bits_COND_SEE_ENEMY)){

					if(tooClose){
						// well within melee range?  Stop even sooner
						m_flWaitFinished -= 0.4;
						m_flameTime -= 0.6;
					}else{
						m_flWaitFinished -= 0.24;
						m_flameTime -= 0.36;
					}
				}else{
					// stop a lot slower.
					m_flWaitFinished -= 0.04;
					m_flameTime -= 0.06;
				}
			}
			// FlameControls( angles.x + 2 * sin(gpGlobals->time*8), angles.y + 28 * sin(gpGlobals->time*8.5) );
			FlameControls( angles.x, angles.y );
		}
		break;

	default:
		CBaseMonster::RunTask( pTask );
		break;
	}
}




//MODDD - new method for determining whether to register a case of damage as worthy of LIGHT_DAMAGE or HEAVY_DAMAGE for the AI.
//        Can result in interrupting the current schedule.
//        This is expected to get called from CBaseMonster's TakeDamage method in combat.cpp. This method may be customized per monster,
//        should start with a copy of this method without calling the parent method, not much here.
void CGargantua::OnTakeDamageSetConditions(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int bitsDamageTypeMod){

	//MODDD - intervention. Timed damage might not affect the AI since it could get needlessly distracting.
	if(bitsDamageTypeMod & (DMG_TIMEDEFFECT|DMG_TIMEDEFFECTIGNORE) ){
		// If this is continual timed damage, don't register as any damage condition. Not worth possibly interrupting the AI.
		return;
	}
	
	//default case from CBaseMonster's TakeDamage.
	//Also count being in a non-combat state to force looking in that direction.
	if(m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT || flDamage > 20 )
	{
		SetConditions(bits_COND_LIGHT_DAMAGE);

		//MODDD NEW - set a timer to forget a flinch-preventing memory bit.
		forgetSmallFlinchTime = gpGlobals->time + DEFAULT_FORGET_SMALL_FLINCH_TIME;
	}

	//MODDD - HEAVY_DAMAGE was unused before.  For using the BIG_FLINCH activity that is (never got communicated)
	//    Stricter requirement:  this attack took 70% of health away.
	//    The agrunt used to use this so that its only flinch was for heavy damage (above 20 in one attack), but that's easy by overriding this OnTakeDamageSetconditions method now.
	//    Keep it to using light damage for that instead.
	//if ( flDamage >= 20 )

	if(gpGlobals->time >= forgetBigFlinchTime && (flDamage >=  pev->max_health * 0.55 || flDamage >= 70) )
	{
		SetConditions(bits_COND_HEAVY_DAMAGE);
		forgetSmallFlinchTime = gpGlobals->time + DEFAULT_FORGET_SMALL_FLINCH_TIME*2.3;
		forgetBigFlinchTime = gpGlobals->time + DEFAULT_FORGET_BIG_FLINCH_TIME*2.2;
	}

/*
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar) == 10){
		//any damage causes me now.
		SetConditions(bits_COND_HEAVY_DAMAGE);
	}
*/
	easyForcePrintLine("%s:%d OnTkDmgSetCond raw:%.2f fract:%.2f", getClassname(), monsterID, flDamage, (flDamage / pev->max_health));

}//END OF OnTakeDamageSetConditions


BOOL CGargantua::usesAdvancedAnimSystem(void){
	return TRUE;
}

int CGargantua::LookupActivityHard(int activity){
	int i = 0;
	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	// any animation events in progress?  Clear it.
	resetEventQueue();


	// Any activity change also releases the grabbed ent (not that this should be interrupted?)
	if(grabbedEnt){
		// release, simply drop
		UTIL_MakeVectors( pev->angles );
		grabbedEnt->pev->velocity = gpGlobals->v_forward * 180 + Vector(0, 0, 48);
		releaseGrabbedEnt();
	}

	// any activity change, by default, doesn't use FastThink
	SetFastThink(FALSE);

	switch(activity){
		case ACT_IDLE:

			if(waitForScriptedTime != -1 || m_IdealMonsterState == MONSTERSTATE_SCRIPT || m_MonsterState == MONSTERSTATE_SCRIPT){
				// Also, only use idle3 for idles, likely going between different animations soon.
				return LookupSequence("idle3");
			}

			if(m_pSchedule == slGargFlamePreAttack){
				// In the pre-attack delay, likely won't last idle for very long, only pick the most basic one.
				return LookupSequence("idle3");
			}else{
				return CBaseAnimating::LookupActivity(activity);
			}
		break;
		case ACT_SMALL_FLINCH:
		case ACT_BIG_FLINCH:			//MODDD NOTE - is this effective?
		case ACT_FLINCH_HEAD:
		case ACT_FLINCH_CHEST:
		case ACT_FLINCH_STOMACH:
		case ACT_FLINCH_LEFTARM:
		case ACT_FLINCH_RIGHTARM:
		case ACT_FLINCH_LEFTLEG:
		case ACT_FLINCH_RIGHTLEG:
			if(g_iSkillLevel == SKILL_HARD){
				m_flFramerateSuggestion = 1.8;
			}else if(g_iSkillLevel == SKILL_MEDIUM){
				m_flFramerateSuggestion = 1.5;
			}else {
				m_flFramerateSuggestion = 1.2;
			}
		break;
		case ACT_WALK:
			// oh.  Just do that then?
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_RUN:
			// If 'pissedRunTime', run.  Otherwise, walk.
			// Also being in SCRIPT forces running, that is expected in most cases.  And will ignore difficulty.
			
			if(m_IdealMonsterState == MONSTERSTATE_SCRIPT){
				m_flFramerateSuggestion = 1.0;
				pev->framerate = 1.0;
				return g_gargantua_run_sequenceID;
			}else{

				if(gpGlobals->time < pissedRunTime){
					if(g_iSkillLevel == SKILL_HARD){
						m_flFramerateSuggestion = 1.15;
					}else if(g_iSkillLevel == SKILL_MEDIUM){
						m_flFramerateSuggestion = 1.05;
					}else{
						m_flFramerateSuggestion = 0.93;
					}
					pev->framerate = m_flFramerateSuggestion;
					return g_gargantua_run_sequenceID;
				}else{
					// not pissed?  power-walk it
					if(g_iSkillLevel == SKILL_HARD){
						m_flFramerateSuggestion = 1.23;
					}else if(g_iSkillLevel == SKILL_MEDIUM){
						m_flFramerateSuggestion = 1.17;
					}else{
						m_flFramerateSuggestion = 1.10;
					}
					pev->framerate = m_flFramerateSuggestion;
					return g_gargantua_walk_sequenceID;
				}
			}
			
		break;
		case ACT_RANGE_ATTACK1:
			// stomp attack
			if(g_iSkillLevel == SKILL_HARD){
				m_flFramerateSuggestion = 1.5;
			}else if(g_iSkillLevel == SKILL_MEDIUM){
				m_flFramerateSuggestion = 1.3;
			}else{
				m_flFramerateSuggestion = 1.05;
			}
			pev->framerate = m_flFramerateSuggestion;
		break;
		case ACT_MELEE_ATTACK1:{
			// melee attack.  Go figure.
			float randoVal = RANDOM_FLOAT(0, 1);

			// TEST, force it
			//randoVal = 0.04;

			BOOL throwAttempt;
			if(EASY_CVAR_GET_DEBUGONLY(sv_gargantua_throwattack) == 1){
				// normal use: 20% of the time
				throwAttempt = (randoVal < 0.2);
			}else if(EASY_CVAR_GET_DEBUGONLY(sv_gargantua_throwattack) == 2){
				// guaranteed
				throwAttempt = TRUE;
			}else{
				// nope
				throwAttempt = FALSE;
			}

			if(throwAttempt){
				// Do a few line-traces?  Clipping the thrown thing through map geometry while it's being tossed
				// isn't pleasant.
				// PROCEDURAL LOOP
				BOOL passedTraces = FALSE;  //reach the end to be TRUE
				while(TRUE){
					TraceResult tr;
					Vector vecStart;

					// good amount of space above me?
					vecStart = pev->origin + Vector(0, 0, pev->maxs.z);
					UTIL_TraceLine(vecStart, vecStart + Vector(0, 0, 60), dont_ignore_monsters, edict(), &tr);
					if(tr.fStartSolid || tr.fAllSolid || tr.flFraction < 1.0f){
						// no.
						break;
					}

					UTIL_MakeVectors(pev->angles);
					
					// To the left?
					vecStart = Center() + Vector(0,0,18);
					UTIL_TraceLine(vecStart, vecStart + -gpGlobals->v_right * 80, dont_ignore_monsters, edict(), &tr);
					if(tr.fStartSolid || tr.fAllSolid || tr.flFraction < 1.0f){
						break;
					}

					// How about forward?
					vecStart = Center() + Vector(0,0,18) + -gpGlobals->v_right * 14;
					UTIL_TraceLine(vecStart, vecStart + gpGlobals->v_forward * 350 + gpGlobals->v_right * 14, dont_ignore_monsters, edict(), &tr);
					if(tr.fStartSolid || tr.fAllSolid || tr.flFraction < 1.0f){
						break;
					}


					// One more check:  Is the thing in front of me suitable for throwing?  Try getting it through a damge-less hull trace
					CBaseEntity* pHurt = GargantuaCheckTraceHull(GARG_MELEEATTACKDIST + 7);

					if (pHurt && !pHurt->IsWorldOrAffiliated() && (pHurt->pev->movetype == MOVETYPE_WALK || pHurt->pev->movetype == MOVETYPE_STEP) )
					{
						// acceptable, proceed
					}else{
						break;
					}

					passedTraces = TRUE;  //ok!
					break;
				}

					
				if(passedTraces){
					m_flFramerateSuggestion = 0.9;
					pev->framerate = m_flFramerateSuggestion;

					this->animEventQueuePush(3.8f / 16.0f, 6);
					this->animEventQueuePush(10.5f / 16.0f, 7);
					this->animEventQueuePush(15.7f / 16.0f, 8);

					// no turnin
					pev->yaw_speed = 0;


					SetFastThink(TRUE);  // do logic more often to keep things more smooth

					m_flFramerateSuggestion = 0.87;
					pev->framerate = m_flFramerateSuggestion;

					return g_gargantua_throwbody_sequenceID;
				}
				
			}//randoValue check for testing throw anim
			

			if(randoVal < 0.5){
				// standard melee
				if(g_iSkillLevel == SKILL_HARD){
					m_flFramerateSuggestion = 1.45;
				}else if(g_iSkillLevel == SKILL_MEDIUM){
					m_flFramerateSuggestion = 1.225;
				}else{
					m_flFramerateSuggestion = 1.10;
				}
				pev->framerate = m_flFramerateSuggestion;
				
				// event already in HandleAnimEvent (given by model)

				return LookupSequence("Attack");
			}else if(randoVal < 0.7){
				// smash
				if(g_iSkillLevel == SKILL_HARD){
					m_flFramerateSuggestion = 0.97;
				}else if(g_iSkillLevel == SKILL_MEDIUM){
					m_flFramerateSuggestion = 0.93;
				}else{
					m_flFramerateSuggestion = 0.89;
				}
				pev->framerate = m_flFramerateSuggestion;

				this->animEventQueuePush(7.4f / 16.0f, 4);

				return LookupSequence("smash");
			}else if(randoVal < 1.0){
				// kick
				if(g_iSkillLevel == SKILL_HARD){
					m_flFramerateSuggestion = 0.97;
				}else if(g_iSkillLevel == SKILL_MEDIUM){
					m_flFramerateSuggestion = 0.93;
				}else{
					m_flFramerateSuggestion = 0.89;
				}
				pev->framerate = m_flFramerateSuggestion;

				this->animEventQueuePush(7.6f / 22.0f, 5);

				return LookupSequence("kickcar");
			}

			
		}break;
		case ACT_MELEE_ATTACK2:
			// flamethrower

			// nevermind doing the check here
			/*
			if(gpGlobals->time >= flameThrowerPreAttackDelay){
				// go ahead, pick the flamethrower as the act suggests.
				return CBaseAnimating::LookupActivity(activity);
			}else{
				// stand instead.
				return CBaseAnimating::LookupActivity(ACT_IDLE);
			}
			*/
			
		break;
	}//END OF switch
	
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}//END OF LookupActivityHard


int CGargantua::tryActivitySubstitute(int activity){
	int i = 0;
	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_WALK:
			// just to say we have something, I forget if the model has ACT_WALK but may as well say we do.
			return CBaseAnimating::LookupActivity(ACT_RUN);
		break;
		case ACT_RUN:
			
		break;
		case ACT_RANGE_ATTACK1:

		break;
	}//END OF switch

	//not handled by above? We're not using the script to determine animation then. Rely on the model's anim for this activity if there is one.
	return CBaseAnimating::LookupActivity(activity);
}//END OF tryActivitySubstitute


void CGargantua::HandleEventQueueEvent(int arg_eventID){

	switch(arg_eventID){
	case 4:{
		// 'smash'.  quick heavy melee damage, shake.
		// HACKHACK!!!
		//MODDD - inflicts bleeding.
		CBaseEntity *pHurt = GargantuaCheckTraceHullAttack( GARG_MELEEATTACKDIST + 7, gSkillData.gargantuaDmgSlash * 1.5, DMG_SLASH, DMG_BLEEDING );
		if (pHurt)
		{
			if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
			{
				pHurt->pev->punchangle.x = -50; // pitch
				pHurt->pev->punchangle.y = 0;	// yaw
				pHurt->pev->punchangle.z = 50;	// roll
												//UTIL_MakeVectors(pev->angles);	// called by CheckTraceHullAttack
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 120 + Vector(0, 0, 180);
				pHurt->pev->flags &= ~FL_ONGROUND;
				pHurt->pev->groundentity = NULL;
				pHurt->pev->origin.z += 1;
			}
			UTIL_PlaySound( edict(), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM - 0.05, 0, 60 + RANDOM_LONG(0,15) );
		}
		else{ // Play a random attack miss sound
			UTIL_PlaySound( edict(), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM - 0.05, 0, 75 + RANDOM_LONG(0,15) );
		}

		// Play the stomp-sound, smacked the ground pretty good at least.
		UTIL_ScreenShake( pev->origin, 12.0, 100.0, 2.0, 1500 );
		UTIL_PlaySound( edict(), CHAN_WEAPON, pStompSounds[ RANDOM_LONG(0,ARRAYSIZE(pStompSounds)-1) ], 1.0, ATTN_NORM - 0.23, 0, PITCH_NORM + RANDOM_LONG(11,16) );

		TraceResult trace;
		Vector decalTraceStart = pev->origin + gpGlobals->v_forward * 86 + Vector(0, 0, 20);
		UTIL_TraceLine( decalTraceStart, decalTraceStart - Vector(0,0,25), ignore_monsters, edict(), &trace );
		if ( trace.flFraction < 1.0 ){
			UTIL_DecalTrace( &trace, DECAL_GARGSTOMP1 );
		}

	}break;
	case 5:{
		// kick.  More of a push-away than real damage.
		// HACKHACK!!!
		// no bleeding for this one.  Also more blunt than a 'slash' but eh.
		CBaseEntity *pHurt = GargantuaCheckTraceHullAttack( GARG_MELEEATTACKDIST + 13, gSkillData.gargantuaDmgSlash * 0.7, DMG_SLASH, 0 );
		if (pHurt)
		{
			if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
			{
				pHurt->pev->punchangle.x = -50; // pitch
				pHurt->pev->punchangle.y = RANDOM_FLOAT(-20, 20);	// yaw
				pHurt->pev->punchangle.z = 25;	// roll
												//UTIL_MakeVectors(pev->angles);	// called by CheckTraceHullAttack
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 520 + Vector(0, 0, 320);
				pHurt->pev->flags &= ~FL_ONGROUND;
				pHurt->pev->groundentity = NULL;
				pHurt->pev->origin.z += 1;
			}
			UTIL_PlaySound( edict(), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM - 0.05, 0, 60 + RANDOM_LONG(0,15) );
		}
		else{ // Play a random attack miss sound
			UTIL_PlaySound( edict(), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM - 0.05, 0, 75 + RANDOM_LONG(0,15) );
		}

	}break;
	case 6:{
		// throwbody: the grab.  Or attempt to.  (little damage)
		CBaseEntity *pHurt = GargantuaCheckTraceHullAttack( GARG_MELEEATTACKDIST + 7, gSkillData.gargantuaDmgSlash * 0.15, DMG_SLASH, DMG_BLEEDING );

		// YES REALLY.  Don't try to grab the world.   Doing anything to that will royally <mess> your <stuff> up.
		// Yeah.   Really.        Lots of stuff will report all space as solid, like hand grenades not going anywhere
		// (stuck in place slowly descending as soon as their thrown), and mp5 grenades exploding the moment they're fired.
		// Not doing a size-check, a gargantua picking up another gargantua and tossing it would be very rare,
		// but just friggin' hilarious to witness.  And it would come from spawning stuff on your own anyway,
		// never even two gargs in one place, nor would they even attack each other.
		// Also. Only pickup things that walk, aiming so low to pickup flyers that could very well be higher might look wonky,
		// and things that never intend on moving (like sentries) will just defy gravity forever, pretty silly.
		// Could force to a movetype of TOSS since being thrown and restore the normal movetype on hitting the ground but.
		// Ehhh.  Is that really worth supporting, every single entity would need default 'Touch' behavior on top of whatever
		// custom 'touch' it already has, although the 'MonsterThink' idea going everywhere really is the same idea on top of
		// the entity's 'Think' event method.
		if (pHurt && !pHurt->IsWorldOrAffiliated() && (pHurt->pev->movetype == MOVETYPE_WALK || pHurt->pev->movetype == MOVETYPE_STEP) )
		{
			grabEnt(pHurt);
		}
		else{ // Play a random attack miss sound
			// ALSO, give up the task.  Doesn't make sense to finish this one on a miss.
			// ...at time 10/16 that is (will be noticed without a grabbedEnt).  Now, event #7 will do that check.
			UTIL_PlaySound( edict(), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM - 0.05, 0, 75 + RANDOM_LONG(0,15) );
		}
	}break;
	case 7:{
		// throwbody: the check.  If I'm not holding anything (grab didn't work), stop now, it's silly to throw with nothing in hand.
		if(grabbedEnt == NULL){
			TaskFail();
		}
	}break;
	case 8:{
		// throwbody:  the 'throw'.  Release and, well, chuck em' across.

		if(grabbedEnt != NULL){
			UTIL_MakeAimVectors( pev->angles );
			grabbedEnt->pev->velocity = gpGlobals->v_forward * 750 + -gpGlobals->v_right * 50 + Vector(0, 0, 130);
			releaseGrabbedEnt();
		}
	}break;
	}//switch


}//HandleEventQueueEvent



int CGargantua::getHullIndexForNodes(void){
    return NODE_LARGE_HULL;  //...ya think?
}

BOOL CGargantua::predictRangeAttackEnd(void){
	// yea go ahead, just the stomp
	// mMMmmmmmm... no, the event happens right at the end, unwise, nevermind
	return FALSE;
}

//MODDD - more range for trying a stomp attack
float CGargantua::getDistTooFar(void){
	//return 1024.0;
	return 1500.0f;
}

float CGargantua::ScriptEventSoundAttn(void){
	return 0.57;
}
float CGargantua::ScriptEventSoundVoiceAttn(void){
	return 0.58;
}


void CGargantua::grabEnt(CBaseEntity* toGrab){
	// no hit sound
	grabbedEnt = toGrab;

	//grabbedEnt->grabbedByGargantua = this;
	// nah just do this.  This is in case the game gets saved while something is grabbed.
	// I can 
	grabbedEnt->isGrabbed = TRUE;
	grabbedEnt->m_vecOldBoundsMins = grabbedEnt->pev->mins;
	grabbedEnt->m_vecOldBoundsMaxs = grabbedEnt->pev->maxs;
	grabbedEnt->m_fOldGravity = grabbedEnt->pev->gravity;

	grabbedEntOldMins = pev->mins;
	grabbedEntOldMaxs = pev->maxs;
	UTIL_SetSize(grabbedEnt->pev, g_vecZero, g_vecZero);

	grabbedEnt->pev->flags &= ~FL_ONGROUND;
	grabbedEnt->pev->groundentity = NULL;
	grabbedEnt->pev->origin.z += 1;

	// disable that gravity
	grabbedEnt->SetGravity(0);

	//pev->nextNormalThink = pev->nextThink;
	//pev->nextThink = gpGlobals->time + 0.01;
	//
}

void CGargantua::releaseGrabbedEnt(void){
	// Special call that sets 'pev->gravity' for most entities, does some extra steps
	// needed for the player here.
	UTIL_SetSize(grabbedEnt->pev, grabbedEntOldMins, grabbedEntOldMaxs);
	grabbedEnt->SetGravity(grabbedEnt->m_fOldGravity);
	
	//grabbedEnt->grabbedByGargantua = NULL;
	// no need?
	//grabbedEnt->pev->mins = grabbedEnt->m_vecOldBoundsMins;
	//grabbedEnt->pev->maxs = grabbedEnt->m_vecOldBoundsMaxs;

	grabbedEnt->isGrabbed = FALSE;
	grabbedEnt = NULL;
}

