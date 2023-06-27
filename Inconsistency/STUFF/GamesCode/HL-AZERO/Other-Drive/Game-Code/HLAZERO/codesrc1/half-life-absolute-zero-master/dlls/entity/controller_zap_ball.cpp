
#include "controller_zap_ball.h"
#include "weapons.h"


LINK_ENTITY_TO_CLASS( controller_energy_ball, CControllerZapBall );



CControllerZapBall::CControllerZapBall(void){

}

void CControllerZapBall::Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "sprites/xspark4.spr");
	pev->rendermode = kRenderTransAdd;
	pev->rendercolor.x = 255;
	pev->rendercolor.y = 255;
	pev->rendercolor.z = 255;
	pev->renderamt = 255;
	pev->scale = 0.5;

	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );

	SetThink( &CControllerZapBall::AnimateThink );
	SetTouch( &CControllerZapBall::ExplodeTouch );

	m_hOwner = Instance( pev->owner );
	pev->dmgtime = gpGlobals->time; // keep track of when ball spawned
	pev->nextthink = gpGlobals->time + 0.1;
}

extern int global_useSentenceSave;
void CControllerZapBall::Precache( void )
{
	PRECACHE_MODEL("sprites/xspark4.spr");
	//NOTE: uh, sounds found marked-out?  whatever, surrounding anyways.

	global_useSentenceSave = TRUE;
	// PRECACHE_SOUND("debris/zap4.wav");
	// PRECACHE_SOUND("weapons/electro4.wav");

	global_useSentenceSave = FALSE;
}

BOOL CControllerZapBall::usesSegmentedMove(void){
	// play it safe
	return FALSE;
}


void CControllerZapBall::AnimateThink( void  )
{
	pev->nextthink = gpGlobals->time + 0.1;
	
	pev->frame = ((int)pev->frame + 1) % 11;

	if (gpGlobals->time - pev->dmgtime > 5 || pev->velocity.Length() < 10)
	{
		SetTouch( NULL );
		UTIL_Remove( this );
	}
}


void CControllerZapBall::ExplodeTouch( CBaseEntity *pOther )
{
	if (pOther->pev->takedamage)
	{
		TraceResult tr = UTIL_GetGlobalTrace( );

		entvars_t	*pevOwner;
		if (m_hOwner != NULL)
		{
			pevOwner = m_hOwner->pev;
		}
		else
		{
			pevOwner = pev;
		}

		ClearMultiDamage( );
		pOther->TraceAttack(pevOwner, gSkillData.controllerDmgBall, pev->velocity.Normalize(), &tr, DMG_ENERGYBEAM ); 
		ApplyMultiDamage( pevOwner, pevOwner );

		UTIL_EmitAmbientSound( ENT(pev), tr.vecEndPos, "weapons/electro4.wav", 0.3, ATTN_NORM, 0, RANDOM_LONG( 90, 99 ), FALSE );

	}

	UTIL_Remove( this );
}



float CControllerZapBall::massInfluence(void){
	return 0.05f;
}//END OF massInfluence

int CControllerZapBall::GetProjectileType(void){
	return PROJECTILE_ENERGYBALL;
}



