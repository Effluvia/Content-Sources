

//Moved to its own file from cbase.h.

#ifndef BASETOGGLE_H
#define BASETOGGLE_H

#include "cbase.h"
#include "animating.h"
#include "eiface.h"


//MODDD - TODO MAJOR.  Kindof.
// A lot of variables here are never used by CBaseMonster.  There may as well be a common "CBaseToggleBasic" for Monster and the actual CBaseToggle to inherit from instead,
// and anything that really needs the old CBaseToggle's stuff can just inherit from that one as usual.
// But low priotity.
///////////////////////////////////////////////////////////////////



//
// generic Toggle entity.
//
#define SF_ITEM_USE_ONLY	256 //  ITEM_USE_ONLY = BUTTON_USE_ONLY = DOOR_USE_ONLY!!! 



class CBaseToggle : public CBaseAnimating
{
public:
	void			KeyValue( KeyValueData *pkvd );

	TOGGLE_STATE		m_toggle_state;
	float			m_flActivateFinished;//like attack_finished, but for doors
	float			m_flMoveDistance;// how far a door should slide or rotate
	float			m_flWait;
	float			m_flLip;
	float			m_flTWidth;// for plats
	float			m_flTLength;// for plats

	Vector				m_vecPosition1;
	Vector				m_vecPosition2;
	Vector				m_vecAngle1;
	Vector				m_vecAngle2;

	int				m_cTriggersLeft;		// trigger_counter only, # of activations remaining
	float			m_flHeight;
	EHANDLE				m_hActivator;
	void (CBaseToggle::*m_pfnCallWhenMoveDone)(void);
	Vector				m_vecFinalDest;
	Vector				m_vecFinalAngle;

	int				m_bitsDamageInflict;	// DMG_ damage type that the door or tigger does

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	void PostRestore(void);

	static	TYPEDESCRIPTION m_SaveData[];

	virtual int	GetToggleState( void ) { return m_toggle_state; }
	virtual float GetDelay( void ) { return m_flWait; }

	// common member functions
	//MODDD - LinearMove and AngularMove made virtual. Shouldn't have to be overridden too often though.
	virtual void LinearMove( Vector	vecDest, float flSpeed );
	void EXPORT LinearMoveDone( void );
	virtual void AngularMove( Vector vecDestAngle, float flSpeed );
	void EXPORT AngularMoveDone( void );
	BOOL IsLockedByMaster( void );

	static float	AxisValue( int flags, const Vector &angles );
	static void		AxisDir( entvars_t *pev );
	static float	AxisDelta( int flags, const Vector &angle1, const Vector &angle2 );

	string_t m_sMaster;		// If this button has a master switch, this is the targetname.
							// A master switch must be of the multisource type. If all 
							// of the switches in the multisource have been triggered, then
							// the button will be allowed to operate. Otherwise, it will be
							// deactivated.
};
#define SetMoveDone( a ) m_pfnCallWhenMoveDone = static_cast <void (CBaseToggle::*)(void)> (a)


#endif //END OF BASETOGGLE_H