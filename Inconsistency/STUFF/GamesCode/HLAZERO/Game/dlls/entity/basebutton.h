


//Moved to its own file from cbase.h.

#ifndef BASEBUTTON_H
#define BASEBUTTON_H

#include "basetoggle.h"
#include "eiface.h"

char *ButtonSound( int sound );				// get string of button sound number

//
// Generic Button
//
class CBaseButton : public CBaseToggle
{
public:
	void Spawn( void );
	virtual void Precache( void );
	void RotSpawn( void );
	virtual void KeyValue( KeyValueData* pkvd);

	virtual BOOL IsWorldAffiliated(void);

	void ButtonActivate( );
	void SparkSoundCache( void );

	void EXPORT ButtonShot( void );
	
	//MODDD - CRITICAL.  Did... did I make this virtual?
	// I have no idea why if so, it's just making console warnings/errors.
	void EXPORT ButtonTouch( CBaseEntity *pOther );
	
	void EXPORT ButtonSpark ( void );
	void EXPORT TriggerAndWait( void );
	void EXPORT ButtonReturn( void );
	void EXPORT ButtonBackHome( void );
	void EXPORT ButtonUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );


	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL


	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	
	enum BUTTON_CODE { BUTTON_NOTHING, BUTTON_ACTIVATE, BUTTON_RETURN };
	BUTTON_CODE	ButtonResponseToTouch( void );
	
	static	TYPEDESCRIPTION m_SaveData[];
	// Buttons that don't take damage can be IMPULSE used
	virtual int ObjectCaps( void ) { return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | (pev->takedamage?0:FCAP_IMPULSE_USE); }

	BOOL	m_fStayPushed;	// button stays pushed in until touched again?
	BOOL	m_fRotating;		// a rotating button?  default is a sliding button.

	string_t m_strChangeTarget;	// if this field is not null, this is an index into the engine string array.
							// when this button is touched, it's target entity's TARGET field will be set
							// to the button's ChangeTarget. This allows you to make a func_train switch paths, etc.

	locksound_t m_ls;			// door lock sounds
	
	BYTE	m_bLockedSound;		// ordinals from entity selection
	BYTE	m_bLockedSentence;	
	BYTE	m_bUnlockedSound;	
	BYTE	m_bUnlockedSentence;
	int	m_sounds;
};


#endif //END OF BASEBUTTON_H