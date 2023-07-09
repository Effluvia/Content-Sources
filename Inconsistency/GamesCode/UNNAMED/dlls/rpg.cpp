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
#if !defined( OEM_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"




enum rpg_e {
	RPG_IDLE = 0,
	RPG_DRAW,
	RPG_FIRE
};

LINK_ENTITY_TO_CLASS(weapon_rpg, CRpg);

#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS(laser_spot, CLaserSpot);

//=========================================================
//=========================================================
CLaserSpot *CLaserSpot::CreateSpot(void)
{
	CLaserSpot *pSpot = GetClassPtr((CLaserSpot *)NULL);
	pSpot->Spawn();

	pSpot->pev->classname = MAKE_STRING("laser_spot");

	return pSpot;
}

//=========================================================
//=========================================================
void CLaserSpot::Spawn(void)
{
	Precache();
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;

	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/laserdot.spr");
	UTIL_SetOrigin(pev, pev->origin);
};

//=========================================================
// Suspend- make the laser sight invisible. 
//=========================================================
void CLaserSpot::Suspend(float flSuspendTime)
{
	pev->effects |= EF_NODRAW;

	SetThink(&CLaserSpot::Revive);
	pev->nextthink = gpGlobals->time + flSuspendTime;
}

//=========================================================
// Revive - bring a suspended laser sight back.
//=========================================================
void CLaserSpot::Revive(void)
{
	pev->effects &= ~EF_NODRAW;

	SetThink(NULL);
}

void CLaserSpot::Precache(void)
{
	PRECACHE_MODEL("sprites/laserdot.spr");
};

LINK_ENTITY_TO_CLASS(rpg_rocket, CRpgRocket);

//=========================================================
//=========================================================
CRpgRocket *CRpgRocket::CreateRpgRocket(Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRpg *pLauncher)
{
	CRpgRocket *pRocket = GetClassPtr((CRpgRocket *)NULL);

	UTIL_SetOrigin(pRocket->pev, vecOrigin);
	pRocket->pev->angles = vecAngles;
	pRocket->Spawn();
	pRocket->SetTouch(&CRpgRocket::RocketTouch);
	pRocket->m_pLauncher = pLauncher;// remember what RPG fired me. 
	//pRocket->m_pLauncher->m_cActiveRockets++;// register this missile as active for the launcher
	pRocket->pev->owner = pOwner->edict();

	return pRocket;
}

//=========================================================
//=========================================================
void CRpgRocket::Spawn(void)
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_BOUNCEMISSILE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/rpgrocket.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin(pev, pev->origin);

	pev->classname = MAKE_STRING("rpg_rocket");

	SetThink(&CRpgRocket::IgniteThink);
	SetTouch(&CRpgRocket::ExplodeTouch);

	//pev->angles.x -= 30;
	//pev->angles.x = -(pev->angles.x + 30);

	///// UTIL_MakeVectors(pev->angles);

	pev->velocity = gpGlobals->v_forward * 1000;
	//pev->gravity = 0.0;

	pev->nextthink = gpGlobals->time;

	pev->dmg = gSkillData.plrDmgRPG;
	pev->angles = UTIL_VecToAngles(pev->velocity);
}

//=========================================================
//=========================================================
void CRpgRocket::RocketTouch(CBaseEntity *pOther)
{
	if (m_pLauncher)
	{
		// my launcher is still around, tell it I'm dead.
		m_pLauncher->m_cActiveRockets--;
	}
	STOP_SOUND(edict(), CHAN_VOICE, "weapons/rocket1.wav");
	ExplodeTouch(pOther);
}

//=========================================================
//=========================================================
void CRpgRocket::Precache(void)
{
	PRECACHE_MODEL("models/rpgrocket.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND("weapons/rocket1.wav");
}


void CRpgRocket::IgniteThink(void)
{
	// pev->movetype = MOVETYPE_TOSS;

	pev->movetype = MOVETYPE_FLY;
//	pev->effects |= EF_LIGHT;

	// make rocket sound
	//EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5);

	// rocket trail
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());	// entity
	WRITE_SHORT(m_iTrail);	// model
	WRITE_BYTE(8); // life
	WRITE_BYTE(3);  // width
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(200);	// brightness

	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	m_flIgniteTime = gpGlobals->time;

	// set to follow laser spot
	//SetThink( &CRpgRocket::FollowThink );
	//pev->nextthink = gpGlobals->time + 0.1;
}


void CRpgRocket::FollowThink(void)
{
	CBaseEntity *pOther = NULL;
	Vector vecTarget;
	Vector vecDir;
	float flDist, flMax, flDot;
	TraceResult tr;

	UTIL_MakeAimVectors(pev->angles);

	vecTarget = gpGlobals->v_forward;
	flMax = 4096;

	// Examine all entities within a reasonable radius
	while ((pOther = UTIL_FindEntityByClassname(pOther, "laser_spot")) != NULL)
	{
		UTIL_TraceLine(pev->origin, pOther->pev->origin, dont_ignore_monsters, ENT(pev), &tr);
		// ALERT( at_console, "%f\n", tr.flFraction );
		if (tr.flFraction >= 0.90)
		{
			vecDir = pOther->pev->origin - pev->origin;
			flDist = vecDir.Length();
			vecDir = vecDir.Normalize();
			flDot = DotProduct(gpGlobals->v_forward, vecDir);
			if ((flDot > 0) && (flDist * (1 - flDot) < flMax))
			{
				flMax = flDist * (1 - flDot);
				vecTarget = vecDir;
			}
		}
	}

	pev->angles = UTIL_VecToAngles(vecTarget);

	// this acceleration and turning math is totally wrong, but it seems to respond well so don't change it.
	float flSpeed = pev->velocity.Length();
	if (gpGlobals->time - m_flIgniteTime < 1.0)
	{
		pev->velocity = pev->velocity * 0.2 + vecTarget * (flSpeed * 0.8 + 400);
		if (pev->waterlevel == 3)
		{
			// go slow underwater
			if (pev->velocity.Length() > 300)
			{
				pev->velocity = pev->velocity.Normalize() * 300;
			}
			UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 4);
		}
		else
		{
			if (pev->velocity.Length() > 2000)
			{
				pev->velocity = pev->velocity.Normalize() * 2000;
			}
		}
	}
	else
	{
		if (pev->effects & EF_LIGHT)
		{
			pev->effects = 0;
			STOP_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav");
		}
		pev->velocity = pev->velocity * 0.2 + vecTarget * flSpeed * 0.798;
		if (pev->waterlevel == 0 && pev->velocity.Length() < 1500)
		{
			Detonate();
		}
	}
	// ALERT( at_console, "%.0f\n", flSpeed );

	pev->nextthink = gpGlobals->time + 0.1;
}
#endif



void CRpg::Reload(void)
{
}

void CRpg::Spawn()
{
	Precache();
	m_iId = WEAPON_RPG;

	SET_MODEL(ENT(pev), "models/w_rpg.mdl");
	m_fSpotActive = 1;

	m_iDefaultAmmo = RPG_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CRpg::Precache(void)
{
	PRECACHE_MODEL("models/w_rpg.mdl");
	PRECACHE_MODEL("models/v_rpg.mdl");
	PRECACHE_MODEL("models/p_rpg.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	//UTIL_PrecacheOther( "laser_spot" );
	UTIL_PrecacheOther("rpg_rocket");

	PRECACHE_SOUND("weapons/rpg_fire.wav");
	PRECACHE_SOUND("weapons/glauncher.wav"); // alternative fire sound

	m_usRpg = PRECACHE_EVENT(1, "events/rpg.sc");
}


int CRpg::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "rockets";
	p->iMaxAmmo1 = ROCKET_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_RPG;
	p->iFlags = 0;
	p->iWeight = RPG_WEIGHT;

	return 1;
}

int CRpg::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CRpg::Deploy()
{
//	if (m_iClip == 0)
//	{
//		return DefaultDeploy("models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW_UL, "rpg");
//	}
	return DefaultDeploy("models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW, "gauss");
}


BOOL CRpg::CanHolster(void)
{
	/*if ( m_fSpotActive && m_cActiveRockets )
	{
	// can't put away while guiding a missile.
	return FALSE;
	}*/

	return TRUE;
}

void CRpg::Holster(int skiplocal /* = 0 */)
{
	m_fInReload = FALSE;// cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(RPG_IDLE);

#ifndef CLIENT_DLL
	if (m_pSpot)
	{
		m_pSpot->Killed(NULL, GIB_NEVER);
		m_pSpot = NULL;
	}
#endif

}



void CRpg::PrimaryAttack()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0)
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

		CRpgRocket *pRocket = CRpgRocket::CreateRpgRocket(vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);// RpgRocket::Create stomps on globals, so remake.
		//pRocket->pev->velocity = /*pRocket->pev->velocity + */gpGlobals->v_forward /* * DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward )*/;
#endif

		// firing RPG no longer turns on the designator. ALT fire is a toggle switch for the LTD.
		// Ken signed up for this as a global change (sjb)

		int flags;
#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usRpg);

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;

#define ROF 0.66

		m_flNextPrimaryAttack = GetNextAttackDelay(ROF);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ROF;
	}
	else
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = GetNextAttackDelay(ROF);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ROF;
	}
	//UpdateSpot( );
}


void CRpg::SecondaryAttack()
{
	/*
	m_fSpotActive = ! m_fSpotActive;

	#ifndef CLIENT_DLL
	if (!m_fSpotActive && m_pSpot)
	{
	m_pSpot->Killed( NULL, GIB_NORMAL );
	m_pSpot = NULL;
	}
	#endif

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
	*/
	Kick();
}


void CRpg::WeaponIdle(void)
{
	//UpdateSpot( );

	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{

		SendWeaponAnim(RPG_IDLE);
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
	}
}



void CRpg::UpdateSpot(void)
{

#ifndef CLIENT_DLL
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CLaserSpot::CreateSpot();
		}

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();;
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		UTIL_SetOrigin(m_pSpot->pev, tr.vecEndPos);
	}
#endif

}


class CRpgAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_rpgammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_rpgammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity *pOther)
	{
		int iGive;

		iGive = AMMO_RPGCLIP_GIVE;

		if (pOther->GiveAmmo(iGive, "rockets", ROCKET_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			pOther->pev->iuser4 = T_PICKEDUPAMMO;
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS(ammo_rpgclip, CRpgAmmo);
LINK_ENTITY_TO_CLASS(ammo_crossbow, CRpgAmmo);























enum devastator_e {
	DEVASTATOR_IDLE = 0,
	DEVASTATOR_DRAW,
	DEVASTATOR_FIRELEFT,
	DEVASTATOR_FIRERIGHT
};

#ifndef CLIENT_DLL

//=========================================================
//=========================================================
CDevastatorRocket *CDevastatorRocket::CreateDevastatorRocket(Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CDevastator *pLauncher)
{
	CDevastatorRocket *pRocket = GetClassPtr((CDevastatorRocket *)NULL);

	UTIL_SetOrigin(pRocket->pev, vecOrigin);
	pRocket->pev->angles = vecAngles;
	pRocket->Spawn();
	pRocket->SetTouch(&CDevastatorRocket::RocketTouch);
	pRocket->m_pLauncher = pLauncher;// remember what DEVASTATOR fired me. 
	//pRocket->m_pLauncher->m_cActiveRockets++;// register this missile as active for the launcher
	pRocket->pev->owner = pOwner->edict();

	return pRocket;
}

//=========================================================
//=========================================================
void CDevastatorRocket::Spawn(void)
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_BOUNCEMISSILE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/rpgrocket.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin(pev, pev->origin);

	pev->classname = MAKE_STRING("devastator_rocket");

	SetThink(&CDevastatorRocket::IgniteThink);
	SetTouch(&CDevastatorRocket::ExplodeTouch);

	//pev->angles.x -= 30;
	//pev->angles.x = -(pev->angles.x + 30);

	///// UTIL_MakeVectors(pev->angles);

	pev->velocity = gpGlobals->v_forward * 1000;
	//pev->gravity = 0.0;

	pev->nextthink = gpGlobals->time;

	pev->dmg = gSkillData.plrDmgGauss;
	pev->angles = UTIL_VecToAngles(pev->velocity);
}

//=========================================================
//=========================================================
void CDevastatorRocket::RocketTouch(CBaseEntity *pOther)
{
	STOP_SOUND(edict(), CHAN_VOICE, "weapons/rocket1.wav");
	SetThink(&CDevastatorRocket::ExplodeThink);
	pev->nextthink = gpGlobals->time;
}


void CDevastatorRocket::RocketThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->waterlevel == 0)
		return;

	UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);
}

void CDevastatorRocket::ExplodeThink(void)
{
	int iContents = UTIL_PointContents(pev->origin);
	int iScale;

	iScale = 10;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	if (iContents != CONTENTS_WATER)
	{
		WRITE_SHORT(g_sModelIndexFireball);
	}
	else
	{
		WRITE_SHORT(g_sModelIndexWExplosion);
	}
	WRITE_BYTE(iScale); // scale * 10
	WRITE_BYTE(15); // framerate
	WRITE_BYTE(TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();

	entvars_t *pevOwner;

	if (pev->owner)
		pevOwner = VARS(pev->owner);
	else
		pevOwner = NULL;

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/explode3.wav", 0.75, 1.05);

	pev->owner = NULL; // can't traceline attack owner if this is set

	::RadiusDamage(pev->origin, pev, pevOwner, pev->dmg, 128, CLASS_NONE, DMG_BLAST | DMG_ALWAYSGIB);

	UTIL_Remove(this);
}

//=========================================================
//=========================================================
void CDevastatorRocket::Precache(void)
{
	PRECACHE_MODEL("models/rpgrocket.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND("weapons/rocket1.wav");
}


void CDevastatorRocket::IgniteThink(void)
{
	// pev->movetype = MOVETYPE_TOSS;

	pev->movetype = MOVETYPE_FLY;
	//	pev->effects |= EF_LIGHT;

	// make rocket sound
	//EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5);

	// rocket trail
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());	// entity
	WRITE_SHORT(m_iTrail);	// model
	WRITE_BYTE(2); // life
	WRITE_BYTE(2);  // width
	WRITE_BYTE(100);   // r, g, b
	WRITE_BYTE(100);   // r, g, b
	WRITE_BYTE(100);   // r, g, b
	WRITE_BYTE(255);	// brightness

	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	m_flIgniteTime = gpGlobals->time;

	if (pev->waterlevel == 0)
		return;

	UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);
}

void CDevastatorRocket::FollowThink(void)
{
	CBaseEntity *pOther = NULL;
	Vector vecTarget;
	Vector vecDir;
	float flDist, flMax, flDot;
	TraceResult tr;

	UTIL_MakeAimVectors(pev->angles);

	vecTarget = gpGlobals->v_forward;
	flMax = 4096;

	// Examine all entities within a reasonable radius
	while ((pOther = UTIL_FindEntityByClassname(pOther, "laser_spot")) != NULL)
	{
		UTIL_TraceLine(pev->origin, pOther->pev->origin, dont_ignore_monsters, ENT(pev), &tr);
		// ALERT( at_console, "%f\n", tr.flFraction );
		if (tr.flFraction >= 0.90)
		{
			vecDir = pOther->pev->origin - pev->origin;
			flDist = vecDir.Length();
			vecDir = vecDir.Normalize();
			flDot = DotProduct(gpGlobals->v_forward, vecDir);
			if ((flDot > 0) && (flDist * (1 - flDot) < flMax))
			{
				flMax = flDist * (1 - flDot);
				vecTarget = vecDir;
			}
		}
	}

	pev->angles = UTIL_VecToAngles(vecTarget);

	// this acceleration and turning math is totally wrong, but it seems to respond well so don't change it.
	float flSpeed = pev->velocity.Length();
	if (gpGlobals->time - m_flIgniteTime < 1.0)
	{
		pev->velocity = pev->velocity * 0.2 + vecTarget * (flSpeed * 0.8 + 400);
		if (pev->waterlevel == 3)
		{
			// go slow underwater
			if (pev->velocity.Length() > 300)
			{
				pev->velocity = pev->velocity.Normalize() * 300;
			}
			UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 4);
		}
		else
		{
			if (pev->velocity.Length() > 2000)
			{
				pev->velocity = pev->velocity.Normalize() * 2000;
			}
		}
	}
	else
	{
		if (pev->effects & EF_LIGHT)
		{
			pev->effects = 0;
			STOP_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav");
		}
		pev->velocity = pev->velocity * 0.2 + vecTarget * flSpeed * 0.798;
		if (pev->waterlevel == 0 && pev->velocity.Length() < 1500)
		{
			Detonate();
		}
	}
	// ALERT( at_console, "%.0f\n", flSpeed );

	pev->nextthink = gpGlobals->time + 0.1;
}
#endif



void CDevastator::Reload(void)
{
}

void CDevastator::Spawn()
{
	Precache();
	m_iId = WEAPON_DEVASTATOR;

	SET_MODEL(ENT(pev), "models/w_devastator.mdl");

	m_iDefaultAmmo = DEVASTATOR_DEFAULT_GIVE;
	iCurrentRocket = 0;

	FallInit();// get ready to fall down.
}

void CDevastator::Precache(void)
{
	PRECACHE_MODEL("models/w_devastator.mdl");
	PRECACHE_MODEL("models/v_devastator.mdl");
	PRECACHE_MODEL("models/p_devastator.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	//UTIL_PrecacheOther( "laser_spot" );
//	UTIL_PrecacheOther("devastator_rocket");
	PRECACHE_SOUND("weapons/explode3.wav");

	PRECACHE_SOUND("weapons/rpg_fire.wav");
	PRECACHE_SOUND("weapons/glauncher.wav"); // alternative fire sound

	m_usRpg = PRECACHE_EVENT(1, "events/deva.sc");
	m_usRpg2 = PRECACHE_EVENT(1, "events/deva2.sc");
}

int CDevastator::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "devastator_rockets";
	p->iMaxAmmo1 = DEVASTATOR_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 7;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_DEVASTATOR;
	p->iFlags = 0;
	p->iWeight = DEVASTATOR_WEIGHT;

	return 1;
}

int CDevastator::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CDevastator::Deploy()
{
	//	if (m_iClip == 0)
	//	{
	//		return DefaultDeploy("models/v_devastator.mdl", "models/p_devastator.mdl", DEVASTATOR_DRAW_UL, "devastator");
	//	}

	return DefaultDeploy("models/v_devastator.mdl", "models/p_devastator.mdl", DEVASTATOR_DRAW, "trip");
}


BOOL CDevastator::CanHolster(void)
{
	/*if ( m_fSpotActive && m_cActiveRockets )
	{
	// can't put away while guiding a missile.
	return FALSE;
	}*/

	return TRUE;
}

void CDevastator::Holster(int skiplocal /* = 0 */)
{
	m_fInReload = FALSE;// cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(DEVASTATOR_IDLE);
}



void CDevastator::PrimaryAttack()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0)
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		// player "shoot" animation
		//m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + Vector(RANDOM_FLOAT(-0.7, 0.7), RANDOM_FLOAT(-0.7, 0.7), RANDOM_FLOAT(-0.7, 0.7)));

		Vector vecSrc;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] % 4 == 0 || m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] % 4 == 3)
			vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
		else
			vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * -8 + gpGlobals->v_up * -8;

		CDevastatorRocket *pRocket = CDevastatorRocket::CreateDevastatorRocket(vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this);

		//UTIL_MakeVectors(m_pPlayer->pev->v_angle); // DevastatorRocket::Create stomps on globals, so remake.
		//pRocket->pev->velocity = /*pRocket->pev->velocity + */gpGlobals->v_forward /* * DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward )*/;
#endif

		// firing DEVASTATOR no longer turns on the designator. ALT fire is a toggle switch for the LTD.
		// Ken signed up for this as a global change (sjb)

		int flags;
#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif


		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] % 4 == 0 || m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] % 4 == 3)
		{
			PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usRpg2);
		}
		else
		{
			PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usRpg);
		}


		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;

		if (iCurrentRocket >= 2)
			iCurrentRocket = 0;
		else
			iCurrentRocket += 1;

	}
	else
	{
		PlayEmptySound();
	}

	float rof;
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] % 4 == 2 || m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] % 4 == 0)
		rof = 0.15;
	else
		rof = 0.066;

	m_flNextPrimaryAttack = GetNextAttackDelay(rof);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.35;
}

BOOL CDevastator::PlayEmptySound(void)
{
	if (m_iPlayEmptySound)
	{
		SendWeaponAnim(DEVASTATOR_IDLE);
		pev->body = 0;
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
		m_iPlayEmptySound = 0;
		return 0;
	}
	return 0;
}

void CDevastator::SecondaryAttack()
{
	/*
	m_fSpotActive = ! m_fSpotActive;

	#ifndef CLIENT_DLL
	if (!m_fSpotActive && m_pSpot)
	{
	m_pSpot->Killed( NULL, GIB_NORMAL );
	m_pSpot = NULL;
	}
	#endif

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
	*/
	Kick();
}


void CDevastator::WeaponIdle(void)
{
	//UpdateSpot( );

	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{

		SendWeaponAnim(DEVASTATOR_IDLE);
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
	}
}

//LINK_ENTITY_TO_CLASS(weapon_devastator, CDevastator);

class CDevastatorAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_gaussammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_gaussammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity *pOther)
	{
		int iGive;

		iGive = AMMO_DEVASTATORCLIP_GIVE;

		if (pOther->GiveAmmo(iGive, "devastator_rockets", DEVASTATOR_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			pOther->pev->iuser4 = T_PICKEDUPAMMO;
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(ammo_devastatorclip, CDevastatorAmmo);
LINK_ENTITY_TO_CLASS(ammo_egonclip, CDevastatorAmmo);
LINK_ENTITY_TO_CLASS(ammo_gaussclip, CDevastatorAmmo);

#endif

