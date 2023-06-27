
#include "extdll.h"

#include "hornet_kingpin.h"

#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "soundent.h"
#include "gamerules.h"






CHornetKingpin::CHornetKingpin(void){

}//END OF CHornetKingpin constructor

LINK_ENTITY_TO_CLASS( hornet_kingpin, CHornetKingpin );


//=========================================================
// Save/Restore
//=========================================================
TYPEDESCRIPTION	CHornetKingpin::m_SaveData[] = 
{

	DEFINE_FIELD( CHornetKingpin, expireTime, FIELD_TIME ),
	DEFINE_FIELD( CHornetKingpin, speedMissileDartTarget, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CHornetKingpin, speedMissileDartTargetOffset, FIELD_VECTOR ),
	DEFINE_FIELD( CHornetKingpin, speedMissileDartDirection, FIELD_VECTOR ),
	
};

IMPLEMENT_SAVERESTORE( CHornetKingpin, CHornet );



void CHornetKingpin::Spawn( void ){
	CHornet::Spawn();
	//do what the parent does.
	
	pev->movetype	= MOVETYPE_BOUNCEMISSILE;

	pev->classname = MAKE_STRING("hornet_kingpin");
	
	//even smaller.  They must not collide with each other mid-flight.
	UTIL_SetSize( pev, Vector( -0.5, -0.5, -0.5 ), Vector( 0.5, 0.5, 0.5 ) );

	//safe default.
	m_hEnemy = NULL;


	//but change the starting think to this to tie into the rest of our own logic instead.
	SetThink(&CHornetKingpin::StartSpeedMissile);
	SetTouch(&CHornetKingpin::SmartDieTouch);

}//END OF Spawn

void CHornetKingpin::Precache( void ){
	CHornet::Precache();
	//do what the parent does.


}//END OF Precache


void CHornetKingpin::setup(CBaseEntity* arg_targetEnt, const Vector& arg_targetOffset){
	//NOTICE - the sent arg_targetEnt can NOT be null or we have no way of getting a target!


	m_hEnemy = arg_targetEnt;
	speedMissileDartTarget = arg_targetEnt->BodyTargetMod(pev->origin);
	

	speedMissileDartTargetOffset = arg_targetOffset;

}
void CHornetKingpin::setup(const Vector& arg_vecTarget, const Vector& arg_targetOffset){

	m_hEnemy = NULL;  //don't have one provided.

	speedMissileDartTarget = arg_vecTarget;

	speedMissileDartTargetOffset = arg_targetOffset;

}



void CHornetKingpin::StartSpeedMissile(void){
	//SetThink( &CBaseEntity::SUB_Remove );
	SetThink( &CHornetKingpin::SpeedMissileDartStart );

	//AHHHH. leave it to SmartDieTouch please!!
	//SetTouch( &CHornetKingpin::DartTouch );

	pev->nextthink = gpGlobals->time + 0.73f;


}


void CHornetKingpin::SpeedMissileDartStart(void){
	//pev->velocity = Vector(0,0,0);

	//ignite the trail here instead, custom.
	PLAYBACK_EVENT_FULL (FEV_GLOBAL, this->edict(), g_sImitation7, 0.0, (float *)&this->pev->origin, (float *)&this->pev->angles, 0.7, 0.0, this->entindex(), 0, 0, 0);


	expireTime = gpGlobals->time + 2.4f;

	if(m_hEnemy != NULL){
		//override: make speedMissileDartTarget the enemy's location now.
		//MODDD TODO - And is BodyTargetMod or EyePosition best??  or even Center()
		speedMissileDartTarget = m_hEnemy->BodyTargetMod(pev->origin) + speedMissileDartTargetOffset;
	}

	speedMissileDartDirection = (speedMissileDartTarget - pev->origin).Normalize();
	

	pev->velocity = speedMissileDartDirection * 600;

	//face the direction I'm speeding in.
	pev->angles = UTIL_VecToAngles( speedMissileDartDirection );

	SetThink( &CHornetKingpin::SpeedMissileDartContinuous );
	//SetTouch( &CHornet::DartTouch );
	pev->nextthink = gpGlobals->time + 0.1;
}

void CHornetKingpin::SpeedMissileDartContinuous(void){
	//if(pev->velocity.Length() < 1600){

		if(m_hEnemy != NULL){
			//override: make speedMissileDartTarget the enemy's location now.

			const float distToEnemy = (m_hEnemy->pev->origin - pev->origin).Length();

			if(distToEnemy < 600){
				//forget the offset.
				speedMissileDartTarget = m_hEnemy->BodyTargetMod(pev->origin);
			}else{
				//use the offset.
				speedMissileDartTarget = m_hEnemy->BodyTargetMod(pev->origin) + speedMissileDartTargetOffset;
			}
			

			speedMissileDartDirection = (speedMissileDartTarget - pev->origin).Normalize();
		}

		pev->velocity = pev->velocity * 0.84f + speedMissileDartDirection * 360;
	//}


	pev->angles = UTIL_VecToAngles( pev->velocity );
	//pev->angles.z = 0;
	//pev->angles.x = 0;

	
	//SetThink( &CHornet::SpeedMissileDartContinuous );
	//SetTouch( &CHornet::DartTouch );
	pev->nextthink = gpGlobals->time + 0.1;

	if(gpGlobals->time >= expireTime){
		//just remove me.
		SetThink( &CBaseEntity::SUB_Remove );
	}

}


//Similar to CHornet's DieTouch, but ignores touches with other hornets.  At least other kingpin_hornet's.
void CHornetKingpin::SmartDieTouch(CBaseEntity* pOther )
{
	//only collide IF the other thing is NOT a hornet_kingpin.
	//if(pOther && !FClassnameIs(pOther->pev, "hornet_kingpin") ){
	if(pOther){
		const char* otherClassname = pOther->getClassname();
	
		//MODDD TODO - should it turn off sounds for "pother->IsWorldOrAffiliated"?
		//func_door's for whatever reason have pev->takedamage on apparently.
		if(pOther->pev->takedamage && !FClassnameIs(pOther->pev, "hornet_kingpin") ){
			switch (RANDOM_LONG(0,2))
			{// buzz when you plug someone
				case 0:	UTIL_PlaySound( ENT(pev), CHAN_VOICE, "hornet/ag_hornethit1.wav", 1, ATTN_NORM);	break;
				case 1:	UTIL_PlaySound( ENT(pev), CHAN_VOICE, "hornet/ag_hornethit2.wav", 1, ATTN_NORM);	break;
				case 2:	UTIL_PlaySound( ENT(pev), CHAN_VOICE, "hornet/ag_hornethit3.wav", 1, ATTN_NORM);	break;
			}
			
			pOther->TakeDamage( pev, VARS( pev->owner ), pev->dmg, DMG_BULLET );
		}

		pev->modelindex = 0;// so will disappear for the 0.1 secs we wait until NEXTTHINK gets rid
		pev->solid = SOLID_NOT;

		SetThink ( &CBaseEntity::SUB_Remove );
		pev->nextthink = gpGlobals->time + 1;// stick around long enough for the sound to finish!
		//pev->nextthink = gpGlobals->time + 0;

	}//END OF other hornet_kingpin check
	
}//END OF SmartDieTouch



Vector CHornetKingpin::GetVelocityLogical(void){
	//probably fine?
	return pev->velocity;
}
//Likewise, if something else wants to change our velocity, and we pay more attention to something other than pev->velocty,
//we need to apply the change to that instead.  Or both, leaving that up to the thing implementing this.
void CHornetKingpin::SetVelocityLogical(const Vector& arg_newVelocity){
	pev->velocity = arg_newVelocity;
	// never use TrackTouch? we never call vecFlightDirTrue.
	//vecFlightDirTrue = arg_newVelocity;
}

void CHornetKingpin::OnDeflected(CBaseEntity* arg_entDeflector){

	//Tell me to stop following behavior.  The die time reset for quick fire is okay too once.
	if(!reflectedAlready){
		reflectedAlready = TRUE;
		this->StartDart();
	}
}

