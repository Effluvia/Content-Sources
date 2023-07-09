#include        "extdll.h"
#include        "util.h"
#include        "cbase.h"
#include        "monsters.h"
#include        "weapons.h"
#include        "soundent.h"
#include        "gamerules.h"
#include        "animation.h"
#include "../engine/studio.h"


class CWall2x2 : public CBaseMonster
{
public:
        void Spawn( void );
        void Precache( void );
        int      Classify ( void );
        
        void SetActivity(int activity);                                 
        int  GetActivity() { return m_Activity; }
        void SetCollisionBox();
        void EXPORT IdleThink();
        int m_Activity;                                                                
        
};
LINK_ENTITY_TO_CLASS(wall2x2,CWall2x2);

void CWall2x2::Spawn(void)
{
        Precache();
        
        pev->movetype   = MOVETYPE_TOSS;
        pev->solid              = SOLID_BBOX;
        pev->takedamage = DAMAGE_NO;
        pev->flags              |= FL_MONSTER;
        pev->health             = 9999999;
        pev->gravity    = 1.0;

	
        
        SET_MODEL(ENT(pev), "models/build/wall2x2.mdl");

        SetActivity(ACT_IDLE);
        SetSequenceBox();
        SetThink(IdleThink);
        pev->nextthink = gpGlobals->time + 1;           

}

void CWall2x2::Precache(void)
{
        PRECACHE_MODEL("models/build/wall2x2.mdl");
}

int     CWall2x2::Classify(void)

{
        
        return  CLASS_NONE;

}

void CWall2x2::SetActivity(int act)
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

void CWall2x2::SetCollisionBox()
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

void CWall2x2::IdleThink()
{
        float flInterval = StudioFrameAdvance();

        pev->nextthink = gpGlobals->time + 1;
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
                SetCollisionBox();
                int i;
                CBaseEntity *pNearestPlayer=NULL;
                float nearestdistance=1000;
		
        }

        else
        {
                UTIL_SetSize(pev,Vector(0,0,0),Vector(0,0,0));
        }
}

