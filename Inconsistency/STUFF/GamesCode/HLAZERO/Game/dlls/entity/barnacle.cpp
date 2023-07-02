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
// barnacle - stationary ceiling mounted 'fishing' monster
//=========================================================

#include "extdll.h"
#include "barnacle.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "gib.h"
#include "schedule.h"
#include "defaultai.h"

extern BOOL globalPSEUDO_germanModel_hgibFound;
extern float cheat_barnacleEatsEverything;
EASY_CVAR_EXTERN_DEBUGONLY(drawBarnacleDebug)
EASY_CVAR_EXTERN_DEBUGONLY(barnacleCanGib)
EASY_CVAR_EXTERN_DEBUGONLY(barnaclePrintout)
EASY_CVAR_EXTERN_DEBUGONLY(barnacleTongueRetractDelay)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST(sv_germancensorship)
EASY_CVAR_EXTERN_DEBUGONLY(allowGermanModels)
EASY_CVAR_EXTERN_DEBUGONLY(germanRobotGibs)
EASY_CVAR_EXTERN_DEBUGONLY(germanRobotGibsDecal)




#define BARNACLE_BODY_HEIGHT	44 // how 'tall' the barnacle's model is.
#define BARNACLE_PULL_SPEED		8
#define BARNACLE_KILL_VICTIM_DELAY	5 // how many seconds after pulling prey in to gib them. 

#define BARNACLE_LUNGE_SPEED		88
#define BARNACLE_CHECK_SPACING	8

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define BARNACLE_AE_PUKEGIB	2



extern DLL_GLOBAL int g_iSkillLevel;




#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_barnacle, CBarnacle );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( barnacle, CBarnacle );
	
	//no extras.
#endif


int CBarnacle::s_iStandardGibID = -1;
int CBarnacle::s_fStandardGibDecal = TRUE;




//MODDD
CBarnacle::CBarnacle(){

	loweredPreviously = FALSE;
	retractedPreviously = FALSE;

	//imply not dead.
	barnacleDeathActivitySet = FALSE;
	retractDelay = -1;

	smallerTest = FALSE;
}

	
int CBarnacle::IRelationship( CBaseEntity *pTarget ){

	if(pTarget->getIsBarnacleVictimException() == TRUE){
		//automatic dislike to allow pulling up.
		return R_DL;
	}

	//First a check to see if the target wants to force a different relationship.
	int forcedRelationshipTest = pTarget->forcedRelationshipWith(this);
	
	if(forcedRelationshipTest != R_DEFAULT){
		//The other monster (pTarget) is forcing this monster to have an attitude towards it other than what the table may suggest.
		return forcedRelationshipTest;
	}else{}  //return below as usual.

	//Just imagine myself as ALIEN_MONSTER. I hate the things it hates.
	//And things with the barnacleVictim exception set.

	//Or in other words, what would an ALIEN_MONSTER hate? If it would hate this, so would I.
	//return IRelationshipOfClass(ALIEN_MONSTER, pTarget);
	int standardRelation = iEnemy[ CLASS_ALIEN_MONSTER ][ pTarget->Classify() ];

	return standardRelation;


}//END OF IRelationship

//Other monsters need to know not to be hostile towards me.
int CBarnacle::forcedRelationshipWith(CBaseEntity *pWith){
	//return R_DEFAULT;
	return R_NO;
}




/*
// Some other killed-related methods if needed (see basemonster.cpp)
setPhysicalHitboxForDeath
DeathAnimationStart
DeathAnimationEnd
onDeathAnimationEnd
*/

void CBarnacle::StartReanimationPost(int preReviveSequence){
	
	//LEAVE THIS OUT. I don't do what most monsters do.
	/*
	SetThink( &CBaseMonster::MonsterInitThink );
	//SetThink ( &CBaseMonster::CallMonsterThink );
	pev->nextthink = gpGlobals->time + 0.1;
	
	SetUse ( &CBaseMonster::MonsterUse );
	*/

	m_IdealMonsterState	= MONSTERSTATE_ALERT;// Assume monster will be alert, having come back from the dead and all.
	m_MonsterState = MONSTERSTATE_ALERT; //!!!

	m_IdealActivity = ACT_IDLE;
	m_Activity = ACT_IDLE; //!!! No sequence changing, force the activity to this now.

	pev->sequence = -1; //force reset.
	SetSequenceByIndex(preReviveSequence, -1, FALSE);

	ChangeSchedule(slWaitForSequence );
}//END OF StartReanimationPost



// The Barnacle typically wants to spawn blood red gibs.
// If this is germancensorship, can replace the blood red gibs with robot gibs provided
// some other CVars allow it. Otherwise don't spawn any.
// If germancensorship is off, depends on violence_hgihs being off.
int CBarnacle::BarnacleGetStandardGibSpawnID(){

	if(CVAR_GET_FLOAT("violence_agibs") == 0){
		//no alien gibs? Already cancels all this.
		return GIB_DUMMY_ID;
	}

	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST(sv_germancensorship) != 1){
		// german censorship is off? this will depend on this CVar.
		if(CVAR_GET_FLOAT("violence_hgibs") != 0){
			return GIB_HUMAN_ID;
		}
	}else{
		// german censorship is on? This will depend on whether robot models are allowed and use gears instead if so.
		if(EASY_CVAR_GET_DEBUGONLY(allowGermanModels)==1 && EASY_CVAR_GET_DEBUGONLY(germanRobotGibs)==1 && globalPSEUDO_germanModel_hgibFound==TRUE){
			return GIB_GERMAN_ID;
		}
	}
	
	return GIB_DUMMY_ID; //not allowed.
}





void CBarnacle::SetActivity ( Activity NewActivity ){

	//just forcing that...
	pev->framerate = 1;

	CBaseMonster::SetActivity(NewActivity);
}

float CBarnacle::getTentacleSuddenAnimFrameRate(void){
	switch(g_iSkillLevel){
	case SKILL_EASY:
		//???
		return 1;
	break;
	case SKILL_MEDIUM:
		return 1.96;
	break;
	case SKILL_HARD:
		return 2.2;
	break;
	default:
		//easy?
		return 1;
	break;
	}
}




TYPEDESCRIPTION	CBarnacle::m_SaveData[] = 
{
	DEFINE_FIELD( CBarnacle, m_flAltitude, FIELD_FLOAT ),
	DEFINE_FIELD( CBarnacle, m_flKillVictimTime, FIELD_TIME ),
	DEFINE_FIELD( CBarnacle, m_cGibs, FIELD_INTEGER ),// barnacle loads up on gibs each time it kills something.
	DEFINE_FIELD( CBarnacle, m_fTongueExtended, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarnacle, m_fLiftingPrey, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarnacle, m_flTongueAdj, FIELD_FLOAT ),

	//MODDD - new
	DEFINE_FIELD( CBarnacle, barnacleDeathActivitySet, FIELD_BOOLEAN),

};

IMPLEMENT_SAVERESTORE( CBarnacle, CBaseMonster );


//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CBarnacle::Classify ( void )
{
	//NOTE:::  uh, there is "CLASS_BARNACLE".  Weird that this isn't used instead?
	//Although nothing (monsters) pays attention to the barnacle ingame because its flags are 0, maybe.
	//Strangely some Xen creatures use CLASS_BARNACLE to signify being ignored though. No idea.

	//Unfortunately it may not be a good idea to use CLASS_BARNACLE without specially checking for it.
	//Its value is 99, and it isn't worth editing the entire relationship array just to add a class for one monster only.
	//Other script for making the barnacle ignore-able by AI is much better.
	
	return	CLASS_ALIEN_MONSTER;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CBarnacle::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case BARNACLE_AE_PUKEGIB:
		CGib::SpawnRandomGibs( pev, 1, CBarnacle::s_iStandardGibID, CBarnacle::s_fStandardGibDecal );	
		break;
	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CBarnacle::Spawn()
{
	Precache( );

	barnacleDeathActivitySet = FALSE;

	SET_MODEL(ENT(pev), "models/barnacle.mdl");
	UTIL_SetSize( pev, Vector(-16, -16, -32), Vector(16, 16, 0) );


	pev->classname = MAKE_STRING("monster_barnacle");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_NONE;
	pev->takedamage		= DAMAGE_AIM;

	//MODDD - ?
	
	//pev->flags |= FL_BARNACLE;

	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= EF_INVLIGHT; // take light from the ceiling 


	//MODDD - now comes from skilldata (text file)
	pev->health			= gSkillData.barnacleHealth; //25;

	//MODDD - since MonsterInit isn't called.
	pev->max_health		= pev->health;


	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flKillVictimTime	= 0;
	m_cGibs				= 0;
	m_fLiftingPrey		= FALSE;
	
	//MODDD - isn't setting this at spawn a good idea?
	m_flAltitude = 0;

	m_flTongueAdj = -100;

	InitBoneControllers();

	SetActivity ( ACT_IDLE );

	SetThink ( &CBarnacle::BarnacleThink );
	pev->nextthink = gpGlobals->time + 0.5;

	UTIL_SetOrigin ( pev, pev->origin );

	//MODDD IMPORTANT - the barnacle does not run "MonsterInit". This is worth pointing out guys!
	// So this monster avoids receiving the FL_MONSTER flag that most things would receive just fine. 
	// Little odd as clearly this is still a "creature" or "monster" in that sense.
	// There are even other places that work to exclude barnacles from checks otherwise.
	pev->flags |= FL_MONSTER;

	
	if(monsterID == -1){
		//MODDD - must do manually since init is skipped.
		monsterID = monsterIDLatest;
		monsterIDLatest++;
	}
	
}//END OF Spawn



GENERATE_TRACEATTACK_IMPLEMENTATION(CBarnacle)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CBarnacle)
{
	if ( bitsDamageType & DMG_CLUB )
	{
		flDamage = pev->health;
	}

	return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}

//=========================================================
//=========================================================
void CBarnacle::BarnacleThink ( void )
{
	CBaseEntity *pTouchEnt;
	CBaseMonster *pVictim;
	float flLength;


	//test flag, no more.
	//this->pev->renderfx = 28;


	pev->nextthink = gpGlobals->time + 0.1;

	if ( m_hEnemy != NULL )
	{
// barnacle has prey.


		//UTIL_printLineVector("BOOOOOOO", m_hEnemy->pev->origin);



		if ( !m_hEnemy->IsAlive() )
		{
			// someone (maybe even the barnacle) killed the prey. Reset barnacle.
			m_fLiftingPrey = FALSE;// indicate that we're not lifting prey.
			m_hEnemy = NULL;
			return;
		}

		if ( m_fLiftingPrey )
		{
			if ( m_hEnemy != NULL && m_hEnemy->pev->deadflag != DEAD_NO )
			{
				// crap, someone killed the prey on the way up.
				m_hEnemy = NULL;
				m_fLiftingPrey = FALSE;
				return;
			}

	// still pulling prey.
			Vector vecNewEnemyOrigin = m_hEnemy->pev->origin;
			vecNewEnemyOrigin.x = pev->origin.x;
			vecNewEnemyOrigin.y = pev->origin.y;


			//MODDD - commented out.  Using a more flexible version... (also, default is "0" of an offset: dead center for most)
			// guess as to where their neck is
			//vecNewEnemyOrigin.x -= 6 * cos(m_hEnemy->pev->angles.y * M_PI/180.0);	
			//vecNewEnemyOrigin.y -= 6 * sin(m_hEnemy->pev->angles.y * M_PI/180.0);

			CBaseMonster* tempMon;
			if( (tempMon = m_hEnemy->GetMonsterPointer()) != NULL){
				vecNewEnemyOrigin.x += tempMon->getBarnacleForwardOffset() * cos(m_hEnemy->pev->angles.y * M_PI/180.0);	
				vecNewEnemyOrigin.y += tempMon->getBarnacleForwardOffset() * sin(m_hEnemy->pev->angles.y * M_PI/180.0);
			}

			m_flAltitude -= BARNACLE_PULL_SPEED;
			vecNewEnemyOrigin.z += BARNACLE_PULL_SPEED;

			//MODDD - attempt to get an offset to where the "head" is.  The vort needs this to look right at the top.
			float tryOffset = 0;
			pVictim = m_hEnemy->MyMonsterPointer();
			if(pVictim != NULL){
				tryOffset = pVictim->getBarnaclePulledTopOffset();
			}

			//easyForcePrintLine("barnacle%d WHATS HAPPENIN %.2f", monsterID, pev->origin.z - ( vecNewEnemyOrigin.z + m_hEnemy->pev->view_ofs.z - 8 + tryOffset )    );

			//if ( fabs( pev->origin.z - ( vecNewEnemyOrigin.z + m_hEnemy->pev->view_ofs.z - 8 + tryOffset ) ) < BARNACLE_BODY_HEIGHT )

			//MODDD - why fabs? if this were negative it would go into the barnacle more, that is okay.
			if( ( pev->origin.z - ( vecNewEnemyOrigin.z + m_hEnemy->pev->view_ofs.z - 8 + tryOffset ) ) < BARNACLE_BODY_HEIGHT )
			{
		// prey has just been lifted into position ( if the victim origin + eye height + 8 is higher than the bottom of the barnacle, it is assumed that the head is within barnacle's body )
				m_fLiftingPrey = FALSE;

				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_bite3.wav", 1, ATTN_NORM );	

				pVictim = m_hEnemy->MyMonsterPointer();

				m_flKillVictimTime = gpGlobals->time + 10;// now that the victim is in place, the killing bite will be administered in 10 seconds.

				//paint a bunch of blood around the perimeter?
				float daBlood = pVictim->BloodColor();
				for(float i = 0; i < 2*M_PI; i+= M_PI/6){
					UTIL_SpawnBlood(
						Vector(
							pev->origin.x + cos(i)*(RANDOM_FLOAT(15.8, 17.8) ),
							pev->origin.y + sin(i)*(RANDOM_FLOAT(15.8, 17.8) ),
							pev->origin.z + pev->mins.z + (RANDOM_FLOAT(8, 23) )
						),
						daBlood,
						RANDOM_LONG(13, 18)
					);
				}//END OF loop



				if ( pVictim )
				{
					pVictim->BarnacleVictimBitten( pev );
					SetActivity ( ACT_EAT );
				}
			}

			UTIL_SetOrigin ( m_hEnemy->pev, vecNewEnemyOrigin );
		}
		else
		{
	// prey is lifted fully into feeding position and is dangling there.

			pVictim = m_hEnemy->MyMonsterPointer();

			if ( m_flKillVictimTime != -1 && gpGlobals->time > m_flKillVictimTime )
			{
				// kill!
				if ( pVictim )
				{
					//MODDD - added "DMG_BARNACLEBITE" for the new bitmask, so that the islave doesn't try to brush it off as friendly fire (ignores that).
					pVictim->TakeDamage ( pev, pev, pVictim->pev->health, DMG_SLASH | DMG_ALWAYSGIB, DMG_BARNACLEBITE );

					//MODDD TODO - should this be cumulative? Right now it just forces the gib count to 3 no matter how many kills it has had.
					m_cGibs = 3;
				}

				return;
			}

			// bite prey every once in a while
			if ( pVictim && ( RANDOM_LONG(0,49) == 0 ) )
			{
				switch ( RANDOM_LONG(0,2) )
				{
				case 0:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_chew1.wav", 1, ATTN_NORM );	break;
				case 1:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_chew2.wav", 1, ATTN_NORM );	break;
				case 2:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_chew3.wav", 1, ATTN_NORM );	break;
				}

				pVictim->BarnacleVictimBitten( pev );
			}

		}
	}
	else
	{
// barnacle has no prey right now, so just idle and check to see if anything is touching the tongue.

		// If idle and no nearby client, don't think so often
		if ( FNullEnt( FIND_CLIENT_IN_PVS( edict() ) ) )
			pev->nextthink = gpGlobals->time + RANDOM_FLOAT(1,1.5);	// Stagger a bit to keep barnacles from thinking on the same frame

		if ( m_fSequenceFinished )
		{// this is done so barnacle will fidget.
			SetActivity ( ACT_IDLE );
			m_flTongueAdj = -100;
		}

		if ( m_cGibs && RANDOM_LONG(0,99) == 1 )
		{
			// cough up a gib.
			CGib::SpawnRandomGibs( pev, 1, CBarnacle::s_iStandardGibID, CBarnacle::s_fStandardGibDecal );
			m_cGibs--;

			switch ( RANDOM_LONG(0,2) )
			{
			case 0:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_chew1.wav", 1, ATTN_NORM );	break;
			case 1:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_chew2.wav", 1, ATTN_NORM );	break;
			case 2:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_chew3.wav", 1, ATTN_NORM );	break;
			}
		}





		

		//hm, time for this...
		//MODDD - implement.
		//g_iSkillLevel

		//SKILL_EASY;
		//SKILL_MEDIUM;
		//SKILL_HARD;



		
		float flLengthMinimal = 5;

		float lungeSpeedMultiplier = 1.0f;

		
		BOOL lickTouchPossible = FALSE;
		BOOL lickTouchVertical = FALSE;
		BOOL triggered = FALSE ;



		
		if(g_iSkillLevel == SKILL_MEDIUM){
			lungeSpeedMultiplier = 0.9f;
		}else if(g_iSkillLevel == SKILL_HARD){
			lungeSpeedMultiplier = 1.25f;
		}

		




		if(g_iSkillLevel > SKILL_EASY){

			smallerTest = FALSE;
			pTouchEnt = TongueTouchEnt( &flLength, &flLengthMinimal );

			triggered = ((pTouchEnt!=NULL) || ( retractDelay != -1 && EASY_CVAR_GET_DEBUGONLY(barnacleTongueRetractDelay) > 0 && retractDelay > gpGlobals->time ) );

			if(pTouchEnt != NULL){
				if(EASY_CVAR_GET_DEBUGONLY(barnacleTongueRetractDelay) > 0){
					retractedPreviously = FALSE;
					retractDelay = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(barnacleTongueRetractDelay);
				}
			}



			//if(!triggered && retractedPreviously == FALSE){
			if(retractedPreviously == FALSE && retractDelay != -1 && retractDelay <= gpGlobals->time){
				retractedPreviously = TRUE;
				loweredPreviously = FALSE;
				retractDelay = -1;

				pev->framerate = -getTentacleSuddenAnimFrameRate();
				this->SetSequenceByName("attack1");

			}




			//EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("BARN1: %d", (pTouchEnt != NULL)));
		
			if(pTouchEnt != NULL){

				
				//something is close enough to trigger lowering.  But are we close enough go grab something yet?
				smallerTest = TRUE;
				CBaseEntity* testTouch = TongueTouchEnt( &flLength, &flLengthMinimal );

				if(testTouch != NULL){
					lickTouchPossible = TRUE;
					pTouchEnt = testTouch;
				}else{
					pTouchEnt = NULL;   //can't grab, not close enough.
				
					if(loweredPreviously == FALSE){
						loweredPreviously = TRUE;
						retractedPreviously = FALSE;

						pev->framerate = getTentacleSuddenAnimFrameRate();
						this->SetSequenceByName("attack1");

					}
				}
				//EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("BARN2: %d", (pTouchEnt != NULL)));
			}else{
				//nothing triggering lowering?
				loweredPreviously = FALSE;
			}
		}else{
			//if easy...
			triggered = TRUE;
			//you're always triggered.  PATRIARCHYYYYY
			
			smallerTest = TRUE;
			pTouchEnt = TongueTouchEnt( &flLength );
			//EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("EDDDDD %d", (pTouchEnt==NULL) ));
			lickTouchPossible = TRUE;   //just pass these along...
			lickTouchVertical = TRUE;

		}

		//Moved from either bracket above's (that is, 1 for NORMAL / HARD, 1 for EASY)
		if(pTouchEnt != NULL){
			//float startPosition = (pev->origin.z - pTouchEnt->EyePosition().z);
			//lickTouchVertical = (-m_flAltitude+startPosition <=  (pTouchEnt->pev->origin.z - pTouchEnt->pev->mins.z) );
			float startPosition = (pev->origin.z + pev->mins.z);  
			lickTouchVertical = ( (pTouchEnt->pev->origin.z + pTouchEnt->pev->maxs.z) < startPosition) && (-m_flAltitude+startPosition <=  (pTouchEnt->EyePosition().z) );  //or ent's origin + maxs.z?  up to you.
		}
		if(pTouchEnt != NULL){
			EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("BARNTOUCHPOSS: %d BARNTOUCHVER %d TONGUE-EXT: %d TAR: %.2f TAREY: %.2f MYZ: %.2f MYZA: %.2f MYZAA: %.2f", lickTouchPossible, lickTouchVertical, m_fTongueExtended, pTouchEnt->pev->origin.z, pTouchEnt->EyePosition().z, pev->origin.z, (pev->origin.z + pev->mins.z), (pev->origin.z+pev->mins.z-m_flAltitude) ));
		}

		//if ( pTouchEnt != NULL && m_fTongueExtended && lickTouchPossible )
		if( pTouchEnt != NULL && m_fTongueExtended && lickTouchPossible && lickTouchVertical) {
			
			// tongue is fully extended, and is touching someone.
			if ( pTouchEnt->FBecomeProne() )
			{
				UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_alert2.wav", 1, ATTN_NORM );	

				SetSequenceByName ( "attack1" );
				m_flTongueAdj = -20;

				m_hEnemy = pTouchEnt;

				pTouchEnt->pev->movetype = MOVETYPE_FLY;
				pTouchEnt->pev->velocity = g_vecZero;
				pTouchEnt->pev->basevelocity = g_vecZero;
				pTouchEnt->pev->origin.x = pev->origin.x;
				pTouchEnt->pev->origin.y = pev->origin.y;

				m_fLiftingPrey = TRUE;// indicate that we should be lifting prey.
				m_flKillVictimTime = -1;// set this to a bogus time while the victim is lifted.

				m_flAltitude = (pev->origin.z - pTouchEnt->EyePosition().z);

				//MODDD - ready to re-do these things in case this touched thing is released.
				retractedPreviously = FALSE;
				loweredPreviously = FALSE;

			}
		}
		else
		{

			float lengthChoice;
			float alterAltSpeed;

			if(!triggered ){
				lengthChoice = flLengthMinimal;
				alterAltSpeed = BARNACLE_LUNGE_SPEED * lungeSpeedMultiplier;
			}else{
				
				lengthChoice = flLength;
				alterAltSpeed = BARNACLE_LUNGE_SPEED * lungeSpeedMultiplier * 0.6;
			}

			if(g_iSkillLevel == SKILL_EASY){
				alterAltSpeed = BARNACLE_PULL_SPEED;
			}




			//lickTouchPossible: if the tongue were extended fully, could I touch it?
			//lickTouchVertical: am I able to lick it, given how far down I am right now?


			EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("IM BARNACLE#%d, AND MY ALT IS %.2f DESIRED %.2f MAX: %.2f okay? %d", monsterID, m_flAltitude, lengthChoice, flLength, m_fTongueExtended));

			// calculate a new length for the tongue to be clear of anything else that moves under it. 
			if ( m_flAltitude < lengthChoice )
			{
				// if tongue is higher than is should be, lower it kind of slowly.
				m_flAltitude += alterAltSpeed;//BARNACLE_LUNGE_SPEED * lungeSpeedMultiplier;
				m_fTongueExtended = FALSE;

				if(m_flAltitude > lengthChoice){
					m_flAltitude = lengthChoice;
					m_fTongueExtended = TRUE;
				}
			}else if( m_flAltitude > lengthChoice )
			{
				// if tongue is above...
				m_flAltitude -= alterAltSpeed;
				m_fTongueExtended = TRUE;
				if(m_flAltitude < lengthChoice){
					m_flAltitude = lengthChoice;
					m_fTongueExtended = TRUE;
				}
			}else{
				//equal?  then this is ok!
				m_fTongueExtended = TRUE;
			}



			/*
			else
			{
				m_flAltitude = flLengthMinimal;
				m_fTongueExtended = TRUE;
			}
			*/


			//EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("AM IIIIII %d", m_fTongueExtended));


		}
		
		
	}// END OF m_hEnemy != NULL check

	// ALERT( at_console, "tounge %f\n", m_flAltitude + m_flTongueAdj );
	SetBoneController( 0, -(m_flAltitude + m_flTongueAdj) );
	StudioFrameAdvance_SIMPLE( 0.1 );

}//END OF BarnacleThink


GENERATE_GIBMONSTER_IMPLEMENTATION(CBarnacle){


	GENERATE_GIBMONSTER_PARENT_CALL(CBaseMonster);
}//END OF GibMonster


GENERATE_GIBMONSTERGIB_IMPLEMENTATION(CBarnacle){
	//replaces parent logic:


	if(CVAR_GET_FLOAT("violence_agibs") != 0)
	{
		BOOL canSpawnBlend = TRUE;

		if(CBarnacle::s_iStandardGibID == GIB_DUMMY_ID){
			// didn't work. Disallow the blend.
			canSpawnBlend = FALSE;
		}

		if(canSpawnBlend){
			// a blend of blood red (or robot) and alien gibs.
			CGib::SpawnRandomGibs( pev, m_cGibs + 2, CBarnacle::s_iStandardGibID, CBarnacle::s_fStandardGibDecal && fGibSpawnsDecal );
			CGib::SpawnRandomGibs( pev, 4, GIB_ALIEN_GREEN_ID, fGibSpawnsDecal );
		}else{
			// all alien gibs.
			CGib::SpawnRandomGibs( pev, 6, GIB_ALIEN_GREEN_ID, fGibSpawnsDecal );
		}

		return TRUE;
	}

	return FALSE;
}//END OF GibMonsterGib


//=========================================================
// Killed.
//=========================================================
GENERATE_KILLED_IMPLEMENTATION(CBarnacle)
{
	CBaseMonster *pVictim;

	//MODDD - barnacle corpse is now immortal (like in retail) ONLY if this CVar is off.
	if(EASY_CVAR_GET_DEBUGONLY(barnacleCanGib) == 0){
		pev->solid = SOLID_NOT;
		pev->takedamage = DAMAGE_NO;
	}
	
	//MODDD - victim release can only happen on "killed" called the first time.
	if (!barnacleDeathActivitySet && m_hEnemy != NULL )
	{
		pVictim = m_hEnemy->MyMonsterPointer();

		if ( pVictim )
		{
			pVictim->BarnacleVictimReleased();
		}

		//released? ok, good. forget this enemy. Yes, this must be said.
		m_hEnemy = NULL;


		if(pev->solid != SOLID_NOT){
			//consider warping the newly released victim a little below the grab point so they don't get stuck in this dead barnacle's model:
			
			float tolerantBottom = pev->origin.z + pev->mins.z*1.1f;
			if(pVictim->pev->origin.z + pVictim->pev->maxs.z > tolerantBottom){
			
				pVictim->pev->origin.z = ( tolerantBottom + -(pVictim->pev->maxs.z + 3)  );
			}
		}

		//TEST IT!!!!
		//EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("BARNAZ: %.2f YOUR BOUND: %.2f YOUR NEW: %.2f", pev->origin.z, pVictim->pev->origin.z, pVictim->pev->maxs.z ));

		/*
		Vector vTarget = Vector(pVictim->pev->origin.x, pVictim->pev->origin.y, ( (pev->origin.z + pev->mins.z*1.3f) + -(pVictim->pev->maxs.z)  ) ); //- -(pVictim->pev->mins.z *2  ) );
		Vector vTowards = vTarget - pVictim->pev->origin;
		UTIL_MoveToOrigin(ENT(pVictim->pev), vTowards, (vTowards.Length()), MOVE_STRAFE);
		*/

		//MODDD - doesn't that make sense?  pVictim is "null" when it is released (no victim now)?  Regardless, not doing it as I feel the devs may
		//have a good reason for not.
		//pVictim = NULL;
	}

//	CGib::SpawnRandomGibs( pev, 4, 1 );

	//MODDD - any references to CallGibMonster replaced with GibMonster. No need for that separation.

	if(EASY_CVAR_GET_DEBUGONLY(barnacleCanGib) == 1){
	//if ( HasMemory( bits_MEMORY_KILLED ) ){
		
		if ( ShouldGibMonster( iGib ) ){
			
			GENERATE_GIBMONSTER_CALL;
			return;
		}else if(barnacleDeathActivitySet == TRUE){
			return;  //also return, since this means the death anim has already been triggered.
		}
	}else if(EASY_CVAR_GET_DEBUGONLY(barnacleCanGib) == 2){
		//harder to gib, but still may be.

		if(pev->deadflag == DEAD_NO){
			pev->health = 60;
		}else{
			if ( ShouldGibMonster( iGib ) ){
				GENERATE_GIBMONSTER_CALL;
			}
			return;
		}
	}

	switch ( RANDOM_LONG ( 0, 1 ) )
	{
	case 0:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_die1.wav", 1, ATTN_NORM );	break;
	case 1:	UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "barnacle/bcl_die3.wav", 1, ATTN_NORM );	break;
	}

	//MODDD: surely, this can't hurt?
	pev->deadflag = DEAD_DYING;
	
	barnacleDeathActivitySet = TRUE;

	SetActivity ( ACT_DIESIMPLE );
	SetBoneController( 0, 0 );

	StudioFrameAdvance_SIMPLE( 0.1 );

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink ( &CBarnacle::WaitTillDead );
}

//=========================================================
//=========================================================
void CBarnacle::WaitTillDead ( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	float flInterval = StudioFrameAdvance_SIMPLE( 0.1 );
	DispatchAnimEvents ( flInterval );

	if ( m_fSequenceFinished )
	{
		// death anim finished. 
		StopAnimation();
		SetThink ( NULL );

		
		//MODDD - is that okay?
		//    this->DeathAnimationEnd();
		//kindof overkill, really just need this.
		pev->deadflag = DEAD_DEAD;

	}
}


extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBarnacle::Precache()
{
	//set the gibID at precache time.
	if(CBarnacle::s_iStandardGibID == -1){
		CBarnacle::s_iStandardGibID = CBarnacle::BarnacleGetStandardGibSpawnID();
		CBarnacle::s_fStandardGibDecal = TRUE; //by default.

		if(CBarnacle::s_iStandardGibID == GIB_GERMAN_ID){
			//extra check.
			if(EASY_CVAR_GET_DEBUGONLY(germanRobotGibsDecal)==0){
				//this disallows robot gib decals made on hitting the ground.
				CBarnacle::s_fStandardGibDecal = FALSE;
			}
		}

	}

	PRECACHE_MODEL("models/barnacle.mdl");

	global_useSentenceSave = TRUE;
	PRECACHE_SOUND("barnacle/bcl_alert2.wav");//happy, lifting food up
	PRECACHE_SOUND("barnacle/bcl_bite3.wav");//just got food to mouth
	PRECACHE_SOUND("barnacle/bcl_chew1.wav");
	PRECACHE_SOUND("barnacle/bcl_chew2.wav");
	PRECACHE_SOUND("barnacle/bcl_chew3.wav");
	PRECACHE_SOUND("barnacle/bcl_die1.wav" );
	PRECACHE_SOUND("barnacle/bcl_die3.wav" );
	global_useSentenceSave = FALSE;
}	



//#define BARNACLE_CHECK_SPACING	8    //MOVED!
CBaseEntity *CBarnacle::TongueTouchEnt ( float *pflLength )
{
	return TongueTouchEnt(pflLength, 0);
}

//=========================================================
// TongueTouchEnt - does a trace along the barnacle's tongue
// to see if any entity is touching it. Also stores the length
// of the trace in the int pointer provided.
//=========================================================
#define BARNACLE_CHECK_SPACING	8
CBaseEntity *CBarnacle::TongueTouchEnt ( float *pflLength, float *pflLengthMinimal )
{
	TraceResult	tr;
	float	length;

	// trace once to hit architecture and see if the tongue needs to change position.
	UTIL_TraceLine ( pev->origin, pev->origin - Vector ( 0 , 0 , 2048 ), ignore_monsters, ENT(pev), &tr );
	length = fabs( pev->origin.z - tr.vecEndPos.z );
	if ( pflLength )
	{
		*pflLength = length;

	}

	if(pflLengthMinimal){
		*pflLengthMinimal = 50;

		if(pflLength && *pflLengthMinimal > *pflLength){
			*pflLengthMinimal = *pflLength;
		}

	}


	//if(m_IdealMonsterState == MONSTERSTATE_PRONE){
	//	//can not attempt to check for monsters when we're getting eaten (by another barnacle???)
	//	return NULL;
	//}

	Vector delta = Vector( BARNACLE_CHECK_SPACING, BARNACLE_CHECK_SPACING, 0 );
	
	Vector mins;
	Vector maxs;
	
	if(smallerTest){
		mins = pev->origin - delta;
		maxs = pev->origin + delta;
		
	}else{
		mins = pev->origin - 4.5f*delta;
		maxs = pev->origin + 4.5f*delta;
	}

	maxs.z = pev->origin.z;
	mins.z -= length;


	if(EASY_CVAR_GET_DEBUGONLY(drawBarnacleDebug) == 1){
		if(smallerTest){
			UTIL_drawBoxFrame(mins, maxs, 3, 255, 255, 0);
		}else{
			UTIL_drawBoxFrame(mins, maxs, 3, 255, 0, 0);
		}
	}
	
	CBaseEntity *pList[10];

	float CVarMem = cheat_barnacleEatsEverything;

	int entityFlags = 0;
	if(CVarMem !=  1){
		entityFlags = FL_CLIENT|FL_MONSTER;
	}

	//UNWISE.  Just go ahead and give it these flags, or else nothing will work.
	entityFlags = FL_CLIENT|FL_MONSTER;



	//MODDD - uses the "entityFlags" that can be intercepted by having the CVar on.
	int count = 0;

	count = UTIL_EntitiesInBox( pList, 10, mins, maxs, (FL_CLIENT|FL_MONSTER) );
	

	if ( count )
	{
		for ( int i = 0; i < count; i++ )
		{
			//EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("WHATS IN THE BOX %s", STRING(pList[i]->pev->classname) ));
			// only clients and monsters
			//MODDD - also allowing "monster_alien_slave" to be eaten.  See the ||.
			//if ( pList[i] != this && IRelationship( pList[i] ) > R_NO && pList[ i ]->pev->deadflag == DEAD_NO )	// this ent is one of our enemies. Barnacle tries to eat it.
			//FClassnameIs(pList[i]->pev, "monster_alien_slave")    obsolete way of checking.
			
			BOOL passedRelationTest = TRUE;
			const char* debugClassname = STRING(pList[i]->pev->classname);

			if( !strcmp(debugClassname, "monster_apache") ){
				int x = 66;
			}

			if(CVarMem != 1){

				//Vector temp = (pList[i]->pev->absmax - pList[i]->pev->absmin);
				Vector temp = pList[i]->pev->maxs - pList[i]->pev->mins;

				float monstaSiz = temp.x * temp.y * temp.z;
				//size of apache: 262144.00

				
				passedRelationTest = (
					// As an ALIEN_MONSTER, I have to dislike them.
					((IRelationship( pList[i] ) > R_NO) ) &&
					//( (pList[i]->pev->size.x * pList[i]->pev->size.y * pList[i]->pev->size.z) < 27000) &&
					//(monstaSiz < 200000) &&   //300000?
					!pList[i]->GetMonsterPointer()->isSizeGiant()
				);
				
				// bound size??
				EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("lets not perhaps %.2f", temp.x * temp.y * temp.z));


				// This 2nd check is to avoid trying to eat ridiculously large entities.
				// Little known fact: barnacles can eat Apaches and Ospreys in the base game, but this would never be seen because
				// they can't be spawned in the same place.
				
				//EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("WHAT IS THE VOLUME? %.2f", pList[i]->pev->size.x * pList[i]->pev->size.y * pList[i]->pev->size.z));
				//EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("WHAT IS THE VOLUME? %.2f", temp.x * temp.y * temp.z));

			}
			//EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("I: %d :::?: %d", i, passedRelationTest));
			
			//MODDD - see a bit above too, this can now be circumvented by that CVar above.
			//if ( pList[i] != this && (IRelationship( pList[i] )  > R_NO ) && pList[ i ]->pev->deadflag == DEAD_NO )	// this ent is one of our enemies. Barnacle tries to eat it.
			
			
			EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("?????? %s %d   %d", FClassname(pList[i]), passedRelationTest, IRelationship( pList[i] )));
			if ( pList[i] != this && ( passedRelationTest ) && pList[ i ]->pev->deadflag == DEAD_NO )	// this ent is one of our enemies. Barnacle tries to eat it.
			
			{
				//EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("EYYYY: %.2f %.2f", (this->pev->origin.z + this->pev->mins.z), (pList[i]->pev->origin.z + pList[i]->pev->maxs.z) ));
				

				//MODDD - also, only try to eat things who's tops are below my bottom.
				//if(this->pev->origin.z + this->pev->size.z/2 > pList[i]->pev->origin.z + pList[i]->pev->size.z/2){
				EASY_CVAR_PRINTIF_PRE(barnaclePrintout, easyPrintLine("WHY NOOOOT %.2f %.2f %.2f %.2f", this->pev->origin.z, this->pev->maxs.z, pList[i]->pev->origin.z, pList[i]->pev->mins.z )  );
				if(this->pev->origin.z + this->pev->maxs.z > pList[i]->pev->origin.z + pList[i]->pev->mins.z){
					return pList[i];
				}
			}//END OF big if-then/
		}//END OF for(int i = 0...)
	}//END OF if(count)

	return NULL;
}
