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
// Alien slave monster
//=========================================================

//MODDD TODO - should an islave force itself to attack even if it feels cowardly, IF it fails to find cover, or is exposed after finding cover at least once?

//Also the light from a lightning charge stays on even if that schedule is interrupted since the light is made to stay on for a set amount of time since it is created.
//That isn't too big of a deal though.


#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "squadmonster.h"
#include "schedule.h"
#include "effects.h"
#include "weapons.h"
#include "soundent.h"
//MODDD
#include "defaultai.h"
#include "nodes.h"
#include "decals.h"


EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveFriendMode)
EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveFriendChance)
EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveFriendRange)
EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveSelfMinDelay)
EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveSelfMaxDelay)
EASY_CVAR_EXTERN_DEBUGONLY(islaveReviveSelfChance)
EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)


//MODDD - anything above its real declaration need to know about it?
extern Schedule_t slSlaveAttack1[];
extern DLL_GLOBAL int g_iSkillLevel;



//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ISLAVE_AE_CLAW		( 1 )
#define ISLAVE_AE_CLAWRAKE	( 2 )
#define ISLAVE_AE_ZAP_POWERUP	( 3 )
#define ISLAVE_AE_ZAP_SHOOT		( 4 )
#define ISLAVE_AE_ZAP_DONE		( 5 )

#define ISLAVE_MAX_BEAMS	8




//MODD - keep track of sequences in the model.
//       Easier to see if something about a model's sequence changes compared to whe the script expects it to be below.
enum islave_sequence {  //key: frames, FPS
	ISLAVE_IDLE1,  //33, 15
	ISLAVE_IDLE2,  //33, 15
	ISLAVE_IDLE3,  //20, 15
	ISLAVE_CROUCH,  //25, 15
	ISLAVE_WALK1,  //34, 15
	ISLAVE_WALK2,  //34, 15
	ISLAVE_RUN1,  //26, 25
	ISLAVE_RIGHT,  //20, 15
	ISLAVE_LEFT,  //15, 15
	ISLAVE_JUMP,  //45, 15
	ISLAVE_STAIRSUP,  //26, 15
	ISLAVE_ATTACK1,  //33, 22
	ISLAVE_ZAPATTACK1,  //35, 15
	ISLAVE_FLINCH2,  //8, 15
	ISLAVE_LAFLINCH,  //11, 15
	ISLAVE_RAFLINCH, //11, 15
	ISLAVE_LLFLINCH,  //11, 15
	ISLAVE_RLFLINCH, //11, 15
	ISLAVE_BIGFLINCH, //40, 15
	ISLAVE_BARNACLE1, //9, 12
	ISLAVE_BARNACLE2, //15, 22
	ISLAVE_BARNACLE3,  //6, 8
	ISLAVE_BARNACLE4,  //31, 10
	ISLAVE_DIEHEADSHOT, //40, 20
	ISLAVE_DIESIMPLE, //80, 20
	ISLAVE_DIEFORWARD,  //30, 20
	ISLAVE_DIEBACKWARD,  //70, 20
	ISLAVE_DIEVIOLENT,  //70, 20  named "dieforward" but mapped to ACT_DIEVIOLENT. and dieforward is the same name as another earlier.
	ISLAVE_COLLAR1, //60, 18
	ISLAVE_COLLAR2,  //30, 18
	ISLAVE_PUSHUP, //50, 20
	ISLAVE_GRAB,  //65, 20
	ISLAVE_UPDOWN,  //50, 20
	ISLAVE_DOWNUP,  //65, 20
	ISLAVE_JIBBER, //60, 30
	ISLAVE_JABBER,  //60, 30
	ISLAVE_PEPSIHIT, //31, 25
	ISLAVE_PEPSITOPPLE,  //101, 18
	ISLAVE_PEPSIIDLE,  //17, 18
	ISLAVE_BUTTONPUSH,  //90, 15
	ISLAVE_VALVEFRY,  //80, 15
	ISLAVE_DIEHEADSHOT_RES,  //40, 20
	ISLAVE_DIESIMPLE_RES,  //39, 20
	ISLAVE_DIEBACKWARD_RES,  //40, 20
	ISLAVE_DIEFORWARD_RES,  //40, 40

};


/*
enum
{
	... = LAST_COMMON_SCHEDULE + 1,
	...,
};
*/

enum
{
	TASK_ISLAVE_SET_REVIVE_SEQUENCE = LAST_COMMON_TASK + 1,
	TASK_ISLAVE_SET_REVIVE_SELF_SEQUENCE,
	TASK_ISLAVE_GO_TO_TARGET,
};



class CISlave : public CSquadMonster
{
public:
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];

	int m_iBravery;

	CBeam* m_pBeam[ISLAVE_MAX_BEAMS];

	int m_iBeams;
	//MODDD - well would ya look at that.
	// Used by something other than CBasePlayer.
	// No idea why CBaseMonster used to have this, never touched it there.
	float m_flNextAttack;
	int m_voicePitch;
	EHANDLE m_hDead;

	//MODDD - the wounded NPC to seek out.
	// ...or not using this var anymore, whoops
	//CBaseMonster* healTargetNPC;


	CISlave(void);
	void MonsterThink(void);

	void makeThingForgetReviveInfo(CBaseEntity* thing, CISlave* thingSpecific);


	BOOL okayToRevive(void);
	void onDeathAnimationEnd(void);

	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  ISoundMask( void );
	int  Classify ( void );

	//MODDD
	virtual BOOL isProvokable(void);
	virtual BOOL isProvoked(void);

	void ReportAIState( void );
	
	BOOL canReviveFriend;
	int LookupActivityHard(int activity);
	int tryActivitySubstitute(int activity);
	BOOL usesAdvancedAnimSystem(void);

	BOOL finishingReviveFriendAnim;

	void riseFromTheGrave(void);
	void StartReanimation(void);
	void StartReanimationPost(int preReviveSequence);

	void forgetReviveTarget(void);

	CISlave* monsterTryingToReviveMe;
	EHANDLE monsterTryingToReviveMeEHANDLE;
	BOOL reviveTargetChosen;
	int beingRevived;
	float reviveFriendAnimStartTime;

	//TODO: save this on saving a game?
	float selfReviveTime;


	int  IRelationship( CBaseEntity *pTarget );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );

	CISlave* findISlaveToRevive(BOOL requireLineTrace, float argStartMaxDist);

	//MODDD - new.
	void SetObjectCollisionBox( void ){
		if(pev->deadflag != DEAD_NO){
			pev->absmin = pev->origin + Vector(-78, -78, 0);
			pev->absmax = pev->origin + Vector(78, 78, 72);
		}else{
			CBaseMonster::SetObjectCollisionBox();
		}
	}


	void CallForHelp( char *szClassname, float flDist, EHANDLE hEnemy, Vector &vecLocation );


	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	float getBarnacleForwardOffset(void);

	BOOL getIsBarnacleVictimException(void);
	float getBarnaclePulledTopOffset(void);



	void DeathSound( void );
	void PainSound( void );
	void AlertSound( void );
	void IdleSound( void );

	GENERATE_KILLED_PROTOTYPE
	
	

    void StartTask ( Task_t *pTask );
	//MODDD
	void RunTask( Task_t *pTask );

	void ScheduleChange(void);

	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType ( int Type );
	CUSTOM_SCHEDULES;

	static TYPEDESCRIPTION m_SaveData[];
	int Save( CSave &save ); 
	int Restore( CRestore &restore );

	void ClearBeams( );
	void ArmBeam( int side );
	void WackBeam( int side, CBaseEntity *pEntity );
	void ZapBeam( int side );
	void BeamGlow( void );

	BOOL violentDeathAllowed(void);
	BOOL violentDeathClear(void);
	int violentDeathPriority(void);

	void onDelete(void);

	void ForgetEnemy(void);


};






#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_alien_slave, CISlave );
	LINK_ENTITY_TO_CLASS( monster_vortigaunt, CISlave );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( islave, CISlave );
	LINK_ENTITY_TO_CLASS( alien_slave, CISlave );
	
	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( vortigaunt, CISlave );
		LINK_ENTITY_TO_CLASS( vort, CISlave );
		LINK_ENTITY_TO_CLASS( slave, CISlave );
		LINK_ENTITY_TO_CLASS( monster_vort, CISlave );
		LINK_ENTITY_TO_CLASS( monster_slave, CISlave );
		LINK_ENTITY_TO_CLASS( monster_islave, CISlave );
		LINK_ENTITY_TO_CLASS( monster_alien_islave, CISlave );

	#endif

	//no extras.
#endif



TYPEDESCRIPTION	CISlave::m_SaveData[] = 
{
	DEFINE_FIELD( CISlave, m_iBravery, FIELD_INTEGER ),

	DEFINE_ARRAY( CISlave, m_pBeam, FIELD_CLASSPTR, ISLAVE_MAX_BEAMS ),
	DEFINE_FIELD( CISlave, m_iBeams, FIELD_INTEGER ),
	DEFINE_FIELD( CISlave, m_flNextAttack, FIELD_TIME ),

	DEFINE_FIELD( CISlave, m_voicePitch, FIELD_INTEGER ),

	DEFINE_FIELD( CISlave, m_hDead, FIELD_EHANDLE ),

	DEFINE_FIELD( CISlave, selfReviveTime, FIELD_TIME ),

	//selfReviveTime = -1;
};

IMPLEMENT_SAVERESTORE( CISlave, CSquadMonster );



const char *CISlave::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CISlave::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CISlave::pPainSounds[] = 
{
	"aslave/slv_pain1.wav",
	"aslave/slv_pain2.wav",
};

const char *CISlave::pDeathSounds[] = 
{
	"aslave/slv_die1.wav",
	"aslave/slv_die2.wav",
};





Task_t	tlISlaveReviveFriend[] =
{
	//hm...?
	//{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_TARGET_CHASE },// If you fail, follow normally
	//case SCHED_TARGET_CHASE:
	//return slFollow;

	{ TASK_ISLAVE_GO_TO_TARGET,(float)110		},
//	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE_SCARED },
	{TASK_ISLAVE_SET_REVIVE_SEQUENCE, (float)0 }
};

Schedule_t	slISlaveReviveFriend[] =
{
	{
		tlISlaveReviveFriend,
		ARRAYSIZE ( tlISlaveReviveFriend ),
		//bits_COND_NEW_ENEMY |
		bits_COND_HEAR_SOUND |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		bits_SOUND_DANGER,
		"ISLaveReviveFriend"
	},
};


Task_t	tlISlaveReviveSelf[] =
{
	{TASK_ISLAVE_SET_REVIVE_SELF_SEQUENCE, (float)0 }

};

Schedule_t	slISlaveReviveSelf[] =
{
	{
		tlISlaveReviveSelf,
		ARRAYSIZE ( tlISlaveReviveSelf ),
		//bits_COND_NEW_ENEMY |
		0,
		0,
		"ISLaveReviveSelf"
	},
};





		

void CISlave::ReportAIState( void )
{
	easyPrintLine("MONSTER ID: %d HEALER ID: %d BEING REV?: %d REVIVINGOTHER?: %d REVIVING MONSTER ID: %d FRAMERATE: %.2f FRAME: %.2f DIST_TO_TARGET: %.2f, ACTM: %d, ACTI: %d, ACT: %d TIMs: %.2f %.2f",
		monsterID,
		(this->monsterTryingToReviveMeEHANDLE!=NULL) ? this->monsterTryingToReviveMe->monsterID : -1,
		beingRevived,
		this->reviveTargetChosen,
		(reviveTargetChosen) ? m_hTargetEnt->MyMonsterPointer()->monsterID : -1,
		pev->framerate,
		pev->frame,
		m_hTargetEnt!=NULL ? (m_hTargetEnt->pev->origin - pev->origin).Length2D() : -1,
		m_movementActivity,
		m_IdealActivity,
		m_Activity,
		gpGlobals->time,
		reviveFriendAnimStartTime
	);

	CSquadMonster::ReportAIState();
}



void CISlave::MonsterThink(void){
	//MODD - just force bravery to 100 at all times.  It seems to be barely implemented, and has serious consequences if it goes under 0 for whatever reason (reduces "bravery", making an islave always run away...).
	m_iBravery = 100;

	//or == -1?
	if(pev->deadflag == DEAD_DEAD && selfReviveTime > 0){
		//going to revive... see if we can yet.

		//easyPrintLine("AAAAAAAAA YAAAHHHHHHH %.2f %.2f", selfReviveTime, gpGlobals->time);
		if(gpGlobals->time > selfReviveTime){

			if(okayToRevive()){

				selfReviveTime = -1;
				this->riseFromTheGrave();
			}else{
				//keep trying...
				selfReviveTime += RANDOM_LONG(5, 11);
			}
		}//END OF time check
	}//END OF revive check


	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1 && this->m_fSequenceFinished){

		switch(RANDOM_LONG(0, 60)){

			case 0:
				this->SetSequenceByName("cw_pepsiidle");
			break;
			case 1:
				this->SetSequenceByName("cw_pepsiidle");
			break;
			case 2:
				this->SetSequenceByName("cw_pepsiidle");
			break;
			case 3:
				this->SetSequenceByName("cw_pepsiidle");
			break;
			case 4:
				this->SetSequenceByName("cw_pepsiidle");
			break;
			case 5:
				this->SetSequenceByName("cw_pepsiidle");
			break;
			case 6:
				this->SetSequenceByName("cw_pepsiidle");
			break;
			case 7:
				this->SetSequenceByName("cw_pepsiidle");
			break;
			case 8:
				this->SetSequenceByName("valvefry");
			break;
			case 9:
				this->SetSequenceByName("electrocute");
			break;
			case 10:
				this->SetSequenceByName("electrocute");
			break;
			case 11:
				this->SetSequenceByName("buttonpush");
			break;
			case 12:
				this->SetSequenceByName("jabber");
			break;
			case 13:
				this->SetSequenceByName("jabber");
			break;
			case 14:
				this->SetSequenceByName("jabber");
			break;
			case 15:
				this->SetSequenceByName("jibber");
			break;
			case 16:
				this->SetSequenceByName("jibber");
			break;
			case 17:
				this->SetSequenceByName("downup");
			break;
			case 18:
				this->SetSequenceByName("downup");
			break;
			case 19:
				this->SetSequenceByName("downup");
			break;
			case 20:
				this->SetSequenceByName("downup");
			break;
			case 21:
				this->SetSequenceByName("updown");
			break;
			case 22:
				this->SetSequenceByName("updown");
			break;
			case 23:
				this->SetSequenceByName("updown");
			break;
			case 24:
				this->SetSequenceByName("updown");
			break;
			case 25:
				this->SetSequenceByName("grab");
			break;
			case 26:
				this->SetSequenceByName("grab");
			break;
			case 27:
				this->SetSequenceByName("grab");
			break;
			case 28:
				this->SetSequenceByName("grab");
			break;
			case 29:
				this->SetSequenceByName("pushup");
			break;
			case 30:
				this->SetSequenceByName("pushup");
			break;
			case 31:
				this->SetSequenceByName("pushup");
			break;
			case 32:
				this->SetSequenceByName("pushup");
			break;
			case 33:
				this->SetSequenceByName("barnacle1");
			break;
			case 34:
				this->SetSequenceByName("barnacle2");
			break;
			case 35:
				this->SetSequenceByName("barnacle3");
			break;
			case 36:
				this->SetSequenceByName("barnacle4");
			break;
			case 37:
				this->SetSequenceByName("collar1");
			break;
			case 38:
				this->SetSequenceByName("collar1");
			break;
			case 39:
				this->SetSequenceByName("collar2");
			break;
			case 40:
				this->SetSequenceByName("collar2");
			break;
			case 41:
				this->SetSequenceByName("laflinch");
			break;
			case 42:
				this->SetSequenceByName("laflinch");
			break;
			case 43:
				this->SetSequenceByName("raflinch");
			break;
			case 44:
				this->SetSequenceByName("raflinch");
			break;
			case 45:
				this->SetSequenceByName("llflinch");
			break;
			case 46:
				this->SetSequenceByName("llflinch");
			break;
			case 47:
				this->SetSequenceByName("rlflinch");
			break;
			case 48:
				this->SetSequenceByName("rlflinch");
			break;
			case 49:
				this->SetSequenceByName("flinch2");
			break;
			case 50:
				this->SetSequenceByName("flinch2");
			break;
			case 51:
				this->SetSequenceByName("flinch2");
			break;
			case 52:
				this->SetSequenceByName("zapattack1");
			break;
			case 53:
				this->SetSequenceByName("attack1");
			break;
			case 54:
				this->SetSequenceByName("attack1");
			break;
			case 55:
				this->SetSequenceByName("jump");
			break;
			case 56:
				this->SetSequenceByName("left");
			break;
			case 57:
				this->SetSequenceByName("left");
			break;
			case 58:
				this->SetSequenceByName("right");
			break;
			case 59:
				this->SetSequenceByName("right");
			break;
			case 60:
				this->SetSequenceByName("idle2");
			break;
		}
	}

	CBaseMonster::MonsterThink();
}


//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CISlave::Classify ( void )
{
	return	CLASS_ALIEN_MILITARY;
}


BOOL CISlave::isProvokable(void){
	//ordinarily, yes.
	return TRUE;
}
BOOL CISlave::isProvoked(void){
	//if we are missing the "wait_until_provoked" spawnflag, always proked (normal hostile).  Otherwise, we need to have been "provoked".
	return ( (! (pev->spawnflags & SF_MONSTER_WAIT_UNTIL_PROVOKED) ) || m_afMemory & bits_MEMORY_PROVOKED );
}

int CISlave::IRelationship( CBaseEntity *pTarget )
{
	
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1){
		return R_NO;
	}

	if ( (pTarget->IsPlayer()) )
		if ( (pev->spawnflags & SF_MONSTER_WAIT_UNTIL_PROVOKED ) && ! (m_afMemory & bits_MEMORY_PROVOKED ))
			return R_NO;
	return CBaseMonster::IRelationship( pTarget );
}


void CISlave::CallForHelp( char *szClassname, float flDist, EHANDLE hEnemy, Vector &vecLocation )
{
	// ALERT( at_aiconsole, "help " );

	// skip ones not on my netname
	if ( FStringNull( pev->netname ))
		return;

	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByString( pEntity, "netname", STRING( pev->netname ))) != NULL)
	{
		float d = (pev->origin - pEntity->pev->origin).Length();
		if (d < flDist)
		{
			CBaseMonster *pMonster = pEntity->MyMonsterPointer( );
			if (pMonster)
			{
				pMonster->m_afMemory |= bits_MEMORY_PROVOKED;
				pMonster->PushEnemy( hEnemy, vecLocation );
			}
		}
	}
}


//=========================================================
// ALertSound - scream
//=========================================================
void CISlave::AlertSound( void )
{
	if ( m_hEnemy != NULL )
	{
		//MODDD - little louder, it is an alert noise after all.  Was 0.85
		SENTENCEG_PlayRndSz(ENT(pev), "SLV_ALERT", 0.95, ATTN_NORM, 0, m_voicePitch);

		CallForHelp( "monster_alien_slave", 512, m_hEnemy, m_vecEnemyLKP );
	}
}

//=========================================================
// IdleSound
//=========================================================
void CISlave::IdleSound( void )
{
	if (RANDOM_LONG( 0, 2 ) == 0)
	{
		SENTENCEG_PlayRndSz(ENT(pev), "SLV_IDLE", 0.85, ATTN_NORM, 0, m_voicePitch);
	}

// ...casual lightning effects disabled?  well alrighty then
#if 0
	int side = RANDOM_LONG( 0, 1 ) * 2 - 1;

	ClearBeams( );
	ArmBeam( side );

	UTIL_MakeAimVectors( pev->angles );
	Vector vecSrc = pev->origin + gpGlobals->v_right * 2 * side;
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(vecSrc.x);	// X
		WRITE_COORD(vecSrc.y);	// Y
		WRITE_COORD(vecSrc.z);	// Z
		WRITE_BYTE( 8 );		// radius * 0.1
		WRITE_BYTE( 255 );		// r
		WRITE_BYTE( 180 );		// g
		WRITE_BYTE( 96 );		// b
		WRITE_BYTE( 10 );		// time * 10
		WRITE_BYTE( 0 );		// decay * 0.1
	MESSAGE_END( );

	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "debris/zap1.wav", 1, ATTN_NORM, 0, 100 );
#endif
}

//=========================================================
// PainSound
//=========================================================
void CISlave::PainSound( void )
{
	if (RANDOM_LONG( 0, 2 ) == 0)
	{
		UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, m_voicePitch );
	}
}

//=========================================================
// DieSound
//=========================================================

void CISlave::DeathSound( void )
{
	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pDeathSounds[ RANDOM_LONG(0,ARRAYSIZE(pDeathSounds)-1) ], 1.0, ATTN_NORM, 0, m_voicePitch );
}


//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CISlave::ISoundMask ( void) 
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_DANGER	|
			bits_SOUND_PLAYER	|
			//MODDD - new
			bits_SOUND_BAIT;
}


//NOTICE: default behavior for "onDeathAnimationEnd" is to turn the think method off. So the check for whether this monster will be self-revived or not makes sense here.
//A self reviving monster can't even check its own countdown timer for revival if its think method is turned off.
void CISlave::onDeathAnimationEnd(void){
	//let's have a planned revive.
	//If I plan on fading though, ignore all this and just let me fade out. No chance of self-revive to avoid spam.

	if(!this->ShouldFadeOnDeath()){
		BOOL canRevive = (EASY_CVAR_GET_DEBUGONLY(islaveReviveSelfChance) > 0 && RANDOM_FLOAT(0, 1) <= EASY_CVAR_GET_DEBUGONLY(islaveReviveSelfChance) );

		if(canRevive){
			selfReviveTime = gpGlobals->time + RANDOM_LONG(EASY_CVAR_GET_DEBUGONLY(islaveReviveSelfMinDelay), EASY_CVAR_GET_DEBUGONLY(islaveReviveSelfMaxDelay) );
			//note that we omitt the think unlink if we plan on reviving.  Need something to count the time left until a self-revive.
		}else{
			//kill the "think" linkup like in normal death.
			SetThink ( NULL );
		}
	}else{
		//parent method would also do nothing so don't call it.
		//...this decision may age poorly.  Whatever just call it even if it denies doing anything at the moment.
		CSquadMonster::onDeathAnimationEnd();
	}

}

GENERATE_KILLED_IMPLEMENTATION(CISlave){
	//MODDD
	forgetReviveTarget();
	beingRevived = 0;
	selfReviveTime = -1;

	ClearBeams( );
	GENERATE_KILLED_PARENT_CALL(CSquadMonster);
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CISlave::SetYawSpeed ( void )
{
	int ys;

	//MODDD
	if(beingRevived != 0){
		ys = 0;
		//can't turn while being raised from the dead.
		//...yes, that is something that must be spelled out.
	}

	/*
	switch ( m_Activity )
	{
	case ACT_WALK:		
		ys = 50;	
		break;
	case ACT_RUN:		
		ys = 70;
		break;
	case ACT_IDLE:		
		ys = 50;
		break;
	default:
		ys = 90;
		break;
	}
	*/

	//MODDD - little faster values.
	switch ( m_Activity )
	{
	case ACT_WALK:		
		ys = 100;	
		break;
	case ACT_RUN:		
		ys = 120;
		break;
	case ACT_IDLE:		
		ys = 80;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CISlave::HandleAnimEvent( MonsterEvent_t *pEvent )
{

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1){
		return;
	}

	// ALERT( at_console, "event %d : %f\n", pEvent->event, pev->frame );
	switch( pEvent->event )
	{
		case ISLAVE_AE_CLAW:
		{
			// SOUND HERE!
			//MODDD - deals bleeding now.
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.slaveDmgClaw, DMG_SLASH, DMG_BLEEDING );
			if ( pHurt )
			{
				if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
				{
					pHurt->pev->punchangle.z = -18;
					pHurt->pev->punchangle.x = 5;
				}
				// Play a random attack hit sound
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, m_voicePitch );
			}
			else
			{
				// Play a random attack miss sound
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, m_voicePitch );
			}
		}
		break;

		case ISLAVE_AE_CLAWRAKE:
		{
			//MODDD - deals bleeding.
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.slaveDmgClawrake, DMG_SLASH, DMG_BLEEDING );
			if ( pHurt )
			{
				if ( (pHurt->pev->flags & (FL_MONSTER|FL_CLIENT)) && !pHurt->blocksImpact() )
				{
					pHurt->pev->punchangle.z = -18;
					pHurt->pev->punchangle.x = 5;
				}
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, m_voicePitch );
			}
			else
			{
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, m_voicePitch );
			}
		}
		break;

		case ISLAVE_AE_ZAP_POWERUP:
		{
			// speed up attack when on hard
			if (g_iSkillLevel == SKILL_HARD)
				pev->framerate = 1.5;

			UTIL_MakeAimVectors( pev->angles );

			if (m_iBeams == 0)
			{
				Vector vecSrc = pev->origin + gpGlobals->v_forward * 2;
				MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
					WRITE_BYTE(TE_DLIGHT);
					WRITE_COORD(vecSrc.x);	// X
					WRITE_COORD(vecSrc.y);	// Y
					WRITE_COORD(vecSrc.z);	// Z
					WRITE_BYTE( 12 );		// radius * 0.1
					WRITE_BYTE( 255 );		// r
					WRITE_BYTE( 180 );		// g
					WRITE_BYTE( 96 );		// b
					WRITE_BYTE( 20 / pev->framerate );		// time * 10
					WRITE_BYTE( 0 );		// decay * 0.1
				MESSAGE_END( );

			}
			if (m_hDead != NULL)
			{
				WackBeam( -1, m_hDead );
				WackBeam( 1, m_hDead );
			}
			else
			{
				ArmBeam( -1 );
				ArmBeam( 1 );
				BeamGlow( );
			}

			//MODDD - NOTE - PERIOD SOUND.  Played constantly.  ...did I mean 'looped' here?
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "debris/zap4.wav", 1, ATTN_NORM, 0, 100 + m_iBeams * 10 );
			pev->skin = m_iBeams / 2;
		}
		break;

		case ISLAVE_AE_ZAP_SHOOT:
		{
			ClearBeams( );

			//MODDD - replacing this call with the more reliable "IsDeadEntity"     (this used to be     m_hDead != NULL  )
			if (UTIL_IsDeadEntity(m_hDead) )
			{
				Vector vecDest = m_hDead->pev->origin + Vector( 0, 0, 38 );
				TraceResult trace;
				UTIL_TraceHull( vecDest, vecDest, dont_ignore_monsters, human_hull, m_hDead->edict(), &trace );

				if ( !trace.fStartSolid )
				{

					/*
					//MODDD - being replaced by proper "revive" code.
					CBaseEntity *pNew = Create( "monster_alien_slave", m_hDead->pev->origin, m_hDead->pev->angles );
					CBaseMonster *pNewMonster = pNew->MyMonsterPointer( );
					pNew->pev->spawnflags |= 1;
					WackBeam( -1, pNew );
					WackBeam( 1, pNew );
					UTIL_Remove( m_hDead );
					UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, RANDOM_LONG( 130, 160 ) );
					
					//...what was this supposed to do?
					pNew->pev->spawnflags |= 1;
					*/


					CBaseMonster* tempMonster = m_hDead->GetMonsterPointer();
					CISlave* tempIslave = tempMonster != NULL ? static_cast<CISlave*>(tempMonster) : NULL;
					
					//easyPrintLine("??????? %d ", tempIslave->okayToRevive());
					if(tempIslave != NULL && tempIslave->okayToRevive()){

						tempIslave->riseFromTheGrave();
						WackBeam( -1, tempIslave );
						WackBeam( 1, tempIslave );
						UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, RANDOM_LONG( 125, 140 ) );
							
					
					}else{

					}
					this->forgetReviveTarget();
					if(tempIslave != NULL){
						tempIslave->forgetReviveTarget();
					}
					reviveTargetChosen = FALSE;
					m_hDead = NULL;


					//MODDD - what is this?  found commented out.
					/*
					CBaseEntity *pEffect = Create( "test_effect", pNew->Center(), pev->angles );
					pEffect->Use( this, this, USE_ON, 1 );
					*/
					break;
				}
			}else{
				this->forgetReviveTarget();
				reviveTargetChosen = FALSE;
				m_hDead = NULL;
			}



			ClearMultiDamage();

			UTIL_MakeAimVectors( pev->angles );

			ZapBeam( -1 );
			ZapBeam( 1 );

			//MODDD - pitch tightened, was 130 to 160
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, RANDOM_LONG( 140, 165 ) );
			// UTIL_StopSound( ENT(pev), CHAN_WEAPON, "debris/zap4.wav" );
			ApplyMultiDamage(pev, pev);

			m_flNextAttack = gpGlobals->time + RANDOM_FLOAT( 0.5, 4.0 );
		}
		break;

		case ISLAVE_AE_ZAP_DONE:
		{
			ClearBeams( );
		}
		break;

		default:
			CSquadMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//=========================================================
// CheckRangeAttack1 - normal beam attack 
//=========================================================
BOOL CISlave::CheckRangeAttack1 ( float flDot, float flDist )
{
	if (m_flNextAttack > gpGlobals->time)
	{
		return FALSE;
	}

	//MODDD NOTE - you guys couldn't be bothered to give this a custom range (flDist?) damn devs!
	//Lucky coincidence 784 makes sense. The electric bolt phyisically only travels 1024 since the trace for it goes that far.
	//Pasted it here anyways because hey, we might want tweaking sometime.

	if ( flDist > 64 && flDist <= 784 && flDot >= 0.5 )
	{
		return TRUE;
	}
	return FALSE;

	//return CSquadMonster::CheckRangeAttack1( flDot, flDist );
}

//=========================================================
// CheckRangeAttack2 - check bravery and try to resurect dead comrades
//=========================================================
BOOL CISlave::CheckRangeAttack2 ( float flDot, float flDist )
{
	// NO.  We're not doing this as a separate attack, that makes a lot less sense.
	return FALSE;
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////


	//MODDD - AHHHHHH.  HOW DID I MISS THIS?!  disabled.
	//return FALSE;

	if(EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendMode) == 0 || EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendMode) == 2){
		//try by for a chance.
		
		//the chance won't happen here, it will occur in GetSchedule (when looking for something new to do) so that it doesn't happen 20 times every second.
		/*
		BOOL canReviveFriend = (EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendChance) > 0 && RANDOM_FLOAT(0, 1) <= EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendChance));
		if(!canReviveFriend){
			return FALSE;
		}
		*/
	}else{
		//nope.
		return FALSE;
	}


	if (m_flNextAttack > gpGlobals->time)
	{
		return FALSE;
	}

	//MODDD - turn this off.  Not sure if forcing to 100 would be better.
	//m_iBravery = 0;

	//already sets m_hDead properly automatically.
	findISlaveToRevive(TRUE, flDist);

	if (m_hDead != NULL)
		return TRUE;
	else
		return FALSE;
}


CISlave* CISlave::findISlaveToRevive(BOOL requireLineTrace, float argStartMaxDist){


	/*
			while ((pEntityScan = UTIL_FindEntityInSphere( pEntityScan, pev->origin, EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendRange) )) != NULL)
			{
				testMon = pEntityScan->MyMonsterPointer();
				//if(testMon != NULL && testMon->pev != this->pev && ( FClassnameIs(testMon->pev, "monster_scientist") || FClassnameIs(testMon->pev, "monster_barney")  ) ){
				
				
				//if(testMon != NULL && FClassnameIs(testMon->pev, "monster_alien_slave" ) && testMon->pev != this->pev){
				//	easyPrintLine("AM I fine ENOUGH FOR YOUR time %d", UTIL_IsDeadEntity(testMon)  );
				//}
				

				if(testMon != NULL && testMon->pev != this->pev && UTIL_IsDeadEntity(testMon) && FClassnameIs(testMon->pev, "monster_alien_slave" ) ){
					thisDistance = (testMon->pev->origin - pev->origin).Length();
					
					thisNameSucks = static_cast<CISlave*>(testMon);
					
					
					//if(thisNameSucks != NULL){
					//	easyPrintLine("AM I fine ENOUGH FOR YOUR TIIIME me:%d them:%d   %d %d %.2f %.2f", monsterID, thisNameSucks->monsterID, thisNameSucks->monsterTryingToReviveMe == NULL, thisNameSucks->beingRevived, thisDistance, leastDistanceYet  );
					//}
					

					//only allow one scientist to try to reach this NPC.  That is, this NPC's own "scientistTryingToHealMe" is null, that is.
					//Also, "should fade on death".  Let's avoid reviving respawnables for the sake of lag.
					if(thisNameSucks != NULL && thisNameSucks->monsterTryingToReviveMe == NULL && thisNameSucks->beingRevived == FALSE && thisNameSucks->ShouldFadeOnDeath() == FALSE ){
						//healTargetNPC = testMon;

						if(thisDistance < leastDistanceYet){
							m_hTargetEnt = testMon;
							bestChoiceYet = thisNameSucks;
							reviveTargetChosen = TRUE;
							leastDistanceYet = thisDistance;
						}
						//break;
					}
				}
			
			}
	*/

	float flDist = argStartMaxDist;

	CISlave* bestChoiceYet = NULL;

	CBaseEntity *pEntity = NULL;
	while ((pEntity = UTIL_FindEntityByClassname( pEntity, "monster_alien_slave" )) != NULL)
	{
		TraceResult tr;
		BOOL tracePass = FALSE;
		
		

		if(requireLineTrace){

			//the other entity's eye position? Isn't it supposed to be dead?
			//UTIL_TraceLine( EyePosition( ), pEntity->EyePosition( ), ignore_monsters, ENT(pev), &tr );
			UTIL_TraceLine( EyePosition( ), pEntity->pev->origin + Vector(0, 0, 10), ignore_monsters, ENT(pev), &tr );
			tracePass = (tr.flFraction == 1.0 || tr.pHit == pEntity->edict());
		}else{
			//skip it.
			tracePass = TRUE;
		}
		
		if(tracePass)
		{
			if(UTIL_IsDeadEntity(pEntity))  //if (pEntity->pev->deadflag == DEAD_DEAD)
			{

				//MODDD - extra requirement: no one is currently trying to revive this one already.
				CBaseMonster* tempMonster = pEntity->GetMonsterPointer();

				//If this monster is fading (since dead and this is true), don't try to revive. Not worth your time.
				if(!tempMonster->ShouldFadeOnDeath()){

					CISlave* tempIslave = tempMonster != NULL ? static_cast<CISlave*>(tempMonster) : NULL;
				
					if(tempIslave != NULL && tempIslave->monsterTryingToReviveMeEHANDLE == NULL && tempIslave->beingRevived == FALSE && tempIslave->ShouldFadeOnDeath() == FALSE){
						float d = (pev->origin - pEntity->pev->origin).Length();
						if (d < flDist)
						{
							m_hDead = pEntity;
							flDist = d;
							bestChoiceYet = tempIslave;
							reviveTargetChosen = TRUE;  //is this line okay??
						}
					}//END OF tempISlave != NULL && ...)

				}//END OF shouldFadeOnDeath check

				//m_iBravery--;
			}
			else
			{
				//m_iBravery++;
			}
		}
	}//END OF while(...)


	//need to do anything if it is null?
	if(bestChoiceYet == NULL){
		m_hDead = NULL;
	}else{
		m_hDead = bestChoiceYet; //implied?
	}

	return bestChoiceYet;
}//END OF findISlaveToRevive


//=========================================================
// StartTask
//=========================================================
void CISlave::StartTask ( Task_t *pTask )
{
	ClearBeams( );


	CBaseMonster* tempMonster = NULL;
	CISlave* tempISlave = NULL;
	CISlave* bestChoiceYet = NULL;


	switch(pTask->iTask){

	case TASK_RANGE_ATTACK1:
		//INJECTION:  Here, do the check for whether reviving is ok or not.

		m_hDead = NULL;
		//MODDD - turn this off.  Not sure if forcing to 100 would be better.
		//m_iBravery = 0;


		if(canReviveFriend && !reviveTargetChosen && EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendMode) == 0 || EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendMode) == 2){

			CBaseEntity *pEntity = NULL;
			float flDist; 
			
			if(m_hEnemy != NULL && UTIL_IsAliveEntity(m_hEnemy) ){
				flDist = (m_hEnemy->pev->origin - pev->origin).Length2D();
				bestChoiceYet = findISlaveToRevive(TRUE, flDist);
				//Notice that "findISlaveToRevive" automatically sets m_hDead to whatever bestChoiceYet receives here, but just as an Edict instead of
				//an islave.
			}else{
				bestChoiceYet = NULL;
				m_hDead = NULL;
			}


			if(m_hDead != NULL)reviveTargetChosen = TRUE;
			
			if (reviveTargetChosen && m_hDead != NULL && bestChoiceYet->okayToRevive() ){
				//return TRUE;
				bestChoiceYet->monsterTryingToReviveMe = this;
				bestChoiceYet->monsterTryingToReviveMeEHANDLE = this;
				
				bestChoiceYet->beingRevived = 1;

				// don't let anything obstruct my target
				UTIL_SetSize(bestChoiceYet->pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

			}else{
				reviveTargetChosen = FALSE;
				m_hDead = NULL;
			}


		}//EMD OF revive check



		CSquadMonster::StartTask(pTask);
	break;

	case TASK_ISLAVE_GO_TO_TARGET:
		//assume target stuff is good.

		//this->FRefreshRoute();
		
		//if(!MoveToTarget(ACT_RUN, 0)){
		//already set it ahead of time, just do "m_vecMoveGoal"
		//HAHA.  Apparently, no.


		//if(m_hTargetEnt == NULL || !MoveToLocation(ACT_RUN, 0, m_hTargetEnt->pev->origin + (0, 0, 12) )){
		
		if(!MoveToTarget(ACT_RUN, 0)){
			TaskFail();
			return;
		};


		m_movementActivity = ACT_RUN;
		m_IdealActivity = ACT_RESET;
		//if(m_Activity != ACT_RUN){
			this->SetActivity(ACT_RUN);
		//}



		if (FRouteClear())
		{
			TaskComplete();
		}
		//????

	break;
	case TASK_ISLAVE_SET_REVIVE_SELF_SEQUENCE:{
		
		//0 = dieheadshot-RES
		//1 = diesimple-RES
		//2 = diebackward-RES
		//3 = dieforward-RES

		//wait.. why do we always say "backwards"?  At least I do.  oh well.

		//???????????
		//int testa = LookupSequence("dieforward");

		//pick a revive anim based on how I died.
		if(pev->sequence == ISLAVE_DIEHEADSHOT){
			this->SetSequenceByIndex(ISLAVE_DIEHEADSHOT_RES);
		}else if(pev->sequence == ISLAVE_DIESIMPLE){
			this->SetSequenceByIndex(ISLAVE_DIESIMPLE_RES);
		}else if(pev->sequence == ISLAVE_DIEBACKWARD){
			this->SetSequenceByIndex(ISLAVE_DIEBACKWARD_RES);
		}else if(pev->sequence == ISLAVE_DIEFORWARD){
			this->SetSequenceByIndex(ISLAVE_DIEFORWARD_RES);
		}else if(pev->sequence == ISLAVE_DIEVIOLENT){
			//the DIE_VIOLENT one?  Just play this in reverse and restore the framerate.
			this->SetSequenceByIndex(pev->sequence, -1, FALSE);
			ChangeSchedule(slWaitForReviveSequence);
		}

		//ResetSequenceInfo();
		//SetYawSpeed();
		//pev->frame = 0;

	break;}
	case TASK_ISLAVE_SET_REVIVE_SEQUENCE:
		//ok.
		
		tempMonster = m_hTargetEnt!=NULL ? m_hTargetEnt->MyMonsterPointer() : NULL;
		if(reviveTargetChosen && m_hTargetEnt != NULL && tempMonster != NULL){
			tempISlave = static_cast<CISlave*>(tempMonster);
			if(tempISlave != NULL && tempISlave->okayToRevive() == TRUE){
				//proceed!
				reviveFriendAnimStartTime = gpGlobals->time;
				
				pev->frame = 0;
				//this->SetSequenceByName("downup");
				this->pev->sequence = ISLAVE_DOWNUP;
				ResetSequenceInfo( );
				SetYawSpeed();

				//easyPrintLine("HELP!!!????");
				//pev->frame = 0;

				tempISlave->beingRevived = 1;
				UTIL_SetSize(tempISlave->pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
			}else{
				forgetReviveTarget();
				TaskFail();
				return;
			}
		}else{
			//failed to get temp ent?
			forgetReviveTarget();
			TaskFail();
			return;
		}
	break;
	default:
		CSquadMonster::StartTask ( pTask );
	break;
	}//END OF switch(...)

}



void CISlave::riseFromTheGrave(void){
	StartReanimation();
}

void CISlave::StartReanimation(){

	// Don't let me be obstructed during this animation
	UTIL_SetSize(this->pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	
	selfReviveTime = -1;
	if(monsterTryingToReviveMeEHANDLE != NULL){
		/*
		monsterTryingToReviveMe->reviveTargetChosen = FALSE;
		monsterTryingToReviveMe->m_hTargetEnt = NULL;
		monsterTryingToReviveMe = NULL;
		*/
		monsterTryingToReviveMe->forgetReviveTarget();
	}
	beingRevived = 2;

	// NOTE - parent calls StartReanimationPost.
	CSquadMonster::StartReanimation();
}//END OF StartReanimation
void CISlave::StartReanimationPost(int preReviveSequence){
	/*
	pev->sequence = -1; //force reset.
	SetSequenceByIndex(preReviveSequence, -1, FALSE);

	ChangeSchedule(slWaitForSequence );
	*/

	m_IdealMonsterState	= MONSTERSTATE_ALERT;// Assume monster will be alert, having come back from the dead and all.
	m_MonsterState = MONSTERSTATE_ALERT; //!!!
	m_IdealActivity = ACT_IDLE;
	m_Activity = ACT_IDLE; //!!! No sequence changing, force the activity to this now.

	//CBaseMonster::StartReanimation();
	ChangeSchedule(slISlaveReviveSelf );

}//END OF StartReanimationPost



void CISlave::makeThingForgetReviveInfo(CBaseEntity* thing, CISlave* thingSpecific){

	if(thingSpecific->beingRevived == 1){
		//reset the bounds.
		//UTIL_SetSize(pev, () () );

		//a copy of "TASK_DIE" 's script.
		if ( !thing->GetMonsterPointer()->BBoxFlat() )
		{
			UTIL_SetSize ( thing->pev, Vector ( -4, -4, 0 ), Vector ( 4, 4, 1 ) );
		}
		else{ // !!!HACKHACK - put monster in a thin, wide bounding box until we fix the solid type/bounding volume problem
			UTIL_SetSize ( thing->pev, Vector ( thing->pev->mins.x, thing->pev->mins.y, thing->pev->mins.z ), Vector ( thing->pev->maxs.x, thing->pev->maxs.y, thing->pev->mins.z + 1 ) );
		}

		thingSpecific->beingRevived = 0;
	}
	thingSpecific->monsterTryingToReviveMe = NULL;
	thingSpecific->monsterTryingToReviveMeEHANDLE = NULL;
}


void CISlave::forgetReviveTarget(void){
	
	//meh, just do both, why not.
	if(monsterTryingToReviveMeEHANDLE != NULL){
		monsterTryingToReviveMe->forgetReviveTarget();
	}

	if(reviveTargetChosen){
		reviveTargetChosen = FALSE;

		//scientistTryingToHealMe
		if(m_hDead == NULL && m_hTargetEnt != NULL && UTIL_IsValidEntity(m_hTargetEnt) ){
			
			CBaseEntity* testEnt = Instance(m_hTargetEnt->pev);
			if(testEnt != NULL){
				CISlave* thisNameSucks = static_cast<CISlave*>(testEnt);
				if(thisNameSucks != NULL){
					makeThingForgetReviveInfo(m_hTargetEnt, thisNameSucks);
				}
			}
		}
		if(m_hDead != NULL){
			//if "m_hDead" is not null, this is probably the old method that doesn't involve targetEnt.
			//m_hTargetEnt = NULL;
			
			
			CBaseEntity* testEnt = Instance(m_hDead->pev);
			if(testEnt != NULL){
				CISlave* thisNameSucks = static_cast<CISlave*>(testEnt);
				if(thisNameSucks != NULL){
					makeThingForgetReviveInfo(m_hDead, thisNameSucks);
				}
			}
		}
	}

	m_hDead = NULL;
}

//is the area right above my corpse empty?  If so, there's space for me to stand on being revived.
BOOL CISlave::okayToRevive(void){
	CBaseEntity *ent = NULL;
	// ... what
	//Vector spot = pev->origin + (0, 0, 20);

	CBaseEntity* theList[32];
	
	// maybe not yet...
	//UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
	Vector livingSizeMins = VEC_HUMAN_HULL_MIN + Vector(-12, -12, 0);
	//MODDD - z-coord here was 60, isn't that a tad excessive?  Why any change really, VEC_HUMAN_HULL_MAX has the correct Z
	Vector livingSizeMaxs = VEC_HUMAN_HULL_MAX + Vector(12, 12, 2);

	int theListSoftMax = UTIL_EntitiesInBox( theList, 32, pev->origin + livingSizeMins, pev->origin + livingSizeMaxs, 0 );


	for(int i = 0; i < theListSoftMax; i++){
		ent = theList[i];

		if (ent->pev->solid == SOLID_NOT || ent->pev->solid == SOLID_TRIGGER) {
			// not collidable?  Skip it.
			continue;
		}

		// "&& ent->MyMonsterPointer() != NULL" check?  unsure if that's a good idea.
		if (
			ent == this || UTIL_IsDeadEntity(ent)
			||  (ent->IsWorldOrAffiliated() && !ent->isBreakableOrChild())
		){
			//easyPrintLine("WHAT THE heck IS YOUR darn ID %d", ent->MyMonsterPointer()->monsterID);
			if(UTIL_IsAliveEntity(ent) && IRelationship(ent) <= R_NO ){
				// maybe send some "scramble position" advisory to get off of me, if friendly?
				//ent->needToMove = TRUE;
			}
			return FALSE;
		}
	}
	
	// this is just a test, don't make any changes yet.
	//UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	return TRUE;
}


void CISlave::RunTask( Task_t *pTask )
{
	//easyPrintLine("BOOM BOOM BOOM %s %d", getScheduleName(), getTaskNumber());
	
	if(m_pSchedule == slISlaveReviveFriend){
		if(!finishingReviveFriendAnim && UTIL_IsValidEntity(m_hTargetEnt) == FALSE){
			m_hTargetEnt = NULL;
			//forget him.
			this->forgetReviveTarget();
			TaskFail();
			return;
		}
		//maybe a check for "beingRevived" ... ?   Nah, we'll let the one rising let us know if this is the case.
	}else if(m_hDead != NULL){
		//trying the old method?

		if(UTIL_IsValidEntity(m_hDead) == FALSE){
			//no longer ok!
			this->forgetReviveTarget();
			m_hDead = NULL;
			TaskFail();
			return;
		}

	}

	float distance2D;

	switch( pTask->iTask )
	{
	case TASK_ISLAVE_GO_TO_TARGET:
		//assume target stuff is good.


		if(m_hTargetEnt == NULL){
			TaskFail();
			return;
		}
		
		//EXPERIMENTAL!!!
		FRefreshRoute();

		if ( FRouteClear() )
		{
			TaskFail();
			return;
		}
		

		distance2D = (m_hTargetEnt->pev->origin - pev->origin).Length2D();
		//if (MovementIsComplete() || distance2D < 120


		//easyPrintLine("WHAT THE derp IS GOING ON????? %.2f", distance2D);

		if (distance2D < 110)
		{
			TaskComplete();
			RouteClear();		// Stop moving
		}


	break;
	case TASK_ISLAVE_SET_REVIVE_SELF_SEQUENCE:

		if(m_fSequenceFinished){
			//up now.
			beingRevived = 0;

			TaskComplete();
		}
	break;
	case TASK_ISLAVE_SET_REVIVE_SEQUENCE:

		//some time since anim start... we revive.  Make our friend revive.


		if(m_hTargetEnt != NULL && gpGlobals->time > reviveFriendAnimStartTime + 1.2f){
			
			CBaseEntity* testEnt = Instance(m_hTargetEnt->pev);
			if(testEnt != NULL){
				CISlave* tempSlave = static_cast<CISlave*>(testEnt);
				if(tempSlave != NULL && tempSlave->beingRevived < 2){

					tempSlave->beingRevived = 2;
					//done in riseFromTheGrave()
					
					//tempSlave->monsterTryingToReviveMe = NULL;
					//tempSlave->monsterTryingToReviveMeEHANDLE = NULL;
					//reviveTargetChosen = FALSE;
					tempSlave->riseFromTheGrave();
					m_hTargetEnt = NULL;
					finishingReviveFriendAnim = TRUE;

					forgetReviveTarget();
				}
			}
			
		}

		if(m_fSequenceFinished){
			finishingReviveFriendAnim = FALSE;
			TaskComplete();
		}
	break;
	default:
		CSquadMonster::RunTask(pTask);
	break;
	}//END OF switch(...)
}


float CISlave::getBarnacleForwardOffset(void){
	return -2.7;
}
BOOL CISlave::getIsBarnacleVictimException(void){
	return TRUE;
}
float CISlave::getBarnaclePulledTopOffset(void){
	return 38;
}



//MODDD - do we need to do something for ACT_BARNACLE_CHEW  too?  ACT_BARNACLE_CHOMP ?
// Those are already in the model. They can be involved below to adjust their framerates similarly.

int CISlave::LookupActivityHard(int activity){


	resetEventQueue();

	this->m_flFramerateSuggestion = 1;
	pev->framerate = 1;

	//HACK - just force it for now,  animation is seamless for the most part?
	//....nah.
	//strafeMode = idealStrafeMode;



	//strafeMode = idealStrafeMode;
		
	/*
	if(! (activity == ACT_DIE_HEADSHOT || activity == ACT_DIESIMPLE || activity == ACT_DIEBACKWARD || activity == ACT_DIEFORWARD) ){
		return CSquadMonster::LookupActivity(activity);
	}
	*/
	//ACT_DIE_HEADSHOT

	//MODDD
	switch(activity){
	//switch( (int)CVAR_GET_FLOAT("testVar") ){
		case ACT_DIE_HEADSHOT:
			//not randomized between this and the "-RES" version for now...
			return ISLAVE_DIEHEADSHOT;
		break;
		case ACT_DIESIMPLE :
			return ISLAVE_DIESIMPLE;
		break;
		case ACT_DIEBACKWARD :
			return ISLAVE_DIEBACKWARD;
		break;
		case ACT_DIEFORWARD :
			return ISLAVE_DIEFORWARD;
		break;
		case ACT_BIG_FLINCH:
			//has the animation hooked up, just a tad slow maybe.
			m_flFramerateSuggestion = 1.32f;
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_BARNACLE_HIT:
			//just some intervention.
			this->m_flFramerateSuggestion = 1.27;
			animFrameStartSuggestion = 68;
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_BARNACLE_PULL:
			//just some intervention.
			this->m_flFramerateSuggestion = 1.14;
			return CBaseAnimating::LookupActivity(activity);
		break;

	}
	
	return CBaseAnimating::LookupActivity(activity);
}


int CISlave::tryActivitySubstitute(int activity){

	switch(activity){
		case ACT_DIE_HEADSHOT:
			//not randomized between this and the "-RES" version for now...
			return ISLAVE_DIEHEADSHOT;
		break;
		case ACT_DIESIMPLE :
			return ISLAVE_DIESIMPLE;
		break;
		case ACT_DIEBACKWARD :
			return ISLAVE_DIEBACKWARD;
		break;
		case ACT_DIEFORWARD :
			return ISLAVE_DIEFORWARD;
		break;
	}

	return CBaseAnimating::LookupActivity(activity);
}


BOOL CISlave::usesAdvancedAnimSystem(void){
	return TRUE;
}


CISlave::CISlave(void){
	canReviveFriend = FALSE;
	selfReviveTime = -1;
	finishingReviveFriendAnim = FALSE;

	monsterTryingToReviveMe = NULL;
	monsterTryingToReviveMeEHANDLE = NULL;

	reviveTargetChosen = FALSE;
	beingRevived = FALSE;
	reviveFriendAnimStartTime = -1;

}

//=========================================================
// Spawn
//=========================================================
void CISlave::Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/islave.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->classname = MAKE_STRING("monster_alien_slave");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	pev->health			= gSkillData.slaveHealth;
	pev->view_ofs		= Vector ( 0, 0, 64 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_RANGE_ATTACK2 | bits_CAP_DOORS_GROUP;

	m_voicePitch		= RANDOM_LONG( 85, 110 );


	MonsterInit();
}

extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CISlave::Precache()
{
	int i;

	PRECACHE_MODEL("models/islave.mdl");
	PRECACHE_MODEL("sprites/lgtning.spr");

	global_useSentenceSave = TRUE;
	PRECACHE_SOUND("debris/zap1.wav");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav", TRUE); //force it. player gausss sound.
	PRECACHE_SOUND("hassault/hw_shoot1.wav");
	PRECACHE_SOUND("zombie/zo_pain2.wav");
	PRECACHE_SOUND("headcrab/hc_headbite.wav");
	//PRECACHE_SOUND("weapons/cbar_miss1.wav", TRUE); why? never uses this.

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

	global_useSentenceSave = FALSE;

	UTIL_PrecacheOther( "test_effect" );
}	


//=========================================================
// TakeDamage - get provoked when injured
//=========================================================

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CISlave)
{
	// don't slash one of your own
	//MODDD - the barancle can now kill the islave.
	//if ((bitsDamageType & DMG_SLASH) && pevAttacker && IRelationship( Instance(pevAttacker) ) < R_DL)
	if ((bitsDamageType & DMG_SLASH) && !(bitsDamageTypeMod & DMG_BARNACLEBITE) && pevAttacker && (IRelationship( Instance(pevAttacker) ) < R_DL)   )
		return 0;

	m_afMemory |= bits_MEMORY_PROVOKED;
	//MODDD - IMPORTANT - also sends the new "bitsDamageTypeMod" bitmask.   Important or else killing the corpse causes a crash (?)
	return GENERATE_TAKEDAMAGE_PARENT_CALL(CSquadMonster);
}


GENERATE_TRACEATTACK_IMPLEMENTATION(CISlave)
{
	if (bitsDamageType & DMG_SHOCK)
		return;

	//???!!!
	switch(ptr->iHitgroup){
		//either of these.
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:

			if(m_pSchedule == slSlaveAttack1){
				// interruptable by this?
				// Might be cleaner than straight "ChangeSchedule", unsure
				m_failSchedule = SCHED_BIG_FLINCH;
				TaskFail();

				//this->SetConditions(bits_COND_HEAVY_DAMAGE);
				//this->m_ifail
				//TaskFail();
			}

		break;
	}//END OF switch
	

	GENERATE_TRACEATTACK_PARENT_CALL(CSquadMonster);
}//END OF TRACEATTACK


//=========================================================
// AI Schedules Specific to this monster
//=========================================================



// primary range attack
Task_t	tlSlaveAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slSlaveAttack1[] =
{
	{ 
		tlSlaveAttack1,
		ARRAYSIZE ( tlSlaveAttack1 ), 
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_HEAR_SOUND |
		bits_COND_HEAVY_DAMAGE, 

		bits_SOUND_DANGER,
		"Slave Range Attack1"
	},
};


DEFINE_CUSTOM_SCHEDULES( CISlave )
{
	slSlaveAttack1,
	//MODDD
	slISlaveReviveFriend,
	slISlaveReviveSelf,
};

IMPLEMENT_CUSTOM_SCHEDULES( CISlave, CSquadMonster );



void CISlave::ScheduleChange(void){
	//OK?
	ClearBeams();

	forgetReviveTarget();
	finishingReviveFriendAnim = FALSE;
	
	CSquadMonster::ScheduleChange();
}

//=========================================================
//=========================================================
Schedule_t *CISlave::GetSchedule( void )
{
	ClearBeams( );

	//MODDD - that okay?
	forgetReviveTarget();
	finishingReviveFriendAnim = FALSE;

/*
	if (pev->spawnflags)
	{
		pev->spawnflags = 0;
		return GetScheduleOfType( SCHED_RELOAD );
	}
*/

	if ( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );

		if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
		if ( pSound->m_iType & bits_SOUND_COMBAT )
			m_afMemory |= bits_MEMORY_PROVOKED;
	}


	//MODDD - random chance of doing a revive.
	//CISlave* reviveTarget;
	CBaseEntity* pEntityScan = NULL;
	float leastDistanceYet = 999999;

	CBaseMonster* testMon;
	CISlave* thisNameSucks;
	CISlave* bestChoiceYet;
	float thisDistance;


	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:

		// dead enemy
		//MODDD - woa, for a while this got pushed way below.  It's fine to go back up here right?
		if ( HasConditions( bits_COND_ENEMY_DEAD ) )
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		//if(EASY_CVAR_GET(islaveCanRevive) == 1){ 
		//canReviveFriend = (RANDOM_LONG(0, 4) == 0);
		//canReviveFriend = TRUE;
		//}
		canReviveFriend = FALSE;
		if(!reviveTargetChosen){
			canReviveFriend = (EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendChance) > 0 && RANDOM_FLOAT(0, 1) <= EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendChance) );
		}

		if(EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendMode) == 1 || EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendMode) == 2 && canReviveFriend && !reviveTargetChosen){

			bestChoiceYet = findISlaveToRevive(FALSE, EASY_CVAR_GET_DEBUGONLY(islaveReviveFriendRange) );
			m_hTargetEnt = m_hDead;

			if(reviveTargetChosen && bestChoiceYet->okayToRevive() ){
				//NOT THE thisNameSucks!!!!
				//thisNameSucks->monsterTryingToReviveMe = this;
				bestChoiceYet->monsterTryingToReviveMe = this;
				bestChoiceYet->monsterTryingToReviveMeEHANDLE = this;
				

				// don't let anything obstruct my target
				UTIL_SetSize(bestChoiceYet->pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

				//???
				ClearBeams();
				//get a path to our target! CLEAR!

				RouteClear();
				//this->m_movementGoal = MOVEGOAL_TARGETENT;
				//TEST!!!
				this->m_movementGoal = MOVEGOAL_LOCATION;
				m_vecMoveGoal = m_hTargetEnt->pev->origin + (0, 0, 10);


				//m_vecMoveGoal = vecStart + dirChoice * lengthGoneWith;
				//pathOkay = FRefreshRoute();
				FRefreshRoute();


				return slISlaveReviveFriend;
			}else{
				//force it off just in case.
				reviveTargetChosen = FALSE;
				m_hTargetEnt = NULL;

			}

			//reviveTarget = attemptFindDeadFriend();
		}//END OF if able to revive a dead islave




		if (pev->health < 20 || m_iBravery < 0)
		{
			if (!HasConditions( bits_COND_CAN_MELEE_ATTACK1 ))
			{
				m_failSchedule = SCHED_CHASE_ENEMY;
				if (HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
				if ( HasConditions ( bits_COND_SEE_ENEMY ) && HasConditions ( bits_COND_ENEMY_FACING_ME ) )
				{
					// ALERT( at_console, "exposed\n");
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
			}
		}



		//MODDD - the rest of the script for the combat-related actions in GetSchedule for injecting.

		/*
		//this was already handled above very specifically.
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
		*/

		if ( HasConditions(bits_COND_NEW_ENEMY) )
		{
			return GetScheduleOfType ( SCHED_WAKE_ANGRY );
		}

		//MODDD - new
		else if(HasConditions(bits_COND_HEAVY_DAMAGE)){
			//MODDD - taking heavy damage is more drastic now with its own check.
			//It won't happen often enough to need memory for blocking.
			return GetScheduleOfType(SCHED_BIG_FLINCH);
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

			// we can see the enemy
			if ( HasConditions(bits_COND_CAN_RANGE_ATTACK1) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}
			//MODDD - the injection.
			if ( HasConditions(bits_COND_CAN_RANGE_ATTACK2) && canReviveFriend )
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
				//easyPrintLine("ducks2");
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
		default:
			//MODDD - call to base method changed to happening only if not in COMBAT.  For COMBAT, we're already using the full script from the base class, so calling the base again makes no sense there.
			return CSquadMonster::GetSchedule( );
		break;
	}
	//MODDD - see shortly above.
	//return CSquadMonster::GetSchedule( );


	return &slError[ 0 ];
	
}


Schedule_t *CISlave::GetScheduleOfType ( int Type ) 
{
	switch	( Type )
	{
	case SCHED_FAIL:
		if (HasConditions( bits_COND_CAN_MELEE_ATTACK1 ))
		{
			return CSquadMonster::GetScheduleOfType( SCHED_MELEE_ATTACK1 ); ;
		}
		break;
	case SCHED_RANGE_ATTACK1:
		return slSlaveAttack1;
	case SCHED_RANGE_ATTACK2:
		return slSlaveAttack1;
	}
	return CSquadMonster::GetScheduleOfType( Type );
}


//=========================================================
// ArmBeam - small beam from arm to nearby geometry
//=========================================================

void CISlave::ArmBeam( int side )
{
	TraceResult tr;
	float flDist = 1.0;
	
	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	UTIL_MakeAimVectors( pev->angles );
	Vector vecSrc = pev->origin + gpGlobals->v_up * 36 + gpGlobals->v_right * side * 16 + gpGlobals->v_forward * 32;

	for (int i = 0; i < 3; i++)
	{
		Vector vecAim = gpGlobals->v_right * side * RANDOM_FLOAT( 0, 1 ) + gpGlobals->v_up * RANDOM_FLOAT( -1, 1 );
		TraceResult tr1;
		UTIL_TraceLine ( vecSrc, vecSrc + vecAim * 512, dont_ignore_monsters, ENT( pev ), &tr1);
		if (flDist > tr1.flFraction)
		{
			tr = tr1;
			flDist = tr.flFraction;
		}
	}

	// Couldn't find anything close enough
	if ( flDist == 1.0 )
		return;


	//MODDD - using Half-Life Source's way instead (put scorch marks).
	// This will keep the decal imposed by a hit breakable or unusual surface if it wants something other than plain bulletholes though.
	// (only those get replaced by scorch marks).
	//DecalGunshot( &tr, BULLET_PLAYER_CROWBAR );
	DecalSafeSmallScorchMark(&tr, DMG_CLUB, 0);



	m_pBeam[m_iBeams] = CBeam::BeamCreate( "sprites/lgtning.spr", 30 );
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit( tr.vecEndPos, entindex( ) );
	m_pBeam[m_iBeams]->SetEndAttachment( side < 0 ? 2 : 1 );
	// m_pBeam[m_iBeams]->SetColor( 180, 255, 96 );
	m_pBeam[m_iBeams]->SetColor( 96, 128, 16 );
	m_pBeam[m_iBeams]->SetBrightness( 64 );
	m_pBeam[m_iBeams]->SetNoise( 80 );
	m_iBeams++;
}


//=========================================================
// BeamGlow - brighten all beams
//=========================================================
void CISlave::BeamGlow( )
{
	int b = m_iBeams * 32;
	if (b > 255)
		b = 255;

	for (int i = 0; i < m_iBeams; i++)
	{
		if (m_pBeam[i]->GetBrightness() != 255) 
		{
			m_pBeam[i]->SetBrightness( b );
		}
	}
}


//=========================================================
// WackBeam - regenerate dead colleagues
//=========================================================
void CISlave::WackBeam( int side, CBaseEntity *pEntity )
{
	Vector vecDest;
	float flDist = 1.0;
	
	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	if (pEntity == NULL)
		return;

	m_pBeam[m_iBeams] = CBeam::BeamCreate( "sprites/lgtning.spr", 30 );
	if (!m_pBeam[m_iBeams])
		return;

	//MODDD - it's on the ground, go a little lower than this.
	//m_pBeam[m_iBeams]->PointEntInit( pEntity->Center(), entindex( ) );
	m_pBeam[m_iBeams]->PointEntInit( pEntity->pev->origin + Vector(0, 0, 6), entindex( ) );

	m_pBeam[m_iBeams]->SetEndAttachment( side < 0 ? 2 : 1 );
	m_pBeam[m_iBeams]->SetColor( 180, 255, 96 );
	m_pBeam[m_iBeams]->SetBrightness( 255 );
	m_pBeam[m_iBeams]->SetNoise( 80 );
	m_iBeams++;
}

//=========================================================
// ZapBeam - heavy damage directly forward
//=========================================================
void CISlave::ZapBeam( int side )
{
	Vector vecSrc, vecAim;
	TraceResult tr;
	CBaseEntity *pEntity;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	vecSrc = pev->origin + gpGlobals->v_up * 36;
	vecAim = ShootAtEnemy( vecSrc );
	float deflection = 0.01;
	vecAim = vecAim + side * gpGlobals->v_right * RANDOM_FLOAT( 0, deflection ) + gpGlobals->v_up * RANDOM_FLOAT( -deflection, deflection );
	UTIL_TraceLine ( vecSrc, vecSrc + vecAim * 1024, dont_ignore_monsters, ENT( pev ), &tr);

	m_pBeam[m_iBeams] = CBeam::BeamCreate( "sprites/lgtning.spr", 50 );
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit( tr.vecEndPos, entindex( ) );
	m_pBeam[m_iBeams]->SetEndAttachment( side < 0 ? 2 : 1 );
	m_pBeam[m_iBeams]->SetColor( 180, 255, 96 );
	m_pBeam[m_iBeams]->SetBrightness( 255 );
	m_pBeam[m_iBeams]->SetNoise( 20 );
	m_iBeams++;

	//MODDD - if this misses, can place a larger scroch mark
	DecalSafeDecal(&tr, DECAL_SMALLSCORCH1+2, DMG_CLUB, 0);


	pEntity = CBaseEntity::Instance(tr.pHit);
	if (pEntity != NULL && pEntity->pev->takedamage)
	{
		//MODDD - don't do damage differently to different hitboxes.
		pEntity->TraceAttack( pev, gSkillData.slaveDmgZap, vecAim, &tr, DMG_SHOCK, DMG_HITBOX_EQUAL );
	}
	//MODDD - pitch lowered, was 140 to 160.
	// Also don't play with soundsentencesave, electro4 is a gauss player weapon (Client) sound.
	UTIL_EmitAmbientSound( ENT(pev), tr.vecEndPos, "weapons/electro4.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG( 135, 150 ), FALSE );
}


//=========================================================
// ClearBeams - remove all beams
//=========================================================
void CISlave::ClearBeams( )
{
	for (int i = 0; i < ISLAVE_MAX_BEAMS; i++)
	{
		if (m_pBeam[i])
		{
			UTIL_Remove( m_pBeam[i] );
			m_pBeam[i] = NULL;
		}
	}
	m_iBeams = 0;
	pev->skin = 0;

	UTIL_StopSound( ENT(pev), CHAN_WEAPON, "debris/zap4.wav" );
}


BOOL CISlave::violentDeathAllowed(void){
	return TRUE;
}
BOOL CISlave::violentDeathClear(void){
	return violentDeathClear_BackwardsCheck(50);
}//END OF violentDeathAllowed
int CISlave::violentDeathPriority(void){
	return 3;
}

void CISlave::onDelete(void){
	//If suddenly removed, clean up my beams
	ClearBeams();
}//END OF onDelete


void CISlave::ForgetEnemy(void) {
	// need to do anything special on being told to forget my enemy?
	// As of yet this only happens by a console command like "chillout", so this
	// isn't too important.

}//END OF ForgetEnemy





