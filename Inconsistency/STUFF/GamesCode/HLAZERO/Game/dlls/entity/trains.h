/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef TRAINS_H
#define TRAINS_H

#include "cbase.h"   //MODDD - why not????

//MODDD - is that safe?
#include "plats.h"



// Tracktrain spawn flags
#define SF_TRACKTRAIN_NOPITCH		0x0001
#define SF_TRACKTRAIN_NOCONTROL		0x0002
#define SF_TRACKTRAIN_FORWARDONLY	0x0004
#define SF_TRACKTRAIN_PASSABLE		0x0008

// Spawnflag for CPathTrack
#define SF_PATH_DISABLED		0x00000001
#define SF_PATH_FIREONCE		0x00000002
#define SF_PATH_ALTREVERSE		0x00000004
#define SF_PATH_DISABLE_TRAIN	0x00000008
#define SF_PATH_ALTERNATE		0x00008000

// Spawnflags of CPathCorner
#define SF_CORNER_WAITFORTRIG	0x001
#define SF_CORNER_TELEPORT		0x002
#define SF_CORNER_FIREONCE		0x004



// was defined above CFuncTrackChange
#define SF_TRACK_ACTIVATETRAIN		0x00000001  //Is this auto movement?
#define SF_TRACK_RELINK				0x00000002  //Completely unused... ?
#define SF_TRACK_ROTMOVE			0x00000004  //This spawnflag is now redundant with other settings, unused and unnecessary.
#define SF_TRACK_STARTBOTTOM		0x00000008
#define SF_TRACK_DONT_MOVE			0x00000010




//MODDD - tried uncommenting this to enable it, can't tell the difference?
//#define PATH_SPARKLE_DEBUG		1	// This makes a particle effect around path_track entities for debugging

class CPathTrack : public CPointEntity
{
public:
	CPathTrack(void);

	void	Spawn( void );
	void	Activate( void );
	void	KeyValue( KeyValueData* pkvd);
	
	virtual BOOL IsWorldAffiliated(void);

	void	SetPrevious( CPathTrack *pprevious );
	void	Link( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	CPathTrack	*ValidPath( CPathTrack *ppath, int testFlag );		// Returns ppath if enabled, NULL otherwise
	void	Project( CPathTrack *pstart, CPathTrack *pend, Vector *origin, float dist );

	static CPathTrack *Instance( edict_t *pent );

	CPathTrack	*LookAhead( Vector *origin, float dist, int move );
	CPathTrack	*Nearest( Vector origin );

	CPathTrack	*GetNext( void );
	CPathTrack	*GetPrevious( void );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];
#if PATH_SPARKLE_DEBUG
	void EXPORT Sparkle(void);
#endif

	float	m_length;
	string_t	m_altName;
	CPathTrack	*m_pnext;
	CPathTrack	*m_pprevious;
	CPathTrack	*m_paltpath;

	int PathTrackID;
	static int PathTrackIDLatest;
};


class CFuncTrackTrain : public CBaseEntity
{
public:
	void Spawn( void );
	void Precache( void );

	void Blocked( CBaseEntity *pOther );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData* pkvd );

	virtual BOOL IsWorldAffiliated(void);

	void EXPORT Next( void );
	void EXPORT Find( void );
	void EXPORT NearestPath( void );
	void EXPORT DeadEnd( void );

	void	NextThink( float thinkTime, BOOL alwaysThink );

	void SetTrack( CPathTrack *track ) { m_ppath = track->Nearest(pev->origin); }
	void SetControls( entvars_t *pevControls );
	BOOL OnControls( entvars_t *pev );

	void StopSound ( void );
	void UpdateSound ( void );
	
	static CFuncTrackTrain *Instance( edict_t *pent );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];
	virtual int ObjectCaps( void ) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DIRECTIONAL_USE; }

	virtual void OverrideReset( void );

	CPathTrack	*m_ppath;
	float	m_length;
	float	m_height;
	float	m_speed;
	float	m_dir;
	float	m_startSpeed;
	Vector		m_controlMins;
	Vector		m_controlMaxs;
	int		m_soundPlaying;
	int		m_sounds;
	float	m_flVolume;
	float	m_flBank;
	float	m_oldSpeed;

private:
	unsigned short m_usAdjustPitch;
};




//
// This entity is a rotating/moving platform that will carry a train to a new track.
// It must be larger in X-Y planar area than the train, since it must contain the
// train within these dimensions in order to operate when the train is near it.
//

typedef enum { TRAIN_SAFE, TRAIN_BLOCKING, TRAIN_FOLLOWING } TRAIN_CODE;

//MODDD - debugging assist
extern const char* TRAIN_CODE_STR[];

class CFuncTrackChange : public CFuncPlatRot
{
public:
	CFuncTrackChange(void);

	void Spawn( void );
	void Precache( void );

//	virtual void Blocked( void );
	virtual void EXPORT GoUp( void );
	virtual void EXPORT GoDown( void );

	void		KeyValue( KeyValueData* pkvd );

	virtual BOOL IsWorldAffiliated(void);

	void		Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void		EXPORT Find( void );
	TRAIN_CODE		EvaluateTrain( CPathTrack *pcurrent );
	void		UpdateTrain( Vector &dest );
	virtual void HitBottom( void );
	virtual void HitTop( void );
	void		Touch( CBaseEntity *pOther );
	virtual void UpdateAutoTargets( int toggleState );
	virtual	BOOL	IsTogglePlat( void ) { return TRUE; }

	void		DisableUse( void ) { m_use = 0; }
	void		EnableUse( void ) { m_use = 1; }
	int			UseEnabled( void ) { return m_use; }

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	virtual void OverrideReset( void );

	virtual void ReportGeneric(void);


	CPathTrack		*m_trackTop;
	CPathTrack		*m_trackBottom;

	CFuncTrackTrain	*m_train;

	int			m_trackTopName;
	int			m_trackBottomName;
	int			m_trainName;
	TRAIN_CODE		m_code;
	int			m_targetState;
	int			m_use;

	//MODDD - NEW debugging feature
	int FuncTrackChangeID;
	static int FuncTrackChangeIDLatest;

};






















#endif
