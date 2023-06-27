//=========================================================
// Bullsquid's spit projectile
//=========================================================

//Originally in bullsquid.cpp, split into its own file for re-use in other files. Good organic projectile sprite / model (depending on CVar, could make customizable per monster that uses it too)

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"

//NOTICE - we need animating functionality in case this is a model!
//class CSquidSpit : public CBaseEntity
class CSquidSpit : public CBaseAnimating
{
public:
	static int iSquidSpitSprite;
	int m_maxFrame;
	BOOL doHalfDuration;
	BOOL useAltFireSound;


	CSquidSpit(void);
	BOOL usesSoundSentenceSave(void);

	void Spawn( void );
	static void precacheStatic(void);

	//MODDD - new field in there.

	static CSquidSpit* Shoot( CBaseMonster* argFiringEntity, Vector vecStart, Vector vecDirection, float argSpeed  );
	static CSquidSpit* Shoot( entvars_t *pevOwner, Vector vecStart, Vector vecDirection, float argSpeed, const Vector& vecDest, const Vector& vecMinBounds, const Vector& vecMaxBounds );
	void Touch( CBaseEntity *pOther );
	void EXPORT Animate( void );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];


	
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	float massInfluence(void);
	int GetProjectileType(void);


};

