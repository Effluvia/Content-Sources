#include        "extdll.h"
#include        "util.h"
#include        "cbase.h"
#include        "monsters.h"
#include        "weapons.h"
#include        "soundent.h"
#include        "gamerules.h"
#include        "animation.h"
#include "../engine/studio.h"
#include "particle_defs.h"
#include "decals.h"

extern int gmsgParticles;

class CBarrel : public CBaseMonster
{
public:
        void Spawn( void );
        void Precache( void );
        int      Classify ( void );
        void Explode( Vector vecSrc, Vector vecAim );
	void Explode( TraceResult *pTrace, int bitsDamageType );
	void	Detonate( void );
        void SetActivity(int activity);                                 
        int  GetActivity() { return m_Activity; }
        void SetCollisionBox();
		void EXPORT WaitTillDead ( void );
		void  Killed( entvars_t *pevAttacker, int iGib );
        void EXPORT IdleThink();
       int BloodColor( void ) { return DONT_BLEED; }
        int m_Activity;                                                                 //What entity is doing (animation)//
		int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
		float  f_NextBurnTime;
        
};
LINK_ENTITY_TO_CLASS(barrel, CBarrel);

void CBarrel::Spawn(void)
{
        Precache();
        
        pev->movetype   = MOVETYPE_STEP;
        pev->solid              = SOLID_BBOX;
        pev->takedamage = DAMAGE_YES;
      //  pev->flags              |= FL_MONSTER;
        pev->health             = 50;
        pev->gravity    = 1.0;
        
        SET_MODEL(ENT(pev), "models/gomod/dynamite.mdl");

        SetActivity(ACT_IDLE);
        SetSequenceBox();
        SetThink(IdleThink);
        pev->nextthink = gpGlobals->time + 0.1;           

}

void CBarrel::Precache(void)
{
        PRECACHE_MODEL("models/gomod/dynamite.mdl");
}

int     CBarrel::Classify(void)

{
        
        return  CLASS_NONE;

}

void CBarrel::SetActivity(int act)
{       
        int sequence = LookupActivity( act ); 
        if ( sequence != ACTIVITY_NOT_AVAILABLE )
        {
                pev->sequence = sequence;
                m_Activity = act; 
                pev->frame = 0;
                ResetSequenceInfo( );
                //m_flFrameRate = 1.0;
        }

}

void CBarrel::SetCollisionBox()
{
        studiohdr_t *pstudiohdr;
        pstudiohdr = (studiohdr_t*)GET_MODEL_PTR( ENT(pev) );

        if(pstudiohdr == NULL)
                ALERT(at_console,"Invalid model ptr! FUCK\n");

        mstudioseqdesc_t        *pseqdesc;

        pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);
        
        Vector min, max;

        min = pseqdesc[ pev->sequence ].bbmin;
        max = pseqdesc[ pev->sequence ].bbmax;
        
        //DrawBox(pev->origin + min, pev->origin + max);

        UTIL_SetSize(pev,min,max);

}

void CBarrel::IdleThink()
{

        float flInterval = StudioFrameAdvance();
	

       
        DispatchAnimEvents(flInterval);

        if(!IsInWorld())
        {
                SetTouch(NULL);
                UTIL_Remove(this);
                return;
        }

        if(pev->deadflag != DEAD_DEAD)
        {
                //SetActivity(ACT_IDLE);

                
 
        }

        else
        {
                UTIL_SetSize(pev,Vector(0,0,0),Vector(0,0,0));
        }


 pev->nextthink = gpGlobals->time + 0.1;
	

}

int CBarrel:: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// Take 30% damage from bullets
	if ( bitsDamageType == DMG_BULLET )
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();
		float flForce = DamageForce( flDamage );
		pev->velocity = pev->velocity + vecDir * flForce;
		flDamage *= 0.3;

			extern int gmsgParticles;

	
		MESSAGE_BEGIN(MSG_ALL, gmsgParticles);

				WRITE_SHORT(0);
				WRITE_BYTE(0);
				WRITE_COORD( pev->origin.x );
				WRITE_COORD( pev->origin.y );
				WRITE_COORD( pev->origin.z );
				WRITE_COORD( 0 );
				WRITE_COORD( 0 );
				WRITE_COORD( 0 );
				WRITE_SHORT(iDefaultFire);
			
		MESSAGE_END();
	}
			


	
	


	// HACK HACK -- until we fix this.
///	if ( IsAlive() )

	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}


void CBarrel :: Killed( entvars_t *pevAttacker, int iGib )
{

	Detonate();

	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;



//	CGib::SpawnRandomGibs( pev, 4, 1 );

	
	SetActivity ( ACT_DIESIMPLE );
	SetBoneController( 0, 0 );

	StudioFrameAdvance( 0.1 );

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink ( WaitTillDead );
}

//=========================================================
//=========================================================
void CBarrel :: WaitTillDead ( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	float flInterval = StudioFrameAdvance( 0.1 );
	DispatchAnimEvents ( flInterval );

	if ( m_fSequenceFinished )
	{
		// death anim finished. 
		StopAnimation();
		SetThink ( NULL );
	}
}

void CBarrel::Explode( Vector vecSrc, Vector vecAim )
{
	TraceResult tr;
	UTIL_TraceLine ( pev->origin, pev->origin + Vector ( 0, 0, -32 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}

// UNDONE: temporary scorching for PreAlpha - find a less sleazy permenant solution.
void CBarrel::Explode( TraceResult *pTrace, int bitsDamageType )
{
	float		flRndSound;// sound randomizer


	float posicion;

	
	pev->model = iStringNull;//invisible
	pev->solid = SOLID_NOT;// intangible
	pev->dmg = 200;

	pev->takedamage = DAMAGE_NO;

	if (pev->waterlevel == 1)
	{
		posicion = pev->origin.z;
	}

	// Pull out of the wall a bit
	if ( pTrace->flFraction != 1.0 )
	{
		pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 0.6);
	}

	int iContents = UTIL_PointContents ( pev->origin );
	
	/*
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_EXPLOSION );		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		if (iContents != CONTENTS_WATER)
		{
			WRITE_SHORT( g_sModelIndexFireball );
		}
		else
		{
			WRITE_SHORT( g_sModelIndexWExplosion );
		}
		WRITE_BYTE( (pev->dmg - 50) * .60  ); // scale * 10
		WRITE_BYTE( 15  ); // framerate
		WRITE_BYTE( TE_EXPLFLAG_NONE );
	MESSAGE_END();
*/
		// create explosion particle system


	extern int gmsgParticles;


	MESSAGE_BEGIN(MSG_ALL, gmsgParticles);

			WRITE_SHORT(0);
			WRITE_BYTE(0);
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_COORD( 0 );
			WRITE_COORD( 0 );
			WRITE_COORD( 0 );
		
			WRITE_SHORT(iDefaultExplosion);
		
	MESSAGE_END(); 

	//InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );

	entvars_t *pevOwner;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	pev->owner = NULL; // can't traceline attack owner if this is set

	RadiusDamage ( pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType );

	if ( RANDOM_FLOAT( 0 , 1 ) < 0.5 )
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH1 );
	}
	else
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH2 );
	}

	flRndSound = RANDOM_FLOAT( 0 , 1 );

	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM);	break;
		case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM);	break;
		case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM);	break;
	}

	pev->effects |= EF_NODRAW;

	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.3;

	if (iContents != CONTENTS_WATER)
	{
		int sparkCount = RANDOM_LONG(0,3);
		for ( int i = 0; i < sparkCount; i++ )
			Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL );
	}
}









void CBarrel::Detonate( void )
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}