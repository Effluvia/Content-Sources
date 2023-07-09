#include        "extdll.h"
#include        "util.h"
#include        "cbase.h"
#include        "monsters.h"
#include        "weapons.h"
#include        "soundent.h"
#include        "gamerules.h"
#include        "animation.h"
#include "../engine/studio.h"
#include "nodes.h"
#include "player.h"
#include "shake.h"



class CBengala : public CBaseMonster
{
public:
        void Spawn( void );
        void Precache( void );
        int      Classify ( void );
        
        void SetActivity(int activity);                                 
        int  GetActivity() { return m_Activity; }
        void SetCollisionBox();
		void  Move( CBaseEntity *pOther, int push );
        void EXPORT IdleThink();
       // int BloodColor() { return BLOOD_COLOR_RED; }
        int m_Activity;                                                                 //What entity is doing (animation)//
        
};
LINK_ENTITY_TO_CLASS(bengala, CBengala);

void CBengala::Spawn(void)
{
        Precache();
        
        pev->movetype   = MOVETYPE_STEP;
        pev->solid              = SOLID_BBOX;
        pev->takedamage = DAMAGE_NO;
        pev->flags              |= FL_MONSTER;
        pev->health             = 80;// strong! (not)
        pev->gravity    = 1.0;
        
        SET_MODEL(ENT(pev), "models/bengala.mdl");

        SetActivity(ACT_IDLE);
        SetSequenceBox();
        SetThink(IdleThink);
        pev->nextthink = gpGlobals->time + 1;  
		

}

void CBengala::Precache(void)
{
        PRECACHE_MODEL("models/bengala.mdl");
}

int     CBengala::Classify(void)

{
        
        return  CLASS_NONE;

}

void CBengala::SetActivity(int act)
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

void CBengala::SetCollisionBox()
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

void CBengala::IdleThink()
{

		
        float flInterval = StudioFrameAdvance();
		int  m_iOrientation;

        pev->nextthink = gpGlobals->time + 0.1;
        DispatchAnimEvents(flInterval);


		UTIL_Sparks( pev->origin );
		UTIL_Sparks( pev->origin );

			// lots of smoke
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( RANDOM_FLOAT( pev->absmin.x, pev->absmax.x ) );
			WRITE_COORD( RANDOM_FLOAT( pev->absmin.y, pev->absmax.y ) );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 12 ); // scale * 10
			WRITE_BYTE( 10 - 0 * 5); // framerate
		MESSAGE_END();


		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(pev->origin.x);	// X
			WRITE_COORD(pev->origin.y);	// Y
			WRITE_COORD(pev->origin.z);	// Z
			WRITE_BYTE( 10 );		// radius * 0.1
			WRITE_BYTE( 0 );		// r
			WRITE_BYTE( 100 );		// g
			WRITE_BYTE( 0 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );




        if(!IsInWorld())
        {
                SetTouch(NULL);
                UTIL_Remove(this);
                return;
        }



     if(pev->deadflag != DEAD_DEAD)
        {
                //SetActivity(ACT_IDLE);
                SetCollisionBox();
                int i;
                CBaseEntity *pNearestPlayer=NULL;
                float nearestdistance=1000;
                for ( i = 1; i <= gpGlobals->maxClients; i++ )
                {
                        CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
                        if(!pPlayer)
                        {
                                continue;
                        }
                        float distance=(pPlayer ->pev ->origin - pev->origin).Length();
                        if(distance<nearestdistance)
                        {
                                nearestdistance=distance;
                                pNearestPlayer=pPlayer;
                        }
                }
                // at this point we know that:
                //pNearestPlayer   points to the nearest player
                //nearestdistance  holds the distance to that player
                if(pNearestPlayer)
                {
                
                        Vector toplayer=pNearestPlayer ->pev ->origin - pev->origin;
                        
                        pev->angles=UTIL_VecToAngles(toplayer);
                        pev->angles.x=0;
                        pev->angles.z=0;
 // run if distance to nearest player is too long
        if (toplayer.Length()>200)
        {
          WALK_MOVE(pev->pContainingEntity, VEC_TO_YAW(toplayer), 19, 0);
          //if (GetActivity()!=ACT_RUN) 
          //  SetActivity(ACT_RUN);
        }
        else
        {
          WALK_MOVE(pev->pContainingEntity, VEC_TO_YAW(toplayer), 6, 0);
          //if (GetActivity()!=ACT_WALK) 
         //   SetActivity(ACT_WALK);
        }
                }
        
        }

        else
        {
                UTIL_SetSize(pev,Vector(0,0,0),Vector(0,0,0));
        }
       
		 
      
        
        
}


void CBengala :: Move( CBaseEntity *pOther, int push )
{
	

	entvars_t*	pevToucher = pOther->pev;


	// Is entity standing on this pushable ?
	if ( FBitSet(pevToucher->flags,FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev )
	{
		// Only push if floating
		if ( pev->waterlevel > 0 )
			pev->velocity.z += pevToucher->velocity.z * 0.1;

		return;
	}


	if ( pOther->IsPlayer() )
	{
		if ( push && !(pevToucher->button & (IN_FORWARD|IN_USE)) )	// Don't push unless the player is pushing forward and NOT use (pull)
			pev->velocity.z += pevToucher->velocity.z * 0.1;	
	}

}