/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"


//what items.cpp would do (for calling back to the client).
extern int gmsgItemPickup;
extern int gmsgUpdateAirTankAirTime;


class CAirtank : public CGrenade
{
	int m_state;

	void Spawn( void );
	void Precache( void );
	void EXPORT TankThink( void );
	void EXPORT TankTouch( CBaseEntity *pOther );
	int  BloodColor( void ) { return DONT_BLEED; };
	
	GENERATE_KILLED_PROTOTYPE

	static	TYPEDESCRIPTION m_SaveData[];
	virtual int	Save( CSave &save ); 
	virtual int	Restore( CRestore &restore );
	

};


LINK_ENTITY_TO_CLASS( item_airtank, CAirtank );
TYPEDESCRIPTION	CAirtank::m_SaveData[] = 
{
	DEFINE_FIELD( CAirtank, m_state, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CAirtank, CGrenade );


void CAirtank::Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/w_oxygen.mdl");
	UTIL_SetSize(pev, Vector( -16, -16, 0), Vector(16, 16, 36));
	UTIL_SetOrigin( pev, pev->origin );

	SetTouch( &CAirtank::TankTouch );
	SetThink( &CAirtank::TankThink );

	pev->flags |= FL_MONSTER;
	pev->takedamage		= DAMAGE_YES;
	pev->health			= 20;
	//MODDD note -  Know why this isn't making an explosion graphic (the animating sprite - orange cloud)?
	//It's because the damage is "50", under some value to make the graphic appear (or be larger than nothing if it scales w/ size).
	//Or, a value of "100" will show the explosion.  No idea.
	pev->dmg			= 50;
	m_state				= 1;
}

void CAirtank::Precache( void )
{
	PRECACHE_MODEL("models/w_oxygen.mdl");
	//PRECACHE_SOUND("doors/aliendoor3.wav");
}


GENERATE_KILLED_IMPLEMENTATION(CAirtank)
{
	pev->owner = ENT( pevAttacker );

	// UNDONE: this should make a big bubble cloud, not an explosion

	//MODDD - the above comment was from valve.  Anyhow, the below seems to be the ONLY reference to this particular type of "Explode" method.
	//Because it was used only this once, it looks like the devs did not notice these arguments getting ignored (see method, they are not used at all).
	//By coincidence, similar values are used from the object though.
	//But the method uses "Vector( 0, 0, -32 )" to mod the given origin instead of this Vector(0,0,-1).
	
	//MODDD - you know what? original disabled.  Changed to call the modified version without any arguments to ease confusion.
	//Explode( pev->origin, Vector( 0, 0, -1 ) );
	Explode();
}


void CAirtank::TankThink( void )
{
	// Fire trigger
	m_state = 1;
	SUB_UseTargets( this, USE_TOGGLE, 0 );
}


void CAirtank::TankTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
		return;

	
	//MODDD - new tank behavior.  Convert the generic "CBaseEntity", pOther to a "CBasePlayer" for accessing
	//"airTankAirTime".  If air time isn't at the max, set it to the max and remove the tank we found from the
	//game.
	//////////////////////////////////////////////////////////////////////
	
	//CBasePlayer * pPlayer = dynamic_cast<CBasePlayer*>(pOther);
	CBasePlayer * pPlayer = static_cast<CBasePlayer*>(pOther);
	
	if(pPlayer->airTankAirTime < PLAYER_AIRTANK_TIME_MAX){
		//no sound.
		//pPlayer->SetSuitUpdate("!HEV_DET4", FALSE, SUIT_NEXT_IN_1MIN);
			
		pPlayer->airTankAirTime = PLAYER_AIRTANK_TIME_MAX;

		//ASSERT( gmsgRadiationP > 0 );
		

		if(gmsgUpdateAirTankAirTime > 0){
			MESSAGE_BEGIN( MSG_ONE, gmsgUpdateAirTankAirTime, NULL, pPlayer->pev );
			WRITE_SHORT( pPlayer->airTankAirTime);
			MESSAGE_END();
		}
		
		MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
		WRITE_STRING( STRING(pev->classname) );
		MESSAGE_END();


		UTIL_PlaySound( pPlayer->edict(), CHAN_ITEM, "items/airtank1.wav", 1, ATTN_NORM, 0, 100, TRUE );



		REMOVE_ENTITY(edict());
	}//END OF if(pPlayer->m_rgItems[ITEM_RADIATION] < 5)
	//////////////////////////////////////////////////////////////////////


	//MODDD - original script.  Commented out.  Seems to be a "rechargable" tank instead, and implies that 
	//the player finds it while underwater (adding 12 to "pOther->pev->air_finished" would only make sense
	//then, when it isn't getting bombarded with resets while not in water for what time counts as running
	//out of water: always 12 seconds in the future when in air).

	//It appears this airTank was not originally intended as a pickup.  Thoughts?
	/*

	if (!m_state)
	{
		// "no oxygen" sound
		EMIT_SOUND( ENT(pev), CHAN_BODY, "player/pl_swim2.wav", 1.0, ATTN_NORM );
		return;
	}
		
	// give player 12 more seconds of air
	pOther->pev->air_finished = gpGlobals->time + 12;

	// suit recharge sound
	EMIT_SOUND( ENT(pev), CHAN_VOICE, "doors/aliendoor3.wav", 1.0, ATTN_NORM );

	// recharge airtank in 30 seconds
	pev->nextthink = gpGlobals->time + 30;
	m_state = 0;
	SUB_UseTargets( this, USE_TOGGLE, 1 );

	*/
}
