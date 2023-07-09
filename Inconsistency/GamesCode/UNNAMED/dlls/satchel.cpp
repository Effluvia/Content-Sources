/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

enum satchel_e {
	SATCHEL_IDLE1 = 0,
	SATCHEL_DRAW,
	SATCHEL_ARM,
	SATCHEL_THROW,
	SATCHEL_THROW2
};

enum satchel_radio_e {
	SATCHEL_RADIO_IDLE1 = 0,
	SATCHEL_RADIO_DRAW,
	SATCHEL_RADIO_FIRE
};

class CSatchelCharge : public CGrenade
{
	void Spawn( void );
	void Precache( void );
	void BounceSound( void );

	void EXPORT SatchelSlide( CBaseEntity *pOther );
	void EXPORT SatchelThink(void);

	int bounces;

public:
	void Deactivate( void );
};
LINK_ENTITY_TO_CLASS( monster_satchel, CSatchelCharge );

//=========================================================
// Deactivate - do whatever it is we do to an orphaned 
// satchel when we don't want it in the world anymore.
//=========================================================
void CSatchelCharge::Deactivate( void )
{
	pev->solid = SOLID_NOT;
	UTIL_Remove( this );
}


void CSatchelCharge :: Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/w_satchel.mdl");
	//UTIL_SetSize(pev, Vector( -16, -16, -4), Vector(16, 16, 32));	// Old box -- size of headcrab monsters/players get blocked by this
	UTIL_SetSize(pev, Vector( -4, -4, -4), Vector(4, 4, 4));	// Uses point-sized, and can be stepped over
	UTIL_SetOrigin( pev, pev->origin );

	SetTouch( &CSatchelCharge::SatchelSlide );
	SetUse( &CSatchelCharge::DetonateUse );
	SetThink( &CSatchelCharge::SatchelThink );
	pev->nextthink = gpGlobals->time + 0.1;

	pev->gravity = 0.5;
	pev->friction = 0.8;

	pev->dmg = gSkillData.plrDmgHandGrenade;
	// ResetSequenceInfo( );
	pev->sequence = 1;

	bounces = 1;
}

void CSatchelCharge::SatchelSlide( CBaseEntity *pOther )
{
	entvars_t	*pevOther = pOther->pev;


	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;


	if (m_flNextAttack < gpGlobals->time && pev->velocity.Length() > 100)
	{
		entvars_t *pevOwner = VARS(pev->owner);
		if (pevOwner)
		{
			TraceResult tr;
			ClearMultiDamage();
			pOther->TraceAttack(pevOwner, 1, gpGlobals->v_forward, &tr, DMG_CLUB);
			ApplyMultiDamage(pev, pevOwner);
		}
		m_flNextAttack = gpGlobals->time + 1.0; // debounce
	}

	// pev->avelocity = Vector (300, 300, 300);
	pev->gravity = 1;// normal gravity now

	// HACKHACK - On ground isn't always set, so look for ground underneath
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin - Vector(0, 0, 10), ignore_monsters, edict(), &tr);


	if ( tr.flFraction < 1.0 )
	{
		// add a bit of static friction
		pev->velocity.x *= 0.6;
		pev->velocity.y *= 0.6;

		//==============================================================
		if (abs(pev->velocity.z) < 100)
			pev->velocity.z *= 0.3;

		pev->velocity.z -= 8 * (abs((pev->velocity.z)) * 0.36); //bounce
		//==============================================================

		pev->avelocity = pev->avelocity * 0.7;
		// play sliding sound, volume based on velocity
	}
	if (!(pev->flags & FL_ONGROUND) && ((pev->velocity.Length2D() > 20) || (abs(pev->velocity.z) > 100)))
	{
		BounceSound();
	}

	StudioFrameAdvance( );
}


void CSatchelCharge::SatchelThink()
{
	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	if (pev->waterlevel == 3)
	{
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = pev->velocity * 0.8;
		pev->avelocity = pev->avelocity * 0.9;
		pev->velocity.z += 8;
	}
	else if (pev->waterlevel == 0)
	{
		pev->movetype = MOVETYPE_BOUNCE;
	}
	else
	{
		pev->velocity.z -= 8;
	}

	if (pev->velocity == Vector(0, 0, 0))
	{
		CBaseEntity *pEntity = NULL;
		if ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin + Vector(0, 0, 5), 8)) != NULL)
			if (pEntity->edict() == pev->owner)
			{
				ALERT(at_aiconsole, ">> %s, you're walking on me!\n", STRING(pEntity->pev->classname));
				if (pEntity->GiveAmmo(1, "Satchel Charge", SATCHEL_MAX_CARRY))
				{
					pOwnedSatchelWeapon->PickupSatchel();
					EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);
					UTIL_Remove(this);
				}
			}
	}

}

void CSatchelCharge :: Precache( void )
{
	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_SOUND("weapons/g_bounce1.wav");
	PRECACHE_SOUND("weapons/g_bounce2.wav");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
	PRECACHE_SOUND("weapons/pipebomb_bounce.wav");
}

void CSatchelCharge :: BounceSound( void )
{
/*	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce1.wav", 1, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce2.wav", 1, ATTN_NORM);	break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce3.wav", 1, ATTN_NORM);	break;
	}
*/
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/pipebomb_bounce.wav", 1, ATTN_NORM, 0, RANDOM_LONG(95,110));
}


//LINK_ENTITY_TO_CLASS( weapon_satchel, CSatchel );
LINK_ENTITY_TO_CLASS( weapon_handgrenade, CSatchel);

//=========================================================
// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
//=========================================================
int CSatchel::AddDuplicate( CBasePlayerItem *pOriginal )
{
	CSatchel *pSatchel;

#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
	{
		pSatchel = (CSatchel *)pOriginal;

		if ( pSatchel->m_chargeReady != 0 )
		{
			// player has some satchels deployed. Refuse to add more.
			return FALSE;
		}
	}

	return CBasePlayerWeapon::AddDuplicate ( pOriginal );
}

//=========================================================
//=========================================================
int CSatchel::AddToPlayer( CBasePlayer *pPlayer )
{
	int bResult = CBasePlayerItem::AddToPlayer( pPlayer );

	pPlayer->pev->weapons |= (1<<m_iId);
	m_chargeReady = 0;// this satchel charge weapon now forgets that any satchels are deployed by it.

	if ( bResult )
	{
		return AddWeapon( );
	}
	return FALSE;
}

void CSatchel::Spawn( )
{
	Precache( );
	m_iId = WEAPON_SATCHEL;
	SET_MODEL(ENT(pev), "models/w_satchel.mdl");

	m_iDefaultAmmo = SATCHEL_DEFAULT_GIVE;
		
	FallInit();// get ready to fall down.
}


void CSatchel::Precache( void )
{
	PRECACHE_MODEL("models/v_satchel.mdl");
	PRECACHE_MODEL("models/v_pipebomb.mdl");
	PRECACHE_MODEL("models/v_satchel_radio.mdl");
	PRECACHE_MODEL("models/v_pipebomb_button.mdl");
	PRECACHE_MODEL("models/w_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel_radio.mdl");

	UTIL_PrecacheOther( "monster_satchel" );
}


int CSatchel::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Satchel Charge";
	p->iMaxAmmo1 = SATCHEL_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 5;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_SATCHEL;
	p->iWeight = SATCHEL_WEIGHT;

	return 1;
}

//=========================================================
//=========================================================
BOOL CSatchel::IsUseable( void )
{
	if ( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] > 0 ) 
	{
		// player is carrying some satchels
		return TRUE;
	}

	if ( m_chargeReady != 0 )
	{
		// player isn't carrying any satchels, but has some out
		return TRUE;
	}

	return FALSE;
}

BOOL CSatchel::CanDeploy( void )
{
	return CSatchel::IsUseable();
}

BOOL CSatchel::Deploy( )
{

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

	if (m_chargeReady == 1)
	{
		return DefaultDeploy("models/v_pipebomb_button.mdl", "models/p_satchel_radio.mdl", SATCHEL_RADIO_DRAW, "crowbar");
	}
	else
	{
		m_chargeReady = 0;
		return DefaultDeploy("models/v_pipebomb.mdl", "models/p_satchel.mdl", SATCHEL_DRAW, "crowbar");
	}
	
	return TRUE;
}

void CSatchel::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	if ( m_chargeReady )
	{
	//	SendWeaponAnim( SATCHEL_RADIO_HOLSTER );
	}
	else
	{
		SendWeaponAnim( SATCHEL_ARM );
	}
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);

	if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && !m_chargeReady )
	{
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_SATCHEL);
		SetThink( &CSatchel::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CSatchel::PickupSatchel()
{
	ALERT(at_aiconsole, ">> Yes, I picked up your Pipebomb indeed");

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{

#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_pipebomb.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel.mdl");
#else
		LoadVModel("models/v_satchel.mdl", m_pPlayer);
#endif

		SendWeaponAnim(SATCHEL_DRAW);

		// use tripmine animations
		strcpy(m_pPlayer->m_szAnimExtention, "crowbar");

		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		//m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_chargeReady = 0;
		return;
	}
}

void CSatchel::PrimaryAttack()
{
	switch (m_chargeReady)
	{
	case 0:
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim(SATCHEL_ARM);
			m_chargeReady = 5;
			//m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.05;
		}
		return;
	case 1:
		{
		m_chargeReady = 2;
		SendWeaponAnim( SATCHEL_RADIO_FIRE );
		edict_t *pPlayer = m_pPlayer->edict( );

		CBaseEntity *pSatchel = NULL;

		while ((pSatchel = UTIL_FindEntityInSphere( pSatchel, m_pPlayer->pev->origin, 4096 )) != NULL)
		{
			if (FClassnameIs( pSatchel->pev, "monster_satchel"))
			{
				if (pSatchel->pev->owner == pPlayer)
				{
					pSatchel->Use( m_pPlayer, m_pPlayer, USE_ON, 0 );
				}
			}
		}

		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		//m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.55;
		break;
		}

	case 2:
		// we're reloading, don't allow fire
		{
		}
		break;
	}
}


void CSatchel::SecondaryAttack( void )
{
//	if ( m_chargeReady != 2 )
//	{
//		Throw( );
//	}
	Kick();
}


void CSatchel::Throw( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		Vector vecSrc = m_pPlayer->pev->origin + Vector(0,0,3);

		//SendWeaponAnim(SATCHEL_THROW);

	//	Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;
		Vector vecThrow; //= gpGlobals->v_forward * 220 + (Vector(m_pPlayer->pev->velocity.x, 0, 0) + Vector(0, m_pPlayer->pev->velocity.y, 0)) / 1.75 + Vector(0, 0, 150);

#ifndef CLIENT_DLL

		if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		{
			vecThrow = gpGlobals->v_forward * 400 + (Vector(m_pPlayer->pev->velocity.x, 0, 0) + Vector(0, m_pPlayer->pev->velocity.y, 0)) / 1.75 + Vector(0, 0, 50);
			SendWeaponAnim(SATCHEL_THROW2);
		}
		else
		{
			vecThrow = gpGlobals->v_forward * 220 + (Vector(m_pPlayer->pev->velocity.x, 0, 0) + Vector(0, m_pPlayer->pev->velocity.y, 0)) / 1.75 + Vector(0, 0, 220);
			SendWeaponAnim(SATCHEL_THROW);
		}


		CBaseEntity *pSatchel = Create("monster_satchel", vecSrc, Vector(0, m_pPlayer->pev->angles.y + 90, 0), m_pPlayer->edict());
		pSatchel->pev->velocity = vecThrow;
		pSatchel->pev->avelocity.z = 400;
		pSatchel->pOwnedSatchelWeapon = this;

//		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_satchel_radio.mdl");
//		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel_radio.mdl");
//#else
//		LoadVModel ( "models/v_satchel_radio.mdl", m_pPlayer );
#endif

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		m_chargeReady = 6;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		//m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}


void CSatchel::WeaponIdle( void )
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	switch( m_chargeReady )
	{
	case 0:
		SendWeaponAnim( SATCHEL_IDLE1 );
		// use tripmine animations
		strcpy( m_pPlayer->m_szAnimExtention, "crowbar" );
		break;
	case 1:

			//if (m_pPlayer->pev->iSatchelPickedup)
			//{
			//	m_chargeReady = 2;
			//	m_pPlayer->pev->iSatchelPickedup = 0;
			//}

			SendWeaponAnim( SATCHEL_RADIO_IDLE1 );
			// use hivehand animations
			strcpy( m_pPlayer->m_szAnimExtention, "crowbar" );
			break;
	case 2:
		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			RetireWeapon();
		}
		else
		{
			#ifndef CLIENT_DLL
				m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_pipebomb.mdl");
				m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel.mdl");
			#else
				LoadVModel("models/v_pipebomb.mdl", m_pPlayer);
			#endif
				SendWeaponAnim(SATCHEL_DRAW);
		}
		m_chargeReady = 0;
		break;
	case 5:
			Throw();
		return;
	case 6:

			#ifndef CLIENT_DLL
					m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_pipebomb_button.mdl");
					m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel_radio.mdl");
			#else
					LoadVModel("models/v_satchel_radio.mdl", m_pPlayer);
			#endif

			m_chargeReady = 1;
			SendWeaponAnim(SATCHEL_RADIO_DRAW);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		return;


#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_pipebomb.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel.mdl");
#else
		LoadVModel ( "models/v_satchel.mdl", m_pPlayer );
#endif

		SendWeaponAnim( SATCHEL_DRAW );

		// use tripmine animations
		strcpy( m_pPlayer->m_szAnimExtention, "crowbar" );

		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		//m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_chargeReady = 0;
		break;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );// how long till we do this again.
}

//=========================================================
// DeactivateSatchels - removes all satchels owned by
// the provided player. Should only be used upon death.
//
// Made this global on purpose.
//=========================================================
void DeactivateSatchels( CBasePlayer *pOwner )
{
	edict_t *pFind; 

	pFind = FIND_ENTITY_BY_CLASSNAME( NULL, "monster_satchel" );

	while ( !FNullEnt( pFind ) )
	{
		CBaseEntity *pEnt = CBaseEntity::Instance( pFind );
		CSatchelCharge *pSatchel = (CSatchelCharge *)pEnt;

		if ( pSatchel )
		{
			if ( pSatchel->pev->owner == pOwner->edict() )
			{
				pSatchel->Deactivate();
			}
		}

		pFind = FIND_ENTITY_BY_CLASSNAME( pFind, "monster_satchel" );
	}
}

#endif
