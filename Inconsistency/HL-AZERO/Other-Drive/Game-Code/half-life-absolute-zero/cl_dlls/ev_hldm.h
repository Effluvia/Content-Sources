//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( EV_HLDMH )
#define EV_HLDMH

#include "../common/pmtrace.h"
#include "util_shared.h"

//MODDD - 'bullet types' enum moved to util_shared.
// ALSO - several things already seen in weapon script files (like some macros and enums for sequences)
// moved to their respective new dlls/<specific weapon>.h files to avoid redundancy, since a lot of this was
// already in their own dlls/<specific weapon>.cpp files.
// Including those files as needed in ev_hldm.cpp or hl/hl_weapons.cpp will get the same effect as before
// without requiring duplicates here and in the .cpp's.


//MODDD - other EV_... method prototypes / externs moved to here instead.  Now hl_events.cpp just has to include this to get all those too.
//        And yes it was found in a "extern 'C'" group.
extern "C"
{

// HLDM
void EV_FireGlock1( struct event_args_s *args  );
void EV_FireGlock2( struct event_args_s *args  );
void EV_FireShotGunSingle( struct event_args_s *args  );
void EV_FireShotGunDouble( struct event_args_s *args  );
void EV_FireMP5( struct event_args_s *args  );
void EV_FireMP52( struct event_args_s *args  );
void EV_FirePython( struct event_args_s *args  );
void EV_FireGauss( struct event_args_s *args  );
void EV_SpinGauss( struct event_args_s *args  );
void EV_Crowbar( struct event_args_s *args  );
void EV_FireCrossbow( struct event_args_s *args  );
void EV_FireCrossbow2( struct event_args_s *args  );
void EV_FireRpg( struct event_args_s *args  );
void EV_EgonFire( struct event_args_s *args  );
void EV_EgonStop( struct event_args_s *args  );
void EV_HornetGunFire( struct event_args_s *args  );
void EV_TripmineFire( struct event_args_s *args  );
void EV_SnarkFire( struct event_args_s *args  );
//MODDD
void EV_ChumToadFire( struct event_args_s *args );

//MODDDMIRROR
void EV_Mirror( struct event_args_s *args ); 

void EV_TrainPitchAdjust( struct event_args_s *args );

//MODDD
void EV_Trail_EngineChoice(event_args_s* args);
void EV_imitation7(event_args_s* args);
void EV_Trail( event_args_s *args  );
void EV_rocketAlphaTrail( event_args_s *args  );

void EV_ShowBalls( event_args_s* args);
void EV_ShowBallsPowerup( event_args_s* args);
void EV_QuakeExplosionEffect( event_args_s* args);
void EV_HLDM_DecalGunshotCustomEvent( event_args_s* args);

void EV_FreakyLight( event_args_s* args);
void EV_FriendlyVomit( event_args_s* args);
void EV_FloaterExplode(struct event_args_s* args);


}//END OF extern "C"




void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName );
void EV_HLDM_DecalGunshot( pmtrace_t *pTrace, int iBulletType );
BOOL EV_HLDM_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount );
void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY );

#endif // EV_HLDMH