//MODDD - new file.
//Child class of CHornet (hornet.h).
//This is the Kingpin's hornet projectile with some particular behavior.


#include "hornet.h"


class CHornetKingpin : public CHornet{
	public:
	CHornetKingpin(void);

	
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];


	
	void Spawn( void );
	void Precache( void );
	
	void setup(CBaseEntity* arg_targetEnt, const Vector& arg_targetOffset);
	void setup(const Vector& arg_vecTarget, const Vector& arg_targetOffset);

	
	//MODDD - new for the kingpin to call.
	void EXPORT StartSpeedMissile(void);
	void EXPORT SpeedMissileDartStart(void);
	void EXPORT SpeedMissileDartContinuous(void);
	
	void EXPORT SmartDieTouch(CBaseEntity* pOther );
	
	Vector GetVelocityLogical(void);
	void SetVelocityLogical(const Vector& arg_newVelocity);
	void OnDeflected(CBaseEntity* arg_entDeflector);

	//MODDD - new.
	float expireTime;
	Vector speedMissileDartTarget;
	Vector speedMissileDartTargetOffset;
	Vector speedMissileDartDirection;




};
