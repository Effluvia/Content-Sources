// NEW FILE.  Moved from cbase.h (class definition) and combat.cpp (implementations)

#ifndef GIB_H
#define GIB_H

#include "cbase.h"


class CGib;



void EstablishGutLoverGib(CGib* pGib, entvars_t* pevVictim, entvars_t* pevPlayer, BOOL isHead);
void EstablishGutLoverGib(CGib* pGib, entvars_t* pevVictim, const Vector gibSpawnOrigin, entvars_t* pevPlayer, BOOL isHead);



//
// A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
//
class CGib : public CBaseEntity
{
public:
	int	m_bloodColor;
	int	m_cBloodDecals;
	int	m_material;
	float m_lifeTime;



	void Spawn(const char* szGibModel);
	void Spawn(const char* szGibModel, BOOL spawnsDecal);

	void EXPORT BounceGibTouch(CBaseEntity* pOther);
	void EXPORT StickyGibTouch(CBaseEntity* pOther);
	void EXPORT WaitTillLand(void);
	void	LimitVelocity(void);

	virtual int ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }


	static void SpawnHeadGib(entvars_t* pevVictim);
	static void SpawnHeadGib(entvars_t* pevVictim, BOOL spawnDecals);
	//MODDD - this version accepts a different origin from the default determined typically.
	static void SpawnHeadGib(entvars_t* pevVictim, const Vector gibSpawnOrigin);
	static void SpawnHeadGib(entvars_t* pevVictim, const Vector gibSpawnOrigin, BOOL spawnDecals);

	//MODDD - third parameter chagned from "human" (as in, "should we spawn human gibs or, otherwise, alien gibs"?) to "argSpawnGibID", to call upon a particular aryGibInfo choice (element of that ID).

	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int argSpawnGibID);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int argSpawnGibID, BOOL spawnDecals);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int argSpawnGibID, BOOL spawnDecals, int argBloodColor);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const GibInfo_t& gibInfoChoice);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const GibInfo_t& gibInfoChoice, BOOL spawnDecals);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const GibInfo_t& gibInfoChoice, BOOL spawnDecals, int argBloodColor);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* argGibPath, int gibBodyMin, int gibBodyMax);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* argGibPath, int gibBodyMin, int gibBodyMax, BOOL spawnDecals);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* argGibPath, int gibBodyMin, int gibBodyMax, BOOL spawnDecals, int argBloodColor);


	static void SpawnStickyGibs(entvars_t* pevVictim, Vector vecOrigin, int cGibs);
	static void SpawnStickyGibs(entvars_t* pevVictim, Vector vecOrigin, int cGibs, BOOL spawnDecals);



	//MODDD - stubs.  Because why not.
	GENERATE_TRACEATTACK_PROTOTYPE
		GENERATE_TAKEDAMAGE_PROTOTYPE

		float massInfluence(void);

};


#endif //GIB_H