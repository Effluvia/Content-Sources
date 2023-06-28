
#include "pickupwalker.h"




// NOTICE!  CChumToadPickupWalker is now unused.  It is skipped, weapon_chumtoad is replaced by
// monster_chumtoad if placed in-world.  monster_chumtoad is also respawnable in multiplayer,
// picked up or killed.




Task_t	tlPickupWalker[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_PICKUPWALKER_IDLE_WAIT_RANDOM,			0		},
	{ TASK_SET_SCHEDULE, (float)SCHED_RANDOMWANDER_UNINTERRUPTABLE }

	//{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	//{ TASK_WAIT_PVS,				0				},
};

Schedule_t	slPickupWalkerIdleWait[] =
{
	{
		tlPickupWalker,
		ARRAYSIZE ( tlPickupWalker ),
		//MODDD - new
		bits_COND_SEE_ENEMY			|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,
		0,
		"slPickupW_IdleWait"
	},
};





DEFINE_CUSTOM_SCHEDULES( CPickupWalker )
{
	slPickupWalkerIdleWait,

};

IMPLEMENT_CUSTOM_SCHEDULES( CPickupWalker, CBaseMonster );



CPickupWalker::CPickupWalker(void){

}


BOOL CPickupWalker::usesSoundSentenceSave(void){
	// Don't use it, uses sounds always precached for the player
	return FALSE;
}




extern int global_useSentenceSave;
void CPickupWalker::Precache( void )
{
	/*
	int i = 0;
	//TEMPLATE
	PRECACHE_MODEL("models/???.mdl");
	
	global_useSentenceSave = TRUE;
	for ( i = 0; i < ARRAYSIZE( pIdleSounds ); i++ )
		PRECACHE_SOUND((char *)pIdleSounds[i]);
	
	global_useSentenceSave = FALSE;
	*/
}



void CPickupWalker::IdleSound( void )
{
	//int pitch = 95 + RANDOM_LONG(0,9);
	// Play a random idle sound
	//UTIL_PlaySound( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}



void CPickupWalker::Spawn( void )
{
	//WARNING: CPickupWalker is not meant to be spawned! This should be inherited by something else and given more details.

	easyForcePrintLine("WARNING: ehhh");

	pev->classname = MAKE_STRING("monster_pickupwalker");
	return;

	/*
	//TEMPLATE
	Precache( );

	SET_MODEL(ENT(pev), "models/???.mdl");
	UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));
	
	pev->classname = MAKE_STRING("monster_???");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	pev->health			= 1;
	pev->view_ofs		= Vector ( 0, 0, 20 );// position of the eyes relative to monster's origin.
	pev->yaw_speed		= 100;//!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();
	*/

}//END OF Spawn(...);





//based off of GetSchedule for CBaseMonster in schedule.cpp.
Schedule_t* CPickupWalker::GetSchedule ( void )
{
	//NO, WE THINK NOT.
	//just wait. it is all you may do.

	return GetScheduleOfType(SCHED_PICKUPWALKER_IDLE_WAIT);



	/*
	//easyForcePrintLine("YA DONE UP");
	//return &slError[ 0 ];

	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("IM PICKIN A SCHEDULE FOR THIS STATE: %d", m_MonsterState) );
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

			//MODDD - are these needed at all?
			////////////////////////////////////////////////////////////////////////////////////
			if( HasConditions(bits_COND_SEE_ENEMY)){
				//run away? is this force okay?
				return GetScheduleOfType( SCHED_TOAD_RUNAWAY);
			}
			if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE) )
			{
				return GetScheduleOfType( SCHED_TOAD_RUNAWAY);
			}
			////////////////////////////////////////////////////////////////////////////////////


			if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_TOAD_ALERT_FACE );
			}

			


			//Try to do an idle wait + hop + croak later?

			return GetScheduleOfType( SCHED_TOAD_IDLE_WAIT);


			break;
		}
	case MONSTERSTATE_ALERT:
		{
			if ( HasConditions( bits_COND_ENEMY_DEAD ) && LookupActivity( ACT_VICTORY_DANCE ) != ACTIVITY_NOT_AVAILABLE )
			{
				//become normal again?
				m_MonsterState = MONSTERSTATE_IDLE;
				m_IdealMonsterState = MONSTERSTATE_IDLE;
				m_hEnemy = NULL;
				return GetSchedule();


				//return GetScheduleOfType ( SCHED_VICTORY_DANCE );
				//

			}

			//MODDD - new
			///////////////////////////////////////////////////////////////
			if( HasConditions(bits_COND_SEE_ENEMY)){
				//run away? is this force okay?
				return GetScheduleOfType( SCHED_TOAD_RUNAWAY);
			}
			///////////////////////////////////////////////////////////////


			if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE) )
			{
				if ( fabs( FlYawDiff() ) < (1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN );
				}
				else
				{
					return GetScheduleOfType( SCHED_TOAD_ALERT_SMALL_FLINCH );
				}
			}

			else if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_TOAD_ALERT_FACE );
			}
			else
			{
				//NOTICE: this includes changing the monster state to "MONSTERSTATE_IDLE" after 20 seconds.  Base class too, yes.
				return GetScheduleOfType( SCHED_TOAD_ALERT_STAND );
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

				//Chumtoad does not get angry. It gets scared and runs.

				//return GetScheduleOfType ( SCHED_TOAD_WAKE_ANGRY );

				return GetScheduleOfType(SCHED_TOAD_RUNAWAY);

			}
			//MODDD - other condition.  If "noFlinchOnHard" is on and the skill is hard, don't flinch from getting hit.
			else if (HasConditions(bits_COND_LIGHT_DAMAGE) && !HasMemory( bits_MEMORY_FLINCHED) && !(EASY_CVAR_GET_DEBUGONLY(noFlinchOnHard)==1 && g_iSkillLevel==SKILL_HARD)  )
			{
				//For the chumtoad, this  has a chance of playing dead?
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}
			else if ( !HasConditions(bits_COND_SEE_ENEMY) )
			{


				
				//MODDD TODO.  Don't see the enemy? uh, ok? Assume we're hidden?
				
				m_MonsterState = MONSTERSTATE_ALERT;
				m_IdealMonsterState = MONSTERSTATE_ALERT;
				m_hEnemy = NULL;
				//change to an... alert? state / schedule instead.  We're okay.
				return GetSchedule();
			}
			else  
			{
				// we can see the enemy
				if ( HasConditions(bits_COND_CAN_RANGE_ATTACK1) )
				{
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

				else
				{
					//lets go runnin' and runnin'!


					EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("ChumToad GetSchedule: Last RUNAWAY sched pick") );
					return GetScheduleOfType(SCHED_TOAD_RUNAWAY);

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

			//if(m_pCine == NULL){
			////	EASY_CVAR_PRINTIF_PRE(panthereyePrintout, easyPrintLine("WARNING: m_pCine IS NULL!"));
			//}

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
	EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("ChumToad: GetSchedule FAIL TO PICK SCHEDULE. BROKEN.") );
	*/
	return &slError[ 0 ];
}//END OF GetSchedule(...)




Schedule_t* CPickupWalker::GetScheduleOfType( int Type){

	//EASY_CVAR_PRINTIF_PRE(pickupWalkerPrintout, easyPrintLine("CPickupWalker: GetScheduleOfType. Type:%d", Type) );

	switch(Type){
		case SCHED_PICKUPWALKER_IDLE_WAIT:
			return &slPickupWalkerIdleWait[0];
		break;

	}//END OF switch(Type)

	return CBaseMonster::GetScheduleOfType(Type);
}//END OF GetScheduleOfType(...)



void CPickupWalker::StartTask ( Task_t *pTask )
{
	//Vector vec_forward;
	//Vector vec_right;
	//Vector vec_up;
	//TraceResult tempTrace;
	//float fracto;
	

	switch ( pTask->iTask )
	{
		case TASK_PICKUPWALKER_IDLE_WAIT_RANDOM:

			m_flWaitFinished = gpGlobals->time + RANDOM_LONG(10, 15);

		break;
		default:
			CBaseMonster::StartTask ( pTask );
		break;
	}//END OF switch(...)

}//END OF StartTask(...)




void CPickupWalker::RunTask ( Task_t *pTask ){
	
	//EASY_CVAR_PRINTIF_PRE(chumtoadPrintout, easyPrintLine("RunTask: sched:%s task:%d", this->m_pSchedule->pName, pTask->iTask) );
	
	switch ( pTask->iTask ){

		case TASK_PICKUPWALKER_IDLE_WAIT_RANDOM:
			if(gpGlobals->time > m_flWaitFinished){
				TaskComplete();
			}
		break;



		default:
			CBaseMonster::RunTask(pTask);
		break;
	}//END OF switch(...)

}//END OF RunTask(...)




//unused. MonsterThink is good enough.
void CPickupWalker::PickupWalkerThink( void ){
	
}//END OF ChumToadThink(...)


//GOOD REF:
/*
int CSqueak::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Snarks";
	p->iMaxAmmo1 = SNARK_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_SNARK;
	p->iWeight = SNARK_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}
*/

//To be filled by child classes.
const char* CPickupWalker::myWeaponClassname(){
	return NULL;  //weapon_???
}
const char* CPickupWalker::myWeaponAmmoName(){
	return NULL;  //Snarks?
}
int CPickupWalker::myWeaponSlot(){
	return -1;  //4?
}
int CPickupWalker::myWeaponMaxAmmo(){
	return -1;//SNARK_MAX_CARRY;
}



BOOL CPickupWalker::skipSpawnStuckCheck(void){
	return TRUE;
}

BOOL CPickupWalker::isOrganic(){
	//probably unnecessary. But just in case, these are typically non-robotic at least.
	return TRUE;
};


// All pickup walkers can spawn regardless of mp_allowmonsters.
BOOL CPickupWalker::bypassAllowMonstersSpawnCheck(void) {
	return TRUE;
}//END OF bypassAllowMonstersSpawnCheck



void CPickupWalker::PickupWalkerTouch( CBaseEntity *pOther )
{
	/*
	//this script kills the item it is run on.
	SetTouch( NULL );
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
	*/
	if(pOther->IsPlayer()){
		//CBaseMonster* monsterTouched = static_cast<CBaseMonster*>(pOther);
		CBasePlayer* playerTouched = static_cast<CBasePlayer*>(pOther);
		
		//could we add our weapon to the player? yes or no?
		// can I have this?
		//MODDD - possible exception to "canHavePlayerItem".  If this is a glock with a silencer, and the player has a glock without a silencer, the player can still pick up to receive the silencer.
		if( !g_pGameRules->CanHavePlayerItem( playerTouched, myWeaponClassname(), myWeaponAmmoName(), myWeaponSlot(), myWeaponMaxAmmo() ) ) //&& !(weaponCanHaveExtraCheck(pPlayer) ) )
		{
			if( gEvilImpulse101 )
			{
				UTIL_Remove( this );
			}
			//easyForcePrintLine("IMPULSE CHECK: %d", gEvilImpulse101);
			return;
		}else{
			//easyForcePrintLine("MOVE ON SONNY");
		}
		//easyForcePrintLine("WELLA??? %d", (this->m_pfnThink == &CBasePlayerWeapon::FallThink) );
		//easyForcePrintLine("WELLB??? %d", (this->m_pfnThink == NULL) );
		//easyForcePrintLine("WELLC??? %d", (this->m_pfnThink == &CBaseEntity::SUB_Remove) );

		//NOTICE - This check, "AddPlayerItem" will fail (false) if the player already has the weapon and just adds its ammo to their existing one.
		if (playerTouched->CanAddPlayerItem( myWeaponSlot(), myWeaponClassname(), myWeaponAmmoName(), myWeaponMaxAmmo() ))
		{
			//easyForcePrintLine("OKAY THERE SONNY");
			//AttachToPlayer( playerTouched );

			//playGunPickupSound(playerTouched->pev);

			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//If the player can take this weapon, spawn a copy of "myWeaponClassname".
			edict_t* createdWeap = playerTouched->GiveNamedItem(myWeaponClassname(), SF_PICKUP_NOREPLACE);
			
			if(createdWeap){
				//createdWeap->v.modelindex = 0;
				createdWeap->v.model = iStringNull;
				createdWeap->v.effects = EF_NODRAW; // ??
				//createdWeap->v.rendermode		= kRenderTransTexture;
				//createdWeap->v.renderamt		= 20;
			}

			// Can this be respawned?
			CheckRespawn();

			UTIL_Remove(this);
		}
		//easyForcePrintLine("DELLA??? %d", (this->m_pfnThink == &CBasePlayerWeapon::FallThink) );
		//easyForcePrintLine("DELLB??? %d", (this->m_pfnThink == NULL) );
		//easyForcePrintLine("DELLC??? %d", (this->m_pfnThink == &CBaseEntity::SUB_Remove) );

		////////SUB_UseTargets( pOther, USE_TOGGLE, 0 ); // UNDONE: when should this happen?
		
	}//END OF if pOther is player check
		//if so, delete.  TODO!!!
		/*
		SetTouch( NULL );
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + .1;
		*/
	
	//MODDD - no longer for fall script... unfortunate.
	/*
	
	pev->nextthink = gpGlobals->time + 0.1;
	easyForcePrintLine("PLEAAASE %d", (pev->flags & FL_ONGROUND)!=0 );


	CBaseMonster* tempMon;
	//if(pOther && (tempMon = pOther->MyMonsterPointer()) ){
	if(pOther){
		easyForcePrintLine("MY NAME IS  %s", pOther->getClassname());
		
	}
	if ( pev->flags & FL_ONGROUND )
	{
		if(initFall)easyForcePrintLine("DEATH BY ASS!");
		Land();
	}
	*/

}//END OF PickupWalkerTouch

void CPickupWalker::MonsterThink ( void )
{
    CBaseMonster::MonsterThink();
    return;
}//END OF MonsterThink



GENERATE_TRACEATTACK_IMPLEMENTATION(CPickupWalker)
{
	return;   //no damns given.
	
	//GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CPickupWalker)
{
	return 0;  //no damns given.

	//return GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);
}

GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(CPickupWalker)
{
	//what?
	return 0;

	//return GENERATE_DEADTAKEDAMAGE_PARENT_CALL(CBaseMonster);
}
GENERATE_KILLED_IMPLEMENTATION(CPickupWalker){

	//what?
	return;

	//GENERATE_KILLED_PARENT_CALL(CBaseMonster);
}



// from weapons.cpp.
void CPickupWalker::CheckRespawn(void)
{
	// TEST
	//Respawn();
	//return;

	switch (g_pGameRules->PickupWalkerShouldRespawn(this))
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}
}

// A modified clone of weapons.cpp's "Respawn" method for handling a AI monstser that should stay dormant during that time and go to the first place this was seen on the map.
CBaseEntity* CPickupWalker::Respawn(void)
{
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity* someEnt = CBaseEntity::CreateManual((char*)STRING(pev->classname), g_pGameRules->VecPickupWalkerRespawnSpot(this), this->respawn_angles, pev->owner);
	
	if (someEnt == NULL)return NULL;  // ???

	DispatchCreated(someEnt->edict());

	CPickupWalker* pNewWeapon = static_cast<CPickupWalker*>(someEnt);

	if (pNewWeapon)
	{
		// MODDD - TODO.  wait, shouldn't we also set pev->solid to SOLID_NOT ??
		// then again it doesn't matter, what else is SOLID_TRIGGER with a NULL TOUCH method anyway?  No impact.

		pNewWeapon->pev->spawnflags &= ~SF_NORESPAWN;

		// pass on my respawn_ info:
		pNewWeapon->respawn_origin = this->respawn_origin;
		pNewWeapon->respawn_angles = this->respawn_angles;

		pNewWeapon->pev->takedamage = DAMAGE_NO;
		pNewWeapon->pev->solid = SOLID_NOT;  // safety
		pNewWeapon->pev->effects |= EF_NODRAW;// invisible for now
		pNewWeapon->SetTouch(NULL);// no touch
		pNewWeapon->SetThink(&CPickupWalker::AttemptToMaterialize);

		DROP_TO_FLOOR(ENT(pev));

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.

		//pNewWeapon->pev->nextthink = gpGlobals->time + 2;
		pNewWeapon->pev->nextthink = g_pGameRules->FlPickupWalkerRespawnTime(this);
	}
	else
	{
		ALERT(at_console, "Respawn failed to create %s!\n", STRING(pev->classname));
	}

	return pNewWeapon;
}




void CPickupWalker::AttemptToMaterialize(void)
{
	float time = g_pGameRules->FlPickupWalkerTryRespawn(this);

	if (time == 0)
	{
		Materialize();
		return;
	}

	pev->nextthink = gpGlobals->time + time;
}

void CPickupWalker::Materialize(void)
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		UTIL_PlaySound(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150, FALSE);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	UTIL_SetOrigin(pev, pev->origin);// link into world.
	
	//SetTouch(&CPickupWalker::DefaultTouch);
	//SetThink(NULL);


	//SetTouch(&CPickupWalker::PickupWalkerTouch);
	//MonsterInit();
	//SetTouch(&CPickupWalker::PickupWalkerTouch);
	//pev->solid = SOLID_TRIGGER;



	// Now then, since I was so rudely interrupted.
	// no!  Just call spawn
	//DispatchSpawn(this->edict());
	Spawn();

	//SetThink(&CBaseMonster::MonsterInitThink);
	//SetTouch(&CPickupWalker::PickupWalkerTouch);
	//pev->nextthink = gpGlobals->time + 0.1;

}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////
//CChumToadPickupWalker!!!

const char* CChumToadPickupWalker::pIdleSounds[] =
{
	"chumtoad/cht_croak_medium.wav",
	"chumtoad/cht_croak_long.wav",
};


extern int global_useSentenceSave;
void CChumToadPickupWalker::Precache( void )
{
	int i = 0;
	//TEMPLATE
	PRECACHE_MODEL("models/chumtoad.mdl");
	
	global_useSentenceSave = TRUE;
	
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	
	global_useSentenceSave = FALSE;
}

void CChumToadPickupWalker::Spawn( void )
{
	//TEMPLATE
	Precache( );

	SET_MODEL(ENT(pev), "models/chumtoad.mdl");
	//UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));
	UTIL_SetSize(pev, Vector( -8.3, -8.3, 0), Vector(8.3, 8.3, 8.4));

	pev->classname = MAKE_STRING("monster_chumtoadpickupwalker");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	pev->health			= 1;
	pev->view_ofs		= Vector ( 0, 0, 20 );// position of the eyes relative to monster's origin.
	pev->yaw_speed		= 100;//!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();
	SetTouch(&CPickupWalker::PickupWalkerTouch );
	pev->solid = SOLID_TRIGGER;

}
#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_chumtoadpickupwalker, CChumToadPickupWalker );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( chumtoadpickupwalker, CChumToadPickupWalker );
	//no extras.
#endif


void CChumToadPickupWalker::IdleSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}

const char* CChumToadPickupWalker::myWeaponClassname(){
	return "weapon_chumtoad";  //weapon_???
}
const char* CChumToadPickupWalker::myWeaponAmmoName(){
	return "Chum Toads";  //Snarks?
}
int CChumToadPickupWalker::myWeaponSlot(){
	//ChumToadWeapon has this:
	//int iItemSlot( void ) { return 5; }
	return 5;
}
int CChumToadPickupWalker::myWeaponMaxAmmo(){
	return CHUMTOAD_MAX_CARRY;
}


BOOL CChumToadPickupWalker::forceIdleFrameReset(void){
	return TRUE;
}
BOOL CChumToadPickupWalker::usesAdvancedAnimSystem(void){
	return TRUE;
}
int CChumToadPickupWalker::LookupActivityHard(int activity){
	
	int i = 0;

	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	//is this safe?
	/*
	m_flFramerateSuggestion = -1;
	//pev->frame = 6;
	*/
	//any animation events in progress?  Clear it.
	resetEventQueue();
	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_IDLE:
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_WALK:
	        m_flFramerateSuggestion = 0.3;
			return CBaseAnimating::LookupActivity(activity);
		break;
	}
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}


int CChumToadPickupWalker::tryActivitySubstitute(int activity){
	int i = 0;
	/*
	m_flFramerateSuggestion = -1;
	//pev->frame = 6;
	*/
	//m_flFramerateSuggestion = -1;
	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_IDLE:
			return LookupSequence("idle");
		break;
		case ACT_WALK:
			return LookupSequence("hop");
		break;
	}
	//not handled by above?  No animations.
	return CBaseAnimating::LookupActivity(activity);
}






//////////////////////////////////////////////////////////////////////////////////
//CSqueakPickupWalker!!!

const char* CSqueakPickupWalker::pIdleSounds[] =
{
	"squeek/sqk_hunt1.wav",
	"squeek/sqk_hunt2.wav",
	"squeek/sqk_hunt3.wav",
	//"squeek/sqk_deploy1.wav",
};


extern int global_useSentenceSave;
void CSqueakPickupWalker::Precache( void )
{
	int i = 0;
	//TEMPLATE
	PRECACHE_MODEL("models/w_sqknest.mdl");
	
	global_useSentenceSave = TRUE;
	for ( i = 0; i < ARRAYSIZE( pIdleSounds ); i++ )
		PRECACHE_SOUND((char *)pIdleSounds[i]);
	
	global_useSentenceSave = FALSE;
}

void CSqueakPickupWalker::Spawn( void )
{
	//TEMPLATE
	Precache( );
	
	SET_MODEL(ENT(pev), "models/w_sqknest.mdl");
	//UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));
	UTIL_SetSize(pev, Vector( -8.3, -8.3, 0), Vector(8.3, 8.3, 8.4));

	pev->classname = MAKE_STRING("monster_snarkpickupwalker");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	pev->health			= 1;
	pev->view_ofs		= Vector ( 0, 0, 20 );// position of the eyes relative to monster's origin.
	pev->yaw_speed		= 100;//!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();
	SetTouch(&CPickupWalker::PickupWalkerTouch );
	//pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

}
#if REMOVE_ORIGINAL_NAMES != 1
	LINK_ENTITY_TO_CLASS( monster_snarkpickupwalker, CSqueakPickupWalker );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( snarkpickupwalker, CSqueakPickupWalker );
	//no extras.
#endif


void CSqueakPickupWalker::IdleSound( void )
{
	int pitch = 95 + RANDOM_LONG(0,9);
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}

const char* CSqueakPickupWalker::myWeaponClassname(){
	return "weapon_snark";  //weapon_???
}
const char* CSqueakPickupWalker::myWeaponAmmoName(){
	return "Snarks";  //Snarks?
}
int CSqueakPickupWalker::myWeaponSlot(){
	return 5;
}
int CSqueakPickupWalker::myWeaponMaxAmmo(){
	return SNARK_MAX_CARRY;
}

BOOL CSqueakPickupWalker::forceIdleFrameReset(void){
	return TRUE;
}
BOOL CSqueakPickupWalker::usesAdvancedAnimSystem(void){
	return TRUE;
}
int CSqueakPickupWalker::LookupActivityHard(int activity){
	
	int i = 0;

	m_flFramerateSuggestion = 1;
	pev->framerate = 1;
	//is this safe?
	/*
	m_flFramerateSuggestion = -1;
	//pev->frame = 6;
	*/
	//any animation events in progress?  Clear it.
	resetEventQueue();
	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_IDLE:
			return LookupSequence("idle");
		break;
		case ACT_WALK:
			return LookupSequence("Walk");
		break;
	}
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(activity);
}


int CSqueakPickupWalker::tryActivitySubstitute(int activity){
	int i = 0;
	/*
	m_flFramerateSuggestion = -1;
	//pev->frame = 6;
	*/
	//m_flFramerateSuggestion = -1;
	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		case ACT_IDLE:
			return LookupSequence("idle");
		break;
		case ACT_WALK:
			return LookupSequence("Walk");
		break;
	}
	//not handled by above?
	return CBaseAnimating::LookupActivity(activity);
}


