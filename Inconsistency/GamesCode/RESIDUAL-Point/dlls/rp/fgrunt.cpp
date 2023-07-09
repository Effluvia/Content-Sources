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
// fgrunt
//=========================================================

//=========================================================
// Hit groups!	
//=========================================================
/*

  1 - Head
  2 - Stomach
  3 - Gun

*/

#include	"extdll.h"
#include	"plane.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"animation.h"
#include	"squadmonster.h"
#include	"weapons.h"
#include	"talkmonster.h"
#include	"soundent.h"
#include	"effects.h"
#include	"customentity.h"
#include	"hgrunt.h"

extern int g_fGruntQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.

extern DLL_GLOBAL int		g_iSkillLevel;

extern Schedule_t	slIdleStand[];
extern Schedule_t	slGruntFail[];
extern Schedule_t	slGruntCombatFail[];
extern Schedule_t	slGruntVictoryDance[];
extern Schedule_t	slGruntEstablishLineOfFire[];
extern Schedule_t	slGruntFoundEnemy[];
extern Schedule_t	slGruntCombatFace[];
extern Schedule_t	slGruntSignalSuppress[];
extern Schedule_t	slGruntSuppress[];
extern Schedule_t	slGruntWaitInCover[];
extern Schedule_t	slGruntTakeCover[];
extern Schedule_t	slGruntGrenadeCover[];
extern Schedule_t	slGruntTossGrenadeCover[];
extern Schedule_t	slGruntTakeCoverFromBestSound[];
extern Schedule_t	slGruntHideReload[];
extern Schedule_t	slGruntSweep[];
extern Schedule_t	slGruntRangeAttack1A[];
extern Schedule_t	slGruntRangeAttack1B[];
extern Schedule_t	slGruntRangeAttack2[];
extern Schedule_t	slGruntRepel[];
extern Schedule_t	slGruntRepelAttack[];
extern Schedule_t	slGruntRepelLand[];

class CFGrunt : public CHGrunt
{
public:
	void Spawn(void);
	void Precache(void);
	int  Classify(void);
	void HandleAnimEvent(MonsterEvent_t *pEvent);
	void DeathSound(void);
	void PainSound(void);
	void IdleSound(void);
	void SpeakSentence(void);

	Schedule_t	*GetSchedule(void);
	Schedule_t  *GetScheduleOfType(int Type);

	int IRelationship(CBaseEntity *pTarget);

	static const char *pGruntSentences[];
};

LINK_ENTITY_TO_CLASS( monster_human_grunt_ally, CFGrunt )

const char *CFGrunt::pGruntSentences[] =
{
	"FG_GREN", // grenade scared grunt
	"FG_ALERT", // sees player
	"FG_MONSTER", // sees monster
	"FG_COVER", // running to cover
	"FG_THROW", // about to throw grenade
	"FG_CHARGE",  // running out to get the enemy
	"FG_TAUNT", // say rude things
};

//=========================================================
// Speak Sentence - say your cued up sentence.
//
// Some grunt sentences (take cover and charge) rely on actually
// being able to execute the intended action. It's really lame
// when a grunt says 'COVER ME' and then doesn't move. The problem
// is that the sentences were played when the decision to TRY
// to move to cover was made. Now the sentence is played after 
// we know for sure that there is a valid path. The schedule
// may still fail but in most cases, well after the grunt has 
// started moving.
//=========================================================
void CFGrunt::SpeakSentence( void )
{
	if( m_iSentence == HGRUNT_SENT_NONE )
	{
		// no sentence cued up.
		return; 
	}

	if( FOkToSpeak() )
	{
		SENTENCEG_PlayRndSz( ENT( pev ), pGruntSentences[m_iSentence], HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
		JustSpoke();
	}
}

//=========================================================
// IRelationship - overridden because Alien Grunts are 
// Human Grunt's nemesis.
//=========================================================
int CFGrunt::IRelationship( CBaseEntity *pTarget )
{
	if( FClassnameIs( pTarget->pev, "monster_barnacle" ) || ( FClassnameIs( pTarget->pev,  "monster_xarnacle" ) ) )
	{
		return R_NM;
	}

	return CSquadMonster::IRelationship( pTarget );
}

void CFGrunt::IdleSound( void )
{
	if( FOkToSpeak() && ( g_fGruntQuestion || RANDOM_LONG( 0, 1 ) ) )
	{
		if( !g_fGruntQuestion )
		{
			// ask question or make statement
			switch( RANDOM_LONG( 0, 2 ) )
			{
			case 0:
				// check in
				SENTENCEG_PlayRndSz( ENT( pev ), "FG_CHECK", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
				g_fGruntQuestion = 1;
				break;
			case 1:
				// question
				SENTENCEG_PlayRndSz( ENT( pev ), "FG_QUEST", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
				g_fGruntQuestion = 2;
				break;
			case 2:
				// statement
				SENTENCEG_PlayRndSz( ENT( pev ), "FG_IDLE", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
				break;
			}
		}
		else
		{
			switch( g_fGruntQuestion )
			{
			case 1:
				// check in
				SENTENCEG_PlayRndSz( ENT( pev ), "FG_CLEAR", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
				break;
			case 2:
				// question
				SENTENCEG_PlayRndSz( ENT( pev ), "FG_ANSWER", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
				break;
			}
			g_fGruntQuestion = 0;
		}
		JustSpoke();
	}
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CFGrunt::Classify( void )
{
	return CLASS_PLAYER_ALLY;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CFGrunt::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector vecShootDir;
	Vector vecShootOrigin;

	switch( pEvent->event )
	{
		case HGRUNT_AE_DROP_GUN:
		{
			Vector vecGunPos;
			Vector vecGunAngles;

			GetAttachment( 0, vecGunPos, vecGunAngles );

			// switch to body group with no gun.
			SetBodygroup( GUN_GROUP, GUN_NONE );

			// now spawn a gun.
			if( FBitSet( pev->weapons, HGRUNT_SHOTGUN ) )
			{
				 DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
			}
			else
			{
				 DropItem( "weapon_9mmAR", vecGunPos, vecGunAngles );
			}

			if( FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ) )
			{
				DropItem( "ammo_ARgrenades", BodyTarget( pev->origin ), vecGunAngles );
			}
		}
			break;
		case HGRUNT_AE_RELOAD:
			EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM );
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions( bits_COND_NO_AMMO_LOADED );
			break;
		case HGRUNT_AE_GREN_TOSS:
		{
			UTIL_MakeVectors( pev->angles );
			// CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector( 0, 0, 32 ), m_vecTossVelocity, 3.5 );
			CGrenade::ShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 3.5 );

			m_fThrowGrenade = FALSE;
			m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
			// !!!LATER - when in a group, only try to throw grenade if ordered.
		}
			break;
		case HGRUNT_AE_GREN_LAUNCH:
		{
			EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/glauncher.wav", 0.8, ATTN_NORM );
			CGrenade::ShootContact( pev, GetGunPosition(), m_vecTossVelocity );
			m_fThrowGrenade = FALSE;
			if( g_iSkillLevel == SKILL_HARD )
				m_flNextGrenadeCheck = gpGlobals->time + RANDOM_FLOAT( 2, 5 );// wait a random amount of time before shooting again
			else
				m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		}
			break;
		case HGRUNT_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
		}
			break;
		case HGRUNT_AE_BURST1:
		{
			if( FBitSet( pev->weapons, HGRUNT_9MMAR ) )
			{
				Shoot();

				// the first round of the three round burst plays the sound and puts a sound in the world sound list.
				if( RANDOM_LONG( 0, 1 ) )
				{
					EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "hgrunt/gr_mgun1.wav", 1, ATTN_NORM );
				}
				else
				{
					EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "hgrunt/gr_mgun2.wav", 1, ATTN_NORM );
				}
			}
			else
			{
				Shotgun();
				EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/sbarrel1.wav", 1, ATTN_NORM );
			}

			CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
		}
			break;
		case HGRUNT_AE_BURST2:
		case HGRUNT_AE_BURST3:
			Shoot();
			break;
		case HGRUNT_AE_KICK:
		{
			CBaseEntity *pHurt = Kick();

			if( pHurt )
			{
				// SOUND HERE!
				UTIL_MakeVectors( pev->angles );
				pHurt->pev->punchangle.x = 15;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;
				pHurt->TakeDamage( pev, pev, gSkillData.hgruntDmgKick, DMG_CLUB );
			}
		}
			break;
		case HGRUNT_AE_CAUGHT_ENEMY:
		{
			if( FOkToSpeak() )
			{
				SENTENCEG_PlayRndSz( ENT( pev ), "FG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
				JustSpoke();
			}
		}
			break;
		default:
			CSquadMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CFGrunt::Spawn()
{
	Precache();

	SET_MODEL( ENT( pev ), "models/hgrunt_opfor.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid		= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	pev->health		= gSkillData.hgruntHealth;
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flNextGrenadeCheck	= gpGlobals->time + 1;
	m_flNextPainTime	= gpGlobals->time;
	m_iSentence		= HGRUNT_SENT_NONE;

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_fEnemyEluded		= FALSE;
	m_fFirstEncounter	= TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector( 0, 0, 55 );

	if( pev->weapons == 0 )
	{
		// initialize to original values
		pev->weapons = HGRUNT_9MMAR | HGRUNT_HANDGRENADE;
		// pev->weapons = HGRUNT_SHOTGUN;
		// pev->weapons = HGRUNT_9MMAR | HGRUNT_GRENADELAUNCHER;
	}

	if( FBitSet( pev->weapons, HGRUNT_SHOTGUN ) )
	{
		SetBodygroup( HEAD_M203, GUN_SHOTGUN );
		m_cClipSize = 8;
	}
	else
	{
		m_cClipSize = GRUNT_CLIP_SIZE;
	}
	m_cAmmoLoaded = m_cClipSize;
/*
	if( RANDOM_LONG( 0, 99 ) < 80 )
		pev->skin = 0;	// light skin
	else
		pev->skin = 1;	// dark skin

	if( FBitSet( pev->weapons, HGRUNT_SHOTGUN ) )
	{
		SetBodygroup( HEAD_GROUP, HEAD_SHOTGUN );
	}
	else if( FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ) )
	{
		SetBodygroup( HEAD_GROUP, HEAD_M203 );
		pev->skin = 1; // alway dark skin
	}
*/
	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CFGrunt::Precache()
{
	PRECACHE_MODEL( "models/hgrunt_opfor.mdl" );

	PRECACHE_SOUND( "hgrunt/gr_mgun1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_mgun2.wav" );

	PRECACHE_SOUND( "fgrunt/death1.wav" );
	PRECACHE_SOUND( "fgrunt/death2.wav" );
	PRECACHE_SOUND( "fgrunt/death3.wav" );
	PRECACHE_SOUND( "fgrunt/death4.wav" );
	PRECACHE_SOUND( "fgrunt/death5.wav" );
	PRECACHE_SOUND( "fgrunt/death6.wav" );

	PRECACHE_SOUND( "fgrunt/gr_pain1.wav" );
	PRECACHE_SOUND( "fgrunt/gr_pain2.wav" );
	PRECACHE_SOUND( "fgrunt/gr_pain3.wav" );
	PRECACHE_SOUND( "fgrunt/gr_pain4.wav" );
	PRECACHE_SOUND( "fgrunt/gr_pain5.wav" );
	PRECACHE_SOUND( "fgrunt/gr_pain6.wav" );

	PRECACHE_SOUND( "hgrunt/gr_reload1.wav" );

	PRECACHE_SOUND( "weapons/glauncher.wav" );

	PRECACHE_SOUND( "weapons/sbarrel1.wav" );
	PRECACHE_SOUND( "zombie/claw_miss2.wav" );// because we use the basemonster SWIPE animation event

	// get voice pitch
	if( RANDOM_LONG( 0, 1 ) )
		m_voicePitch = 109 + RANDOM_LONG( 0, 7 );
	else
		m_voicePitch = 100;

	m_iBrassShell = PRECACHE_MODEL( "models/shell.mdl" );// brass shell
	m_iShotgunShell = PRECACHE_MODEL( "models/shotgunshell.mdl" );
}

//=========================================================
// PainSound
//=========================================================
void CFGrunt::PainSound( void )
{
	if( gpGlobals->time > m_flNextPainTime )
	{
#if 0
		if( RANDOM_LONG( 0, 99 ) < 5 )
		{
			// pain sentences are rare
			if( FOkToSpeak() )
			{
				SENTENCEG_PlayRndSz( ENT( pev ), "FG_PAIN", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM );
				JustSpoke();
				return;
			}
		}
#endif
		switch( RANDOM_LONG( 0, 6 ) )
		{
		case 0:	
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/gr_pain3.wav", 1, ATTN_NORM );	
			break;
		case 1:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/gr_pain4.wav", 1, ATTN_NORM );	
			break;
		case 2:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/gr_pain5.wav", 1, ATTN_NORM );	
			break;
		case 3:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/gr_pain6.wav", 1, ATTN_NORM );	
			break;
		case 4:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/gr_pain1.wav", 1, ATTN_NORM );	
			break;
		case 5:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/gr_pain2.wav", 1, ATTN_NORM );
			break;
		}
		m_flNextPainTime = gpGlobals->time + 1;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CFGrunt::DeathSound( void )
{
	switch( RANDOM_LONG( 0, 5 ) )
	{
	case 0:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/death1.wav", 1, ATTN_IDLE );
		break;
	case 1:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/death2.wav", 1, ATTN_IDLE );
		break;
	case 2:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/death3.wav", 1, ATTN_IDLE );
		break;
	case 3:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/death4.wav", 1, ATTN_IDLE );
		break;
	case 4:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/death5.wav", 1, ATTN_IDLE );
		break;
	case 5:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/death6.wav", 1, ATTN_IDLE );
		break;
	}
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//=========================================================
// Get Schedule!
//=========================================================
Schedule_t *CFGrunt::GetSchedule( void )
{

	// clear old sentence
	m_iSentence = HGRUNT_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if( pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE )
	{
		if( pev->flags & FL_ONGROUND )
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			return GetScheduleOfType( SCHED_GRUNT_REPEL_LAND );
		}
		else
		{
			// repel down a rope, 
			if( m_MonsterState == MONSTERSTATE_COMBAT )
				return GetScheduleOfType( SCHED_GRUNT_REPEL_ATTACK );
			else
				return GetScheduleOfType( SCHED_GRUNT_REPEL );
		}
	}

	// grunts place HIGH priority on running away from danger sounds.
	if( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if( pSound )
		{
			if( pSound->m_iType & bits_SOUND_DANGER )
			{
				// dangerous sound nearby!

				//!!!KELLY - currently, this is the grunt's signal that a grenade has landed nearby,
				// and the grunt should find cover from the blast
				// good place for "SHIT!" or some other colorful verbal indicator of dismay.
				// It's not safe to play a verbal order here "Scatter", etc cause 
				// this may only affect a single individual in a squad. 
				if( FOkToSpeak() )
				{
					SENTENCEG_PlayRndSz( ENT( pev ), "FG_GREN", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
					JustSpoke();
				}
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
			}
			/*
			if( !HasConditions( bits_COND_SEE_ENEMY ) && ( pSound->m_iType & ( bits_SOUND_PLAYER | bits_SOUND_COMBAT ) ) )
			{
				MakeIdealYaw( pSound->m_vecOrigin );
			}
			*/
		}
	}
	switch( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{
			// dead enemy
			if( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster::GetSchedule();
			}

			// new enemy
			if( HasConditions( bits_COND_NEW_ENEMY ) )
			{
				if( InSquad() )
				{
					MySquadLeader()->m_fEnemyEluded = FALSE;

					if( !IsLeader() )
					{
						return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
					}
					else 
					{
						//!!!KELLY - the leader of a squad of grunts has just seen the player or a 
						// monster and has made it the squad's enemy. You
						// can check pev->flags for FL_CLIENT to determine whether this is the player
						// or a monster. He's going to immediately start
						// firing, though. If you'd like, we can make an alternate "first sight" 
						// schedule where the leader plays a handsign anim
						// that gives us enough time to hear a short sentence or spoken command
						// before he starts pluggin away.
						if( FOkToSpeak() )// && RANDOM_LONG( 0, 1 ) )
						{
							if( ( m_hEnemy != 0 ) && m_hEnemy->IsPlayer() )
								// player
								SENTENCEG_PlayRndSz( ENT( pev ), "FG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
							else if( ( m_hEnemy != 0 ) &&
									( m_hEnemy->Classify() != CLASS_PLAYER_ALLY ) && 
									( m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE ) && 
									( m_hEnemy->Classify() != CLASS_MACHINE ) )
								// monster
								SENTENCEG_PlayRndSz( ENT( pev ), "FG_MONST", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );

							JustSpoke();
						}

						if( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
						{
							return GetScheduleOfType( SCHED_GRUNT_SUPPRESS );
						}
						else
						{
							return GetScheduleOfType( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
						}
					}
				}
			}
			// no ammo
			else if( HasConditions( bits_COND_NO_AMMO_LOADED ) )
			{
				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 
				return GetScheduleOfType( SCHED_GRUNT_COVER_AND_RELOAD );
			}
			// damaged just a little
			else if( HasConditions( bits_COND_LIGHT_DAMAGE ) )
			{
				// if hurt:
				// 90% chance of taking cover
				// 10% chance of flinch.
				int iPercent = RANDOM_LONG( 0, 99 );

				if( iPercent <= 90 && m_hEnemy != 0 )
				{
					// only try to take cover if we actually have an enemy!

					//!!!KELLY - this grunt was hit and is going to run to cover.
					if( FOkToSpeak() ) // && RANDOM_LONG( 0, 1 ) )
					{
						//SENTENCEG_PlayRndSz( ENT( pev ), "FG_COVER", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
						m_iSentence = HGRUNT_SENT_COVER;
						//JustSpoke();
					}
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
				else
				{
					return GetScheduleOfType( SCHED_SMALL_FLINCH );
				}
			}
			// can kick
			else if( HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
			}
			// can grenade launch
			else if( FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER) && HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
			{
				// shoot a grenade if you can
				return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
			}
			// can shoot
			else if( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				if( InSquad() )
				{
					// if the enemy has eluded the squad and a squad member has just located the enemy
					// and the enemy does not see the squad member, issue a call to the squad to waste a 
					// little time and give the player a chance to turn.
					if( MySquadLeader()->m_fEnemyEluded && !HasConditions( bits_COND_ENEMY_FACING_ME ) )
					{
						MySquadLeader()->m_fEnemyEluded = FALSE;
						return GetScheduleOfType( SCHED_GRUNT_FOUND_ENEMY );
					}
				}

				if( OccupySlot( bits_SLOTS_HGRUNT_ENGAGE ) )
				{
					// try to take an available ENGAGE slot
					return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
				}
				else if( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
				{
					// throw a grenade if can and no engage slots are available
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				else
				{
					// hide!
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
			}
			// can't see enemy
			else if( HasConditions( bits_COND_ENEMY_OCCLUDED ) )
			{
				if( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
				{
					//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
					if( FOkToSpeak() )
					{
						SENTENCEG_PlayRndSz( ENT( pev ), "FG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
						JustSpoke();
					}
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				else if( OccupySlot( bits_SLOTS_HGRUNT_ENGAGE ) )
				{
					//!!!KELLY - grunt cannot see the enemy and has just decided to 
					// charge the enemy's position. 
					if( FOkToSpeak() )// && RANDOM_LONG( 0, 1 ) )
					{
						//SENTENCEG_PlayRndSz( ENT( pev ), "FG_CHARGE", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
						m_iSentence = HGRUNT_SENT_CHARGE;
						//JustSpoke();
					}

					return GetScheduleOfType( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
				}
				else
				{
					//!!!KELLY - grunt is going to stay put for a couple seconds to see if
					// the enemy wanders back out into the open, or approaches the
					// grunt's covered position. Good place for a taunt, I guess?
					if( FOkToSpeak() && RANDOM_LONG( 0, 1 ) )
					{
						SENTENCEG_PlayRndSz( ENT( pev ), "FG_TAUNT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
						JustSpoke();
					}
					return GetScheduleOfType( SCHED_STANDOFF );
				}
			}

			if( HasConditions( bits_COND_SEE_ENEMY ) && !HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				return GetScheduleOfType( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
			}
		}
		break;
	default:
		break;
	}

	// no special cases here, call the base class
	return CSquadMonster::GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t *CFGrunt::GetScheduleOfType( int Type ) 
{
	switch( Type )
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			if( InSquad() )
			{
				if( g_iSkillLevel == SKILL_HARD && HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
				{
					if( FOkToSpeak() )
					{
						SENTENCEG_PlayRndSz( ENT( pev ), "FG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
						JustSpoke();
					}
					return slGruntTossGrenadeCover;
				}
				else
				{
					return &slGruntTakeCover[0];
				}
			}
			else
			{
				if( RANDOM_LONG( 0, 1 ) )
				{
					return &slGruntTakeCover[0];
				}
				else
				{
					return &slGruntGrenadeCover[0];
				}
			}
		}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slGruntTakeCoverFromBestSound[0];
		}
	case SCHED_GRUNT_TAKECOVER_FAILED:
		{
			if( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) && OccupySlot( bits_SLOTS_HGRUNT_ENGAGE ) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}

			return GetScheduleOfType( SCHED_FAIL );
		}
		break;
	case SCHED_GRUNT_ELOF_FAIL:
		{
			// human grunt is unable to move to a position that allows him to attack the enemy.
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
		}
		break;
	case SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE:
		{
			return &slGruntEstablishLineOfFire[0];
		}
		break;
	case SCHED_RANGE_ATTACK1:
		{
			// randomly stand or crouch
			if( RANDOM_LONG( 0, 9 ) == 0 )
				m_fStanding = RANDOM_LONG( 0, 1 );

			if( m_fStanding )
				return &slGruntRangeAttack1B[0];
			else
				return &slGruntRangeAttack1A[0];
		}
	case SCHED_RANGE_ATTACK2:
		{
			return &slGruntRangeAttack2[0];
		}
	case SCHED_COMBAT_FACE:
		{
			return &slGruntCombatFace[0];
		}
	case SCHED_GRUNT_WAIT_FACE_ENEMY:
		{
			return &slGruntWaitInCover[0];
		}
	case SCHED_GRUNT_SWEEP:
		{
			return &slGruntSweep[0];
		}
	case SCHED_GRUNT_COVER_AND_RELOAD:
		{
			return &slGruntHideReload[0];
		}
	case SCHED_GRUNT_FOUND_ENEMY:
		{
			return &slGruntFoundEnemy[0];
		}
	case SCHED_VICTORY_DANCE:
		{
			if( InSquad() )
			{
				if( !IsLeader() )
				{
					return &slGruntFail[0];
				}
			}

			return &slGruntVictoryDance[0];
		}
	case SCHED_GRUNT_SUPPRESS:
		{
			if( m_hEnemy->IsPlayer() && m_fFirstEncounter )
			{
				m_fFirstEncounter = FALSE;// after first encounter, leader won't issue handsigns anymore when he has a new enemy
				return &slGruntSignalSuppress[0];
			}
			else
			{
				return &slGruntSuppress[0];
			}
		}
	case SCHED_FAIL:
		{
			if( m_hEnemy != 0 )
			{
				// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
				return &slGruntCombatFail[0];
			}

			return &slGruntFail[0];
		}
	case SCHED_GRUNT_REPEL:
		{
			if( pev->velocity.z > -128 )
				pev->velocity.z -= 32;
			return &slGruntRepel[0];
		}
	case SCHED_GRUNT_REPEL_ATTACK:
		{
			if( pev->velocity.z > -128 )
				pev->velocity.z -= 32;
			return &slGruntRepelAttack[0];
		}
	case SCHED_GRUNT_REPEL_LAND:
		{
			return &slGruntRepelLand[0];
		}
	default:
		{
			return CSquadMonster::GetScheduleOfType( Type );
		}
	}
}

//=========================================================
// CNGruntRepel - when triggered, spawns a monster_nari_grunt
// repelling down a line.
//=========================================================
class CFGruntRepel : public CHGruntRepel
{
public:
	void Spawn(void);
	void Precache(void);
	void EXPORT RepelUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS( monster_grunt_repel_ally, CFGruntRepel )

void CFGruntRepel::Spawn( void )
{
	Precache();
	pev->solid = SOLID_NOT;

	SetUse( &CFGruntRepel::RepelUse );
}

void CFGruntRepel::Precache( void )
{
	UTIL_PrecacheOther( "monster_human_grunt_ally" );

	m_iSpriteTexture = PRECACHE_MODEL( "sprites/rope.spr" );
}

void CFGruntRepel::RepelUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	TraceResult tr;
	UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, -4096.0 ), dont_ignore_monsters, ENT( pev ), &tr );
	/*
	if( tr.pHit && Instance( tr.pHit )->pev->solid != SOLID_BSP )
		return NULL;
	*/

	CBaseEntity *pEntity = Create( "monster_human_grunt_ally", pev->origin, pev->angles );
	CBaseMonster *pGrunt = pEntity->MyMonsterPointer();
	pGrunt->pev->movetype = MOVETYPE_FLY;
	pGrunt->pev->velocity = Vector( 0, 0, RANDOM_FLOAT( -196, -128 ) );
	pGrunt->SetActivity( ACT_GLIDE );
	// UNDONE: position?
	pGrunt->m_vecLastPosition = tr.vecEndPos;

	CBeam *pBeam = CBeam::BeamCreate( "sprites/rope.spr", 10 );
	pBeam->PointEntInit( pev->origin + Vector( 0, 0, 112 ), pGrunt->entindex() );
	pBeam->SetFlags( BEAM_FSOLID );
	pBeam->SetColor( 255, 255, 255 );
	pBeam->SetThink( &CBaseEntity::SUB_Remove );
	pBeam->pev->nextthink = gpGlobals->time + -4096.0 * tr.flFraction / pGrunt->pev->velocity.z + 0.5;

	UTIL_Remove( this );
}

//=========================================================
// DEAD FGRUNT PROP
//=========================================================
class CDeadFGrunt : public CBaseMonster
{
public:
	void Spawn( void );
	int Classify( void ) { return CLASS_HUMAN_MILITARY; }

	void KeyValue( KeyValueData *pkvd );

	int m_iPose;// which sequence to display	-- temporary, don't need to save
	static const char *m_szPoses[3];
};

const char *CDeadFGrunt::m_szPoses[] = { "deadstomach", "deadside", "deadsitting" };

void CDeadFGrunt::KeyValue( KeyValueData *pkvd )
{
	if( FStrEq( pkvd->szKeyName, "pose" ) )
	{
		m_iPose = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}

LINK_ENTITY_TO_CLASS( monster_human_grunt_ally_dead, CDeadFGrunt )

//=========================================================
// ********** DeadFGrunt SPAWN **********
//=========================================================
void CDeadFGrunt::Spawn( void )
{
	PRECACHE_MODEL( "models/hgrunt_opfor.mdl" );
	SET_MODEL( ENT( pev ), "models/hgrunt_opfor.mdl" );

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	m_bloodColor		= BLOOD_COLOR_RED;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );

	if( pev->sequence == -1 )
	{
		ALERT( at_console, "Dead fgrunt with bad pose\n" );
	}

	// Corpses have less health
	pev->health = 8;

	// map old bodies onto new bodies
	switch( pev->body )
	{
	case 0:
	case 1:
		// Grunt with Gun
		pev->body = 0;
		SetBodygroup( HEAD_M203, HEAD_GRUNT );
		break;
	case 2:
	case 3:
		// Grunt no Gun
		pev->body = 0;
		SetBodygroup( HEAD_M203, HEAD_M203 );
		break;
	}

	MonsterInitDead();
}
