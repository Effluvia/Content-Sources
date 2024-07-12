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
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"


#define	KNIFE_BODYHIT_VOLUME 128
#define	KNIFE_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS( weapon_knife, CKnife );



enum knife_e {
	KNIFE_IDLE = 0,
	KNIFE_DRAW,
	KNIFE_HOLSTER,
	KNIFE_SLASH1,
	KNIFE_SLASH2
};


void CKnife::Spawn( )
{
	Precache( );
	m_iId = WEAPON_KNIFE;
	SET_MODEL(ENT(pev), "models/w_kitchenknife.mdl");
	m_iClip = -1;

	FallInit();// get ready to fall down.
	
#ifndef CLIENT_DLL
    ClearEntitiesHitThisSwing();
#endif
}


void CKnife::Precache( void )
{
	PRECACHE_MODEL("models/v_kitchenknife.mdl");
	PRECACHE_MODEL("models/w_kitchenknife.mdl");
	PRECACHE_MODEL("models/p_crowbar.mdl");
	PRECACHE_SOUND("weapons/knife_hit1.wav");
	PRECACHE_SOUND("weapons/knife_hit2.wav");
	PRECACHE_SOUND("weapons/knife_wall1.wav");
	PRECACHE_SOUND("weapons/knife_wall2.wav");
	PRECACHE_SOUND("weapons/knife_swing1.wav");

	m_usKnife = PRECACHE_EVENT ( 1, "events/knife.sc" );
}

int CKnife::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iWeight = KNIFE_WEIGHT;
	return 1;
}



BOOL CKnife::Deploy( )
{
	return DefaultDeploy( "models/v_kitchenknife.mdl", "models/p_crowbar.mdl", KNIFE_DRAW, "Knife" );
}

void CKnife::Holster( int skiplocal /* = 0 */ )
{
	KillLaser();
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim( KNIFE_HOLSTER );
}

#define KNIFE_MIN_SWING_SPEED 30
#define KNIFE_LENGTH 15

void CKnife::ItemPostFrame()
{
    MakeLaser();
#ifndef CLIENT_DLL
    Vector weaponVelocity = m_pPlayer->GetWeaponVelocity();
	float speed = weaponVelocity.Length();
	if (speed >= KNIFE_MIN_SWING_SPEED)
	{
		CheckSmack(speed);
	}
	else
	{
		hitCount = 0;
		ClearEntitiesHitThisSwing();
	}
#endif
}

//#define SHOW_KNIFE_DAMAGE_LINE;

void CKnife::MakeLaser( void )
{

#ifndef CLIENT_DLL

    //This is for debugging the knife
#ifdef SHOW_KNIFE_DAMAGE_LINE
	TraceResult tr;

	// ALERT( at_console, "serverflags %f\n", gpGlobals->serverflags );

	UTIL_MakeVectors (m_pPlayer->GetWeaponViewAngles());
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd	= vecSrc + gpGlobals->v_up * KNIFE_LENGTH;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( pev ), &tr );

	float flBeamLength = tr.flFraction;

	if (!g_pLaser || !(g_pLaser->pev)) {
		g_pLaser = CBeam::BeamCreate(g_pModelNameLaser, 3);
	}

	g_pLaser->PointsInit( vecSrc, vecEnd );
	g_pLaser->SetColor( 214, 34, 34 );
	g_pLaser->SetScrollRate( 255 );
	g_pLaser->SetBrightness( 96 );
	g_pLaser->pev->spawnflags |= SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
	g_pLaser->pev->owner = m_pPlayer->edict();
#else
	//Normally the knife doesn't have a laser sight
	KillLaser();
#endif //SHOW_KNIFE_DAMAGE_LINE

#endif
}

#ifndef CLIENT_DLL
void CKnife::CheckSmack(float speed)
	{
	UTIL_MakeVectors (m_pPlayer->GetWeaponViewAngles());
	Vector vecSrc	= m_pPlayer->GetGunPosition();
	Vector vecEnd	= vecSrc + gpGlobals->v_up * KNIFE_LENGTH;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (!tr.fStartSolid && !tr.fAllSolid && tr.flFraction < 1.0)	// we hit something!
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		if (pEntity && HasNotHitThisEntityThisSwing(pEntity))
		{
			RememberHasHitThisEntityThisSwing(pEntity);

		// play thwack, smack, or dong sound
                float flVol = 1.0;

			hitCount++;

			ClearMultiDamage();

			//It's a knife, nasty sharp edges, it doesn't have to be swung very hard to inflict full damage but it shouldn't inflight more than
			//its full damage
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgKnife * (1.f / hitCount), gpGlobals->v_up, &tr, DMG_CLUB);
			ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

			if( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				// play thwack or smack sound
				switch( RANDOM_LONG( 0, 1 ) )
				{
				case 0:
					EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/knife_hit1.wav", 1, ATTN_NORM); break;
				case 1:
					EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/knife_hit2.wav", 1, ATTN_NORM); break;
				}
				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
				if (pEntity->IsAlive())
				{
					flVol = 0.25;
				}
				else
				{
					return; // why?
			}
		}
			else
		{
			float fvolbar = TEXTURETYPE_PlaySound( &tr, vecSrc, vecSrc + ( vecEnd - vecSrc ) * 2, BULLET_PLAYER_CROWBAR );

			if( g_pGameRules->IsMultiplayer() )
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play knife strike
			switch( RANDOM_LONG( 0, 1 ) )
			{
			case 0:
				EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hitwall1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) );
				break;
			case 1:
				EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hitwall2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) );
				break;
			}
		}

			//vibrate a bit
			char buffer[256];
			sprintf(buffer, "vibrate 90.0 %i 1.0\n", 1-(int)CVAR_GET_FLOAT("hand"));
			SERVER_COMMAND(buffer);

			m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;
			DecalGunshot(&tr, BULLET_PLAYER_CROWBAR);
		}
	}
}

bool CKnife::HasNotHitThisEntityThisSwing(CBaseEntity *pEntity)
{
	int hitEntitiesCount = sizeof(hitEntities) / sizeof(EHANDLE);
	for (int i = 0; i < hitEntitiesCount; i++)
	{
		if (((CBaseEntity*)hitEntities[i]) == pEntity)
		{
			return false;
		}
	}
	return true;
}

 void CKnife::RememberHasHitThisEntityThisSwing(CBaseEntity *pEntity)
{
	int hitEntitiesCount = sizeof(hitEntities) / sizeof(EHANDLE);
	for (int i = 0; i < hitEntitiesCount; i++)
		{
		if (hitEntities[i])
			{
			continue;
			}
			else
			{
			hitEntities[i] = pEntity;
			return;
			}
		}
}

 void CKnife::ClearEntitiesHitThisSwing()
{
	int hitEntitiesCount = sizeof(hitEntities) / sizeof(EHANDLE);
	for (int i = 0; i < hitEntitiesCount; i++)
	{
		hitEntities[i] = NULL;
	}
}

#endif