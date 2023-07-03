#include "cbase.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "items.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MODEL_FLAKJACKET "models/w_flakjacket/w_flakjacket.mdl"

// This is the power multiplier relative to sk_battery.
// HL1 version used sk_flakjacket (75, 50, 35).
#define FLAKJACKET_MULTIPLIER 3.3

class CItemFlakJacket : public CItem
{
public:
	DECLARE_CLASS( CItemFlakJacket, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( MODEL_FLAKJACKET );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel( MODEL_FLAKJACKET );
		PrecacheScriptSound( "ItemBattery.Touch" );
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>( pPlayer );
		return ( pHL2Player && pHL2Player->ApplyBattery(FLAKJACKET_MULTIPLIER) );
	}
};

LINK_ENTITY_TO_CLASS(item_flakjacket, CItemFlakJacket);
PRECACHE_REGISTER(item_flakjacket);