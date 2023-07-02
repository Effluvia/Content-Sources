//=========================================================
// SNAP BUG (snapbug)
//=========================================================

#ifndef SNAPBUG_H
#define SNAPBUG_H


#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"


class CSnapBug : public CBaseMonster{
public:
	
	CUSTOM_SCHEDULES;
	
	//save info
	//////////////////////////////////////////////////////////////////////////////////
	virtual int	Save( CSave &save ); 
	virtual int	Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];
	//////////////////////////////////////////////////////////////////////////////////



	
};//END OF class CSnapBug



#endif //END OF #ifdef SNAPBUG_H
