// NEW FILE.  Holds extra classes epexcted to be used by the player, 
// moved from player.cpp to here and player_extra.cpp.

#ifndef PLAYER_EXTRA_H
#define PLAYER_EXTRA_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "basemonster.h"




//==============================================
// !!!UNDONE:ultra temporary SprayCan entity to apply
// decal frame at a time. For PreAlpha CD
//==============================================
class CSprayCan : public CBaseEntity
{
public:
	void Spawn ( entvars_t *pevOwner );
	void Think( void );

	virtual int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
};


//=========================================================
// Dead HEV suit prop
//=========================================================
class CDeadHEV : public CBaseMonster
{
public:
	void Spawn( void );
	int Classify ( void ) { return	CLASS_HUMAN_MILITARY; }

	void KeyValue( KeyValueData *pkvd );

	int m_iPose;// which sequence to display	-- temporary, don't need to save
	static char *m_szPoses[4];
};


class CStripWeapons : public CPointEntity
{
public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

private:
};


class CRevertSaved : public CPointEntity
{
public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT MessageThink( void );
	void EXPORT LoadThink( void );
	void KeyValue( KeyValueData *pkvd );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	inline	float Duration( void ) { return pev->dmg_take; }
	inline	float HoldTime( void ) { return pev->dmg_save; }
	inline	float MessageTime( void ) { return m_messageTime; }
	inline	float LoadTime( void ) { return m_loadTime; }

	inline	void SetDuration( float duration ) { pev->dmg_take = duration; }
	inline	void SetHoldTime( float hold ) { pev->dmg_save = hold; }
	inline	void SetMessageTime( float time ) { m_messageTime = time; }
	inline	void SetLoadTime( float time ) { m_loadTime = time; }

private:
	float m_messageTime;
	float m_loadTime;
};



//MODDDMIRROR
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//==========================================================
// player marker for right mirroring a player in env_mirror
//==========================================================
class CPlayerMarker : public CBaseEntity
{
public:
	void Spawn( void );
	void Precache ( void );
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//=========================================================
// Multiplayer intermission spots.
//=========================================================
class CInfoIntermission:public CPointEntity
{
	void Spawn( void );
	void Think( void );
};



#endif //PLAYER_EXTRA_H