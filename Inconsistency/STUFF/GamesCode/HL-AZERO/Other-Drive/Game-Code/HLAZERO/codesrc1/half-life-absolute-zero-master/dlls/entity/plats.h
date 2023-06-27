

#ifndef PLATS_H
#define PLATS_H


#include "util.h"
#include "cbase.h"
#include "basetoggle.h"




static void PlatSpawnInsideTrigger(entvars_t* pevPlatform);

#define SF_PLAT_TOGGLE		0x0001

class CBasePlatTrain : public CBaseToggle
{
public:
	virtual int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void KeyValue( KeyValueData* pkvd);

	virtual BOOL IsWorldAffiliated(void);

	void Precache( void );

	// This is done to fix spawn flag collisions between this class and a derived class
	virtual BOOL IsTogglePlat( void ) { return (pev->spawnflags & SF_PLAT_TOGGLE) ? TRUE : FALSE; }

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	BYTE	m_bMoveSnd;			// sound a plat makes while moving
	BYTE	m_bStopSnd;			// sound a plat makes when it stops
	float m_volume;			// Sound volume
};












class CFuncPlat : public CBasePlatTrain
{
public:
	void Spawn( void );
	void Precache( void );
	void Setup( void );

	virtual void Blocked( CBaseEntity *pOther );


	void EXPORT PlatUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual BOOL IsWorldAffiliated(void);

	void EXPORT CallGoDown( void ) { GoDown(); }
	void EXPORT CallHitTop( void  ) { HitTop(); }
	void EXPORT CallHitBottom( void ) { HitBottom(); }

	virtual void GoUp( void );
	virtual void GoDown( void );
	virtual void HitTop( void );
	virtual void HitBottom( void );
};





class CFuncPlatRot : public CFuncPlat
{
public:
	void Spawn( void );
	void SetupRotation( void );

	virtual void GoUp( void );
	virtual void GoDown( void );
	virtual void HitTop( void );
	virtual void HitBottom( void );
	
	void		RotMove( Vector &destAngle, float time );
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	Vector	m_end, m_start;
};









#endif //END OF PLATS_H
