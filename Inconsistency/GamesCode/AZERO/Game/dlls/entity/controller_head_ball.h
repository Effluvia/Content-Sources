//=========================================================
// Controller bouncy ball attack
//=========================================================

//Originally in controller.cpp, split into its own file for re-use in other files. Good generic following (crudely) sphere of doom.

#ifndef CONTROLLER_HEAD_BALL_H
#define CONTROLLER_HEAD_BALL_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"

class CControllerHeadBall : public CBaseMonster
{
public:
	float nextNormalThinkTime;
	int m_iTrail;
	int m_flNextAttack;
	float timeOpacity;
	Vector m_vecIdeal;
	EHANDLE m_hOwner;
	BOOL alreadyZapped;


	CControllerHeadBall(void);

	// good idea?  doesn't seem helpful, didn't think so though.
	// Something sprite-based might not be able to benefit from collision-less bounds like this
	// (no physical model to tell what parts were really hit within this)
	void SetObjectCollisionBox( void ){
		pev->absmin = pev->origin + Vector(-10,-10,-10);
		pev->absmax = pev->origin + Vector(10,10,10);
	}

	void Spawn( void );
	void Precache( void );
	void EXPORT HuntThink( void );
	void EXPORT DieThink( void );
	void EXPORT BounceTouch( CBaseEntity *pOther );

	virtual BOOL usesSegmentedMove(void);

	virtual void MovetoTarget( Vector vecTarget );

	virtual void Crawl( void );

	virtual float massInfluence(void);
	virtual int GetProjectileType(void);

	virtual float nearZapDamage(void);
	// How much opacity do I lose per think cycle (0.1 seconds)?  Slow it down to last longer.
	// NOTICE - This is for the natural fade while traveling around normally.
	// The shot-down fadeout is much faster, doesn't involve this.
	virtual float getFadeOutAmount(void);
	virtual float StartHealth(void);
	virtual float GetMaxSpeed(void);

	void velocityCheck(const float& arg_maxSpeed);

	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL
	GENERATE_KILLED_PROTOTYPE
	void EXPORT DieFadeThink(void);

	void AdministerZap(CBaseEntity* pEntity, TraceResult& tr);

	int getNodeTypeAllowed(void);
	int getHullIndexForNodes(void);
	int getHullIndexForGroundNodes(void);


};


#endif //CONTROLLER_HEAD_BALL_H