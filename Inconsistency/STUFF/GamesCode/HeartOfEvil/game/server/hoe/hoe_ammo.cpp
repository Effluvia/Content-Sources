#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// should probably put my code into item_ammo.cpp
extern int ITEM_GiveAmmo( CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false );

// ========================================================================

#define SIZE_AMMO_762BOX 200

class C762AmmoBox : public CItem
{
public:
	DECLARE_CLASS( C762AmmoBox, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/M60/w_m60box/w_m60box.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/M60/w_m60box/w_m60box.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		// Hack -- M21 fires 7_62mm but we have special "m21" ammo with more kick/damage.
		// So give both types of ammo.
		int g1 = ITEM_GiveAmmo( pPlayer, SIZE_AMMO_762BOX, "7_62mm");
		int g2 = ITEM_GiveAmmo( pPlayer, SIZE_AMMO_762BOX, "m21");
		if ( g1 || g2 )
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_762mmbox, C762AmmoBox);

// ========================================================================

#define SIZE_AMMO_GAS 20

class CAmmoGas : public CItem
{
public:
	DECLARE_CLASS( CAmmoGas, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/Chainsaw/w_gas/w_gas.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/Chainsaw/w_gas/w_gas.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_GAS, "Gas"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_gas, CAmmoGas);

// ========================================================================

#define SIZE_AMMO_AK47CLIP 30

class CAK47AmmoClip : public CItem
{
public:
	DECLARE_CLASS( CAK47AmmoClip, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/ak47/w_ak47clip/w_ak47clip.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/ak47/w_ak47clip/w_ak47clip.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_AK47CLIP, "7_62x39mm_M1943"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_ak47clip, CAK47AmmoClip);

// ========================================================================

#define SIZE_AMMO_BUCKSHOT_EVIL 10

class CBuckshot : public CItem
{
public:
	DECLARE_CLASS( CBuckshot, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/870/w_shot_10shells/w_shot_10shells.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/870/w_shot_10shells/w_shot_10shells.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_BUCKSHOT_EVIL, "Buckshot"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_buckshot, CBuckshot);

// ========================================================================

#define SIZE_AMMO_BUCKSHOT_LARGE 22

class CBuckshotLarge : public CItem
{
public:
	DECLARE_CLASS( CBuckshotLarge, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/870/w_shot_22shells/w_shot_22shells.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/870/w_shot_22shells/w_shot_22shells.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_BUCKSHOT_LARGE, "Buckshot"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_buckshot_large, CBuckshotLarge);

// ========================================================================

#define SIZE_AMMO_COLT1911A1 7

class C1143Ammo : public CItem
{
public:
	DECLARE_CLASS( C1143Ammo, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/colt1911A1/w_1143mmclip/w_1143mmclip.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/colt1911A1/w_1143mmclip/w_1143mmclip.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_COLT1911A1, "11_43mm"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_1143mm, C1143Ammo);

// ========================================================================

class C1143Ammo_40Box : public CItem
{
public:
	DECLARE_CLASS( C1143Ammo_40Box, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/colt1911A1/w_1143mm_40box/w_1143mm_40box.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/colt1911A1/w_1143mm_40box/w_1143mm_40box.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, 40, "11_43mm"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_1143mm_40box, C1143Ammo_40Box);

// ========================================================================

class C1143Ammo_50Box : public CItem
{
public:
	DECLARE_CLASS( C1143Ammo_50Box, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/colt1911A1/w_1143mm_50box/w_1143mm_50box.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/colt1911A1/w_1143mm_50box/w_1143mm_50box.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, 50, "11_43mm"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_1143mm_50box, C1143Ammo_50Box);

// ========================================================================

#define SIZE_AMMO_ELEPHANTSHOT 10

class CElephantShot : public CItem
{
public:
	DECLARE_CLASS( CElephantShot, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/870/w_shot_10shells/w_elephant_10shells.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/870/w_shot_10shells/w_elephant_10shells.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_ELEPHANTSHOT, "Elephantshot"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_elephantshot, CElephantShot);

// ========================================================================

#define SIZE_AMMO_ELEPHANTSHOT_LARGE 22

class CElephantShotLarge : public CItem
{
public:
	DECLARE_CLASS( CElephantShotLarge, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/870/w_shot_22shells/w_elephant_22shells.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/870/w_shot_22shells/w_elephant_22shells.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_ELEPHANTSHOT_LARGE, "Elephantshot"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_elephantshot_large, CElephantShotLarge);

// ========================================================================

#define SIZE_AMMO_M16BOX 150

class CM16AmmoBox : public CItem
{
public:
	DECLARE_CLASS( CM16AmmoBox, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/w_m16box.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/w_m16box.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_M16BOX, "5_56mm"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_m16box, CM16AmmoBox);

// ========================================================================

#define SIZE_AMMO_M16CLIP 30

class CM16AmmoClip : public CItem
{
public:
	DECLARE_CLASS( CM16AmmoClip, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/w_m16clip.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/w_m16clip.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_M16CLIP, "5_56mm"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_m16clip, CM16AmmoClip);

// ========================================================================

#define SIZE_AMMO_M79VEST 20

class CM79Vest : public CItem
{
public:
	DECLARE_CLASS( CM79Vest, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/M79/w_m79vest/w_m79vest.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/M79/w_m79vest/w_m79vest.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_M79VEST, "AR_Grenade"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_m79vest, CM79Vest);

// ========================================================================

#define SIZE_AMMO_203BOX 2

class CMP5AmmoGrenade : public CItem
{
public:
	DECLARE_CLASS( CMP5AmmoGrenade, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/w_ARgrenades.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/w_ARgrenades.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_203BOX, "AR_Grenade"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}

	// Override global function to always give 2 on any difficulty setting.
	// Otherwise Alamo isn't fun when choosing M79 at the beginning.
	int ITEM_GiveAmmo( CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false )
	{
		int iAmmoType = GetAmmoDef()->Index(pszAmmoName);
		if (iAmmoType == -1)
		{
			Msg("ERROR: Attempting to give unknown ammo type (%s)\n",pszAmmoName);
			return 0;
		}

//		flCount *= g_pGameRules->GetAmmoQuantityScale(iAmmoType);

		// Don't give out less than 1 of anything.
		flCount = max( 1.0f, flCount );

		return pPlayer->GiveAmmo( flCount, iAmmoType, bSuppressSound );
	}
};
LINK_ENTITY_TO_CLASS(ammo_ARgrenades, CMP5AmmoGrenade);

// ========================================================================

#define SIZE_AMMO_RPG 1

class CRpgAmmo : public CItem
{
public:
	DECLARE_CLASS( CRpgAmmo, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/w_rpgammo.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/w_rpgammo.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_RPG, "RPG_Round"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_rpgclip, CRpgAmmo);

//=========================================================
// Soda can -- made physics-enabled for HOE
//=========================================================
class CItemSoda : public CItem
{
public:
	DECLARE_CLASS( CItemSoda, CItem );
	DECLARE_DATADESC();

	CItemSoda()
	{
		m_bShot = false;
	}

	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/can.mdl" );

		if ( m_nSkin < 0 || m_nSkin > 6 )
			m_nSkin = 6;
		if ( m_nSkin == 6 )
			m_nSkin = random->RandomInt( 0, 5 );

		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel("models/can.mdl");
		PrecacheScriptSound( "NPC_MikeForce.Glug" );
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->TakeHealth( 1, DMG_GENERIC ) )
		{
			pPlayer->EmitSound( "NPC_MikeForce.Glug" );
			UTIL_Remove(this);	
			return true;
		}
		return false;
	}
	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
	{
		if ( !m_bShot && (info.GetDamageType() & DMG_BULLET) )
		{
			CollisionProp()->WorldToCollisionSpace( ptr->endpos, &m_vecAttackOrigin );
			m_flShotTime = gpGlobals->curtime;
			m_bShot = true;

			SetThink( &CItemSoda::SprayThink );
			SetNextThink( gpGlobals->curtime );
		}
		return BaseClass::TraceAttack( info, vecDir, ptr );
	}
	void SprayThink( void )
	{
#define SODA_SPRAY_TIME 5.0f
		if ( m_flShotTime <= gpGlobals->curtime - SODA_SPRAY_TIME )
		{
			SetThink( NULL );
			return;
		}

		float flDecay = 1.0 - (gpGlobals->curtime - m_flShotTime) / SODA_SPRAY_TIME;
//		flDecay = clamp( flDecay, 0, 1 );

		Vector vecSprayOrigin;
		CollisionProp()->CollisionToWorldSpace( m_vecAttackOrigin, &vecSprayOrigin);

		Vector vecSprayDir = vecSprayOrigin - CollisionProp()->WorldSpaceCenter();

		int amount = 100 * flDecay;
		CPVSFilter filter( vecSprayOrigin );
		te->BloodStream( filter, 0.0, &vecSprayOrigin, &vecSprayDir, 128, 128, 128, 255, amount );

		Vector vecSprayForce = CollisionProp()->WorldSpaceCenter() - vecSprayOrigin;
		VectorNormalize( vecSprayForce );
		vecSprayForce *= flDecay * 50;
		VPhysicsGetObject()->ApplyForceOffset( vecSprayForce, vecSprayOrigin );

		SetNextThink( gpGlobals->curtime + 0.1f );
	}

	bool m_bShot;
	float m_flShotTime;
	Vector m_vecAttackOrigin; // shot location in collision-space
};
LINK_ENTITY_TO_CLASS( item_sodacan, CItemSoda );

BEGIN_DATADESC( CItemSoda )
	DEFINE_FIELD( m_bShot, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flShotTime, FIELD_TIME ),
	DEFINE_FIELD( m_vecAttackOrigin, FIELD_VECTOR ),
	DEFINE_THINKFUNC( SprayThink ),
END_DATADESC()


//-----------------------------------------------------------------------------
class CItemWhisky : public CItem
{
public:
	DECLARE_CLASS( CItemWhisky, CItem );

	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/w_whisky.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel("models/w_whisky.mdl");
		PrecacheScriptSound( "NPC_MikeForce.Glug" );
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->TakeHealth( 5, DMG_GENERIC ) )
		{
			pPlayer->EmitSound( "NPC_MikeForce.Glug" );
			UTIL_Remove(this);	
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS( item_whisky, CItemWhisky);