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
// Squadmonster  functions
//=========================================================
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "basemonster.h"
#include "util_model.h"
#include "saverestore.h"
#include "squadmonster.h"
#include "plane.h"


EASY_CVAR_EXTERN_DEBUGONLY(monsterSpawnPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(altSquadRulesRuntime)
EASY_CVAR_EXTERN_DEBUGONLY(squadmonsterPrintout)

EASY_CVAR_EXTERN_DEBUGONLY(leaderlessSquadAllowed)


extern Schedule_t slChaseEnemyFailed[];



//=========================================================
// Save/Restore
//=========================================================
TYPEDESCRIPTION	CSquadMonster::m_SaveData[] = 
{
	DEFINE_FIELD( CSquadMonster, m_hSquadLeader, FIELD_EHANDLE ),
	DEFINE_ARRAY( CSquadMonster, m_hSquadMember, FIELD_EHANDLE, MAX_SQUAD_MEMBERS - 1 ),

	// DEFINE_FIELD( CSquadMonster, m_afSquadSlots, FIELD_INTEGER ), // these need to be reset after transitions!
	DEFINE_FIELD( CSquadMonster, m_fEnemyEluded, FIELD_BOOLEAN ),
	DEFINE_FIELD( CSquadMonster, m_flLastEnemySightTime, FIELD_TIME ),

	DEFINE_FIELD( CSquadMonster, m_iMySlot, FIELD_INTEGER ),


};

IMPLEMENT_SAVERESTORE( CSquadMonster, CBaseMonster );


CSquadMonster::CSquadMonster(void){
	//default this var.
	skipSquadStartup = FALSE;
	disableLeaderChange = FALSE;
	alreadyDoneNetnameLeaderCheck = FALSE;
	m_flLastEnemySightTime = -1;
}



void CSquadMonster::SetActivity( Activity NewActivity){
	CBaseMonster::SetActivity(NewActivity);
}


//=========================================================
// OccupySlot - if any slots of the passed slots are 
// available, the monster will be assigned to one.
//=========================================================
BOOL CSquadMonster::OccupySlot( int iDesiredSlots )
{
	int i;
	int iMask;
	int iSquadSlots;

	//TEST!!! Do anything
	//return TRUE;

	if ( !InSquad() )
	{
		return TRUE;
	}

	if ( SquadEnemySplit() )
	{
		// if the squad members aren't all fighting the same enemy, slots are disabled
		// so that a squad member doesn't get stranded unable to engage his enemy because
		// all of the attack slots are taken by squad members fighting other enemies.
		m_iMySlot = bits_SLOT_SQUAD_SPLIT;
		return TRUE;
	}

	CSquadMonster *pSquadLeader = MySquadLeader();

	if ( !( iDesiredSlots ^ pSquadLeader->m_afSquadSlots ) )
	{
		// none of the desired slots are available. 
		return FALSE;
	}

	iSquadSlots = pSquadLeader->m_afSquadSlots;

	for ( i = 0; i < NUM_SLOTS; i++ )
	{
		iMask = 1<<i;
		if ( iDesiredSlots & iMask ) // am I looking for this bit?
		{
			if ( !(iSquadSlots & iMask) )	// Is it already taken?
			{
				// No, use this bit
				pSquadLeader->m_afSquadSlots |= iMask;
				m_iMySlot = iMask;
//				ALERT ( at_aiconsole, "Took slot %d - %d\n", i, m_hSquadLeader->m_afSquadSlots );
				return TRUE;
			}
		}
	}

	return FALSE;
}

//=========================================================
// VacateSlot 
//=========================================================
void CSquadMonster::VacateSlot()
{
	if ( m_iMySlot != bits_NO_SLOT && InSquad() )
	{
//		ALERT ( at_aiconsole, "Vacated Slot %d - %d\n", m_iMySlot, m_hSquadLeader->m_afSquadSlots );
		MySquadLeader()->m_afSquadSlots &= ~m_iMySlot;
		m_iMySlot = bits_NO_SLOT;
	}
}

//=========================================================
// ScheduleChange
//=========================================================
void CSquadMonster::ScheduleChange ( void )
{
	VacateSlot();
	CBaseMonster::ScheduleChange(); //MODDD - call the parent!!
}

//=========================================================
// Killed
//=========================================================
GENERATE_KILLED_IMPLEMENTATION(CSquadMonster){
	VacateSlot();

	if ( InSquad() )
	{
		MySquadLeader()->SquadRemove( this );
	}

	GENERATE_KILLED_PARENT_CALL(CBaseMonster);
}

//MODDD TODO - should what's above be done in "onDelete" instead, so that even unusual removal guarantees removing this from the squad?
//             may still work out regardless with Edicts detecting when something goes NULL.



// These functions are still awaiting conversion to CSquadMonster 


//=========================================================
//
// SquadRemove(), remove pRemove from my squad.
// If I am pRemove, promote m_pSquadNext to leader

//MODDD NOTE: THERE IS NO "m_pSquadNext", no promotion takes place.  Probably unfinished / planned behavior.
//
//=========================================================
void CSquadMonster::SquadRemove( CSquadMonster *pRemove )
{
	ASSERT( pRemove!=NULL );
	ASSERT( this->IsLeader() );
	ASSERT( pRemove->m_hSquadLeader == this );

	// If I'm the leader, get rid of my squad
	if (pRemove == MySquadLeader())
	{
		for (int i = 0; i < MAX_SQUAD_MEMBERS-1;i++)
		{
			CSquadMonster *pMember = MySquadMember(i);
			if (pMember)
			{
				pMember->m_hSquadLeader = NULL;
				m_hSquadMember[i] = NULL;
			}
		}
	}
	else
	{
		CSquadMonster *pSquadLeader = MySquadLeader();
		if (pSquadLeader)
		{
			for (int i = 0; i < MAX_SQUAD_MEMBERS-1;i++)
			{
				//MODDD NOTICE: shouldn't "this" be "pRemove"?  We want to remove "pRemove" from the squad, not necessarily whatever wants the thing removed (not necessarily itself?).
				//However, the only use of "pRemove" sends "this" as an arg, so it happens to fall in place either way.
				if (pSquadLeader->m_hSquadMember[i] == this)
				{
					pSquadLeader->m_hSquadMember[i] = NULL;
					break;
				}
			}
		}
	}

	pRemove->m_hSquadLeader = NULL;
}

//=========================================================
//
// SquadAdd(), add pAdd to my squad
//
//=========================================================
BOOL CSquadMonster::SquadAdd( CSquadMonster *pAdd )
{
	ASSERT( pAdd!=NULL );
	ASSERT( !pAdd->InSquad() );
	ASSERT( this->IsLeader() );

	for (int i = 0; i < MAX_SQUAD_MEMBERS-1; i++)
	{
		if (m_hSquadMember[i] == NULL)
		{
			m_hSquadMember[i] = pAdd;
			pAdd->m_hSquadLeader = this;
			return TRUE;
		}
	}
	easyPrintLine("WARNING: AddSquad failed! Tried to add:%s:%d Receiver:%s:%d", pAdd->getClassname(), pAdd->monsterID, this->getClassname(), this->monsterID);
	return FALSE;
	// should complain here
}


//=========================================================
// 
// SquadPasteEnemyInfo - called by squad members that have
// current info on the enemy so that it can be stored for 
// members who don't have current info.
//
//=========================================================
void CSquadMonster::SquadPasteEnemyInfo ( void )
{
	CSquadMonster *pSquadLeader = MySquadLeader( );
	if (pSquadLeader){
		pSquadLeader->setEnemyLKP(m_vecEnemyLKP, m_flEnemyLKP_zOffset);
		pSquadLeader->OnAlertedOfEnemy();
	}
}

//=========================================================
//
// SquadCopyEnemyInfo - called by squad members who don't
// have current info on the enemy. Reads from the same fields
// in the leader's data that other squad members write to,
// so the most recent data is always available here.
//
//=========================================================
void CSquadMonster::SquadCopyEnemyInfo ( void )
{
	CSquadMonster *pSquadLeader = MySquadLeader( );
	if (pSquadLeader){
		setEnemyLKP(pSquadLeader->m_vecEnemyLKP, pSquadLeader->m_flEnemyLKP_zOffset);
		OnAlertedOfEnemy();
	}
}

//=========================================================
// 
// SquadMakeEnemy - makes everyone in the squad angry at
// the same entity.
//
//=========================================================
void CSquadMonster::SquadMakeEnemy ( CBaseEntity *pEnemy )
{
	if (!InSquad())
		return;

	if ( !pEnemy )
	{
		ALERT ( at_console, "ERROR: SquadMakeEnemy() - pEnemy is NULL!\n" );
		return;
	}

	CSquadMonster *pSquadLeader = MySquadLeader( );
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
		if (pMember)
		{
			int myID = monsterID;
			int theirID = pMember->monsterID;
			// reset members who aren't activly engaged in fighting
			if (pMember->m_hEnemy != pEnemy && !pMember->HasConditions( bits_COND_SEE_ENEMY))
			{
				if ( pMember->m_hEnemy != NULL) 
				{
					// remember their current enemy
					pMember->PushEnemy( pMember->m_hEnemy, pMember->m_vecEnemyLKP );
				}
				// give them a new enemy
				pMember->m_hEnemy = pEnemy;
				
				//MODDD - involve the ent
				pMember->setEnemyLKP(pEnemy);
				
				// Let them know.
				pMember->OnAlertedOfEnemy();

				//MODDD NOTICE - need a special marker to keep the condition from getting
				// forgotten between frames.  At the start of the next frame the other monster
				// will see this condition and be able to act on it.
				
				pMember->SetNextFrameConditions(bits_COND_NEW_ENEMY);

				// No need for this hackiness anymore. (that was me another time)
				//if (pMember->m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY) {
				//	pMember->SetConditions(bits_COND_NEW_ENEMY);
				//	pMember->SetState(MONSTERSTATE_COMBAT);
				//	pMember->ChangeSchedule(GetSchedule());
				//}

			}
		}
	}
}


//=========================================================
//
// SquadCount(), return the number of members of this squad
// callable from leaders & followers
//
//=========================================================
int CSquadMonster::SquadCount( void )
{
	if (!InSquad())
		return 0;

	CSquadMonster *pSquadLeader = MySquadLeader();
	int squadCount = 0;
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		if (pSquadLeader->MySquadMember(i) != NULL)
			squadCount++;
	}

	return squadCount;
}


//=========================================================
//
// SquadRecruit(), get some monsters of my classification and
// link them as a group.  returns the group size
//
//=========================================================

//MODDD - implemented (wasn't before)
void CSquadMonster::SquadJoin ( int searchRadius, int maxMembers ){

	int squadCount;
	int iMyClass = Classify();// cache this monster's class


	// Don't recruit if I'm already in a group
	if ( InSquad() )
		return;

	if ( maxMembers < 2 )
		return;

	// Start out with no leader.
	m_hSquadLeader = NULL;
	squadCount = 1;

	CBaseEntity *pEntity = NULL;

	if ( !FStringNull( pev->netname ) )
	{
		// I have a netname, so unconditionally JOIN everyone else with that name.
		pEntity = UTIL_FindEntityByString( pEntity, "netname", STRING( pev->netname ) );
		while ( pEntity )
		{
			CSquadMonster *pRecruit = pEntity->MySquadMonsterPointer();
			
			BOOL canPrintOut = FALSE;
			if( pRecruit && pRecruit->InSquad() && !pRecruit->isForceHated(this) && pRecruit->Classify() == iMyClass && pRecruit != this && pRecruit->m_afCapability & bits_CAP_SQUAD && pRecruit->SquadCount() < maxMembers )   {
				canPrintOut = TRUE;
				EASY_CVAR_PRINTIF_PRE(squadmonsterPrintout, easyForcePrintLine("ATTEMPT JOIN, HAS NETNAME..."));
			}

			if ( pRecruit )
			{
				if ( pRecruit->InSquad() && !pRecruit->isForceHated(this) && pRecruit->Classify() == iMyClass && pRecruit != this && pRecruit->m_afCapability & bits_CAP_SQUAD && pRecruit->SquadCount() < maxMembers )
				{
					// minimum protection here against user error.in worldcraft. 
					if (!pRecruit->MySquadLeader()->SquadAdd( this )){
						if(canPrintOut){
							EASY_CVAR_PRINTIF_PRE(squadmonsterPrintout, easyForcePrintLine("FAIL 1"));
						}
						//break;
					}else{
						if(canPrintOut){
							EASY_CVAR_PRINTIF_PRE(squadmonsterPrintout, easyForcePrintLine("JOINED!! %d", SquadCount()) );
						}
						break;
					}

					//squadCount++;
				}else{
					if(canPrintOut){
						EASY_CVAR_PRINTIF_PRE(squadmonsterPrintout, easyForcePrintLine("FAIL 2"));
					}
					
				}
			}
	
			pEntity = UTIL_FindEntityByString( pEntity, "netname", STRING( pev->netname ) );
		}
	}
	else 
	{
		while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, searchRadius )) != NULL)
		{
			CSquadMonster *pRecruit = pEntity->MySquadMonsterPointer( );

			BOOL canPrintOut = FALSE;

			//m_afCapability		= bits_CAP_SQUAD
			if(pRecruit && pRecruit != this && ((iMyClass != CLASS_ALIEN_MONSTER) || FStrEq(STRING(pev->classname), STRING(pRecruit->pev->classname)) && pRecruit->m_afCapability & bits_CAP_SQUAD && pRecruit->SquadCount() < maxMembers   )    ){
				canPrintOut = TRUE;
				EASY_CVAR_PRINTIF_PRE(squadmonsterPrintout, easyForcePrintLine("ATTEMPT JOIN, NO NETNAME: (%d && %d  && %d && (%d || %d) && %d :::%s, %s : hated? %d  ) ", pRecruit->InSquad(), pRecruit->Classify() == iMyClass, pRecruit->SquadCount() < maxMembers, (iMyClass != CLASS_ALIEN_MONSTER), FStrEq(STRING(pev->classname), STRING(pRecruit->pev->classname)),  FStringNull( pRecruit->pev->netname ), STRING(pev->classname), STRING(pRecruit->pev->classname), pRecruit->isForceHated(this)) );
			}
			
			if ( pRecruit && pRecruit != this && pRecruit->IsAlive() && !pRecruit->m_pCine && pRecruit->m_afCapability & bits_CAP_SQUAD && pRecruit->SquadCount() < maxMembers )
			{
				// Can we JOIN this guy?
				if ( pRecruit->InSquad() && !pRecruit->isForceHated(this) && pRecruit->Classify() == iMyClass  &&
				   ( (iMyClass != CLASS_ALIEN_MONSTER) || FStrEq(STRING(pev->classname), STRING(pRecruit->pev->classname))) &&
				    FStringNull( pRecruit->pev->netname ) )
				{
					TraceResult tr;
					UTIL_TraceLine( pev->origin + pev->view_ofs, pRecruit->pev->origin + pev->view_ofs, ignore_monsters, pRecruit->edict(), &tr );// try to hit recruit with a traceline.
					if ( tr.flFraction == 1.0 )
					{
						if (!pRecruit->MySquadLeader()->SquadAdd( this )){
							EASY_CVAR_PRINTIF_PRE(squadmonsterPrintout, easyForcePrintLine("FAILED JOIN 1"));
							
						}else{
							EASY_CVAR_PRINTIF_PRE(squadmonsterPrintout, easyForcePrintLine("JOINED!! %d", SquadCount()) );
							break;
						}

						//squadCount++;
					}
				}
			}else{
				if(canPrintOut){
					EASY_CVAR_PRINTIF_PRE(squadmonsterPrintout, easyForcePrintLine("FAILED JOIN 2"));
				}
			}
		}
	}

	/*
	// no single member squads
	if (squadCount == 1)
	{
		m_hSquadLeader = NULL;
	}
	*/

	//why return?
	return;
}



//MODDD - NOTE.  Copied over to hgrunt and hassault for more specifics over there.
int CSquadMonster::SquadRecruit( int searchRadius, int maxMembers )
{
	int squadCount;
	int iMyClass = Classify();// cache this monster's class

	// Don't recruit if I'm already in a group
	if ( InSquad() )
		return 0;

	if ( maxMembers < 2 )
		return 0;

	// I am my own leader
	m_hSquadLeader = this;
	squadCount = 1;

	CBaseEntity *pEntity = NULL;

	if ( !FStringNull( pev->netname ) )
	{
		// I have a netname, so unconditionally recruit everyone else with that name.
		pEntity = UTIL_FindEntityByString( pEntity, "netname", STRING( pev->netname ) );
		while ( pEntity )
		{
			CSquadMonster *pRecruit = pEntity->MySquadMonsterPointer();

			if ( pRecruit )
			{
				if ( !pRecruit->InSquad() && !pRecruit->isForceHated(this) && pRecruit->Classify() == iMyClass && pRecruit != this )
				{
					// minimum protection here against user error.in worldcraft. 
					if (!SquadAdd( pRecruit ))
						break;
					squadCount++;
				}
			}
	
			pEntity = UTIL_FindEntityByString( pEntity, "netname", STRING( pev->netname ) );
		}
	}
	else 
	{
		while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, searchRadius )) != NULL)
		{
			CSquadMonster *pRecruit = pEntity->MySquadMonsterPointer( );

			if ( pRecruit && pRecruit != this && pRecruit->IsAlive() && !pRecruit->m_pCine )
			{
				// Can we recruit this guy?
				if ( !pRecruit->InSquad() && !pRecruit->isForceHated(this) && pRecruit->Classify() == iMyClass &&
				   ( (iMyClass != CLASS_ALIEN_MONSTER) || FStrEq(STRING(pev->classname), STRING(pRecruit->pev->classname))) &&
				    FStringNull( pRecruit->pev->netname ) )
				{
					TraceResult tr;
					UTIL_TraceLine( pev->origin + pev->view_ofs, pRecruit->pev->origin + pev->view_ofs, ignore_monsters, pRecruit->edict(), &tr );// try to hit recruit with a traceline.
					if ( tr.flFraction == 1.0 )
					{
						if (!SquadAdd( pRecruit ))
							break;

						squadCount++;
					}
				}
			}
		}
	}
	// no single member squads
	if (squadCount == 1)
	{
		m_hSquadLeader = NULL;
	}
	return squadCount;
}




//MODDD - moved here from CHGrunt.
// Don't really see why any squadmonster wouldn't work the same way here, and this didn't involve anyting specific
// to CHGrunts only.  May as well be for all squadmonsters.
void CSquadMonster::PrescheduleThink ( void )
{
	BOOL seenEnemy = FALSE;
	if ( HasConditions ( bits_COND_SEE_ENEMY ) )
	{
		seenEnemy = TRUE;
		m_flLastEnemySightTime = gpGlobals->time;
	}

	//MODDD - bits_COND_SEE_ENEMY check here.

	if ( InSquad() && m_hEnemy != NULL )
	{
		//MODDD - each squad member giving updating its squad leader's 'm_flLastEnemySightTime' makes sense,
		// but why do the '5 seconds passed since seeing enemy' check for the squad leader?
		// That's potentially 4 squad members (including the leader itself) doing checks for whether to
		// set 'm_fEnemeyEluded' to FALSE.
		// And why not use this for other logic too?  Why only for the squad leader when this monster
		// saw the enemy too?
		// (see PrescheduleThink in as-is for what this is describing, this method has been shifted around since)
		
		if ( seenEnemy && !IsLeader() )
		{
			// update the squad's last enemy sighting time.
			MySquadLeader()->m_flLastEnemySightTime = this->m_flLastEnemySightTime;
		}

		// Leader only now
		if ( this->IsLeader() && gpGlobals->time - m_flLastEnemySightTime > 5 )
		{
			// been a while since we've seen the enemy
			m_fEnemyEluded = TRUE;
		}
	}
}


//=========================================================
// CheckEnemy
//=========================================================
int CSquadMonster::CheckEnemy ( CBaseEntity *pEnemy )
{
	int iUpdatedLKP;

	//MODDD - m_hEnemy -> pEnemy replacement.
	// ALSO, moved above the "SquadLeader->enemy == NULL" check below, so that 'return 0' doesn't skip this.
	iUpdatedLKP = CBaseMonster::CheckEnemy ( pEnemy );

	if (MySquadLeader()->m_hEnemy == NULL) {
		//MODDD - WAIT!  Don't only 'return 0'.  Still check the enemy through CBaseMonster in case its deadflag changed
		// between runs.
		// Actually doing this by moving the BaseMonster CheckEnemy call above this section.
		return 0;  //don't proceed
	}

	// communicate with squad members about the enemy IF this individual has the same enemy as the squad leader.
	//MODDD - use the parameter you got, dangit!
	// Even though it will always end up being m_hEnemy anyway.
	//if ( InSquad() && (CBaseEntity *)m_hEnemy == MySquadLeader()->m_hEnemy )
	if ( InSquad() && pEnemy->edict() == MySquadLeader()->m_hEnemy.Get() )
	{
		if ( iUpdatedLKP )
		{
			// have new enemy information, so paste to the squad.
			SquadPasteEnemyInfo();
		}
		else
		{
			// enemy unseen, copy from the squad knowledge.
			SquadCopyEnemyInfo();
		}
	}
	return iUpdatedLKP;
}

void CSquadMonster::ChangeLeader(CSquadMonster* oldLeader, CSquadMonster* newLeader){
	for (int i = 0; i < MAX_SQUAD_MEMBERS-1; i++){
		if (oldLeader->m_hSquadMember[i] != NULL){
			CSquadMonster *pMember = oldLeader->MySquadMember(i);

			newLeader->m_hSquadMember[i] = NULL;

			if (pMember && pMember != newLeader){
				//the new leader cannot be included in "m_hSquadMember".
				pMember->m_hSquadLeader = newLeader;
				newLeader->m_hSquadMember[i] = pMember;
			}
			oldLeader->m_hSquadMember[i] = NULL;
		}
	}//for thru squaddies
	newLeader->m_hSquadLeader = newLeader;

	//just in case... (old leader is just a member of the squad now)
	oldLeader->m_hSquadLeader = NULL;
	newLeader->SquadAdd(oldLeader);
}


BOOL CSquadMonster::checkLeaderlessSquadByNetname(void){
	BOOL foundLeaderYet = FALSE;
	int iMyClass = Classify();// cache this monster's class
	CBaseEntity* pEntity = NULL;
	
	pEntity = UTIL_FindEntityByString( pEntity, "netname", STRING( pev->netname ) );
	while ( pEntity )
	{
		CSquadMonster *pRecruit = pEntity->MySquadMonsterPointer();
		if(pRecruit){
			//check: could this be a valid recruit?
			if(!pRecruit->InSquad() && !pRecruit->isForceHated(this) && pRecruit->Classify() == iMyClass){

				//if so, finally:
				if(pRecruit->pev->spawnflags & SF_SQUADMONSTER_LEADER){
					//found a leader
					foundLeaderYet = TRUE;
				}
				//this one should not re-check its peers, no need.
				pRecruit->alreadyDoneNetnameLeaderCheck = TRUE;
			}
		}//END OF if(pEntity)
		
		pEntity = UTIL_FindEntityByString( pEntity, "netname", STRING( pev->netname ) );
	}//END OF while(pEntity)

	return !foundLeaderYet;

}



//=========================================================
// StartMonster
//=========================================================
void CSquadMonster::StartMonster( void )
{
	//EASY_CVAR_PRINTIF_PRE(squadmonsterPrintout, easyForcePrintLine("START MONSTER?!"));
	//pev->spawnflags &= ~ SF_SQUADMONSTER_LEADER;
	CBaseMonster::StartMonster();

	if ( ( m_afCapability & bits_CAP_SQUAD ) && !InSquad() )
	{
		if ( !FStringNull( pev->netname ) )
		{
			BOOL possibleExceptionToRule = FALSE;
			if(EASY_CVAR_GET_DEBUGONLY(leaderlessSquadAllowed) == 1 && !this->alreadyDoneNetnameLeaderCheck){
				alreadyDoneNetnameLeaderCheck = TRUE;
				//or will have soon...
				//Check all of netname.
				possibleExceptionToRule = checkLeaderlessSquadByNetname();
			}
			//MODDD - do a check.   See if any member has the "SF_SQUADMONSTER_LEADER" flag set.
			//EASY_CVAR_GET_DEBUGONLY(leaderlessSquadAllowed) == 0
			// if I have a groupname, I can only recruit if I'm flagged as leader
			//NOTE:::joining is now possible, but the old behavior is probably okay.
			if ( !possibleExceptionToRule && !( pev->spawnflags & SF_SQUADMONSTER_LEADER ) )
			{
				return;
			}
		}

		if(skipSquadStartup){
			//it appears this is completely unnecessaray, but oh well.  (being assigned to a squad right at creation already disables this whole startup block)
			EASY_CVAR_PRINTIF_PRE(squadmonsterPrintout, easyForcePrintLine("STARTUP SKIPPED!"));
			return;
		}

		// try to form squads now.
		int iSquadSize = SquadRecruit( 1024, 4 );

		if ( iSquadSize )
		{
		  ALERT ( at_aiconsole, "Squad of %d %s formed\n", iSquadSize, STRING( pev->classname ) );
		}

		//easyForcePrintLine("ARE YOU bros SERIOUS RIGHT NOW %.2f %d %d", EASY_CVAR_GET_DEBUGONLY(altSquadRulesRuntime), FStringNull(pev->netname), iSquadSize);
		//If "altSquadRule" is something (1 or 2), and is either 2 OR, if 1, also spawned in real-time (not coming from the map), try joining a squad.
		if( EASY_CVAR_GET_DEBUGONLY(altSquadRulesRuntime) > 0 && (EASY_CVAR_GET_DEBUGONLY(altSquadRulesRuntime) == 2 || FStringNull( pev->netname ) ) ){
			if(iSquadSize == 1){
				
				if(EASY_CVAR_GET_DEBUGONLY(monsterSpawnPrintout) == 1){
				easyForcePrintLine("ITS MONDAY my acquaintance!  Get myself a squad... %s:%d", this->getClassname(), this->monsterID);
				}
				//found no other unassigned members to start a squad with.  Try joining instead?
				SquadJoin(1024, 4);
			}
		}

		//if ( IsLeader() && FClassnameIs ( pev, "monster_human_grunt" ) ){

		//	//MODDD - removed, as grunts no longer have other possible heads (just the gasmask)
		//	//SetBodygroup( 1, 1 ); // UNDONE: truly ugly hack
		//	
		//	pev->skin = 0;
		//	//unsure if the skin thing is necessary anymore.
		//	
		//}
		
		//MODDD - TODO - put some custom event here like "onPostStartSquadCheck" for the hgrunt to latch onto?
		// If we care enough.
		
	}//END OF has bits_CAP_SQUAD and not already in a squad check
}//END OF StartMonster









//=========================================================
// NoFriendlyFire - checks for possibility of friendly fire
//
// Builds a large box in front of the grunt and checks to see 
// if any squad members are in that box. 
//=========================================================
BOOL CSquadMonster::NoFriendlyFire( void )
{
	if ( !InSquad() )
	{
		return TRUE;
	}

	CPlane	backPlane;
	CPlane  leftPlane;
	CPlane	rightPlane;

	Vector	vecLeftSide;
	Vector	vecRightSide;
	Vector	v_left;

	//!!!BUGBUG - to fix this, the planes must be aligned to where the monster will be firing its gun, not the direction it is facing!!!

	if ( m_hEnemy != NULL )
	{
		UTIL_MakeVectors ( UTIL_VecToAngles( m_hEnemy->Center() - pev->origin ) );
	}
	else
	{
		// if there's no enemy, pretend there's a friendly in the way, so the grunt won't shoot.
		return FALSE;
	}

	//UTIL_MakeVectors ( pev->angles );
	
	vecLeftSide = pev->origin - ( gpGlobals->v_right * ( pev->size.x * 1.5 ) );
	vecRightSide = pev->origin + ( gpGlobals->v_right * ( pev->size.x * 1.5 ) );
	v_left = gpGlobals->v_right * -1;

	leftPlane.InitializePlane ( gpGlobals->v_right, vecLeftSide );
	rightPlane.InitializePlane ( v_left, vecRightSide );
	backPlane.InitializePlane ( gpGlobals->v_forward, pev->origin );

/*
	ALERT ( at_console, "LeftPlane: %f %f %f : %f\n", leftPlane.m_vecNormal.x, leftPlane.m_vecNormal.y, leftPlane.m_vecNormal.z, leftPlane.m_flDist );
	ALERT ( at_console, "RightPlane: %f %f %f : %f\n", rightPlane.m_vecNormal.x, rightPlane.m_vecNormal.y, rightPlane.m_vecNormal.z, rightPlane.m_flDist );
	ALERT ( at_console, "BackPlane: %f %f %f : %f\n", backPlane.m_vecNormal.x, backPlane.m_vecNormal.y, backPlane.m_vecNormal.z, backPlane.m_flDist );
*/

	CSquadMonster *pSquadLeader = MySquadLeader();
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
		if (pMember && pMember != this)
		{

			if ( backPlane.PointInFront  ( pMember->pev->origin ) &&
				 leftPlane.PointInFront  ( pMember->pev->origin ) && 
				 rightPlane.PointInFront ( pMember->pev->origin) )
			{
				// this guy is in the check volume! Don't shoot!
				return FALSE;
			}
		}
	}

	return TRUE;
}







//=========================================================
// GetIdealState - surveys the Conditions information available
// and finds the best new state for a monster.
//=========================================================
MONSTERSTATE CSquadMonster::GetIdealState ( void )
{
	int iConditions;

	iConditions = IScheduleFlags();
	
	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch ( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
	case MONSTERSTATE_ALERT:
		if ( HasConditions ( bits_COND_NEW_ENEMY ) && InSquad() )
		{
			SquadMakeEnemy ( m_hEnemy );
		}
		break;
	}


	return CBaseMonster::GetIdealState();
}

//=========================================================
// FValidateCover - determines whether or not the chosen
// cover location is a good one to move to. (currently based
// on proximity to others in the squad)
//=========================================================
BOOL CSquadMonster::FValidateCover ( const Vector &vecCoverLocation )
{
	if ( !InSquad() )
	{
		return TRUE;
	}

	if (SquadMemberInRange( vecCoverLocation, 128 ))
	{
		// another squad member is too close to this piece of cover.
		return FALSE;
	}

	return TRUE;
}

//=========================================================
// SquadEnemySplit- returns TRUE if not all squad members
// are fighting the same enemy. 
//=========================================================
BOOL CSquadMonster::SquadEnemySplit ( void )
{
	if (!InSquad())
		return FALSE;

	CSquadMonster	*pSquadLeader = MySquadLeader();
	CBaseEntity		*pEnemy	= pSquadLeader->m_hEnemy;

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
		if (pMember != NULL && pMember->m_hEnemy != NULL && pMember->m_hEnemy != pEnemy)
		{
			return TRUE;
		}
	}
	return FALSE;
}

//=========================================================
// FValidateCover - determines whether or not the chosen
// cover location is a good one to move to. (currently based
// on proximity to others in the squad)
//=========================================================
BOOL CSquadMonster::SquadMemberInRange ( const Vector &vecLocation, float flDist )
{
	if (!InSquad())
		return FALSE;

	CSquadMonster *pSquadLeader = MySquadLeader();

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pSquadMember = pSquadLeader->MySquadMember(i);
		if (pSquadMember && (vecLocation - pSquadMember->pev->origin ).Length2D() <= flDist)
			return TRUE;
	}
	return FALSE;
}


Schedule_t *CSquadMonster::GetScheduleOfType( int iType )
{
	switch ( iType )
	{

	case SCHED_CHASE_ENEMY_FAILED:
		{
			return &slChaseEnemyFailed[ 0 ];
		}
	
	default:
		return CBaseMonster::GetScheduleOfType( iType );
	}
}

//MODDD - overridable event for subclasses, for anything else to happen alongside being given
// new info about the enemy (setting m_hEnemy or the LKP).
// Best to call this parent method from those (sets m_flLastEnemySightTime).
void CSquadMonster::OnAlertedOfEnemy(void){
	m_flLastEnemySightTime = gpGlobals->time;
}


GENERATE_TRACEATTACK_IMPLEMENTATION_ROUTETOPARENT(CSquadMonster, CBaseMonster)
GENERATE_TAKEDAMAGE_IMPLEMENTATION_ROUTETOPARENT(CSquadMonster, CBaseMonster)


//MODDD - new.
void CSquadMonster::Activate( void ){
	CBaseMonster::Activate();
}
void CSquadMonster::Spawn(void){
	CBaseMonster::Spawn();
}

//MODDD - too inspecific.
const char* CSquadMonster::getGermanModel(void){
	return NULL;
}
const char* CSquadMonster::getNormalModel(void){
	return NULL;
}
//MODDD
void CSquadMonster::setModel(void){
	CSquadMonster::setModel(NULL);
}
void CSquadMonster::setModel(const char* m){
	//nothing broad applies to the specific talker NPCs.
	CBaseMonster::setModel(m);
}
