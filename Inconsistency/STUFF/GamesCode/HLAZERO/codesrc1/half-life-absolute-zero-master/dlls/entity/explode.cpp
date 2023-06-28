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
/*

===== explode.cpp ========================================================

  Explosion-related code

*/
#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "basemonster.h"

#include "decals.h"
#include "explode.h"

//MODDD
EASY_CVAR_EXTERN_DEBUGONLY(sparksExplosionMulti)
EASY_CVAR_EXTERN(cl_explosion)

// Spark Shower
class CShower : public CBaseEntity
{
	void Spawn( void );
	void Think( void );
	void Touch( CBaseEntity *pOther );
	int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
};

LINK_ENTITY_TO_CLASS( spark_shower, CShower );

void CShower::Spawn( void )
{
	pev->velocity = RANDOM_FLOAT( 200, 300 ) * pev->angles;
	pev->velocity.x += RANDOM_FLOAT(-100.f,100.f);
	pev->velocity.y += RANDOM_FLOAT(-100.f,100.f);
	if ( pev->velocity.z >= 0 )
		pev->velocity.z += 200;
	else
		pev->velocity.z -= 200;
	pev->movetype = MOVETYPE_BOUNCE;
	pev->gravity = 0.5;
	pev->nextthink = gpGlobals->time + 0.1;
	pev->solid = SOLID_NOT;
	SET_MODEL( edict(), "models/grenade.mdl");	// Need a model, just use the grenade, we don't draw it anyway
	UTIL_SetSize(pev, g_vecZero, g_vecZero );
	pev->effects |= EF_NODRAW;
	pev->speed = RANDOM_FLOAT( 0.5, 1.5 );

	pev->angles = g_vecZero;
}


void CShower::Think( void )
{
	UTIL_Sparks( pev->origin, DEFAULT_SPARK_BALLS, EASY_CVAR_GET_DEBUGONLY(sparksExplosionMulti) );
	
	pev->speed -= 0.1;
	if ( pev->speed > 0 )
		pev->nextthink = gpGlobals->time + 0.1;
	else
		UTIL_Remove( this );
	pev->flags &= ~FL_ONGROUND;
}

void CShower::Touch( CBaseEntity *pOther )
{
	if ( pev->flags & FL_ONGROUND )
		pev->velocity = pev->velocity * 0.1;
	else
		pev->velocity = pev->velocity * 0.6;

	if ( (pev->velocity.x*pev->velocity.x+pev->velocity.y*pev->velocity.y) < 10.0 )
		pev->speed = 0;
}

class CEnvExplosion : public CBaseMonster
{
public:
    BOOL isOrganic(void);
	void Spawn( );
	void EXPORT Smoke ( void );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	int m_iMagnitude;// how large is the fireball? how much damage?
	int m_spriteScale; // what's the exact fireball sprite scale? 
};

TYPEDESCRIPTION	CEnvExplosion::m_SaveData[] = 
{
	DEFINE_FIELD( CEnvExplosion, m_iMagnitude, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvExplosion, m_spriteScale, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CEnvExplosion, CBaseMonster );
LINK_ENTITY_TO_CLASS( env_explosion, CEnvExplosion );

void CEnvExplosion::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "iMagnitude"))
	{
		m_iMagnitude = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

BOOL CEnvExplosion::isOrganic(void){
	return FALSE;
}

void CEnvExplosion::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;

	pev->movetype = MOVETYPE_NONE;
	/*
	if ( m_iMagnitude > 250 )
	{
		m_iMagnitude = 250;
	}
	*/

	float flSpriteScale;
	flSpriteScale = ( m_iMagnitude - 50) * 0.6;
	
	/*
	if ( flSpriteScale > 50 )
	{
		flSpriteScale = 50;
	}
	*/
	if ( flSpriteScale < 10 )
	{
		flSpriteScale = 10;
	}

	m_spriteScale = (int)flSpriteScale;
}

void CEnvExplosion::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	TraceResult tr;

	pev->model = iStringNull;//invisible
	pev->solid = SOLID_NOT;// intangible

	Vector		vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT(pev), & tr);
	
	// Pull out of the wall a bit
	if ( tr.flFraction != 1.0 )
	{
		pev->origin = tr.vecEndPos + (tr.vecPlaneNormal * (m_iMagnitude - 24) * 0.6);
	}
	else
	{
		pev->origin = pev->origin;
	}

	// draw decal
	if (! ( pev->spawnflags & SF_ENVEXPLOSION_NODECAL))
	{
		if ( RANDOM_FLOAT( 0 , 1 ) < 0.5 )
		{
			UTIL_DecalTrace( &tr, DECAL_SCORCH1 );
		}
		else
		{
			UTIL_DecalTrace( &tr, DECAL_SCORCH2 );
		}
	}

	// draw fireball
	//MODDD - either way, uses the new filter.
	if ( !( pev->spawnflags & SF_ENVEXPLOSION_NOFIREBALL ) )
	{
		UTIL_Explosion(MSG_PAS, pev->origin, NULL, pev, pev->origin, g_sModelIndexFireball, (BYTE) m_spriteScale, 15, TE_EXPLFLAG_NONE);
	}
	else
	{
		// no sprite (0 after g_sModelIndexFireball means that?)
		UTIL_Explosion(MSG_PAS, pev->origin, NULL, pev, pev->origin, g_sModelIndexFireball, 0, 15, TE_EXPLFLAG_NONE);
	}
	
	//return;
	// do damage
	if ( !( pev->spawnflags & SF_ENVEXPLOSION_NODAMAGE ) )
	{
		RadiusDamageAutoRadius ( pev, pev, m_iMagnitude, CLASS_NONE, DMG_BLAST );
	}

	SetThink( &CEnvExplosion::Smoke );
	pev->nextthink = gpGlobals->time + 0.3;

	//MODDD - only generate sparks if allowed.
	if(UTIL_getExplosionsHaveSparks()) {
		// draw sparks
		if ( !( pev->spawnflags & SF_ENVEXPLOSION_NOSPARKS ) )
		{
			int sparkCount = RANDOM_LONG(0,3);

			for ( int i = 0; i < sparkCount; i++ )
			{
				Create( "spark_shower", pev->origin, tr.vecPlaneNormal, NULL );
			}
		}
	}
	
}


void CEnvExplosion::Smoke( void )
{
	//MODDD - may need to still be around for players that have
	// a different cl_explosion value.  Although this CVar is no
	// longer serverside.
	/*
	if(EASY_CVAR_GET(cl_explosion) == 1){
		//does not smoke.
		UTIL_Remove( this );
		return;
	}
	*/

	if ( !( pev->spawnflags & SF_ENVEXPLOSION_NOSMOKE ) )
	{
		//MODDD - replace with call
		/*
		MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( (BYTE)m_spriteScale ); // scale * 10
			WRITE_BYTE( 12  ); // framerate
		MESSAGE_END();
		*/
		UTIL_ExplosionSmoke(MSG_PAS, pev->origin, NULL, pev->origin, 0, 0, 0, g_sModelIndexSmoke, (BYTE)m_spriteScale, 12);

	}
	
	if ( !(pev->spawnflags & SF_ENVEXPLOSION_REPEATABLE) )
	{
		UTIL_Remove( this );
	}
}



void ExplosionCreate(const Vector& center, edict_t* pOwner, int magnitude, BOOL doDamage) {
	ExplosionCreate(center, g_vecZero, pOwner, magnitude, doDamage, 0);
}
void ExplosionCreate(const Vector& center, const Vector& angles, edict_t* pOwner, int magnitude, BOOL doDamage) {
	ExplosionCreate(center, angles, pOwner, magnitude, doDamage, 0);
}
void ExplosionCreate(const Vector& center, edict_t* pOwner, int magnitude, BOOL doDamage, float startExplosionDelay) {
	ExplosionCreate(center, g_vecZero, pOwner, magnitude, doDamage, startExplosionDelay);
}

void ExplosionCreate(const Vector& center, const Vector& angles, edict_t* pOwner, int magnitude, BOOL doDamage, float startExplosionDelay) {
	KeyValueData kvd;
	char buf[128];

	CBaseEntity* pExplosion = CBaseEntity::Create("env_explosion", center, angles, pOwner);
	sprintf(buf, "%3d", magnitude);
	kvd.szKeyName = "iMagnitude";
	kvd.szValue = buf;
	pExplosion->KeyValue(&kvd);
	if (!doDamage)
		pExplosion->pev->spawnflags |= SF_ENVEXPLOSION_NODAMAGE;


	pExplosion->Spawn();
	if (startExplosionDelay <= 0) {
		// start instantly.
		pExplosion->Use(NULL, NULL, USE_TOGGLE, 0);
	}
	else {
		// delay
		pExplosion->SetThink(&CBaseEntity::SUB_CallUseToggle);
		pExplosion->pev->nextthink = gpGlobals->time + startExplosionDelay;
	}
}


