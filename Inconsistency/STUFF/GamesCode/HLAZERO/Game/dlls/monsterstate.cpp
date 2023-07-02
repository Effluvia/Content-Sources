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
// monsterstate.cpp - base class monster functions for 
// controlling core AI.
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "basemonster.h"
#include "util_model.h"
#include "saverestore.h"
#include "soundent.h"


EASY_CVAR_EXTERN_DEBUGONLY(crazyMonsterPrintouts)


//=========================================================
// SetState
//=========================================================
void CBaseMonster::SetState ( MONSTERSTATE State )
{
/*
	if ( State != m_MonsterState )
	{
		ALERT ( at_aiconsole, "State Changed to %d\n", State );
	}
*/

	if (FClassnameIs(pev, "monster_gargantua")) {
		int eckz = 666;
	}


	//MODDD - force the state to stay "PRONE" if caught by a barnacle!
	// Any other state can be picked after this monster is freed.  Or, rather, 'if'.
	// UNUSED, nothing set barnacleLocked anymore.  Why not check for PRONE anyway.
	// NEVERMIND.  Not really possible to want a state queued, no AI running looking for
	// enemies to change states while barnacle'd.  Assuming any change in activity is from
	// being freed (apply now) is fine.
	// Oh.  Although, the queued-state was meant to be saved on getting barnacle'd, like remember
	// being in the combat state from before being barnacle'd on getting freed.
	// Ehhh, no need, re-aquired soon enough anyway.  (checking for the target 'State' being
	// MONSTERSTATE_PRONE, restoring on existing state being MONSTERSTATE_PRONE, would be the proper way anyway)
	/*
	//if(barnacleLocked == TRUE){
	if(m_MonsterState == MONSTERSTATE_PRONE){
		//easyForcePrintLine("LOCKED: %d", State);

		switch( State )
		{
		case MONSTERSTATE_PRONE:
			m_MonsterState = State;
			m_IdealMonsterState = State;
		break;
		default:
			queuedMonsterState = State;
		break;
		}

		return;
	}
	*/
	
	switch( State )
	{
	
	// Drop enemy pointers when going to idle
	case MONSTERSTATE_IDLE:

		if ( m_hEnemy != NULL )
		{
			m_hEnemy = NULL;// not allowed to have an enemy anymore.
			ALERT ( at_aiconsole, "Stripped\n" );
		}
		break;
	}

	m_MonsterState = State;
	m_IdealMonsterState = State;
}

//=========================================================
// RunAI
//=========================================================
void CBaseMonster::RunAI ( void )
{
	//MODDD - NOTE.  Confused as to how the enemy gets dropped on finding it to be dead?
	// See schedule.cpp, on bits_COND_ENEMY_DEAD, the current m_hEnemy gets NULL'd.



	//these conditions are reset each frame for safety now that schedules rememeber conditions after being set.
	//ClearConditions(bits_COND_NEW_ENEMY);
	

	//MODDD TODO - this might still be flimsy? and why don't bits_COND_LIGHT_DAMAGE and HEAVY_DAMAGE (or LIGHT if that's all that's used) get remembered? takedamage calls
	//occur earlier than this perhaps and get overwritten here and so never seen by scheduling logic below?
	
	
	//ClearConditionsExcept_ThisFrame(bits_COND_TASK_FAILED | bits_COND_SCHEDULE_DONE | bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE | bits_COND_SEE_ENEMY | bits_COND_ENEMY_OCCLUDED);
	//ClearAllConditionsExcept_ThisFrame();


	//MODDD - new. This mostly copy of the conditions will be maintained throughout this frame.
	//clearAllConditions_Frame();
	//MODDD!!!!!!!!!!!!!!!!!!!!! 
	// ALSO - do not clear the DAMAGE bits!  It seems these are set independently of think cycles (takedamage stuff are events in the engine), so they never
	// reach the think method in time to be understood in the same frame they happen.  The next frame, they can be read and reacted to.
	// So, instead turn them off after this method finishes.  Kind of like as-is did.    Huh, imagine that.
	

	// ... &= ~(1111 & ~0100)
	// ... &= ~(1111 & 1011)
	// ... &= ~(1011)
	// ... &= 0100
	// wait, can't we just do this then?
	//m_afConditionsFrame &= ~(0xFFFFFFFF & ~(bits_COND_TASK_FAILED | bits_COND_SCHEDULE_DONE | bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE));
	// keep ONLY these bits, all others not mentioned get reset.
	ClearAllConditionsExcept_ThisFrame(bits_COND_TASK_FAILED | bits_COND_SCHEDULE_DONE | bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	ClearAllConditionsMod_ThisFrame();
	
	// apply any set manually last frame, in case they didn't make it.
	SetConditions(m_afConditionsNextFrame);
	SetConditionsMod(m_afConditionsModNextFrame);
	ClearAllConditions_NextFrame();
	ClearAllConditionsMod_NextFrame();



	// to test model's eye height
	//UTIL_ParticleEffect ( pev->origin + pev->view_ofs, g_vecZero, 255, 10 );

	// IDLE sound permitted in ALERT state is because monsters were silent in ALERT state. Only play IDLE sound in IDLE state
	// once we have sounds for that state.
	//MODDD - extra condition. Must also have a deadflag of NO.
	if ( pev->deadflag == DEAD_NO &&   ( m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT ) && RANDOM_LONG(0,99) == 0 && !(pev->flags & SF_MONSTER_GAG) )
	{
		IdleSound();
	}


	if ( m_MonsterState != MONSTERSTATE_NONE	&& 
		 m_MonsterState != MONSTERSTATE_PRONE   && 
		 m_MonsterState != MONSTERSTATE_DEAD )// don't bother with this crap if monster is prone. 
	{

		// collect some sensory Condition information.
		// don't let monsters outside of the player's PVS act up, or most of the interesting
		// things will happen before the player gets there!
		// UPDATE: We now let COMBAT state monsters think and act fully outside of player PVS. This allows the player to leave 
		// an area where monsters are fighting, and the fight will continue.
		
		if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts)){
			easyForcePrintLine("%s: Can I look and listen?! (%d || %d) %.2f", this->getClassname(), !FNullEnt( FIND_CLIENT_IN_PVS( edict() ) ),  ( m_MonsterState == MONSTERSTATE_COMBAT ), m_flDistLook );
		}

		if(monsterID == 9){
			int x = 45;
		}
		//MODDD - some monsters may need to check for enemies and ignore the PVS check.
		//        The archer is unable to detect a client and look for enemies if the player is past the waterlevel, strangely.
		//        This is not good for monsters meant to rise to the top and do attacks from water to land. The player should not
		//        have to have been in the water to initate this.  It's the point of being able to see through water (another addition)
		//if ( !FNullEnt( FIND_CLIENT_IN_PVS( edict() ) ) || ( m_MonsterState == MONSTERSTATE_COMBAT ) )
		if ( (noncombat_Look_ignores_PVS_check() || !FNullEnt( FIND_CLIENT_IN_PVS( edict() ) )) || ( m_MonsterState == MONSTERSTATE_COMBAT ) )
		{
			Look( m_flDistLook );
			Listen();// check for audible sounds. 

			// now filter conditions.
			ClearConditions( IgnoreConditions() );
			// ???  probably not.
			//ClearConditionsMod(IgnoreConditionsMod());

			if(monsterID == 9){
				int x = 45;
			}
			GetEnemy();
		}


		/*
		BOOL enemyIsNull = (m_hEnemy == NULL);
		BOOL enemyPrivIsNull1 = TRUE;
		BOOL enemyPrivIsNull2 = TRUE;
		
		if (!enemyIsNull) {
			enemyPrivIsNull1 = (m_hEnemy.GetEntity() == NULL);
			enemyPrivIsNull2 = (m_hEnemy.Get() == NULL);


			CBaseEntity* peepee = m_hEnemy;
			const char* crashme = peepee->getClassname();
		}
		*/

		// If our enemy has lost its private data, it has been deleted in an unusual way.  Drop it!
		// NEW.  Do this check right before even calling MonsterThink (the virtual version).
		// This way monster-specific MonsterThink's don't break just from being a little late to this check.
		/*
		if (m_hEnemy != NULL && m_hEnemy.GetEntity() == NULL) {
			m_hEnemy = NULL;
		}
		*/


		// do these calculations if monster has an enemy.
		if ( m_hEnemy != NULL )
		{
			if(monsterID == 9){
				int x = 45;
			}
			CheckEnemy( m_hEnemy );
		}

		CheckAmmo();
	}
	

	FCheckAITrigger();
	
	PrescheduleThink();

	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
		easyForcePrintLine("SHUUUT1 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}
	
	//     : )
	MaintainSchedule();

	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
		easyForcePrintLine("SHUUUT2 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}

	// if the monster didn't use these conditions during the above call to MaintainSchedule() or CheckAITrigger()
	// we throw them out cause we don't want them sitting around through the lifespan of a schedule
	// that doesn't use them.

	ClearConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	
	

	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
		easyForcePrintLine("SHUUUT3 %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}
	
	if(EASY_CVAR_GET_DEBUGONLY(crazyMonsterPrintouts) == 1){
		easyForcePrintLine("IM SO flamin\' amazing %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1));
	}
}


//=========================================================
// GetIdealState - surveys the Conditions information available
// and finds the best new state for a monster.
//=========================================================
// MODDD - now that schedule.cpp's MaintainSchedule uses the returned result of this call (go figure),
// no need to set 'm_IdealMonsterState' to whatever is about to be returned.
MONSTERSTATE CBaseMonster::GetIdealState ( void )
{
	int iConditions;
	iConditions = IScheduleFlags();


	if (FClassnameIs(pev, "monster_gargantua")) {
		int xxx = 666;
	}
	
	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch ( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
	/*
	IDLE goes to ALERT upon hearing a sound
	-IDLE goes to ALERT upon being injured
	IDLE goes to ALERT upon seeing food
	-IDLE goes to COMBAT upon sighting an enemy
	IDLE goes to HUNT upon smelling food
	*/
	{
		if ( iConditions & bits_COND_NEW_ENEMY )			
		{
			// new enemy! This means an idle monster has seen someone it dislikes, or 
			// that a monster in combat has found a more suitable target to attack
			return MONSTERSTATE_COMBAT;
		}
		else if ( iConditions & bits_COND_LIGHT_DAMAGE )
		{
			//MODDD - BUG NOTE.   On never seeing the enemy that dealt the damage, this leaves m_vecEnemyLKP at the
			// default 0,0,0, so the monster wants to face the origin of the map in any flinch method that assumes
			// the ideal_yaw had a point.   OOOooooops..?
			// IDEA: Only involve this if m_vecEnemyLKP was ever set before, new var:
			if(m_fEnemyLKP_EverSet){
				MakeIdealYaw ( m_vecEnemyLKP );
			}
			return MONSTERSTATE_ALERT;
		}
		else if ( iConditions & bits_COND_HEAVY_DAMAGE )
		{
			if(m_fEnemyLKP_EverSet){
				MakeIdealYaw ( m_vecEnemyLKP );
			}
			return MONSTERSTATE_ALERT;
		}
		else if ( iConditions & bits_COND_HEAR_SOUND )
		{
			CSound *pSound;
			pSound = PBestSound();
			ASSERT( pSound != NULL );
			if ( pSound )
			{
				MakeIdealYaw ( pSound->m_vecOrigin );
				if (pSound->m_iType & (bits_SOUND_COMBAT | bits_SOUND_DANGER)) {
					return MONSTERSTATE_ALERT;
				}
			}
		}
		else if ( iConditions & (bits_COND_SMELL | bits_COND_SMELL_FOOD) )
		{
			return MONSTERSTATE_ALERT;
		}

		break;
	}
	case MONSTERSTATE_ALERT:
	/*
	ALERT goes to IDLE upon becoming bored
	-ALERT goes to COMBAT upon sighting an enemy
	ALERT goes to HUNT upon hearing a noise
	*/
	{
		/*
		if(FClassnameIs(pev, "monster_scientist")){
			easyForcePrintLine("hey ya. %d %d", iConditions & (bits_COND_NEW_ENEMY), iConditions & (bits_COND_SEE_ENEMY) );
		}
		*/

		if ( iConditions & (bits_COND_NEW_ENEMY|bits_COND_SEE_ENEMY) )			
		{
			// see an enemy we MUST attack
			return MONSTERSTATE_COMBAT;
		}
		else if ( iConditions & bits_COND_HEAR_SOUND )
		{
			CSound *pSound = PBestSound();
			ASSERT( pSound != NULL );
			if (pSound) {
				MakeIdealYaw(pSound->m_vecOrigin);
			}
			return MONSTERSTATE_ALERT;
		}
		break;
	}
	case MONSTERSTATE_COMBAT:
	/*
	COMBAT goes to HUNT upon losing sight of enemy
	COMBAT goes to ALERT upon death of enemy
	*/
	{
		if ( m_hEnemy == NULL )
		{
			// pev->effects = EF_BRIGHTFIELD;
			ALERT ( at_aiconsole, "***Combat state with no enemy!\n" );
			return MONSTERSTATE_ALERT;
		}
	break;
	}
	
	//MODDD - whoopsie, completely unused.  Out you go.
	/*
	case MONSTERSTATE_HUNT:
		//HUNT goes to ALERT upon seeing food
		//HUNT goes to ALERT upon being injured
		//HUNT goes to IDLE if goal touched
		//HUNT goes to COMBAT upon seeing enemy
		{
			break;
		}
	*/
	case MONSTERSTATE_SCRIPT:
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//MODDD - interesting point perhaps?
		if ( iConditions & (bits_COND_TASK_FAILED|bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE) )
		{
			ExitScriptedSequence();	// This will set the ideal state
			//MODDD - may as well end here.
			return m_IdealMonsterState;
		}
	break;

	case MONSTERSTATE_DEAD:
		//  well.  That isn't very exciting.
		return MONSTERSTATE_DEAD;
	break;
	}//END OF switch(m_MonsterState)

	// Made it here?  Return the current ideal one I guess.
	return m_IdealMonsterState;
}

