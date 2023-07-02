// NEW FILE.

#include "player_extra.h"
#include "player.h"
#include "decals.h"
//#include "game.h"
#include "gamerules.h"
#include "shake.h"

EASY_CVAR_EXTERN_DEBUGONLY(customLogoSprayMode)


void CSprayCan::Spawn ( entvars_t *pevOwner )
{
	pev->origin = pevOwner->origin + Vector ( 0 , 0 , 32 );
	pev->angles = pevOwner->v_angle;
	pev->owner = ENT(pevOwner);
	pev->frame = 0;

	pev->nextthink = gpGlobals->time + 0.1;
	//MODDD - soundsentencesave. This one's ok to play through it.
	UTIL_PlaySound(ENT(pev), CHAN_VOICE, "player/sprayer.wav", 1, ATTN_NORM, 0, 100, FALSE);
}

void CSprayCan::Think( void )
{
	TraceResult	tr;	
	int playernum;
	int nFrames;
	CBasePlayer *pPlayer;


	pPlayer = (CBasePlayer *)GET_PRIVATE(pev->owner);

	if (pPlayer)
		nFrames = pPlayer->GetCustomDecalFrames();
	else
		nFrames = -1;

	playernum = ENTINDEX(pev->owner);
	
	// ALERT(at_console, "Spray by player %i, %i of %i\n", playernum, (int)(pev->frame + 1), nFrames);


	//MODDD - NOTE.  Aha!  This pev->angles is a copy of the player's v_angle so this is ok
	UTIL_MakeVectors(pev->angles);
	UTIL_TraceLine ( pev->origin, pev->origin + gpGlobals->v_forward * 128, ignore_monsters, pev->owner, & tr);


	switch( (int)EASY_CVAR_GET_DEBUGONLY(customLogoSprayMode) ){
		case 1:{
			nFrames = 6;
			//easyForcePrintLine("MY NUMBERS %d", playernum);
			UTIL_DecalTrace( &tr, DECAL_LAMBDA1 + pev->frame );
			//UTIL_PlayerDecalTrace( &tr, playernum, DECAL_LAMBDA1 + pev->frame, FALSE );
		
			if ( pev->frame++ >= (nFrames - 1))
				UTIL_Remove( this );
			break;
		}

		//case 2: ... etc.

		default:{  //0 or other unrecognized values
			// No customization present.
			if (nFrames == -1)
			{
				UTIL_DecalTrace( &tr, DECAL_LAMBDA6 );
				UTIL_Remove( this );
			}
			else
			{
				UTIL_PlayerDecalTrace( &tr, playernum, pev->frame, TRUE );
				// Just painted last custom frame.
				if ( pev->frame++ >= (nFrames - 1))
					UTIL_Remove( this );
			}
			break;
		}
	}//END OF else OF customLogoSprayMode

	pev->nextthink = gpGlobals->time + 0.1;
}




char *CDeadHEV::m_szPoses[] = { "deadback", "deadsitting", "deadstomach", "deadtable" };

void CDeadHEV::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}

LINK_ENTITY_TO_CLASS( monster_hevsuit_dead, CDeadHEV );

//=========================================================
// ********** DeadHEV SPAWN **********
//=========================================================
void CDeadHEV::Spawn( void )
{
	PRECACHE_MODEL("models/player.mdl");
	SET_MODEL(ENT(pev), "models/player.mdl");

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	pev->body			= 1;
	m_bloodColor		= BLOOD_COLOR_RED;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );

	if (pev->sequence == -1)
	{
		ALERT ( at_console, "Dead hevsuit with bad pose\n" );
		pev->sequence = 0;
		pev->effects = EF_BRIGHTFIELD;
	}

	// Corpses have less health
	pev->health			= 8;

	MonsterInitDead();
}



LINK_ENTITY_TO_CLASS( player_weaponstrip, CStripWeapons );

void CStripWeapons::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = NULL;

	if ( pActivator && pActivator->IsPlayer() )
	{
		pPlayer = (CBasePlayer *)pActivator;
	}
	else if ( !g_pGameRules->IsDeathmatch() )
	{
		pPlayer = (CBasePlayer *)CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );
	}

	if ( pPlayer ){
		//I think it is okay to remove items here?  unsure.
		pPlayer->RemoveAllItems( FALSE );
	}
}


LINK_ENTITY_TO_CLASS( player_loadsaved, CRevertSaved );

TYPEDESCRIPTION	CRevertSaved::m_SaveData[] = 
{
	DEFINE_FIELD( CRevertSaved, m_messageTime, FIELD_FLOAT ),	// These are not actual times, but durations, so save as floats
	DEFINE_FIELD( CRevertSaved, m_loadTime, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CRevertSaved, CPointEntity );

void CRevertSaved::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "duration"))
	{
		SetDuration( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holdtime"))
	{
		SetHoldTime( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "messagetime"))
	{
		SetMessageTime( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "loadtime"))
	{
		SetLoadTime( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else 
		CPointEntity::KeyValue( pkvd );
}

void CRevertSaved::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	UTIL_ScreenFadeAll( pev->rendercolor, Duration(), HoldTime(), pev->renderamt, FFADE_OUT );
	pev->nextthink = gpGlobals->time + MessageTime();
	SetThink( &CRevertSaved::MessageThink );
}


void CRevertSaved::MessageThink( void )
{
	UTIL_ShowMessageAll( STRING(pev->message) );
	float nextThink = LoadTime() - MessageTime();
	if ( nextThink > 0 ) 
	{
		pev->nextthink = gpGlobals->time + nextThink;
		SetThink( &CRevertSaved::LoadThink );
	}
	else
		LoadThink();
}


void CRevertSaved::LoadThink( void )
{
	if ( !gpGlobals->deathmatch )
	{
		SERVER_COMMAND("reload\n");
	}
}




//MODDDMIRROR
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//NOTICE - the player marker is no longer physically necessary. It only exists to force the rendered to render at least once while
//         the mirror is in view, but that may even already happen regardless. This can't hurt.
//         In short, null.mdl is no longer required.
LINK_ENTITY_TO_CLASS( player_marker, CPlayerMarker );

void CPlayerMarker::Spawn( void )
{
	Precache();
	//SET_MODEL( ENT(pev), "models/null.mdl" );
	SET_MODEL( ENT(pev), "models/player.mdl" );

	ALERT(at_aiconsole, "DEBUG: Player_marker coordinates is %f %f %f \n", pev->origin.x, pev->origin.y, pev->origin.z);
	
	//this->pev->renderfx = FX_DUMMY;
	this->pev->renderfx = kRenderFxDummy;

	//MODDD
	//pev->effects |= 128;
}


void CPlayerMarker::Precache( void )
{
	//PRECACHE_MODEL( "models/null.mdl" );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void CInfoIntermission::Spawn( void )
{
	UTIL_SetOrigin( pev, pev->origin );
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
	pev->v_angle = g_vecZero;

	pev->nextthink = gpGlobals->time + 2;// let targets spawn!

}

void CInfoIntermission::Think ( void )
{
	edict_t *pTarget;

	// find my target
	pTarget = FIND_ENTITY_BY_TARGETNAME( NULL, STRING(pev->target) );

	if ( !FNullEnt(pTarget) )
	{
		pev->v_angle = UTIL_VecToAngles( (pTarget->v.origin - pev->origin).Normalize() );
		pev->v_angle.x = -pev->v_angle.x;
	}
}
LINK_ENTITY_TO_CLASS( info_intermission, CInfoIntermission );



