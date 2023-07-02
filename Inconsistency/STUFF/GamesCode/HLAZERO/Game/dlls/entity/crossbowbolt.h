
#ifndef CROSSBOWBOLT_H
#define CROSSBOWBOLT_H

#include "extdll.h"
//#include "util.h"
//#include "cbase.h"
#include "weapons.h"


#define BOLT_AIR_VELOCITY	2000
#define BOLT_WATER_VELOCITY	1000


// UNDONE: Save/restore this?  Don't forget to set classname and LINK_ENTITY_TO_CLASS()
// 
// OVERLOADS SOME ENTVARS:
//
// speed - the ideal magnitude of my velocity
class CCrossbowBolt : public CBaseEntity
{

	//MODDD - a ... defined constructor defaults to private, but the auto-generated one didn't? I got nothing.
public:

	static CCrossbowBolt* BoltCreate(const Vector& arg_velocity, float arg_speed);
	static CCrossbowBolt* BoltCreate(const Vector& arg_velocity, float arg_speed, BOOL useTracer, BOOL arg_noDamage);


	CCrossbowBolt(void);

	static TYPEDESCRIPTION m_SaveData[];
	int Save(CSave& save);
	int Restore(CRestore& restore);

	void Spawn(void);
	void Spawn(BOOL useTracer, BOOL arg_noDamage);
	void Precache(void);
	int  Classify(void);
	void EXPORT BoltThink(void);
	void EXPORT BoltTouch(CBaseEntity* pOther);
	void EXPORT ExplodeThink(void);

	float massInfluence(void);
	int GetProjectileType(void);

	Vector GetVelocityLogical(void);
	void SetVelocityLogical(const Vector& arg_newVelocity);


	int m_iTrail;
	Vector recentVelocity;
	Vector m_velocity;  //velocity set by some outside source. Kept for memory in case of other influences.
	float m_speed;  //why is this set separately? isn't it redundant being supplied this and a velocity, which is kinda both a direction and speed in one? who knows.
	float realNextThink;
	BOOL noDamage;
	BOOL hitSomething;

	void hitEffect(const TraceResult& tr);

};


#endif //#ifndef CROSSBOWBOLT_H
