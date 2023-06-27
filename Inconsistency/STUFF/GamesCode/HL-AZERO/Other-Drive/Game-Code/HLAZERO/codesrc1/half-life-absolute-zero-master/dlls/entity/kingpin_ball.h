

#include "controller_head_ball.h"



#ifndef KINGPIN_BALL_H
#define KINGPIN_BALL_H


//MODDD - clone of the CControllerHeadBall with less homing ability, little less damage.

class CKingpinBall : public CControllerHeadBall{

public:

	float pathfind_RouteRetryTime;
	BOOL pathfind_onRoute;
	int pathfind_consecutiveFails;


	CKingpinBall(void);
	
	void Spawn(void);
	void Precache(void);

	void MovetoTarget( Vector vecTarget );
	void pathfind_move(const float& arg_distToEnemy);

	float nearZapDamage(void);


	void startSmartFollow(void);

	void EXPORT BounceTouch( CBaseEntity *pOther );

	void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );

	BOOL ShouldAdvanceRoute( float flWaypointDist, float flInterval );

	float MoveYawDegreeTolerance(void);
	void Stop(void);
	void SetActivity(Activity NewActivity);
	
	int CheckLocalMove( const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, BOOL doZCheck, float* pflDist );
	
	float getFadeOutAmount(void);
	float StartHealth(void);
	float GetMaxSpeed(void);

	GENERATE_KILLED_PROTOTYPE


};


#endif //KINGPIN_BALL_H