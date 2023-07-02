
#include "cbase.h"



//MODDD - moved to make light classes includable.

class CLight : public CPointEntity
{
public:
	virtual void KeyValue( KeyValueData* pkvd ); 
	virtual void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	//MODDD - new.
	void TurnOn(void);
	void TurnOff(void);


	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

private:
	int	m_iStyle;
	int	m_iszPattern;
};




class CEnvLight : public CLight
{
public:
	void KeyValue( KeyValueData* pkvd ); 
	void Spawn( void );
};


