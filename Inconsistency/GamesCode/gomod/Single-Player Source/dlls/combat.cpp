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

===== combat.cpp ========================================================

  functions dealing with damage infliction & death

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "soundent.h"
#include "decals.h"
#include "animation.h"
#include "weapons.h"
#include "func_break.h"
#include "gamerules.h"

#include "player.h"

#include "particle_defs.h"
extern int gmsgParticles;

extern float r_colorx;
extern float r_colory;
extern float r_colorz;
extern float r_render;
extern float r_renderfx;
extern float r_solid_pos;

extern cvar_t remove_npcs;

char *monster_name;

extern DLL_GLOBAL Vector		g_vecAttackDir;
extern DLL_GLOBAL int			g_iSkillLevel;

extern Vector VecBModelOrigin( entvars_t* pevBModel );
extern entvars_t *g_pevLastInflictor;

#define GERMAN_GIB_COUNT		4
#define	HUMAN_GIB_COUNT			6
#define ALIEN_GIB_COUNT			4
int monster_type;

// HACKHACK -- The gib velocity equations don't work
void CGib :: LimitVelocity( void )
{
	float length = pev->velocity.Length();

	// ceiling at 1500.  The gib velocity equation is not bounded properly.  Rather than tune it
	// in 3 separate places again, I'll just limit it here.
	if ( length > 1500.0 )
		pev->velocity = pev->velocity.Normalize() * 1500;		// This should really be sv_maxvelocity * 0.75 or something
}


void CGib :: SpawnStickyGibs( entvars_t *pevVictim, Vector vecOrigin, int cGibs )
{
	int i;

	if ( g_Language == LANGUAGE_GERMAN )
	{
		// no sticky gibs in germany right now!
		return; 
	}

	for ( i = 0 ; i < cGibs ; i++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/stickygib.mdl" );
		pGib->pev->body = RANDOM_LONG(0,2);

		if ( pevVictim )
		{
			pGib->pev->origin.x = vecOrigin.x + RANDOM_FLOAT( -3, 3 );
			pGib->pev->origin.y = vecOrigin.y + RANDOM_FLOAT( -3, 3 );
			pGib->pev->origin.z = vecOrigin.z + RANDOM_FLOAT( -3, 3 );

			/*
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) );
			*/

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.15, 0.15 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.15, 0.15 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.15, 0.15 );

			pGib->pev->velocity = pGib->pev->velocity * 900;

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 250, 400 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 250, 400 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
		
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			
			pGib->pev->movetype = MOVETYPE_TOSS;
			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector ( 0, 0 ,0 ), Vector ( 0, 0, 0 ) );
			pGib->SetTouch ( StickyGibTouch );
			pGib->SetThink (NULL);
		}
		pGib->LimitVelocity();
	}
}

void CGib :: SpawnHeadGib( entvars_t *pevVictim )
{
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	if ( g_Language == LANGUAGE_GERMAN )
	{
		pGib->Spawn( "models/germangibs.mdl" );// throw one head
		pGib->pev->body = 0;
	}
	else
	{
		pGib->Spawn( "models/hgibs.mdl" );// throw one head
		pGib->pev->body = 0;
	}

	if ( pevVictim )
	{
		pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;
		
		edict_t		*pentPlayer = FIND_CLIENT_IN_PVS( pGib->edict() );
		
		if ( RANDOM_LONG ( 0, 100 ) <= 5 && pentPlayer )
		{
			// 5% chance head will be thrown at player's face.
			entvars_t	*pevPlayer;

			pevPlayer = VARS( pentPlayer );
			pGib->pev->velocity = ( ( pevPlayer->origin + pevPlayer->view_ofs ) - pGib->pev->origin ).Normalize() * 300;
			pGib->pev->velocity.z += 100;
		}
		else
		{
			pGib->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
		}


		pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
		pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

		// copy owner's blood color
		pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
	
		if ( pevVictim->health > -50)
		{
			pGib->pev->velocity = pGib->pev->velocity * 0.7;
		}
		else if ( pevVictim->health > -200)
		{
			pGib->pev->velocity = pGib->pev->velocity * 2;
		}
		else
		{
			pGib->pev->velocity = pGib->pev->velocity * 4;
		}
	}
	pGib->LimitVelocity();
}

void CGib :: SpawnRandomGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0 ; cSplat < cGibs ; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		if ( g_Language == LANGUAGE_GERMAN )
		{
			pGib->Spawn( "models/germangibs.mdl" );
			pGib->pev->body = RANDOM_LONG(0,GERMAN_GIB_COUNT-1);
		}
		else
		{
			if ( human )
			{
				// human pieces
				pGib->Spawn( "models/hgibs.mdl" );
				pGib->pev->body = RANDOM_LONG(1,HUMAN_GIB_COUNT-1);// start at one to avoid throwing random amounts of skulls (0th gib)
			}
			else
			{
				// aliens
				pGib->Spawn( "models/agibs.mdl" );
				pGib->pev->body = RANDOM_LONG(0,ALIEN_GIB_COUNT-1);
			}
		}

		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) + 1;	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT ( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT ( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT ( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();
			
			if ( pevVictim->health > -50)
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200)
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize ( pGib->pev, Vector( 0 , 0 , 0 ), Vector ( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}


BOOL CBaseMonster :: HasHumanGibs( void )
{
	int myClass = Classify();

	if ( myClass == CLASS_HUMAN_MILITARY ||
		 myClass == CLASS_PLAYER_ALLY	||
		 myClass == CLASS_HUMAN_PASSIVE  ||
		 myClass == CLASS_PLAYER )

		 return TRUE;

	return FALSE;
}


BOOL CBaseMonster :: HasAlienGibs( void )
{
	int myClass = Classify();

	if ( myClass == CLASS_ALIEN_MILITARY ||
		 myClass == CLASS_ALIEN_MONSTER	||
		 myClass == CLASS_ALIEN_PASSIVE  ||
		 myClass == CLASS_INSECT  ||
		 myClass == CLASS_ALIEN_PREDATOR  ||
		 myClass == CLASS_ALIEN_PREY )

		 return TRUE;

	return FALSE;
}


void CBaseMonster::FadeMonster( void )
{
	StopAnimation();
	pev->velocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->avelocity = g_vecZero;
	pev->animtime = gpGlobals->time;
	pev->effects |= EF_NOINTERP;
	SUB_StartFadeOut();
}

//=========================================================
// GibMonster - create some gore and get rid of a monster's
// model.
//=========================================================
void CBaseMonster :: GibMonster( void )
{
	TraceResult	tr;
	BOOL		gibbed = FALSE;

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM);		

	// only humans throw skulls !!!UNDONE - eventually monsters will have their own sets of gibs
	if ( HasHumanGibs() )
	{
		if ( CVAR_GET_FLOAT("violence_hgibs") != 0 )	// Only the player will ever get here
		{
			CGib::SpawnHeadGib( pev );
			CGib::SpawnRandomGibs( pev, 4, 1 );	// throw some human gibs.
		}
		gibbed = TRUE;
	}
	else if ( HasAlienGibs() )
	{
		if ( CVAR_GET_FLOAT("violence_agibs") != 0 )	// Should never get here, but someone might call it directly
		{
			CGib::SpawnRandomGibs( pev, 4, 0 );	// Throw alien gibs
		}
		gibbed = TRUE;
	}

	if ( !IsPlayer() )
	{
		if ( gibbed )
		{
			// don't remove players!
			SetThink ( SUB_Remove );
			pev->nextthink = gpGlobals->time;
		}
		else
		{
			FadeMonster();
		}
	}
}

//=========================================================
// GetDeathActivity - determines the best type of death
// anim to play.
//=========================================================
Activity CBaseMonster :: GetDeathActivity ( void )
{
	Activity	deathActivity;
	BOOL		fTriedDirection;
	float		flDot;
	TraceResult	tr;
	Vector		vecSrc;

	if ( pev->deadflag != DEAD_NO )
	{
		// don't run this while dying.
		return m_IdealActivity;
	}

	vecSrc = Center();

	fTriedDirection = FALSE;
	deathActivity = ACT_DIESIMPLE;// in case we can't find any special deaths to do.

	UTIL_MakeVectors ( pev->angles );
	flDot = DotProduct ( gpGlobals->v_forward, g_vecAttackDir * -1 );

	switch ( m_LastHitGroup )
	{
		// try to pick a region-specific death.
	case HITGROUP_HEAD:
		deathActivity = ACT_DIE_HEADSHOT;
		break;

	case HITGROUP_STOMACH:
		deathActivity = ACT_DIE_GUTSHOT;
		break;

	case HITGROUP_GENERIC:
		// try to pick a death based on attack direction
		fTriedDirection = TRUE;

		if ( flDot > 0.3 )
		{
			deathActivity = ACT_DIEFORWARD;
		}
		else if ( flDot <= -0.3 )
		{
			deathActivity = ACT_DIEBACKWARD;
		}
		break;

	default:
		// try to pick a death based on attack direction
		fTriedDirection = TRUE;

		if ( flDot > 0.3 )
		{
			deathActivity = ACT_DIEFORWARD;
		}
		else if ( flDot <= -0.3 )
		{
			deathActivity = ACT_DIEBACKWARD;
		}
		break;
	}


	// can we perform the prescribed death?
	if ( LookupActivity ( deathActivity ) == ACTIVITY_NOT_AVAILABLE )
	{
		// no! did we fail to perform a directional death? 
		if ( fTriedDirection )
		{
			// if yes, we're out of options. Go simple.
			deathActivity = ACT_DIESIMPLE;
		}
		else
		{
			// cannot perform the ideal region-specific death, so try a direction.
			if ( flDot > 0.3 )
			{
				deathActivity = ACT_DIEFORWARD;
			}
			else if ( flDot <= -0.3 )
			{
				deathActivity = ACT_DIEBACKWARD;
			}
		}
	}

	if ( LookupActivity ( deathActivity ) == ACTIVITY_NOT_AVAILABLE )
	{
		// if we're still invalid, simple is our only option.
		deathActivity = ACT_DIESIMPLE;
	}

	if ( deathActivity == ACT_DIEFORWARD )
	{
			// make sure there's room to fall forward
			UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr );

			if ( tr.flFraction != 1.0 )
			{
				deathActivity = ACT_DIESIMPLE;
			}
	}

	if ( deathActivity == ACT_DIEBACKWARD )
	{
			// make sure there's room to fall backward
			UTIL_TraceHull ( vecSrc, vecSrc - gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr );

			if ( tr.flFraction != 1.0 )
			{
				deathActivity = ACT_DIESIMPLE;
			}
	}

	return deathActivity;
}

//=========================================================
// GetSmallFlinchActivity - determines the best type of flinch
// anim to play.
//=========================================================
Activity CBaseMonster :: GetSmallFlinchActivity ( void )
{
	Activity	flinchActivity;
	BOOL		fTriedDirection;
	float		flDot;

	fTriedDirection = FALSE;
	UTIL_MakeVectors ( pev->angles );
	flDot = DotProduct ( gpGlobals->v_forward, g_vecAttackDir * -1 );
	
	switch ( m_LastHitGroup )
	{
		// pick a region-specific flinch
	case HITGROUP_HEAD:
		flinchActivity = ACT_FLINCH_HEAD;
		break;
	case HITGROUP_STOMACH:
		flinchActivity = ACT_FLINCH_STOMACH;
		break;
	case HITGROUP_LEFTARM:
		flinchActivity = ACT_FLINCH_LEFTARM;
		break;
	case HITGROUP_RIGHTARM:
		flinchActivity = ACT_FLINCH_RIGHTARM;
		break;
	case HITGROUP_LEFTLEG:
		flinchActivity = ACT_FLINCH_LEFTLEG;
		break;
	case HITGROUP_RIGHTLEG:
		flinchActivity = ACT_FLINCH_RIGHTLEG;
		break;
	case HITGROUP_GENERIC:
	default:
		// just get a generic flinch.
		flinchActivity = ACT_SMALL_FLINCH;
		break;
	}


	// do we have a sequence for the ideal activity?
	if ( LookupActivity ( flinchActivity ) == ACTIVITY_NOT_AVAILABLE )
	{
		flinchActivity = ACT_SMALL_FLINCH;
	}

	return flinchActivity;
}


void CBaseMonster::BecomeDead( void )
{
	pev->takedamage = DAMAGE_YES;// don't let autoaim aim at corpses.
	
	// give the corpse half of the monster's original maximum health. 
	pev->health = pev->max_health / 2;
	pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.

	// make the corpse fly away from the attack vector
	pev->movetype = MOVETYPE_TOSS;
	//pev->flags &= ~FL_ONGROUND;
	//pev->origin.z += 2;
	//pev->velocity = g_vecAttackDir * -1;
	//pev->velocity = pev->velocity * RANDOM_FLOAT( 300, 400 );
}


BOOL CBaseMonster::ShouldGibMonster( int iGib )
{
	if ( ( iGib == GIB_NORMAL && pev->health < GIB_HEALTH_VALUE ) || ( iGib == GIB_ALWAYS ) )
		return TRUE;
	
	return FALSE;
}


void CBaseMonster::CallGibMonster( void )
{
	BOOL fade = FALSE;

	if ( HasHumanGibs() )
	{
		if ( CVAR_GET_FLOAT("violence_hgibs") == 0 )
			fade = TRUE;
	}
	else if ( HasAlienGibs() )
	{
		if ( CVAR_GET_FLOAT("violence_agibs") == 0 )
			fade = TRUE;
	}

	pev->takedamage = DAMAGE_NO;
	pev->solid = SOLID_NOT;// do something with the body. while monster blows up

	if ( fade )
	{
		FadeMonster();
	}
	else
	{
		pev->effects = EF_NODRAW; // make the model invisible.
		GibMonster();
	}

	pev->deadflag = DEAD_DEAD;
	FCheckAITrigger();

	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}
	
	if ( ShouldFadeOnDeath() && !fade )
		UTIL_Remove(this);
}


/*
============
Killed
============
*/
void CBaseMonster :: Killed( entvars_t *pevAttacker, int iGib )
{
	unsigned int	cCount = 0;
	BOOL			fDone = FALSE;

	if ( HasMemory( bits_MEMORY_KILLED ) )
	{
		if ( ShouldGibMonster( iGib ) )
			CallGibMonster();
		return;
	}


	Remember( bits_MEMORY_KILLED );

	// clear the deceased's sound channels.(may have been firing or reloading when killed)
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);
	m_IdealMonsterState = MONSTERSTATE_DEAD;
	// Make sure this condition is fired too (TakeDamage breaks out before this happens on death)
	SetConditions( bits_COND_LIGHT_DAMAGE );
	

	// NEW: Physics Adapt corpses to floors angle
	Vector savedangles = Vector( 0, 0, 0 );
	TraceResult tr;		
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,64), ignore_monsters, edict(), &tr );
	Vector forward, right, angdir, angdiry;
	Vector Angles = pev->angles;

				UTIL_MakeVectorsPrivate( Angles, forward, right, NULL );
				angdir = forward;
				Vector left = -right;
				angdiry = left;
				pev->angles.x = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
				pev->angles.y = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).y;
				pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;

	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	if ( pOwner )
	{
		pOwner->DeathNotice( pev );
	}

	if	( ShouldGibMonster( iGib ) )
	{
		CallGibMonster();
		return;
	}
	else if ( pev->flags & FL_MONSTER )
	{
		SetTouch( NULL );
		BecomeDead();
	}
	
	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}
	
	//pev->enemy = ENT( pevAttacker );//why? (sjb)
	
	m_IdealMonsterState = MONSTERSTATE_DEAD;
}

//
// fade out - slowly fades a entity out, then removes it.
//
// DON'T USE ME FOR GIBS AND STUFF IN MULTIPLAYER! 
// SET A FUTURE THINK AND A RENDERMODE!!
void CBaseEntity :: SUB_StartFadeOut ( void )
{
	if (pev->rendermode == kRenderNormal)
	{
		pev->renderamt = 255;
		pev->rendermode = kRenderTransTexture;
	}

	pev->solid = SOLID_NOT;
	pev->avelocity = g_vecZero;

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink ( SUB_FadeOut );
}

void CBaseEntity :: SUB_FadeOut ( void  )
{
	if ( pev->renderamt > 7 )
	{
		pev->renderamt -= 7;
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else 
	{
		pev->renderamt = 0;
		pev->nextthink = gpGlobals->time + 0.2;
		SetThink ( SUB_Remove );
	}
}

//=========================================================
// WaitTillLand - in order to emit their meaty scent from
// the proper location, gibs should wait until they stop 
// bouncing to emit their scent. That's what this function
// does.
//=========================================================
void CGib :: WaitTillLand ( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	if ( pev->velocity == g_vecZero )
	{
		SetThink (SUB_StartFadeOut);
		pev->nextthink = gpGlobals->time + m_lifeTime;

		// If you bleed, you stink!
		if ( m_bloodColor != DONT_BLEED )
		{
			// ok, start stinkin!
			CSoundEnt::InsertSound ( bits_SOUND_MEAT, pev->origin, 384, 25 );
		}
	}
	else
	{
		// wait and check again in another half second.
		pev->nextthink = gpGlobals->time + 0.5;
	}

	WaterFloat ();
}

//
// Gib bounces on the ground or wall, sponges some blood down, too!
//
void CGib :: BounceGibTouch ( CBaseEntity *pOther )
{
	Vector	vecSpot;
	TraceResult	tr;
	
	//if ( RANDOM_LONG(0,1) )
	//	return;// don't bleed everytime

	if (pev->flags & FL_ONGROUND)
	{
		pev->velocity = pev->velocity * 0.9;
		pev->angles.x = 0;
		pev->angles.z = 0;
		pev->avelocity.x = 0;
		pev->avelocity.z = 0;
	}
	else
	{

		if ( g_Language != LANGUAGE_GERMAN && m_cBloodDecals > 0 && m_bloodColor != DONT_BLEED )
		{
			vecSpot = pev->origin + Vector ( 0 , 0 , 8 );//move up a bit, and trace down.
			UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -24 ),  ignore_monsters, ENT(pev), & tr);

			UTIL_BloodDecalTrace( &tr, m_bloodColor );

		MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
			WRITE_SHORT(0);
			WRITE_BYTE(0);
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( 0 );
			WRITE_COORD( 0 );
			WRITE_COORD( 0 );
			WRITE_SHORT(iDefaultParticle);
		MESSAGE_END();

			m_cBloodDecals--; 
		}

		if ( m_material != matNone && RANDOM_LONG(0,2) == 0 )
		{
			float volume;
			float zvel = fabs(pev->velocity.z);
		
			volume = 0.8 * min(1.0, ((float)zvel) / 450.0);

			CBreakable::MaterialSoundRandom( edict(), (Materials)m_material, volume );
		}
	}
}

//
// Sticky gib puts blood on the wall and stays put. 
//
void CGib :: StickyGibTouch ( CBaseEntity *pOther )
{
	Vector	vecSpot;
	TraceResult	tr;
	
	SetThink ( SUB_Remove );
	pev->nextthink = gpGlobals->time + 10;

	if ( !FClassnameIs( pOther->pev, "worldspawn" ) )
	{
		pev->nextthink = gpGlobals->time;
		return;
	}

	UTIL_TraceLine ( pev->origin, pev->origin + pev->velocity * 32,  ignore_monsters, ENT(pev), & tr);

	UTIL_BloodDecalTrace( &tr, m_bloodColor );

	pev->velocity = tr.vecPlaneNormal * -1;
	pev->angles = UTIL_VecToAngles ( pev->velocity );
	pev->velocity = g_vecZero; 
	pev->avelocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
}

//
// Throw a chunk
//
void CGib :: Spawn( const char *szGibModel )
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->friction = 0.55; // deading the bounce a bit
	
	// sometimes an entity inherits the edict from a former piece of glass,
	// and will spawn using the same render FX or rendermode! bad!
	pev->renderamt = 255;
	pev->rendermode = kRenderNormal;
	pev->renderfx = kRenderFxNone;
	pev->solid = SOLID_SLIDEBOX;/// hopefully this will fix the VELOCITY TOO LOW crap
	pev->classname = MAKE_STRING("gib");

	SET_MODEL(ENT(pev), szGibModel);
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));

	pev->nextthink = gpGlobals->time + 4;
	m_lifeTime = 25;
	SetThink ( WaitTillLand );
	SetTouch ( BounceGibTouch );

	m_material = matNone;
	m_cBloodDecals = 5;// how many blood decals this gib can place (1 per bounce until none remain). 
}
//***********************************************************************
// Nuevas Physics para Gibs												*
// Con esta funcion activamos el "daño" en los Gibs con lo que podemos	*
// causar el desplazamiento de los Gibs. Tambien los Gibs flotan en agua*
// Solo Reaccionan a explosiones y crossbow POR AHORA, investigando....	*
//***********************************************************************
int CGib::WaterFloat()
{
pev->nextthink = gpGlobals->time + 0.1;
//if ( gib_physics.value != 0  )
//{
if (pev->waterlevel == 3)
	{
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = pev->velocity * 0.8;
		pev->avelocity = pev->avelocity * 0.9;
		pev->velocity.z += 8;
		pev->avelocity.y = RANDOM_FLOAT(0,11);
	}
	else if (pev->waterlevel == 0)
	{
		pev->movetype = MOVETYPE_BOUNCE;
		//pev->velocity.z -= 8;
	}
	else
	{
		pev->velocity.z -= 8;
	}
//}
	return 1;
}
// take health
int CBaseMonster :: TakeHealth (float flHealth, int bitsDamageType)
{
	if (!pev->takedamage)
		return 0;

	// clear out any damage types we healed.
	// UNDONE: generic health should not heal any
	// UNDONE: time-based damage

	m_bitsDamageType &= ~(bitsDamageType & ~DMG_TIMEBASED);
	
	return CBaseEntity::TakeHealth(flHealth, bitsDamageType);
}

/*
============
TakeDamage

The damage is coming from inflictor, but get mad at attacker
This should be the only function that ever reduces health.
bitsDamageType indicates the type of damage sustained, ie: DMG_SHOCK

Time-based damage: only occurs while the monster is within the trigger_hurt.
When a monster is poisoned via an arrow etc it takes all the poison damage at once.



GLOBALS ASSUMED SET:  g_iSkillLevel
============
*/
int CBaseMonster :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	float	flTake;
	Vector	vecDir;

	if (!pev->takedamage)
		return 0;

	if ( !IsAlive() )
	{
		return DeadTakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
	}

	if ( pev->deadflag == DEAD_NO )
	{
		// no pain sound during death animation.
		PainSound();// "Ouch!"
	}

	//!!!LATER - make armor consideration here!
	flTake = flDamage;

	// set damage type sustained
	m_bitsDamageType |= bitsDamageType;

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector( 0, 0, 0 );
	if (!FNullEnt( pevInflictor ))
	{
		CBaseEntity *pInflictor = CBaseEntity :: Instance( pevInflictor );
		if (pInflictor)
		{
			vecDir = ( pInflictor->Center() - Vector ( 0, 0, 10 ) - Center() ).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

	// add to the damage total for clients, which will be sent as a single
	// message at the end of the frame
	// todo: remove after combining shotgun blasts?
	if ( IsPlayer() )
	{
		if ( pevInflictor )
			pev->dmg_inflictor = ENT(pevInflictor);

		pev->dmg_take += flTake;

		// check for godmode or invincibility
		if ( pev->flags & FL_GODMODE )
		{
			return 0;
		}
	}

	// if this is a player, move him around!
	if ( ( !FNullEnt( pevInflictor ) ) && (pev->movetype == MOVETYPE_WALK) && (!pevAttacker || pevAttacker->solid != SOLID_TRIGGER) )
	{
		pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );
	}

	// do the damage
	pev->health -= flTake;

	
	// HACKHACK Don't kill monsters in a script.  Let them break their scripts first
	if ( m_MonsterState == MONSTERSTATE_SCRIPT )
	{
		SetConditions( bits_COND_LIGHT_DAMAGE );
		return 0;
	}

	if ( pev->health <= 0 )
	{
		g_pevLastInflictor = pevInflictor;

		if ( bitsDamageType & DMG_ALWAYSGIB )
		{
			Killed( pevAttacker, GIB_ALWAYS );
		}
		else if ( bitsDamageType & DMG_NEVERGIB )
		{
			Killed( pevAttacker, GIB_NEVER );
		}
		else
		{
			Killed( pevAttacker, GIB_NORMAL );
		}

		g_pevLastInflictor = NULL;

		return 0;
	}

	// react to the damage (get mad)
	if ( (pev->flags & FL_MONSTER) && !FNullEnt(pevAttacker) )
	{
		if ( pevAttacker->flags & (FL_MONSTER | FL_CLIENT) )
		{// only if the attack was a monster or client!
			
			// enemy's last known position is somewhere down the vector that the attack came from.
			if (pevInflictor)
			{
				if (m_hEnemy == NULL || pevInflictor == m_hEnemy->pev || !HasConditions(bits_COND_SEE_ENEMY))
				{
					m_vecEnemyLKP = pevInflictor->origin;
				}
			}
			else
			{
				m_vecEnemyLKP = pev->origin + ( g_vecAttackDir * 64 ); 
			}

			MakeIdealYaw( m_vecEnemyLKP );

			// add pain to the conditions 
			// !!!HACKHACK - fudged for now. Do we want to have a virtual function to determine what is light and 
			// heavy damage per monster class?
			if ( flDamage > 0 )
			{
				SetConditions(bits_COND_LIGHT_DAMAGE);
			}

			if ( flDamage >= 20 )
			{
				SetConditions(bits_COND_HEAVY_DAMAGE);
			}
		}
	}

	return 1;
}

//=========================================================
// DeadTakeDamage - takedamage function called when a monster's
// corpse is damaged.
//=========================================================
int CBaseMonster :: DeadTakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	Vector			vecDir;

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector( 0, 0, 0 );
	if (!FNullEnt( pevInflictor ))
	{
		CBaseEntity *pInflictor = CBaseEntity :: Instance( pevInflictor );
		if (pInflictor)
		{
			vecDir = ( pInflictor->Center() - Vector ( 0, 0, 10 ) - Center() ).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

// turn this back on when the bounding box issues are resolved.

	pev->flags &= ~FL_ONGROUND;
	pev->origin.z += 1;
	
		
	
	// let the damage scoot the corpse around a bit.
	if ( !FNullEnt(pevInflictor) && (pevAttacker->solid != SOLID_TRIGGER) )
	{
		pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );


	// NEW: Physics Adapt corpses to floors angle
	Vector savedangles = Vector( 0, 0, 0 );
	TraceResult tr;		
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,64), ignore_monsters, edict(), &tr );
	Vector forward, right, angdir, angdiry;
	Vector Angles = pev->angles;

				UTIL_MakeVectorsPrivate( Angles, forward, right, NULL );
				angdir = forward;
				Vector left = -right;
				angdiry = left;
				pev->angles.x = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
				pev->angles.y = UTIL_VecToAngles( angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal).y;
				pev->angles.z = UTIL_VecToAngles( angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;




	}



	// kill the corpse if enough damage was done to destroy the corpse and the damage is of a type that is allowed to destroy the corpse.
	if ( bitsDamageType & DMG_GIB_CORPSE )
	{
		if ( pev->health <= flDamage )
		{
			pev->health = -50;
			Killed( pevAttacker, GIB_ALWAYS );
			return 0;
		}
		// Accumulate corpse gibbing damage, so you can gib with multiple hits
		pev->health -= flDamage * 0.1;
	}
	
	return 1;
}


float CBaseMonster :: DamageForce( float damage )
{ 
	float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;
	
	if ( force > 1000.0) 
	{
		force = 1000.0;
	}

	return force;
}

//
// RadiusDamage - this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
// 
// only damage ents that can clearly be seen by the explosion!

	
void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType )
{
	CBaseEntity *pEntity = NULL;
	TraceResult	tr;
	float		flAdjustedDamage, falloff;
	Vector		vecSpot;

	if ( flRadius )
		falloff = flDamage / flRadius;
	else
		falloff = 1.0;

	int bInWater = (UTIL_PointContents ( vecSrc ) == CONTENTS_WATER);

	vecSrc.z += 1;// in case grenade is lying on the ground

	if ( !pevAttacker )
		pevAttacker = pevInflictor;

	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere( pEntity, vecSrc, flRadius )) != NULL)
	{
		if ( pEntity->pev->takedamage != DAMAGE_NO )
		{
			// UNDONE: this should check a damage mask, not an ignore
			if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
			{// houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			// blast's don't tavel into or out of water
			if (bInWater && pEntity->pev->waterlevel == 0)
				continue;
			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			vecSpot = pEntity->BodyTarget( vecSrc );
			
			UTIL_TraceLine ( vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr );

			if ( tr.flFraction == 1.0 || tr.pHit == pEntity->edict() )
			{// the explosion can 'see' this entity, so hurt them!
				if (tr.fStartSolid)
				{
					// if we're stuck inside them, fixup the position and distance
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0.0;
				}
				
				// decrease damage for an ent that's farther from the bomb.
				flAdjustedDamage = ( vecSrc - tr.vecEndPos ).Length() * falloff;
				flAdjustedDamage = flDamage - flAdjustedDamage;
			
				if ( flAdjustedDamage < 0 )
				{
					flAdjustedDamage = 0;
				}
			
				// ALERT( at_console, "hit %s\n", STRING( pEntity->pev->classname ) );
				if (tr.flFraction != 1.0)
				{
					ClearMultiDamage( );
					pEntity->TraceAttack( pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize( ), &tr, bitsDamageType );
					ApplyMultiDamage( pevInflictor, pevAttacker );
				}
				else
				{
					pEntity->TakeDamage ( pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType );
				}
			}
		}
	}
}


void CBaseMonster :: RadiusDamage(entvars_t* pevInflictor, entvars_t*	pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	::RadiusDamage( pev->origin, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType );
}


void CBaseMonster :: RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	::RadiusDamage( vecSrc, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType );
}


//=========================================================
// CheckTraceHullAttack - expects a length to trace, amount 
// of damage to do, and damage type. Returns a pointer to
// the damaged entity in case the monster wishes to do
// other stuff to the victim (punchangle, etc)
//
// Used for many contact-range melee attacks. Bites, claws, etc.
//=========================================================
CBaseEntity* CBaseMonster :: CheckTraceHullAttack( float flDist, int iDamage, int iDmgType )
{
	TraceResult tr;

	if (IsPlayer())
		UTIL_MakeVectors( pev->angles );
	else
		UTIL_MakeAimVectors( pev->angles );

	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist );

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
	
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		if ( iDamage > 0 )
		{
			pEntity->TakeDamage( pev, pev, iDamage, iDmgType );
		}

		return pEntity;
	}

	return NULL;
}


//=========================================================
// FInViewCone - returns true is the passed ent is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
BOOL CBaseMonster :: FInViewCone ( CBaseEntity *pEntity )
{
	Vector2D	vec2LOS;
	float	flDot;

	UTIL_MakeVectors ( pev->angles );
	
	vec2LOS = ( pEntity->pev->origin - pev->origin ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

	if ( flDot > m_flFieldOfView )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//=========================================================
// FInViewCone - returns true is the passed vector is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
BOOL CBaseMonster :: FInViewCone ( Vector *pOrigin )
{
	Vector2D	vec2LOS;
	float		flDot;

	UTIL_MakeVectors ( pev->angles );
	
	vec2LOS = ( *pOrigin - pev->origin ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

	if ( flDot > m_flFieldOfView )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target
//=========================================================
BOOL CBaseEntity :: FVisible ( CBaseEntity *pEntity )
{
	TraceResult tr;
	Vector		vecLookerOrigin;
	Vector		vecTargetOrigin;
	
	if (FBitSet( pEntity->pev->flags, FL_NOTARGET ))
		return FALSE;

	// don't look through water
	if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3) 
		|| (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
		return FALSE;

	vecLookerOrigin = pev->origin + pev->view_ofs;//look through the caller's 'eyes'
	vecTargetOrigin = pEntity->EyePosition();

	UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore_glass, ENT(pev)/*pentIgnore*/, &tr);
	
	if (tr.flFraction != 1.0)
	{
		return FALSE;// Line of sight is not established
	}
	else
	{
		return TRUE;// line of sight is valid.
	}
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target vector
//=========================================================
BOOL CBaseEntity :: FVisible ( const Vector &vecOrigin )
{
	TraceResult tr;
	Vector		vecLookerOrigin;
	
	vecLookerOrigin = EyePosition();//look through the caller's 'eyes'

	UTIL_TraceLine(vecLookerOrigin, vecOrigin, ignore_monsters, ignore_glass, ENT(pev)/*pentIgnore*/, &tr);
	
	if (tr.flFraction != 1.0)
	{
		return FALSE;// Line of sight is not established
	}
	else
	{
		return TRUE;// line of sight is valid.
	}
}

/*
================
TraceAttack
================
*/
void CBaseEntity::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	if ( pev->takedamage )
	{
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );

		int blood = BloodColor();
		
		if ( blood != DONT_BLEED )
		{
			SpawnBlood(vecOrigin, blood, flDamage);// a little surface blood.
			TraceBleed( flDamage, vecDir, ptr, bitsDamageType );
		}
	}
}


/*
//=========================================================
// TraceAttack
//=========================================================
void CBaseMonster::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	ALERT ( at_console, "%d\n", ptr->iHitgroup );


	if ( pev->takedamage )
	{
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );

		int blood = BloodColor();
		
		if ( blood != DONT_BLEED )
		{
			SpawnBlood(vecOrigin, blood, flDamage);// a little surface blood.
		}
	}
}
*/

//=========================================================
// TraceAttack
//=========================================================
void CBaseMonster :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if ( pev->takedamage )
	{
		m_LastHitGroup = ptr->iHitgroup;

		switch ( ptr->iHitgroup )
		{
		case HITGROUP_GENERIC:
			break;
		case HITGROUP_HEAD:
			flDamage *= gSkillData.monHead;
			break;
		case HITGROUP_CHEST:
			flDamage *= gSkillData.monChest;
			break;
		case HITGROUP_STOMACH:
			flDamage *= gSkillData.monStomach;
			break;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			flDamage *= gSkillData.monArm;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			flDamage *= gSkillData.monLeg;
			break;
		default:
			break;
		}

		SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);// a little surface blood.
		TraceBleed( flDamage, vecDir, ptr, bitsDamageType );
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );
	}
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Monsters.
================
*/
void CBaseEntity::FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker )
{
	static int tracerCount;
	int tracer;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;

	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for (ULONG iShot = 1; iShot <= cShots; iShot++)
	{
		// get circular gaussian spread
		float x, y, z;
		do {
			x = RANDOM_FLOAT(-0.5,0.5) + RANDOM_FLOAT(-0.5,0.5);
			y = RANDOM_FLOAT(-0.5,0.5) + RANDOM_FLOAT(-0.5,0.5);
			z = x*x+y*y;
		} while (z > 1);

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);

		tracer = 0;
		if (iTracerFreq != 0 && (tracerCount++ % iTracerFreq) == 0)
		{
			Vector vecTracerSrc;

			if ( IsPlayer() )
			{// adjust tracer position for player
				vecTracerSrc = vecSrc + Vector ( 0 , 0 , -4 ) + gpGlobals->v_right * 2 + gpGlobals->v_forward * 16;
			}
			else
			{
				vecTracerSrc = vecSrc;
			}
			
			if ( iTracerFreq != 1 )		// guns that always trace also always decal
				tracer = 1;
			switch( iBulletType )
			{
			case BULLET_MONSTER_MP5:
			case BULLET_MONSTER_9MM:
			case BULLET_MONSTER_12MM:
			default:
				MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, vecTracerSrc );
					WRITE_BYTE( TE_TRACER );
					WRITE_COORD( vecTracerSrc.x );
					WRITE_COORD( vecTracerSrc.y );
					WRITE_COORD( vecTracerSrc.z );
					WRITE_COORD( tr.vecEndPos.x );
					WRITE_COORD( tr.vecEndPos.y );
					WRITE_COORD( tr.vecEndPos.z );
				MESSAGE_END();
				break;
			}
		}
		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		if ( pEntity->IsBSPModel() )
		{
			char chTextureType;
			char szbuffer[64];//64
			const char *pTextureName;
			float rgfl1[3];
			float rgfl2[3];
			float fattn = ATTN_NORM;

			chTextureType = 0;

			if (pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
				// hit body
				chTextureType = CHAR_TEX_FLESH;
			else
			{
				vecSrc.CopyToArray(rgfl1);
				vecEnd.CopyToArray(rgfl2);

				if (pEntity)
					pTextureName = TRACE_TEXTURE( ENT(pEntity->pev), rgfl1, rgfl2 );
				else
					pTextureName = TRACE_TEXTURE( ENT(0), rgfl1, rgfl2 );
					
				if ( pTextureName )
				{
					if (*pTextureName == '-' || *pTextureName == '+')
						pTextureName += 2;

					if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
						pTextureName++;
					strcpy(szbuffer, pTextureName);
					szbuffer[CBTEXTURENAMEMAX - 1] = 0;
					chTextureType = TEXTURETYPE_Find(szbuffer);	
				
			
				MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
								WRITE_SHORT(0);
								WRITE_BYTE(0);
								WRITE_COORD( tr.vecEndPos.x );
								WRITE_COORD( tr.vecEndPos.y );
								WRITE_COORD( tr.vecEndPos.z );
								WRITE_COORD( 0 );
								WRITE_COORD( 0 );
								WRITE_COORD( 0 );
								if (chTextureType == CHAR_TEX_GLASS)	
								{
									WRITE_SHORT(iDefaultImpactGlass);	// Glass Particles
								}
								else if (chTextureType == CHAR_TEX_GRASS)	
								{
									WRITE_SHORT(iDefaultImpactGrass); // Grass particles
								}
								else if (chTextureType == CHAR_TEX_CEMENT)	
								{
									WRITE_SHORT(iDefaultImpactCement); // Cement particles
								}
								else if(chTextureType == CHAR_TEX_WOOD)	
								{
									WRITE_SHORT(iDefaultImpactWood); // Wood particles
								}
								else if (chTextureType == CHAR_TEX_BROWN)	
								{
									WRITE_SHORT(iDefaultImpactBrown); // Wall particles
								}
								else if (chTextureType == CHAR_TEX_SAND)	
								{
									WRITE_SHORT(iDefaultImpactSand); // Sand particles ( beach sand )
								}
								else
								{

									WRITE_SHORT(iDefaultSuperPro);
								}
				MESSAGE_END();
				}
			}
		}
			if ( iDamage )
			{
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB) );
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );
			} 
			else switch(iBulletType)
			{
			default:
			case BULLET_MONSTER_9MM:
				pEntity->TraceAttack(pevAttacker, gSkillData.monDmg9MM, vecDir, &tr, DMG_BULLET);
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );

				break;

			case BULLET_MONSTER_MP5:
				pEntity->TraceAttack(pevAttacker, gSkillData.monDmgMP5, vecDir, &tr, DMG_BULLET);
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );

				break;

			case BULLET_MONSTER_12MM:		
				pEntity->TraceAttack(pevAttacker, gSkillData.monDmg12MM, vecDir, &tr, DMG_BULLET); 
				if ( !tracer )
				{
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot( &tr, iBulletType );
				}
				break;
			
			case BULLET_NONE: // FIX 
				pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				}

				break;
			}
		}
		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0 );
	}
	ApplyMultiDamage(pev, pevAttacker);
}


/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Players, uses the random seed generator to sync client and server side shots.
================
*/
CBaseEntity *FindEntityForward8( CBaseEntity *pMe )
{
	TraceResult tr;

	UTIL_MakeVectors(pMe->pev->v_angle);
	UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs,pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 8192,dont_ignore_monsters, pMe->edict(), &tr );
	if ( tr.flFraction != 1.0 && !FNullEnt( tr.pHit) )
	{
		CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
		return pHit;
	}
	return NULL;
}

Vector CBaseEntity::FireBulletsPlayer ( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y, z;



	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	

	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{

	

		
		//Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ) , -0.5, 0.5 );
		y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5, 0.5 );
		z = x * x + y * y;

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);
		
		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		if ( pEntity->IsBSPModel() )
		{
			char chTextureType;
			char szbuffer[64];//64
			const char *pTextureName;
			float rgfl1[3];
			float rgfl2[3];
			float fattn = ATTN_NORM;

			chTextureType = 0;

			if (pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
				// hit body
				chTextureType = CHAR_TEX_FLESH;
			else
			{
				vecSrc.CopyToArray(rgfl1);
				vecEnd.CopyToArray(rgfl2);

				if (pEntity)
					pTextureName = TRACE_TEXTURE( ENT(pEntity->pev), rgfl1, rgfl2 );
				else
					pTextureName = TRACE_TEXTURE( ENT(0), rgfl1, rgfl2 );
					
				if ( pTextureName )
				{
					if (*pTextureName == '-' || *pTextureName == '+')
						pTextureName += 2;

					if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
						pTextureName++;
					strcpy(szbuffer, pTextureName);
					szbuffer[CBTEXTURENAMEMAX - 1] = 0;
					chTextureType = TEXTURETYPE_Find(szbuffer);	
				
			
				MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
								WRITE_SHORT(0);
								WRITE_BYTE(0);
								WRITE_COORD( tr.vecEndPos.x );
								WRITE_COORD( tr.vecEndPos.y );
								WRITE_COORD( tr.vecEndPos.z );
								WRITE_COORD( 0 );
								WRITE_COORD( 0 );
								WRITE_COORD( 0 );
								if (chTextureType == CHAR_TEX_BROWN)	
									WRITE_SHORT(iDefaultImpactBrown); // Wall particles
								else if (chTextureType == CHAR_TEX_GLASS)	
									WRITE_SHORT(iDefaultImpactGlass);
								else if (chTextureType == CHAR_TEX_GRASS)	
									WRITE_SHORT(iDefaultImpactGrass); // Grass particles
								else if (chTextureType == CHAR_TEX_CEMENT)	
									WRITE_SHORT(iDefaultImpactCement); // Cement particles
								else if (chTextureType == CHAR_TEX_WOOD)	
									WRITE_SHORT(iDefaultImpactWood); // Wood particles
								else if (chTextureType == CHAR_TEX_SAND)	
									WRITE_SHORT(iDefaultImpactSand); // Sand particles ( beach sand )
								else
									WRITE_SHORT(iDefaultSuperPro);
				MESSAGE_END();
				}
			}

	
		}
		
		
			if ( iDamage )
			{
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB) );
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );

			} 
			else switch(iBulletType)
			{
			default:
			case BULLET_PLAYER_9MM:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &tr, DMG_BULLET); 
				break;

			case BULLET_PLAYER_MP5:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMP5, vecDir, &tr, DMG_BULLET); 
				break;

			case BULLET_PLAYER_BUCKSHOT:	
				 // make distance based!
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgBuckshot, vecDir, &tr, DMG_BULLET); 
				break;

			case BULLET_PLAYER_P228:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgP228, vecDir, &tr, DMG_BULLET); 
				break;



			case BULLET_PLAYER_GLOCK18:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgGLOCK18, vecDir, &tr, DMG_BULLET); 
				break;


			case BULLET_PLAYER_P90:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgP90, vecDir, &tr, DMG_BULLET); 
				break;


			case BULLET_PLAYER_USP:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgUSP, vecDir, &tr, DMG_BULLET); 
				break;




			case BULLET_PLAYER_M4A1:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgM4A1, vecDir, &tr, DMG_BULLET); 
				break;


			case BULLET_PLAYER_FIVE:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgFIVE, vecDir, &tr, DMG_BULLET); 
				break;



			case BULLET_PLAYER_MAC10:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMAC10, vecDir, &tr, DMG_BULLET); 
				break;



			case BULLET_PLAYER_HKMP5:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgHKMP5, vecDir, &tr, DMG_BULLET); 
				break;



			case BULLET_PLAYER_AK47:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgHKMP5, vecDir, &tr, DMG_BULLET); 
				break;


			
			case BULLET_PLAYER_UMP:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgUMP, vecDir, &tr, DMG_BULLET); 
				break;


			
			case BULLET_PLAYER_357:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &tr, DMG_BULLET); 
				break;
				
			case BULLET_NONE: // FIX 
				pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				}

				break;


			}
		}
			
			
		
			
		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0 );

		
	}
	ApplyMultiDamage(pev, pevAttacker);


	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}











/*
================
FireBullets FOR REMOVETOOL

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Players, uses the random seed generator to sync client and server side shots.
================
*/

CBaseEntity *FindEntityForward3( CBaseEntity *pMe )
{
	TraceResult tr;

	UTIL_MakeVectors(pMe->pev->v_angle);
	UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs,pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 8192,dont_ignore_monsters, pMe->edict(), &tr );
	if ( tr.flFraction != 1.0 && !FNullEnt( tr.pHit) )
	{
		CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
		return pHit;
	}
	return NULL;
}

Vector CBaseEntity::FireBulletsRemoveTool( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y, z,i;
	CBaseEntity *pEntity;
	
	x= 0;
	y= 0;
	z = 0;
	i = 0;
	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

		
pEntity = FindEntityForward3( this );
/*
	#ifdef CLIENT_DLL
	if ( !bIsMultiplayer() )
#else
	if ( pEntity->IsPlayer )
#endif
	
	{
		return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );;
	}
	*/

	if ( CVAR_GET_FLOAT("remove_npcs") == 1) // Can we remove NPCs in MP ?
	{
		if ( pEntity )
		{
			if ( pevAttacker == NULL )
			{
				pevAttacker = pev;
			}
			else
			{
			//if ( pEntity->pev->takedamage )
				pEntity->SetThink(SUB_Remove);
				pEntity->pev->nextthink = gpGlobals->time;
			}
			
		}
	}// endif remove_npcs

	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{
		
	}
	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}


























Vector CBaseEntity::FireBulletsRemoveToolRender( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	CBaseEntity *pEntity;
	float x,y;

	x = 0;
	y = 0;

	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;


	pEntity = FindEntityForward3( this );
		if ( pEntity )
		{
		
				pEntity->pev->rendercolor.x = r_colorx;
				pEntity->pev->rendercolor.y = r_colory;
				pEntity->pev->rendercolor.z = r_colorz;

				if( r_render == 1)
				{
					pEntity->pev->rendermode = kRenderNormal;
					pEntity->pev->renderamt = 255;
				}

				if( r_render == 2)
				{
					pEntity->pev->rendermode = kRenderGlow;
					pEntity->pev->renderamt = 255;
				}
				

				if( r_render == 3)
				{
					pEntity->pev->rendermode = kRenderTransColor,
					pEntity->pev->renderamt = 255;
				}



				if( r_render == 4)
				{
					pEntity->pev->rendermode = kRenderTransAlpha,
					pEntity->pev->renderamt = 255;
				}



				if( r_render == 5)
				{
					pEntity->pev->rendermode = kRenderTransAdd,
					pEntity->pev->renderamt = 255;
				}



				if( r_render == 6)
				{
					pEntity->pev->rendermode = kRenderTransTexture,
					pEntity->pev->renderamt = 255;
				}


				/////////////////////////////////////////////

				if( r_renderfx == 1)
				{
					pEntity->pev->renderfx = kRenderFxNone,
					pEntity->pev->renderamt = 255;
				}



				if( r_renderfx == 2)
				{
					pEntity->pev->renderfx = kRenderFxPulseSlow,
					pEntity->pev->renderamt = 255;
				}




				if( r_renderfx == 3)
				{
					pEntity->pev->renderfx = kRenderFxPulseFast,
					pEntity->pev->renderamt = 255;
				}



				if( r_renderfx == 4)
				{
					pEntity->pev->renderfx = kRenderFxPulseSlowWide,
					pEntity->pev->renderamt = 255;
				}



				if( r_renderfx == 5)
				{
					pEntity->pev->renderfx = kRenderFxPulseFastWide,
					pEntity->pev->renderamt = 255;
				}



				
				if( r_renderfx == 6)
				{
					pEntity->pev->renderfx = kRenderFxFadeSlow,
					pEntity->pev->renderamt = 255;
				}



				if( r_renderfx == 7)
				{
					pEntity->pev->renderfx = kRenderFxFadeFast,
					pEntity->pev->renderamt = 255;
				}




				if( r_renderfx == 8)
				{
					pEntity->pev->renderfx = kRenderFxSolidSlow,
					pEntity->pev->renderamt = 255;
				}





				if( r_renderfx == 9)
				{
					pEntity->pev->renderfx = kRenderFxSolidFast,
					pEntity->pev->renderamt = 255;
				}




				if( r_renderfx == 10)
				{
					pEntity->pev->renderfx = kRenderFxStrobeSlow,
					pEntity->pev->renderamt = 255;
				}




				if( r_renderfx == 11)
				{
					pEntity->pev->renderfx = kRenderFxStrobeFast,
					pEntity->pev->renderamt = 255;
				}




				if( r_renderfx == 12)
				{
					pEntity->pev->renderfx = kRenderFxFlickerSlow,
					pEntity->pev->renderamt = 255;
				}
				


				if( r_renderfx == 13)
				{
					pEntity->pev->renderfx = 	kRenderFxFlickerFast,
					pEntity->pev->renderamt = 255;
				}







				if( r_renderfx == 14)
				{
					pEntity->pev->renderfx = 	kRenderFxNoDissipation,
					pEntity->pev->renderamt = 255;
				}
			




				if( r_renderfx == 15)
				{
					pEntity->pev->renderfx = 	kRenderFxDistort,
					pEntity->pev->renderamt = 255;
				}
				



				if( r_renderfx == 16)
				{
					pEntity->pev->renderfx = 	kRenderFxHologram,
					pEntity->pev->renderamt = 255;
				}




				if( r_renderfx == 17)
				{
					pEntity->pev->renderfx = 	kRenderFxExplode,
					pEntity->pev->renderamt = 255;
				}





				if( r_renderfx == 18)
				{
					pEntity->pev->renderfx = 	kRenderFxGlowShell,
					pEntity->pev->renderamt = 255;
				}


				
		}

	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{
		
	}
	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}








void CBaseEntity :: TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	if (BloodColor() == DONT_BLEED)
		return;
	
	if (flDamage == 0)
		return;

	if (! (bitsDamageType & (DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_MORTAR)))
		return;
	
	// make blood decal on the wall! 
	TraceResult Bloodtr;
	Vector vecTraceDir; 
	float flNoise;
	int cCount;
	int i;

/*
	if ( !IsAlive() )
	{
		// dealing with a dead monster. 
		if ( pev->max_health <= 0 )
		{
			// no blood decal for a monster that has already decalled its limit.
			return; 
		}
		else
		{
			pev->max_health--;
		}
	}
*/

	if (flDamage < 10)
	{
		flNoise = 0.1;
		cCount = 1;
	}
	else if (flDamage < 25)
	{
		flNoise = 0.2;
		cCount = 2;
	}
	else
	{
		flNoise = 0.3;
		cCount = 4;
	}

	for ( i = 0 ; i < cCount ; i++ )
	{
		vecTraceDir = vecDir * -1;// trace in the opposite direction the shot came from (the direction the shot is going)

		vecTraceDir.x += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.y += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.z += RANDOM_FLOAT( -flNoise, flNoise );

		UTIL_TraceLine( ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * -172, ignore_monsters, ENT(pev), &Bloodtr);

		if ( Bloodtr.flFraction != 1.0 )
		{
			UTIL_BloodDecalTrace( &Bloodtr, BloodColor() );
		}
	}
}

//=========================================================
//=========================================================
void CBaseMonster :: MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir )
{
	// make blood decal on the wall! 
	TraceResult Bloodtr;
	Vector vecTraceDir; 
	int i;

	if ( !IsAlive() )
	{
		// dealing with a dead monster. 
		if ( pev->max_health <= 0 )
		{
			// no blood decal for a monster that has already decalled its limit.
			return; 
		}
		else
		{
			pev->max_health--;
		}
	}

	for ( i = 0 ; i < cCount ; i++ )
	{
		vecTraceDir = vecDir;

		vecTraceDir.x += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.y += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.z += RANDOM_FLOAT( -flNoise, flNoise );

		UTIL_TraceLine( ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * 172, ignore_monsters, ENT(pev), &Bloodtr);

/*
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SHOWLINE);
			WRITE_COORD( ptr->vecEndPos.x );
			WRITE_COORD( ptr->vecEndPos.y );
			WRITE_COORD( ptr->vecEndPos.z );
			
			WRITE_COORD( Bloodtr.vecEndPos.x );
			WRITE_COORD( Bloodtr.vecEndPos.y );
			WRITE_COORD( Bloodtr.vecEndPos.z );
		MESSAGE_END();
*/

		if ( Bloodtr.flFraction != 1.0 )
		{
			UTIL_BloodDecalTrace( &Bloodtr, BloodColor() );
		}
	}
}

























/*
================
FireBullets FOR PHYSGUN

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Players, uses the random seed generator to sync client and server side shots.
================
*/

CBaseEntity *FindEntityForward4( CBaseEntity *pMe )
{
	TraceResult tr;

	UTIL_MakeVectors(pMe->pev->v_angle);
	UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs,pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 8192,dont_ignore_monsters, pMe->edict(), &tr );
	if ( tr.flFraction != 1.0 && !FNullEnt( tr.pHit) )
	{
		CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
		return pHit;
	}
	return NULL;
}

Vector CBaseEntity::FireBulletsPhysGun( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y, z;
	CBaseEntity *pEntity;




	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	

	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{
				//Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ) , -0.5, 0.5 );
		y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5, 0.5 );
		z = x * x + y * y;

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);

		pEntity = FindEntityForward4( this );
		if ( pEntity )
		{
			//if ( pEntity->pev->takedamage )
			pEntity->pev->movetype = MOVETYPE_FLY;
				pEntity->pev->origin = pev->origin + vecDirShooting + gpGlobals->v_forward* 120;
			//	pEntity->pev->angles = pevAttacker->angles;
			
			//	pEntity->pev->flags &= ~FL_ONGROUND;
			//	pEntity->pev->origin.z += 1;
					
			
		}
		/*
		//Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ) , -0.5, 0.5 );
		y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5, 0.5 );
		z = x * x + y * y;

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*///, &tr);
		
		// do damage, paint decals
		/*
		if (tr.flFraction != 1.0)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

			if ( iDamage )
			{
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB) );
				
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot( &tr, iBulletType );
			} 
			else switch(iBulletType)
			{
			default:
			case BULLET_PLAYER_9MM:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &tr, DMG_BULLET); 
				break;

			case BULLET_PLAYER_MP5:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMP5, vecDir, &tr, DMG_BULLET); 
				break;

			case BULLET_PLAYER_BUCKSHOT:	
				 // make distance based!
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgBuckshot, vecDir, &tr, DMG_BULLET); 
				break;
			
			case BULLET_PLAYER_357:		
				pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &tr, DMG_BULLET); 
				break;
				
			case BULLET_NONE: // FIX 
				pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				// only decal glass
				if ( !FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
				{
					UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0,2) );
				}

				break;
			}
		}
		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0 );
	}
	ApplyMultiDamage(pev, pevAttacker);
	*/
	}

	
	
	//ClearMultiDamage();
	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}


















Vector CBaseEntity::FireBulletsPhysGunElevator( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y;
	CBaseEntity *pEntity;

	x = 0;
	y = 0;


	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	

	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{
		pEntity = FindEntityForward4( this );
		if ( pEntity )
		{
			
			if (r_solid_pos == 1)									//SOLID UP
			{
				pEntity->pev->movetype = MOVETYPE_FLY;
			
				pEntity->pev->velocity = g_vecZero;
				pEntity->pev->basevelocity = g_vecZero;
				pEntity->pev->origin.x = 	pEntity->pev->origin.x;
				pEntity->pev->origin.y = 	pEntity->pev->origin.y;
				pEntity->pev->origin.z = pEntity->pev->origin.z +1;
				
			}

			if (r_solid_pos == 2)									//SOLID DOWN
			{
					
				pEntity->pev->origin.z = pEntity->pev->origin.z -1;
			}



			if (r_solid_pos == 3)									//SOLID LEFT
			{
					
				pEntity->pev->origin.y = pEntity->pev->origin.y +1;
			}



			if (r_solid_pos == 4)									//SOLID RIGHT
			{
					
				pEntity->pev->origin.y = pEntity->pev->origin.y -1;
			}


			
			//	pEntity->pev->flags &= ~FL_ONGROUND;
			//	pEntity->pev->origin.z += 1;
					
			
		}
	}

	
	
	//ClearMultiDamage();
	return Vector( x * vecSpread.x, y * vecSpread.y, 99999 );
}




/*
================
FireBullets FOR DUPLICATOR

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Players, uses the random seed generator to sync client and server side shots.
================
*/

CBaseEntity *FindEntityForward5( CBaseEntity *pMe )
{
	TraceResult tr;

	UTIL_MakeVectors(pMe->pev->v_angle);
	UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs,pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 8192,dont_ignore_monsters, pMe->edict(), &tr );
	if ( tr.flFraction != 1.0 && !FNullEnt( tr.pHit) )
	{
		CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
		return pHit;
	}
	return NULL;
}

Vector CBaseEntity::FireBulletsDuplicatorSelect( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y;
	CBaseEntity *pEntity;

	
	x = 0;
	y = 0;

	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;


	pEntity = FindEntityForward5( this );
		if ( pEntity )
		{
			
			pEntity->pev->classname;
			if( STRING(pEntity->pev->classname) == "monster_barney") //CHECK FOR BARNEY MONSTER
			{
				monster_type = 1;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_scientist") //CHECK FOR SCIENTIST MONSTER
			{
				monster_type = 2;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_headcrab") //CHECK FOR HEADCRAB MONSTER
			{
				monster_type = 3;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_zombie") //CHECK FOR ZOMBIE MONSTER
			{
				monster_type = 4;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_human_grunt") //CHECK FOR HGRUNT MONSTER
			{
				monster_type = 5;
			
			}



			if( STRING(pEntity->pev->classname) == "monster_gargantua") //CHECK FOR GARGANTUA MONSTER
			{
				monster_type = 6;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_alien_slave") //CHECK FOR ALIEN SLAVE MONSTER
			{
				monster_type = 7;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_bullchicken") //CHECK FOR BULLCHICKEN MONSTER
			{
				monster_type = 8;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_tentacle") //CHECK FOR TENTACLE MONSTER
			{
				monster_type = 9;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_nihilanth") //CHECK FOR NIHILANT MONSTER
			{
				monster_type = 10;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_houndeye") //CHECK FOR HOUNDEYE MONSTER
			{
				monster_type = 11;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_gman") //CHECK FOR GMAN MONSTER
			{
				monster_type = 12;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_alien_controller") //CHECK FOR ALIEN CONTROLLER MONSTER
			{
				monster_type = 13;
			
			}

			if( STRING(pEntity->pev->classname) == "monster_human_assassin") //CHECK FOR HASSASSIN MONSTER
			{
				monster_type = 14;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_alien_grunt") //CHECK FOR ALIEN GRUNT MONSTER
			{
				monster_type = 15;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_black_headcrab") //CHECK FOR BLACK HEADCRAB MONSTER
			{
				monster_type = 16;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_fast_headcrab") //CHECK FOR FAST HEADCRAB MONSTER
			{
				monster_type = 17;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_leech") //CHECK FOR LEECH MONSTER
			{
				monster_type = 18;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_bigmomma") //CHECK FOR BIGMOMMA
			{
				monster_type = 19;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_cockroach") //CHECK FOR COCKROACH
			{
				monster_type = 20;
			
			}


			if( STRING(pEntity->pev->classname) == "monster_sentry") //CHECK FOR SENTRY
			{
				monster_type = 21;
			
			}
		


			if( STRING(pEntity->pev->classname) == "wall2x2") //CHECK FOR WALL2x2
			{
				monster_type = 22;
			
			}
		
		
		

		}

	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{
	
	}
	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}

















//DUPLICATE MONSTER

Vector CBaseEntity::FireBulletsDuplicator( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y, z;
	

	

	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;


			
	

		

	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{

		//Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ) , -0.5, 0.5 );
		y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5, 0.5 );
		z = x * x + y * y;

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev)/*pentIgnore*/, &tr);




			if(	monster_type == 1)//DUPLICATE MONSTER BARNEY
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_barney", tr.vecEndPos, pev->angles);
			}
			
			if(	monster_type == 2)//DUPLICATE MONSTER SCIENTIST
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_scientist", tr.vecEndPos, pev->angles);
			}

			if(	monster_type == 3)//DUPLICATE MONSTER HEADCRAB
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_headcrab",tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 4)//DUPLICATE MONSTER ZOMBIE
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_zombie", tr.vecEndPos, pev->angles);
			}

			if(	monster_type == 5)//DUPLICATE MONSTER HUMAN GRUNT
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_human_grunt", tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 6)//DUPLICATE MONSTER GARGANTUA 
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_gargantua", tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 7)//DUPLICATE MONSTER ALIEN SLAVE 
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_alien_slave", tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 8)//DUPLICATE MONSTER BULLCHICKEN
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_bullchicken", tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 9)//DUPLICATE MONSTER TENTACLE
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_tentacle", tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 10)//DUPLICATE MONSTER NIHILANTH
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_nihilanth", tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 11)//DUPLICATE MONSTER HOUNDEYE
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_houndeye",tr.vecEndPos, pev->angles);
			}



			if(	monster_type == 12)//DUPLICATE MONSTER GMAN
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_gman",tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 13)//DUPLICATE MONSTER ALIEN CONTROLLER
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_alien_controller",tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 14)//DUPLICATE MONSTER HUMAN ASSASSIN
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_human_assassin",tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 15)//DUPLICATE MONSTER ALIEN GRUNT
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_alien_grunt",tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 16)//DUPLICATE MONSTER BLACK HEADCRAB
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_black_headcrab",tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 17)//DUPLICATE MONSTER FAST HEADCRAB
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_fast_headcrab",tr.vecEndPos, pev->angles);
			}
		


			if(	monster_type == 18)//DUPLICATE MONSTER LEECH
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_leech",tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 19)//DUPLICATE MONSTER BIGMOMMA
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_bigmomma",tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 20)//DUPLICATE MONSTER COCKROACH
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_cockroach",tr.vecEndPos, pev->angles);
			}


			if(	monster_type == 21)//DUPLICATE MONSTER SENTRY
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("monster_sentry",tr.vecEndPos, pev->angles);
			}



			if(	monster_type == 22)//DUPLICATE WALL2x2
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				CBaseEntity::Create("wall2x2",tr.vecEndPos, pev->angles);
			}
		
		
	
	}
	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}





















Vector CBaseEntity::FireBulletsPoserTool( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y;

	x = 0;
	y = 0;


	CBaseEntity *pEntity;

	if ( pevAttacker == NULL )
		pevAttacker = pev;  // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;






	for ( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{


	
	pEntity = FindEntityForward3( this );

	if ( pEntity )
	{


	
		switch( RANDOM_LONG(0,1) )
		{
		case 0: pEntity->pev->frame =0; break;
		case 1: pEntity->pev->frame= 1; break;
		case 2: pEntity->pev->frame= 2; break;
		case 3: pEntity->pev->frame= 3; break;
		case 4: pEntity->pev->frame= 4; break;
		case 5: pEntity->pev->frame= 5; break;
		case 6: pEntity->pev->frame= 6; break;
		case 7: pEntity->pev->frame= 7; break;
		case 8: pEntity->pev->frame= 8; break;
		case 9: pEntity->pev->frame= 9; break;
		case 10: pEntity->pev->frame= 10; break;
		case 11: pEntity->pev->frame= 11; break;
		case 12: pEntity->pev->frame= 12; break;
		case 13: pEntity->pev->frame= 13; break;
		case 14: pEntity->pev->frame= 14; break;
		case 15: pEntity->pev->frame= 15; break;
		case 16: pEntity->pev->frame= 16; break;
		case 17: pEntity->pev->frame= 17; break;
		case 18: pEntity->pev->frame= 18; break;
		case 19: pEntity->pev->frame= 19; break;
		case 20: pEntity->pev->frame= 20; break;
		case 21: pEntity->pev->frame= 21; break;
		case 22: pEntity->pev->frame= 22; break;
		case 23: pEntity->pev->frame= 23; break;
		case 24: pEntity->pev->frame= 24; break;
		case 25: pEntity->pev->frame= 25; break;
		case 26: pEntity->pev->frame= 26; break;
		case 27: pEntity->pev->frame= 27; break;

		case 28: pEntity->pev->frame= 28; break;
		case 29: pEntity->pev->frame= 29; break;
		case 30: pEntity->pev->frame= 30; break;
		case 31: pEntity->pev->frame= 31; break;
		case 32: pEntity->pev->frame= 32; break;
		case 33: pEntity->pev->frame= 33; break;
		case 34: pEntity->pev->frame= 34; break;
		case 35: pEntity->pev->frame= 35; break;
		case 36: pEntity->pev->frame= 36; break;
		case 37: pEntity->pev->frame= 37; break;
		case 38: pEntity->pev->frame= 38; break;
		case 39: pEntity->pev->frame= 39; break;
		case 40: pEntity->pev->frame= 40; break;
		case 41: pEntity->pev->frame= 41; break;
		case 42: pEntity->pev->frame= 42; break;
		case 43: pEntity->pev->frame= 43; break;
		case 44: pEntity->pev->frame= 44; break;
		case 45: pEntity->pev->frame= 45; break;
		case 46: pEntity->pev->frame= 46; break;
		case 47: pEntity->pev->frame= 47; break;
		case 48: pEntity->pev->frame= 48; break;
		case 49: pEntity->pev->frame= 49; break;
		case 50: pEntity->pev->frame= 50; break;
		case 51: pEntity->pev->frame= 51; break;
		case 52: pEntity->pev->frame= 52; break;
		case 53: pEntity->pev->frame= 53; break;
		case 54: pEntity->pev->frame= 54; break;
		case 55: pEntity->pev->frame= 55; break;
		}
			
			  
				
			
	}
				 
				
				
		
	
	}
	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}
