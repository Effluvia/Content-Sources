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

===== h_cycler.cpp ========================================================

  The Halflife Cycler Monsters
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"
#include "util_model.h"
#include "weapons.h"
#include "player.h"


#define TEMP_FOR_SCREEN_SHOTS
#ifdef TEMP_FOR_SCREEN_SHOTS //===================================================

class CCycler : public CBaseMonster
{
public:
	void GenericCyclerSpawn(char *szModel, Vector vecMin, Vector vecMax);
	virtual int ObjectCaps( void ) { return (CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE); }

	//MODDD NOTE - This is the one that had child classes but didn't make these methods virtual? At least they didn't override the method.
	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL

	void Spawn( void );
	void Think( void );
	//void Pain( float flDamage );
	void Use ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual BOOL IsWorldAffiliated(void);

	// Don't treat as a live target
	virtual BOOL IsAlive( void ) { return FALSE; }

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	int m_animate;
};

TYPEDESCRIPTION	CCycler::m_SaveData[] = 
{
	DEFINE_FIELD( CCycler, m_animate, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CCycler, CBaseMonster );


//
// we should get rid of all the other cyclers and replace them with this.
//
class CGenericCycler : public CCycler
{
public:
	void Spawn( void ) { GenericCyclerSpawn( (char *)STRING(pev->model), Vector(-16, -16, 0), Vector(16, 16, 72) ); }
};
LINK_ENTITY_TO_CLASS( cycler, CGenericCycler );



// Probe droid imported for tech demo compatibility
//
// PROBE DROID
//
class CCyclerProbe : public CCycler
{
public:	
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS( cycler_prdroid, CCyclerProbe );
void CCyclerProbe::Spawn( void )
{
	pev->origin = pev->origin + Vector ( 0, 0, 16 );
	GenericCyclerSpawn( "models/prdroid.mdl", Vector(-16,-16,-16), Vector(16,16,16));
}



// Cycler member functions

void CCycler::GenericCyclerSpawn(char *szModel, Vector vecMin, Vector vecMax)
{
	if (!szModel || !*szModel)
	{
		ALERT(at_error, "cycler at %.0f %.0f %0.f missing modelname", pev->origin.x, pev->origin.y, pev->origin.z );
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	pev->classname		= MAKE_STRING("cycler");
	PRECACHE_MODEL( szModel );
	setModel(szModel);

	CCycler::Spawn( );

	UTIL_SetSize(pev, vecMin, vecMax);
}


void CCycler::Spawn( )
{
	InitBoneControllers();
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_NONE;
	pev->takedamage		= DAMAGE_YES;
	pev->effects		= 0;
	pev->health			= 80000;// no cycler should die
	pev->yaw_speed		= 5;
	pev->ideal_yaw		= pev->angles.y;
	ChangeYaw( 360 );
	
	m_flFrameRate		= 75;
	m_flGroundSpeed		= 0;

	pev->nextthink		+= 1.0;

	ResetSequenceInfo( );

	if (pev->sequence != 0 || pev->frame != 0)
	{
		m_animate = 0;
		pev->framerate = 0;
	}
	else
	{
		m_animate = 1;
	}
}


//
// cycler think
//
void CCycler::Think( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_animate)
	{
		StudioFrameAdvance_SIMPLE( );
	}
	if (m_fSequenceFinished && !m_fSequenceLoops)
	{
		// ResetSequenceInfo();
		// hack to avoid reloading model every frame
		pev->animtime = gpGlobals->time;
		pev->framerate = 1.0;
		m_fSequenceFinished = FALSE;
		m_flLastEventCheck = gpGlobals->time;
		pev->frame = 0;
		if (!m_animate)
			pev->framerate = 0.0;	// FIX: don't reset framerate
	}
}

//
// CyclerUse - starts a rotation trend
//
void CCycler::Use ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_animate = !m_animate;
	if (m_animate)
		pev->framerate = 1.0;
	else
		pev->framerate = 0.0;
}

BOOL CCycler::IsWorldAffiliated(){
    return TRUE; //some animating thing? sounds fitting.
}

//
// CyclerPain , changes sequences when shot
//
//void CCycler::Pain( float flDamage )


GENERATE_TRACEATTACK_IMPLEMENTATION(CCycler)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CCycler)
{
	if (m_animate)
	{
		pev->sequence++;

		ResetSequenceInfo( );

		if (m_flFrameRate == 0.0)
		{
			pev->sequence = 0;
			ResetSequenceInfo( );
		}
		pev->frame = 0;
	}
	else
	{
		pev->framerate = 1.0;
		StudioFrameAdvance ( 0.1 );
		pev->framerate = 0;
		ALERT( at_console, "sequence: %d, frame %.0f\n", pev->sequence, pev->frame );
	}

	return 0;
}

#endif  //END OF TEMP_FOR_SCREEN_SHOTS check.    What.


class CCyclerSprite : public CBaseEntity
{
public:
	void Spawn( void );
	void Think( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int ObjectCaps( void ) { return (CBaseEntity::ObjectCaps() | FCAP_DONT_SAVE | FCAP_IMPULSE_USE); }
 
	//mODDD - extra damage bitmask added.
	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL
 
	void Animate( float frames );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	inline int	ShouldAnimate( void ) { return m_animate && m_maxFrame > 1.0; }
	int		m_animate;
	float	m_lastTime;
	float	m_maxFrame;
};

LINK_ENTITY_TO_CLASS( cycler_sprite, CCyclerSprite );

TYPEDESCRIPTION	CCyclerSprite::m_SaveData[] = 
{
	DEFINE_FIELD( CCyclerSprite, m_animate, FIELD_INTEGER ),
	DEFINE_FIELD( CCyclerSprite, m_lastTime, FIELD_TIME ),
	DEFINE_FIELD( CCyclerSprite, m_maxFrame, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CCyclerSprite, CBaseEntity );


void CCyclerSprite::Spawn( void )
{
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_NONE;
	pev->takedamage		= DAMAGE_YES;
	pev->effects		= 0;

	pev->frame			= 0;
	pev->nextthink		= gpGlobals->time + 0.1;
	m_animate			= 1;
	m_lastTime			= gpGlobals->time;

	PRECACHE_MODEL( (char *)STRING(pev->model) );
	setModel(STRING(pev->model) );

	m_maxFrame = (float) MODEL_FRAMES( pev->modelindex ) - 1;
}


void CCyclerSprite::Think( void )
{
	if ( ShouldAnimate() )
		Animate( pev->framerate * (gpGlobals->time - m_lastTime) );

	pev->nextthink		= gpGlobals->time + 0.1;
	m_lastTime = gpGlobals->time;
}


void CCyclerSprite::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_animate = !m_animate;
	ALERT( at_console, "Sprite: %s\n", STRING(pev->model) );
}


GENERATE_TRACEATTACK_IMPLEMENTATION(CCyclerSprite)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseEntity);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CCyclerSprite)
{
	if ( m_maxFrame > 1.0 )
	{
		Animate( 1.0 );
	}
	return 1;
}

void CCyclerSprite::Animate( float frames )
{ 
	pev->frame += frames;
	if ( m_maxFrame > 0 )
		pev->frame = fmod( pev->frame, m_maxFrame );
}



class CWeaponCycler : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	int iItemSlot( void ) { return 1; }
	int GetItemInfo(ItemInfo *p) {return 0; }

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iszModel;
	int m_iModel;
};
LINK_ENTITY_TO_CLASS( cycler_weapon, CWeaponCycler );


void CWeaponCycler::Spawn( )
{
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_NONE;

	PRECACHE_MODEL( (char *)STRING(pev->model) );
	setModel(STRING(pev->model) );
	m_iszModel = pev->model;
	m_iModel = pev->modelindex;

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch( &CBasePlayerItem::DefaultTouch );
}



BOOL CWeaponCycler::Deploy( )
{
	m_pPlayer->pev->viewmodel = m_iszModel;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	SendWeaponAnim( 0 );
	m_iClip = 0;
	return TRUE;
}


void CWeaponCycler::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
}


void CWeaponCycler::PrimaryAttack()
{

	SendWeaponAnim( pev->sequence );

	m_flNextPrimaryAttack = gpGlobals->time + 0.3;
}


void CWeaponCycler::SecondaryAttack( void )
{
	float flFrameRate, flGroundSpeed;

	pev->sequence = (pev->sequence + 1) % 8;

	pev->modelindex = m_iModel;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	GetSequenceInfo( pmodel, pev, &flFrameRate, &flGroundSpeed );
	pev->modelindex = 0;

	if (flFrameRate == 0.0)
	{
		pev->sequence = 0;
	}

	SendWeaponAnim( pev->sequence );

	m_flNextSecondaryAttack = gpGlobals->time + 0.3;
}



// Flaming Wreakage
class CWreckage : public CBaseMonster
{
	int	Save( CSave &save );
	int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn( void );
	void Precache( void );
	void Think( void );

	int m_flStartTime;
};
TYPEDESCRIPTION	CWreckage::m_SaveData[] = 
{
	DEFINE_FIELD( CWreckage, m_flStartTime, FIELD_TIME ),
};
IMPLEMENT_SAVERESTORE( CWreckage, CBaseMonster );


LINK_ENTITY_TO_CLASS( cycler_wreckage, CWreckage );

void CWreckage::Spawn( void )
{
	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;
	pev->takedamage		= 0;
	pev->effects		= 0;

	pev->frame			= 0;
	pev->nextthink		= gpGlobals->time + 0.1;

	if (pev->model)
	{
		PRECACHE_MODEL( (char *)STRING(pev->model) );
		setModel(STRING(pev->model) );
	}
	// pev->scale = 5.0;

	m_flStartTime		= gpGlobals->time;
}

void CWreckage::Precache( )
{
	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING(pev->model) );
}

void CWreckage::Think( void )
{
	StudioFrameAdvance_SIMPLE( );
	pev->nextthink = gpGlobals->time + 0.2;

	if (pev->dmgtime)
	{
		if (pev->dmgtime < gpGlobals->time)
		{
			UTIL_Remove( this );
			return;
		}
		else if (RANDOM_FLOAT( 0, pev->dmgtime - m_flStartTime ) > pev->dmgtime - gpGlobals->time)
		{
			return;
		}
	}
	
	Vector VecSrc;
	
	VecSrc.x = RANDOM_FLOAT( pev->absmin.x, pev->absmax.x );
	VecSrc.y = RANDOM_FLOAT( pev->absmin.y, pev->absmax.y );
	VecSrc.z = RANDOM_FLOAT( pev->absmin.z, pev->absmax.z );

	UTIL_Smoke(MSG_PVS, VecSrc, NULL, VecSrc, 0, 0, 0, g_sModelIndexSmoke, RANDOM_LONG(0,49) + 50, RANDOM_LONG(0, 3) + 8);

}
