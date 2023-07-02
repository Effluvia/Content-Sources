

#include "controller_head_ball.h"


#ifndef ARCHER_BALL_H
#define ARCHER_BALL_H

//MODDD - clone of the CControllerHeadBall with less homing ability, little less damage.

class CArcherBall : public CControllerHeadBall{

public:
	CArcherBall(void);
	
	void Spawn(void);
	void Precache(void);

	void MovetoTarget( Vector vecTarget );

	void EXPORT BounceTouch( CBaseEntity *pOther );

	float nearZapDamage(void);
	
	//How much opacity do I lose per think cycle (0.1 seconds)?  Slow it down to last longer.
	float getFadeOutAmount(void);
	float StartHealth(void);
	float GetMaxSpeed(void);

	GENERATE_KILLED_PROTOTYPE
	
};


#endif //ARCHER_BALL_H

