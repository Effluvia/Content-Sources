/*
NOTE:
In the original SDK NPCs can't open func_doors, only prop_doors.

In HOE all the doors are func_doors. Doh!

In order to give func_doors and prop_doors the same behaviour I created a new base class
called CBaseDoor which is nearly identical to the original CBasePropDoor. Both func_doors and
prop_doors are derived from this new class.

ORIGINAL SDK                    HOE
---------------------------------------------------
CBasePropDoor                   CBaseDoor<>
CPropDoorRotating               CBaseDoorRotating<>

class CPropDoorRotating : public CBaseDoorRotating<CDynamicProp>
class CFuncDoorRotating : public CBaseDoorRotating<CBaseEntity>

Also the linear-moving door class:
class CFuncDoor : public CBaseDoorLinear<CBaseEntity>
*/

#include "cbase.h"
#include "doors.h"
#include "ai_basenpc.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "locksounds.h"
#include "filters.h"
#include "physics.h"
#include "vphysics_interface.h"
#include "entityoutput.h"
#include "vcollide_parse.h"
#include "studio.h"
#include "explode.h"
#include "utlrbtree.h"
#include "tier1/strtools.h"
#include "physics_impact_damage.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "scriptevent.h"
#include "entityblocker.h"
#include "soundent.h"
#include "EntityFlame.h"
#include "game.h"
#include "physics_prop_ragdoll.h"
#include "decals.h"
#include "hierarchy.h"
#include "shareddefs.h"
#include "physobj.h"
#include "physics_npc_solver.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "datacache/imdlcache.h"
#include "model_types.h"
#if 1
#include "CollisionUtils.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//ConVar g_debug_doors( "g_debug_doors", "0" );

//=============================================================================================================
// BASE DOOR
//=============================================================================================================
//
// Private activities.
//
static int ACT_DOOR_OPEN = 0;
static int ACT_DOOR_LOCKED = 0;

//
// Anim events.
//
enum
{
	AE_DOOR_OPEN = 1,	// The door should start opening.
};

#define DOOR_SENTENCEWAIT	6
#define DOOR_SOUNDWAIT		1
#define BUTTON_SOUNDWAIT	0.5

//-----------------------------------------------------------------------------
// Purpose: play door or button locked or unlocked sounds. 
//			NOTE: this routine is shared by doors and buttons
// Input  : pEdict - 
//			pls - 
//			flocked - if true, play 'door is locked' sound, otherwise play 'door
//				is unlocked' sound.
//			fbutton - 
//-----------------------------------------------------------------------------
void PlayLockSounds(CBaseEntity *pEdict, locksound_t *pls, int flocked, int fbutton)
{
	if ( pEdict->HasSpawnFlags( SF_DOOR_SILENT ) )
	{
		return;
	}
	float flsoundwait = ( fbutton ) ? BUTTON_SOUNDWAIT : DOOR_SOUNDWAIT;

	if ( flocked )
	{
		int		fplaysound = (pls->sLockedSound != NULL_STRING && gpGlobals->curtime > pls->flwaitSound);
		int		fplaysentence = (pls->sLockedSentence != NULL_STRING && !pls->bEOFLocked && gpGlobals->curtime > pls->flwaitSentence);
		float	fvol = ( fplaysound && fplaysentence ) ? 0.25f : 1.0f;

		// if there is a locked sound, and we've debounced, play sound
		if (fplaysound)
		{
			// play 'door locked' sound
			CPASAttenuationFilter filter( pEdict );

			EmitSound_t ep;
			ep.m_nChannel = CHAN_ITEM;
			ep.m_pSoundName = (char*)STRING(pls->sLockedSound);
			ep.m_flVolume = fvol;
			ep.m_SoundLevel = SNDLVL_NORM;

			CBaseEntity::EmitSound( filter, pEdict->entindex(), ep );
			pls->flwaitSound = gpGlobals->curtime + flsoundwait;
		}

		// if there is a sentence, we've not played all in list, and we've debounced, play sound
		if (fplaysentence)
		{
			// play next 'door locked' sentence in group
			int iprev = pls->iLockedSentence;
			
			pls->iLockedSentence = SENTENCEG_PlaySequentialSz(	pEdict->edict(), 
																STRING(pls->sLockedSentence), 
																0.85f, 
																SNDLVL_NORM, 
																0, 
																100, 
																pls->iLockedSentence, 
																FALSE);
			pls->iUnlockedSentence = 0;

			// make sure we don't keep calling last sentence in list
			pls->bEOFLocked = (iprev == pls->iLockedSentence);
		
			pls->flwaitSentence = gpGlobals->curtime + DOOR_SENTENCEWAIT;
		}
	}
	else
	{
		// UNLOCKED SOUND

		int fplaysound = (pls->sUnlockedSound != NULL_STRING && gpGlobals->curtime > pls->flwaitSound);
		int fplaysentence = (pls->sUnlockedSentence != NULL_STRING && !pls->bEOFUnlocked && gpGlobals->curtime > pls->flwaitSentence);
		float fvol;

		// if playing both sentence and sound, lower sound volume so we hear sentence
		fvol = ( fplaysound && fplaysentence ) ? 0.25f : 1.0f;

		// play 'door unlocked' sound if set
		if (fplaysound)
		{
			CPASAttenuationFilter filter( pEdict );

			EmitSound_t ep;
			ep.m_nChannel = CHAN_ITEM;
			ep.m_pSoundName = (char*)STRING(pls->sUnlockedSound);
			ep.m_flVolume = fvol;
			ep.m_SoundLevel = SNDLVL_NORM;

			CBaseEntity::EmitSound( filter, pEdict->entindex(), ep );
			pls->flwaitSound = gpGlobals->curtime + flsoundwait;
		}

		// play next 'door unlocked' sentence in group
		if (fplaysentence)
		{
			int iprev = pls->iUnlockedSentence;
			
			pls->iUnlockedSentence = SENTENCEG_PlaySequentialSz(pEdict->edict(), STRING(pls->sUnlockedSentence), 
					  0.85, SNDLVL_NORM, 0, 100, pls->iUnlockedSentence, FALSE);
			pls->iLockedSentence = 0;

			// make sure we don't keep calling last sentence in list
			pls->bEOFUnlocked = (iprev == pls->iUnlockedSentence);
			pls->flwaitSentence = gpGlobals->curtime + DOOR_SENTENCEWAIT;
		}
	}
}

BEGIN_DATADESC_NO_BASE(locksound_t)

	DEFINE_FIELD( sLockedSound,	FIELD_STRING),
	DEFINE_FIELD( sLockedSentence,	FIELD_STRING ),
	DEFINE_FIELD( sUnlockedSound,	FIELD_STRING ),
	DEFINE_FIELD( sUnlockedSentence, FIELD_STRING ),
	DEFINE_FIELD( iLockedSentence, FIELD_INTEGER ),
	DEFINE_FIELD( iUnlockedSentence, FIELD_INTEGER ),
	DEFINE_FIELD( flwaitSound,		FIELD_FLOAT ),
	DEFINE_FIELD( flwaitSentence,	FIELD_FLOAT ),
	DEFINE_FIELD( bEOFLocked,		FIELD_CHARACTER ),
	DEFINE_FIELD( bEOFUnlocked,	FIELD_CHARACTER ),

END_DATADESC()

template < class ENTITY_CLASS=CBaseEntity >
class CBaseDoor : public ENTITY_CLASS, public IDoor
{
public:

	DECLARE_CLASS( CBaseDoor, ENTITY_CLASS );
	DECLARE_DATADESC();

	CBaseDoor( void );

	void Spawn();
	void Precache();
	void Activate();
	int	ObjectCaps();

	class Accessor : public IDoorAccessor
	{
	public:
		Accessor(CBaseDoor<ENTITY_CLASS> *pDoor)
			: m_pDoor(pDoor)
		{
			Init( pDoor );
		};
		CBaseDoor<ENTITY_CLASS> *m_pDoor;

		bool IsDoorOpen() { return m_pDoor->IsDoorOpen(); }
		bool IsDoorAjar()  { return m_pDoor->IsDoorAjar(); }
		bool IsDoorOpening()  { return m_pDoor->IsDoorOpening(); }
		bool IsDoorClosed()  { return m_pDoor->IsDoorClosed(); }
		bool IsDoorClosing()  { return m_pDoor->IsDoorClosing(); }
		bool IsDoorLocked( CBaseEntity *pActivator ) { return m_pDoor->IsDoorLocked( pActivator ); }
		bool IsDoorBlocked() const { return m_pDoor->IsDoorBlocked(); }
		bool IsNPCOpening(CAI_BaseNPC *pNPC) { return m_pDoor->IsNPCOpening( pNPC ); }
		bool IsPlayerOpening() { return m_pDoor->IsPlayerOpening(); }
		bool IsOpener(CBaseEntity *pEnt) { return m_pDoor->IsOpener( pEnt ); }

		bool NPCOpenDoor(CAI_BaseNPC *pNPC) { return m_pDoor->NPCOpenDoor( pNPC ); }
		bool TestCollision( const Ray_t &ray, unsigned int mask, trace_t& trace ) { return m_pDoor->TestCollision( ray, mask, trace ); }

		bool DoorCanClose( bool bAutoClose ) { return m_pDoor->DoorCanClose( bAutoClose ); }
		bool DoorCanOpen( void ) { return m_pDoor->DoorCanOpen(); }

		void GetNPCOpenData(CAI_BaseNPC *pNPC, opendata_t &opendata) { m_pDoor->GetNPCOpenData( pNPC, opendata ); }
		float GetOpenInterval(void) { return m_pDoor->GetOpenInterval(); }
	};
	IDoorAccessor *GetDoorAccessor( void ) { return m_pAccessor; }
	Accessor *m_pAccessor;

	// Base class services.
	// Do not make the functions in this block virtual!!
	// {
	inline bool IsDoorOpen();
	inline bool IsDoorAjar();
	inline bool IsDoorOpening();
	inline bool IsDoorClosed();
	inline bool IsDoorClosing();
	inline bool IsDoorLocked( CBaseEntity *pActivator );
	inline bool IsDoorBlocked() const;
	inline bool IsNPCOpening(CAI_BaseNPC *pNPC);
	inline bool IsPlayerOpening();
	inline bool IsOpener(CBaseEntity *pEnt);

	bool NPCOpenDoor(CAI_BaseNPC *pNPC);
	// }

	// Implement these in your leaf class.
	// {
	virtual bool DoorCanClose( bool bAutoClose ) { return true; }
	virtual bool DoorCanOpen( void ) { return true; }

	virtual void GetNPCOpenData(CAI_BaseNPC *pNPC, opendata_t &opendata) = 0;
	virtual float GetOpenInterval(void) = 0;
	// }

protected:

	enum DoorState_t
	{
		DOOR_STATE_CLOSED = 0,
		DOOR_STATE_OPENING,
		DOOR_STATE_OPEN,
		DOOR_STATE_CLOSING,
		DOOR_STATE_AJAR,
	};

	// dvs: FIXME: make these private
	void DoorClose();

	CBaseDoor *GetMaster( void ) { return m_hMaster; }
	bool HasSlaves( void ) { return ( m_hDoorList.Count() > 0 ); }

	inline void SetDoorState( DoorState_t eDoorState );

	float m_flAutoReturnDelay;	// How many seconds to wait before automatically closing, -1 never closes automatically.
	CUtlVector< CHandle< CBaseDoor > >	m_hDoorList;	// List of doors linked to us

	inline CBaseEntity *GetActivator();

	void GetWorldSpaceAABB( Vector *mins, Vector *maxs )
	{
		CollisionProp()->WorldSpaceAABB( mins, maxs );

		const model_t *pModel = GetModel();
		if ( modelinfo->GetModelType( pModel ) == mod_brush )
		{
			// Subtract 1 from each dimension because the engine expands bboxes by 1 in all directions making the size too big
			*mins += Vector(1,1,1);
			*maxs -= Vector(1,1,1);
		}
	}

private:

	// Implement these in your leaf class.
	// {
	virtual void OnUseLocked( void );
	virtual void OnUseUnlocked( void );

	// Called when the door becomes fully open.
	virtual void OnDoorOpened() {}

	// Called when the door becomes fully closed.
	virtual void OnDoorClosed() {}

	// Called to tell the door to start opening.
	virtual void BeginOpening(CBaseEntity *pOpenAwayFrom) = 0;

	// Called to tell the door to start closing.
	virtual void BeginClosing( void ) = 0;

	// Called when blocked to tell the door to stop moving.
	virtual void DoorStop( void ) = 0;

	// Called when blocked to tell the door to continue moving.
	virtual void DoorResume( void ) = 0;
	
	// Called to send the door instantly to its spawn positions.
	virtual void DoorTeleportToSpawnPosition() = 0;
	// }

protected:
	virtual void CalcDoorSounds( void );

	locksound_t m_ls;			// The sounds the door plays when being locked, unlocked, etc.
	string_t m_SoundMoving;
	string_t m_SoundOpen;
	string_t m_SoundClose;

	bool DoorActivate(); // was private with comment "Do not make the functions in this block virtual!!"

private:

	void DoorTouch( CBaseEntity *pOther );

	// Main entry points for the door base behaviors.
	// Do not make the functions in this block virtual!!
	// {
	void DoorOpen( CBaseEntity *pOpenAwayFrom );
	void OpenIfUnlocked(CBaseEntity *pActivator, CBaseEntity *pOpenAwayFrom);

	void DoorOpenMoveDone();
	void DoorCloseMoveDone();
	void DoorAutoCloseThink();

	void Lock();
	void Unlock();

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void OnUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	inline bool WillAutoReturn() { return m_flAutoReturnDelay != -1; }

	void StartBlocked(CBaseEntity *pOther);
	void OnStartBlocked( CBaseEntity *pOther );
	void MasterStartBlocked( CBaseEntity *pOther );

	void Blocked(CBaseEntity *pOther);
	void EndBlocked(void);
	void OnEndBlocked( void );

	void UpdateAreaPortals(bool bOpen);

	// Input handlers
	void InputClose(inputdata_t &inputdata);
	void InputLock(inputdata_t &inputdata);
	void InputOpen(inputdata_t &inputdata);
	void InputOpenAwayFrom(inputdata_t &inputdata);
	void InputToggle(inputdata_t &inputdata);
	void InputUnlock(inputdata_t &inputdata);

	void SetDoorBlocker( CBaseEntity *pBlocker );

	void SetMaster( CBaseDoor *pMaster ) { m_hMaster = pMaster; }
	// }
	
	DoorState_t m_eDoorState;	// Holds whether the door is open, closed, opening, or closing.

	EHANDLE		m_hActivator;		
	
	bool	m_bLocked;				// True if the door is locked.
	float	m_flUseLockedTime;		// Controls how often we fire the OnUseLocked output.
	EHANDLE	m_hBlocker;				// Entity blocking the door currently
	bool	m_bFirstBlocked;		// Marker for being the first door (in a group) to be blocked (needed for motion control)

	bool m_bForceClosed;			// True if this door must close no matter what.

	// dvs: FIXME: can we remove m_flSpeed from CBaseEntity?
	//float m_flSpeed;			// Rotation speed when opening or closing in degrees per second.

	string_t m_SlaveName;

	CHandle< CBaseDoor > m_hMaster;

#define DOOR_MASTER // HL1-style master
#ifdef DOOR_MASTER
	string_t m_sMaster;
#endif

	static void RegisterPrivateActivities();

	// Outputs
	COutputEvent m_OnBlockedClosing;		// Triggered when the door becomes blocked while closing.
	COutputEvent m_OnBlockedOpening;		// Triggered when the door becomes blocked while opening.
	COutputEvent m_OnUnblockedClosing;		// Triggered when the door becomes unblocked while closing.
	COutputEvent m_OnUnblockedOpening;		// Triggered when the door becomes unblocked while opening.
	COutputEvent m_OnFullyClosed;			// Triggered when the door reaches the fully closed position.
	COutputEvent m_OnFullyOpen;				// Triggered when the door reaches the fully open position.
	COutputEvent m_OnClose;					// Triggered when the door is told to close.
	COutputEvent m_OnOpen;					// Triggered when the door is told to open.
	COutputEvent m_OnUseLocked;
};

#define BASE_DOOR_DATADESC \
	/*DEFINE_FIELD(m_bLockedSentence, FIELD_CHARACTER), */ \
	/*DEFINE_FIELD(m_bUnlockedSentence, FIELD_CHARACTER),*/ \
	DEFINE_KEYFIELD(m_flAutoReturnDelay, FIELD_FLOAT, "returndelay"), \
	DEFINE_FIELD( m_hActivator, FIELD_EHANDLE ), \
	DEFINE_KEYFIELD(m_SoundMoving, FIELD_SOUNDNAME, "soundmoveoverride"), \
	DEFINE_KEYFIELD(m_SoundOpen, FIELD_SOUNDNAME, "soundopenoverride"), \
	DEFINE_KEYFIELD(m_SoundClose, FIELD_SOUNDNAME, "soundcloseoverride"), \
	DEFINE_KEYFIELD(m_ls.sLockedSound, FIELD_SOUNDNAME, "soundlockedoverride"), \
	DEFINE_KEYFIELD(m_ls.sUnlockedSound, FIELD_SOUNDNAME, "soundunlockedoverride"), \
	DEFINE_KEYFIELD(m_SlaveName, FIELD_STRING, "slavename" ), \
	DEFINE_KEYFIELD( m_sMaster, FIELD_STRING, "master" ), /* DOOR_MASTER */\
	DEFINE_FIELD(m_bLocked, FIELD_BOOLEAN), \
	/*DEFINE_KEYFIELD(m_flBlockDamage, FIELD_FLOAT, "dmg"),*/ \
	DEFINE_KEYFIELD( m_bForceClosed, FIELD_BOOLEAN, "forceclosed" ), \
	DEFINE_FIELD(m_eDoorState, FIELD_INTEGER), \
	DEFINE_FIELD( m_hMaster, FIELD_EHANDLE ), \
	DEFINE_FIELD( m_hBlocker, FIELD_EHANDLE ), \
	DEFINE_FIELD( m_bFirstBlocked, FIELD_BOOLEAN ), \
	DEFINE_FIELD( m_flUseLockedTime, FIELD_TIME ), \
	/*DEFINE_FIELD(m_hDoorList, FIELD_CLASSPTR),	// Reconstructed */ \
	\
	DEFINE_INPUTFUNC(FIELD_VOID, "Open", InputOpen), \
	DEFINE_INPUTFUNC(FIELD_STRING, "OpenAwayFrom", InputOpenAwayFrom), \
	DEFINE_INPUTFUNC(FIELD_VOID, "Close", InputClose), \
	DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle), \
	DEFINE_INPUTFUNC(FIELD_VOID, "Lock", InputLock), \
	DEFINE_INPUTFUNC(FIELD_VOID, "Unlock", InputUnlock), \
	\
	DEFINE_OUTPUT(m_OnBlockedOpening, "OnBlockedOpening"), \
	DEFINE_OUTPUT(m_OnBlockedClosing, "OnBlockedClosing"), \
	DEFINE_OUTPUT(m_OnUnblockedOpening, "OnUnblockedOpening"), \
	DEFINE_OUTPUT(m_OnUnblockedClosing, "OnUnblockedClosing"), \
	DEFINE_OUTPUT(m_OnFullyClosed, "OnFullyClosed"), \
	DEFINE_OUTPUT(m_OnFullyOpen, "OnFullyOpen"), \
	DEFINE_OUTPUT(m_OnClose, "OnClose"), \
	DEFINE_OUTPUT(m_OnOpen, "OnOpen"), \
	DEFINE_OUTPUT( m_OnUseLocked, "OnUseLocked" ), \
	\
	DEFINE_EMBEDDED( m_ls ), \
	\
	/* Function Pointers */\
	DEFINE_THINKFUNC(DoorOpenMoveDone), \
	DEFINE_THINKFUNC(DoorCloseMoveDone), \
	DEFINE_THINKFUNC(DoorAutoCloseThink), \
	DEFINE_ENTITYFUNC(DoorTouch),

//IMPLEMENT_SERVERCLASS_ST(CBaseDoor, DT_BaseDoor)
//END_SEND_TABLE()

template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::SetDoorState( DoorState_t eDoorState )
{
	m_eDoorState = eDoorState;
}

template < class ENTITY_CLASS >
bool CBaseDoor<ENTITY_CLASS>::IsDoorOpen()
{
	return m_eDoorState == DOOR_STATE_OPEN;
}

template < class ENTITY_CLASS >
bool CBaseDoor<ENTITY_CLASS>::IsDoorAjar()
{
	return ( m_eDoorState == DOOR_STATE_AJAR );
}

template < class ENTITY_CLASS >
bool CBaseDoor<ENTITY_CLASS>::IsDoorOpening()
{
	return m_eDoorState == DOOR_STATE_OPENING;
}

template < class ENTITY_CLASS >
bool CBaseDoor<ENTITY_CLASS>::IsDoorClosed()
{
	return m_eDoorState == DOOR_STATE_CLOSED;
}

template < class ENTITY_CLASS >
bool CBaseDoor<ENTITY_CLASS>::IsDoorClosing()
{
	return m_eDoorState == DOOR_STATE_CLOSING;
}

template < class ENTITY_CLASS >
bool CBaseDoor<ENTITY_CLASS>::IsDoorLocked( CBaseEntity *pActivator )
{
	if ( pActivator && pActivator->IsNPC() && HasSpawnFlags( SF_DOOR_NONPCS ) )
		return true;
#ifdef DOOR_MASTER
	if ( !UTIL_IsMasterTriggered( m_sMaster, pActivator ) )
		return true;
#endif
	return m_bLocked;
}

template < class ENTITY_CLASS >
CBaseEntity *CBaseDoor<ENTITY_CLASS>::GetActivator()
{
	return m_hActivator;
}

template < class ENTITY_CLASS >
bool CBaseDoor<ENTITY_CLASS>::IsDoorBlocked() const
{
	return ( m_hBlocker != NULL );
}

template < class ENTITY_CLASS >
bool CBaseDoor<ENTITY_CLASS>::IsNPCOpening( CAI_BaseNPC *pNPC )
{
	return ( pNPC == ( CAI_BaseNPC * )GetActivator() );
}

template < class ENTITY_CLASS >
inline bool CBaseDoor<ENTITY_CLASS>::IsPlayerOpening()
{
	return ( GetActivator() && GetActivator()->IsPlayer() );
}

template < class ENTITY_CLASS >
inline bool CBaseDoor<ENTITY_CLASS>::IsOpener(CBaseEntity *pEnt)
{
	return ( GetActivator() == pEnt );
}

template < class ENTITY_CLASS >
CBaseDoor<ENTITY_CLASS>::CBaseDoor( void )
{
	m_hMaster = NULL;
	m_pAccessor = new Accessor( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::Spawn()
{
	BaseClass::Spawn();

	Precache();

	DoorTeleportToSpawnPosition();

	if (HasSpawnFlags(SF_DOOR_LOCKED))
	{
		m_bLocked = true;
	}

	SetMoveType(MOVETYPE_PUSH);
	
	if (m_flSpeed == 0)
	{
		m_flSpeed = 100;
	}

	SetTouch( &CBaseDoor::DoorTouch );
	
	SetSolid(SOLID_VPHYSICS);
	VPhysicsInitShadow(false, false);

	SetDoorBlocker( NULL );

	// Fills out the m_Soundxxx members.
	CalcDoorSounds();
}


//-----------------------------------------------------------------------------
// Purpose: Returns our capabilities mask.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
int	CBaseDoor<ENTITY_CLASS>::ObjectCaps()
{
	return BaseClass::ObjectCaps() | ( HasSpawnFlags( SF_DOOR_IGNORE_USE ) ? 0 : (FCAP_IMPULSE_USE|FCAP_USE_IN_RADIUS) );
};


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::Precache(void)
{
	BaseClass::Precache();

	RegisterPrivateActivities();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::RegisterPrivateActivities(void)
{
	static bool bRegistered = false;

	if (bRegistered)
		return;

	REGISTER_PRIVATE_ACTIVITY( ACT_DOOR_OPEN );
	REGISTER_PRIVATE_ACTIVITY( ACT_DOOR_LOCKED );

//	bRegistered = true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::Activate( void )
{
	BaseClass::Activate();
	
	UpdateAreaPortals( !IsDoorClosed() );

	// If we have a name, we may be linked
	if ( GetEntityName() != NULL_STRING )
	{
		CBaseEntity	*pTarget = NULL;

		// Find our slaves.
		// If we have a specified slave name, then use that to find slaves.
		// Otherwise, see if there are any other doors that match our name (Backwards compatability).
		string_t iszSearchName = GetEntityName();
		if ( m_SlaveName != NULL_STRING )
		{
			const char *pSlaveName = STRING(m_SlaveName);
			if ( pSlaveName && pSlaveName[0] )
			{
				iszSearchName = m_SlaveName;
			}
		}

		while ( ( pTarget = gEntList.FindEntityByName( pTarget, iszSearchName ) ) != NULL )
		{
			if ( pTarget != this )
			{
				CBaseDoor *pDoor = dynamic_cast<CBaseDoor *>(pTarget);

				if ( pDoor != NULL && pDoor->HasSlaves() == false )
				{
					m_hDoorList.AddToTail( pDoor );
					pDoor->SetMaster( this );
					pDoor->SetOwnerEntity( this );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::CalcDoorSounds( void )
{
	// Make sure we have real, precachable sound names in all cases.
	UTIL_ValidateSoundName( m_SoundMoving, "DoorSound.Null" );
	UTIL_ValidateSoundName( m_SoundOpen, "DoorSound.Null" );
	UTIL_ValidateSoundName( m_SoundClose, "DoorSound.Null" );
	UTIL_ValidateSoundName( m_ls.sLockedSound, "DoorSound.Null" );
	UTIL_ValidateSoundName( m_ls.sUnlockedSound, "DoorSound.Null" );

	PrecacheScriptSound( STRING( m_SoundMoving ) );
	PrecacheScriptSound( STRING( m_SoundOpen ) );
	PrecacheScriptSound( STRING( m_SoundClose ) );
	PrecacheScriptSound( STRING( m_ls.sLockedSound ) );
	PrecacheScriptSound( STRING( m_ls.sUnlockedSound ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : isOpen - 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::UpdateAreaPortals(bool isOpen)
{
	string_t name = GetEntityName();
	if (!name)
		return;
	
	CBaseEntity *pPortal = NULL;
	while ((pPortal = gEntList.FindEntityByClassname(pPortal, "func_areaportal")) != NULL)
	{
		if (pPortal->HasTarget(name))
		{
			// USE_ON means open the portal, off means close it
			pPortal->Use(this, this, isOpen?USE_ON:USE_OFF, 0);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::SetDoorBlocker( CBaseEntity *pBlocker )
{ 
	m_hBlocker = pBlocker; 

	if ( m_hBlocker == NULL )
	{
		m_bFirstBlocked = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when the player uses the door.
// Input  : pActivator - 
//			pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if ( GetMaster() != NULL )
	{
		// Tell our owner we've been used
		GetMaster()->Use( pActivator, pCaller, useType, value );
	}
	else
	{
		// Just let it through
		OnUse( pActivator, pCaller, useType, value );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::OnUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_hActivator = pActivator;

	// If we're blocked while closing, open away from our blocker. This will
	// liberate whatever bit of detritus is stuck in us.
	if ( IsDoorBlocked() && IsDoorClosing() )
	{
		DoorOpen( m_hBlocker );
		return;
	}

	if (IsDoorClosed() || (IsDoorOpen() && HasSpawnFlags(SF_DOOR_USE_CLOSES)))
	{
		// Ready to be opened or closed.
		if ( IsDoorLocked( pActivator ) )
		{
			OnUseLocked();
			if ( gpGlobals->curtime > m_flUseLockedTime )
			{
				m_OnUseLocked.FireOutput( pActivator, this );
				m_flUseLockedTime = gpGlobals->curtime + 0.5;
			}
		}
		else
		{
			OnUseUnlocked();
		}
	}
	else if ( IsDoorOpening() )
	{
		// We've been used while opening, close.
		DoorClose();
	}
	else if ( IsDoorClosing() || IsDoorAjar() )
	{
		DoorOpen( m_hActivator );
	}
}

//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::OnUseLocked( void )
{
	PlayLockSounds(this, &m_ls, TRUE, FALSE);
}

//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::OnUseUnlocked( void )
{
	PlayLockSounds(this, &m_ls, FALSE, FALSE);
	DoorActivate();
}

//-----------------------------------------------------------------------------
// Purpose: Closes the door if it is not already closed.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::InputClose(inputdata_t &inputdata)
{
	if (!IsDoorClosed())
	{	
		m_OnClose.FireOutput(inputdata.pActivator, this);
		DoorClose();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Input handler that locks the door.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::InputLock(inputdata_t &inputdata)
{
	Lock();
}

//-----------------------------------------------------------------------------
// Purpose: Opens the door if it is not already open.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::InputOpen(inputdata_t &inputdata)
{
	OpenIfUnlocked(inputdata.pActivator, NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Opens the door away from a specified entity if it is not already open.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::InputOpenAwayFrom(inputdata_t &inputdata)
{
	CBaseEntity *pOpenAwayFrom = gEntList.FindEntityByName( NULL, inputdata.value.String(), NULL, inputdata.pActivator, inputdata.pCaller );
	OpenIfUnlocked(inputdata.pActivator, pOpenAwayFrom);
}

//-----------------------------------------------------------------------------
// Purpose: 
// 
// FIXME: This function should be combined with DoorOpen, but doing that
//		  could break existing content. Fix after shipping!	
//
// Input  : *pOpenAwayFrom - 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::OpenIfUnlocked(CBaseEntity *pActivator, CBaseEntity *pOpenAwayFrom)
{
	// I'm locked, can't open
	if ( IsDoorLocked( pActivator ) )
		return; 

	if (!IsDoorOpen() && !IsDoorOpening())
	{	
		// Play door unlock sounds.
		PlayLockSounds(this, &m_ls, false, false);
		m_OnOpen.FireOutput(pActivator, this);
		DoorOpen(pOpenAwayFrom);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Opens the door if it is not already open.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::InputToggle(inputdata_t &inputdata)
{
	// I'm locked, can't open
	if ( IsDoorLocked( inputdata.pActivator ) )
		return; 

	if (IsDoorClosed())
	{	
		DoorOpen(NULL);
	}
	else if (IsDoorOpen())
	{
		DoorClose();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Input handler that unlocks the door.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::InputUnlock(inputdata_t &inputdata)
{
	Unlock();
}

//-----------------------------------------------------------------------------
// Purpose: Locks the door so that it cannot be opened.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::Lock(void)
{
	m_bLocked = true;
}

//-----------------------------------------------------------------------------
// Purpose: Unlocks the door so that it can be opened.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::Unlock(void)
{
#if 0 // FIXME: move to prop_door?
	if (!m_nHardwareType)
	{
		// Doors with no hardware must always be locked.
		DevWarning(1, "Unlocking prop_door '%s' at (%.0f %.0f %.0f) with no hardware. All openable doors must have hardware!\n", GetDebugName(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
	}
#endif
	m_bLocked = false;
}

//-----------------------------------------------------------------------------
// Purpose: Causes the door to "do its thing", i.e. start moving, and cascade activation.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
bool CBaseDoor<ENTITY_CLASS>::DoorActivate( void )
{
	if ( IsDoorOpen() && DoorCanClose( false ) )
	{
		DoorClose();
	}
	else
	{
		DoorOpen( m_hActivator );
	}

	return true;
}

//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::DoorTouch( CBaseEntity *pOther )
{
	// Ignore touches by anything but players.
	if ( !pOther->IsPlayer() )
	{
		return;
	}

	// If door is not opened by touch, do nothing.
	if ( !HasSpawnFlags( SF_DOOR_PTOUCH ) )
	{
		return; 
	}

	// Pretend we got +used
	Use( pOther, this, USE_TOGGLE, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Starts the door opening.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::DoorOpen(CBaseEntity *pOpenAwayFrom)
{
	// Don't bother if we're already doing this
	if ( IsDoorOpen() || IsDoorOpening() )
		return;

	UpdateAreaPortals(true);

	// It could be going-down, if blocked.
	ASSERT( IsDoorClosed() || IsDoorClosing() || IsDoorAjar() );

	// Emit door moving and stop sounds on CHAN_STATIC so that the multicast doesn't
	// filter them out and leave a client stuck with looping door sounds!
	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		EmitSound( STRING( m_SoundMoving ) );

		if ( m_hActivator && m_hActivator->IsPlayer() && !HasSpawnFlags( SF_DOOR_SILENT_TO_NPCS ) )
		{
			CSoundEnt::InsertSound( SOUND_PLAYER, GetAbsOrigin(), 512, 0.5, this );//<<TODO>>//magic number
		}
	}

	// disable touch until movement is complete
	SetTouch( NULL );

	SetDoorState( DOOR_STATE_OPENING );
	
	SetMoveDone(&CBaseDoor<ENTITY_CLASS>::DoorOpenMoveDone);

	// Virtual function that starts the door moving for whatever type of door this is.
	BeginOpening(pOpenAwayFrom);

	m_OnOpen.FireOutput(this, this);

	// Tell all the slaves
	if ( HasSlaves() )
	{
		int	numDoors = m_hDoorList.Count();

		CBaseDoor *pLinkedDoor = NULL;

		// Open all linked doors
		for ( int i = 0; i < numDoors; i++ )
		{
			pLinkedDoor = m_hDoorList[i];

			if ( pLinkedDoor != NULL )
			{
				// If the door isn't already moving, get it moving
				pLinkedDoor->m_hActivator = m_hActivator;
				pLinkedDoor->DoorOpen( pOpenAwayFrom );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: The door has reached the open position. Either close automatically
//			or wait for another activation.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::DoorOpenMoveDone(void)
{
	SetDoorBlocker( NULL );

	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		EmitSound( STRING( m_SoundOpen ) );
	}

	ASSERT(IsDoorOpening());
	SetDoorState( DOOR_STATE_OPEN );
	
	if (WillAutoReturn())
	{
		// In flWait seconds, DoorClose will fire, unless wait is -1, then door stays open
		SetMoveDoneTime(m_flAutoReturnDelay + 0.1);
		SetMoveDone(&CBaseDoor<ENTITY_CLASS>::DoorAutoCloseThink);

		if (m_flAutoReturnDelay == -1)
		{
			SetNextThink( TICK_NEVER_THINK );
		}
	}
	else
	{
		// Re-instate touch method, movement is complete
		SetTouch( &CBaseDoor::DoorTouch );
	}

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>(m_hActivator.Get());
	if (pNPC)
	{
		// Notify the NPC that opened us.
		pNPC->OnDoorFullyOpen(this);
	}

	m_OnFullyOpen.FireOutput(this, this);

	// Let the leaf class do its thing.
	OnDoorOpened();

	m_hActivator = NULL;
}


//-----------------------------------------------------------------------------
// Purpose: Think function that tries to close the door. Used for autoreturn.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::DoorAutoCloseThink(void)
{
	// When autoclosing, we check both sides so that we don't close in the player's
	// face, or in an NPC's face for that matter, because they might be shooting
	// through the doorway.
	if ( !DoorCanClose( true ) )
	{
		if (m_flAutoReturnDelay == -1)
		{
			SetNextThink( TICK_NEVER_THINK );
		}
		else
		{
			// In flWait seconds, DoorClose will fire, unless wait is -1, then door stays open
			SetMoveDoneTime(m_flAutoReturnDelay + 0.1);
			SetMoveDone(&CBaseDoor<ENTITY_CLASS>::DoorAutoCloseThink);
		}

		return;
	}

	DoorClose();
}

//-----------------------------------------------------------------------------
// Purpose: Starts the door closing.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::DoorClose(void)
{
	// Don't bother if we're already doing this
	if ( IsDoorClosed() || IsDoorClosing() )
		return;

	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		EmitSound( STRING( m_SoundMoving ) );

		if ( m_hActivator && m_hActivator->IsPlayer() )
		{
			CSoundEnt::InsertSound( SOUND_PLAYER, GetAbsOrigin(), 512, 0.5, this );//<<TODO>>//magic number
		}
	}
	
	ASSERT(IsDoorOpen() || IsDoorOpening());
	SetDoorState( DOOR_STATE_CLOSING );

	// disable touch until movement is complete
	SetTouch( NULL );

	SetMoveDone(&CBaseDoor<ENTITY_CLASS>::DoorCloseMoveDone);

	// This will set the movedone time.
	BeginClosing();

	m_OnClose.FireOutput(this, this);

	// Tell all the slaves
	if ( HasSlaves() )
	{
		int	numDoors = m_hDoorList.Count();

		CBaseDoor *pLinkedDoor = NULL;

		// Open all linked doors
		for ( int i = 0; i < numDoors; i++ )
		{
			pLinkedDoor = m_hDoorList[i];

			if ( pLinkedDoor != NULL )
			{
				// If the door isn't already moving, get it moving
				pLinkedDoor->DoorClose();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: The door has reached the closed position. Return to quiescence.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::DoorCloseMoveDone(void)
{
	SetDoorBlocker( NULL );

	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		StopSound( STRING( m_SoundMoving ) );
		EmitSound( STRING( m_SoundClose ) );
	}

	ASSERT(IsDoorClosing());
	SetDoorState( DOOR_STATE_CLOSED );

	// Re-instate touch method, movement is complete
	SetTouch( &CBaseDoor::DoorTouch );

	m_OnFullyClosed.FireOutput(m_hActivator, this);
	UpdateAreaPortals(false);

	// Let the leaf class do its thing.
	OnDoorClosed();

	m_hActivator = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::MasterStartBlocked( CBaseEntity *pOther )
{
	if ( HasSlaves() )
	{
		int	numDoors = m_hDoorList.Count();

		CBaseDoor *pLinkedDoor = NULL;

		// Open all linked doors
		for ( int i = 0; i < numDoors; i++ )
		{
			pLinkedDoor = m_hDoorList[i];

			if ( pLinkedDoor != NULL )
			{
				// If the door isn't already moving, get it moving
				pLinkedDoor->OnStartBlocked( pOther );
			}
		}
	}

	// Start ourselves blocked
	OnStartBlocked( pOther );
}

//-----------------------------------------------------------------------------
// Purpose: Called the first frame that the door is blocked while opening or closing.
// Input  : pOther - The blocking entity.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::StartBlocked( CBaseEntity *pOther )
{
	m_bFirstBlocked = true;

	if ( GetMaster() != NULL )
	{
		GetMaster()->MasterStartBlocked( pOther );
		return;
	}

	// Start ourselves blocked
	OnStartBlocked( pOther );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::OnStartBlocked( CBaseEntity *pOther )
{
	if ( m_bFirstBlocked == false )
	{
		DoorStop();
	}

	SetDoorBlocker( pOther );

	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		StopSound( STRING( m_SoundMoving ) );
	}

	//
	// Fire whatever events we need to due to our blocked state.
	//
	if (IsDoorClosing())
	{
		// Closed into an NPC, open.
		if ( pOther->MyNPCPointer() )
		{
			DoorOpen( pOther );
		}
		m_OnBlockedClosing.FireOutput(pOther, this);
	}
	else
	{
		// Opened into an NPC, close.
		if ( pOther->MyNPCPointer() )
		{
			DoorClose();
		}

		CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC *>(m_hActivator.Get());
		
		if ( pNPC != NULL )
		{
			// Notify the NPC that tried to open us.
			pNPC->OnDoorBlocked( this );
		}

		m_OnBlockedOpening.FireOutput( pOther, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame when the door is blocked while opening or closing.
// Input  : pOther - The blocking entity.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::Blocked(CBaseEntity *pOther)
{
	// dvs: TODO: will prop_door apply any blocking damage?
	// Hurt the blocker a little.
	//if (m_flBlockDamage)
	//{
	//	pOther->TakeDamage(CTakeDamageInfo(this, this, m_flBlockDamage, DMG_CRUSH));
	//}

	if ( m_bForceClosed && ( pOther->GetMoveType() == MOVETYPE_VPHYSICS ) &&
		 ( pOther->m_takedamage == DAMAGE_NO || pOther->m_takedamage == DAMAGE_EVENTS_ONLY ) )
	{
		EntityPhysics_CreateSolver( this, pOther, true, 4.0f );
	}
	else if ( m_bForceClosed && ( pOther->GetMoveType() == MOVETYPE_VPHYSICS ) && ( pOther->m_takedamage == DAMAGE_YES ) )
	{
		pOther->TakeDamage( CTakeDamageInfo( this, this, pOther->GetHealth(), DMG_CRUSH ) );
	}

	// If we're set to force ourselves closed, keep going
	if ( m_bForceClosed )
		return;

	// If a door has a negative wait, it would never come back if blocked,
	// so let it just squash the object to death real fast.
//	if (m_flAutoReturnDelay >= 0)
//	{
//		if (IsDoorClosing())
//		{
//			DoorOpen();
//		}
//		else
//		{
//			DoorClose();
//		}
//	}

	// Block all door pieces with the same targetname here.
//	if (GetEntityName() != NULL_STRING)
//	{
//		CBaseEntity pTarget = NULL;
//		for (;;)
//		{
//			pTarget = gEntList.FindEntityByName(pTarget, GetEntityName() );
//
//			if (pTarget != this)
//			{
//				if (!pTarget)
//					break;
//
//				if (FClassnameIs(pTarget, "prop_door_rotating"))
//				{
//					CBaseDoorRotating *pDoor = (CBaseDoorRotating *)pTarget;
//
//					if (pDoor->m_fAutoReturnDelay >= 0)
//					{
//						if (pDoor->GetAbsVelocity() == GetAbsVelocity() && pDoor->GetLocalAngularVelocity() == GetLocalAngularVelocity())
//						{
//							// this is the most hacked, evil, bastardized thing I've ever seen. kjb
//							if (FClassnameIs(pTarget, "prop_door_rotating"))
//							{
//								// set angles to realign rotating doors
//								pDoor->SetLocalAngles(GetLocalAngles());
//								pDoor->SetLocalAngularVelocity(vec3_angle);
//							}
//							else
//							//{
//							//	// set origin to realign normal doors
//							//	pDoor->SetLocalOrigin(GetLocalOrigin());
//							//	pDoor->SetAbsVelocity(vec3_origin);// stop!
//							//}
//						}
//
//						if (IsDoorClosing())
//						{
//							pDoor->DoorOpen();
//						}
//						else
//						{
//							pDoor->DoorClose();
//						}
//					}
//				}
//			}
//		}
//	}
}

//-----------------------------------------------------------------------------
// Purpose: Called the first frame that the door is unblocked while opening or closing.
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::EndBlocked( void )
{
	if ( GetMaster() != NULL )
	{
		GetMaster()->EndBlocked();
		return;
	}

	if ( HasSlaves() )
	{
		int	numDoors = m_hDoorList.Count();

		CBaseDoor *pLinkedDoor = NULL;

		// Check all links as well
		for ( int i = 0; i < numDoors; i++ )
		{
			pLinkedDoor = m_hDoorList[i];

			if ( pLinkedDoor != NULL )
			{
				// Make sure they can close as well
				pLinkedDoor->OnEndBlocked();
			}
		}
	}

	// Emit door moving and stop sounds on CHAN_STATIC so that the multicast doesn't
	// filter them out and leave a client stuck with looping door sounds!
	if (!HasSpawnFlags(SF_DOOR_SILENT))
	{
		EmitSound( STRING( m_SoundMoving ) );
	}

	//
	// Fire whatever events we need to due to our unblocked state.
	//
	if (IsDoorClosing())
	{
		m_OnUnblockedClosing.FireOutput(this, this);
	}
	else
	{
		m_OnUnblockedOpening.FireOutput(this, this);
	}

	OnEndBlocked();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
void CBaseDoor<ENTITY_CLASS>::OnEndBlocked( void )
{
	if ( m_bFirstBlocked )
		return;

	// Restart us going
	DoorResume();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pNPC - 
//-----------------------------------------------------------------------------
template < class ENTITY_CLASS >
bool CBaseDoor<ENTITY_CLASS>::NPCOpenDoor( CAI_BaseNPC *pNPC )
{
	// dvs: TODO: use activator filter here
	// dvs: TODO: outboard entity containing rules for whether door is operable?
	
	if ( IsDoorClosed() )
	{
		// Use the door
		Use( pNPC, pNPC, USE_ON, 0 );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Custom trace filter for doors
// Will only test against entities and rejects physics objects below a mass threshold
//-----------------------------------------------------------------------------

class CTraceFilterDoor : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterDoor );
	
	CTraceFilterDoor( const IHandleEntity *pDoor, const IHandleEntity *passentity, int collisionGroup )
		: m_pDoor(pDoor), m_pPassEnt(passentity), m_collisionGroup(collisionGroup)
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !StandardFilterRules( pHandleEntity, contentsMask ) )
			return false;

		if ( !PassServerEntityFilter( pHandleEntity, m_pDoor ) )
			return false;

		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;

		// Don't test if the game code tells us we should ignore this collision...
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );
		
		if ( pEntity )
		{
			if ( !pEntity->ShouldCollide( m_collisionGroup, contentsMask ) )
				return false;
			
			if ( !g_pGameRules->ShouldCollide( m_collisionGroup, pEntity->GetCollisionGroup() ) )
				return false;

			// If objects are small enough and can move, close on them
			if ( pEntity->GetMoveType() == MOVETYPE_VPHYSICS )
			{
				IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
				Assert(pPhysics);
				
				// Must either be squashable or very light
				if ( pPhysics->IsMoveable() && pPhysics->GetMass() < 32 )
					return false;
			}
#if 1
			// Brush model bboxes are expanded by 1 in all directions reporting collisions when there are none,
			// blocking doors.
			if ( ( pEntity->GetModel() != NULL ) && ( modelinfo->GetModelType( pEntity->GetModel() ) == mod_brush ) )
			{
				Vector mins, maxs;
				pEntity->CollisionProp()->WorldSpaceAABB( &mins, &maxs );

				// Subtract 1 from each dimension because the engine expands bboxes by 1 in all directions making the size too big
				mins += Vector(1,1,1);
				maxs -= Vector(1,1,1);

				if ( !IsBoxIntersectingBox( mins, maxs, m_mins, m_maxs ) )
					return false;
			}
#endif
		}

		return true;
	}

#if 1
	Vector m_mins, m_maxs;
#endif
private:

	const IHandleEntity *m_pDoor;
	const IHandleEntity *m_pPassEnt;
	int m_collisionGroup;
};

template< class ENTITY_CLASS>
inline void TraceHull_Door( const CBaseDoor<ENTITY_CLASS> *pDoor, const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &hullMin, 
					 const Vector &hullMax,	unsigned int mask, const CBaseEntity *ignore, 
					 int collisionGroup, trace_t *ptr )
{
	Ray_t ray;
	ray.Init( vecAbsStart, vecAbsEnd, hullMin, hullMax );
	CTraceFilterDoor traceFilter( pDoor, ignore, collisionGroup );
#if 1
	traceFilter.m_mins = hullMin; // this only works when vecAbsStart==vecAbsEnd
	traceFilter.m_maxs = hullMax;
#endif
	enginetrace->TraceRay( ray, mask, &traceFilter, ptr );
}

// Check directions for door movement
enum doorCheck_e
{
	DOOR_CHECK_FORWARD,		// Door's forward opening direction
	DOOR_CHECK_BACKWARD,	// Door's backward opening direction
	DOOR_CHECK_FULL,		// Door's complete movement volume
};


enum BaseDoorRotatingSpawnPos_t
{
	DOOR_SPAWN_CLOSED = 0,
	DOOR_SPAWN_OPEN_FORWARD,
	DOOR_SPAWN_OPEN_BACK,
	DOOR_SPAWN_AJAR,
};

enum BaseDoorRotatingOpenDirection_e
{
	DOOR_ROTATING_OPEN_BOTH_WAYS = 0,
	DOOR_ROTATING_OPEN_FORWARD,
	DOOR_ROTATING_OPEN_BACKWARD,
};

//===============================================
// Base rotating door
//===============================================

template< class ENTITY_CLASS >
class CBaseDoorRotating : public CBaseDoor< ENTITY_CLASS >
{
	DECLARE_CLASS( CBaseDoorRotating, CBaseDoor );

public:

	~CBaseDoorRotating();

	int		DrawDebugTextOverlays(void);

	void	Spawn( void );
	void	MoveDone( void );
	void	BeginOpening(CBaseEntity *pOpenAwayFrom);
	void	BeginClosing( void );
	void	OnRestore( void );

	void	DoorTeleportToSpawnPosition();

	void	GetNPCOpenData(CAI_BaseNPC *pNPC, opendata_t &opendata);

	void	DoorClose( void );
	bool	DoorCanClose( bool bAutoClose );
	void	DoorOpen( CBaseEntity *pOpenAwayFrom );

	void	OnDoorOpened();
	void	OnDoorClosed();

	void	DoorResume( void );
	void	DoorStop( void );

	float	GetOpenInterval();

	bool	OverridePropdata() { return true; }

	DECLARE_DATADESC();

private:

	virtual bool IsMovementReversed( void ) { return false; }

	void	AngularMove(const QAngle &vecDestAngle, float flSpeed);
	void	CalculateDoorVolume( QAngle closedAngles, QAngle openAngles, Vector *destMins, Vector *destMaxs );

	bool	CheckDoorClear( doorCheck_e state );
	
	doorCheck_e	GetOpenState( void );

	void	InputSetRotationDistance ( inputdata_t &inputdata );			// Set the degree difference between open and closed

	void	CalcOpenAngles ( void );		// Subroutine to setup the m_angRotation QAngles based on the m_flDistance variable

	Vector	m_vecAxis;					// The axis of rotation.
	float	m_flDistance;				// How many degrees we rotate between open and closed.

	BaseDoorRotatingSpawnPos_t m_eSpawnPosition;
	BaseDoorRotatingOpenDirection_e m_eOpenDirection;

	QAngle	m_angRotationAjar;			// Angles to spawn at if we are set to spawn ajar.
	QAngle	m_angRotationClosed;		// Our angles when we are fully closed.
	QAngle	m_angRotationOpenForward;	// Our angles when we are fully open towards our forward vector.
	QAngle	m_angRotationOpenBack;		// Our angles when we are fully open away from our forward vector.

	QAngle	m_angGoal;

	Vector	m_vecForwardBoundsMin;
	Vector	m_vecForwardBoundsMax;
	Vector	m_vecBackBoundsMin;
	Vector	m_vecBackBoundsMax;

	CHandle<CEntityBlocker>	m_hDoorBlocker;
};

#define BASE_DOOR_ROTATING_DATADESC \
	DEFINE_KEYFIELD(m_eSpawnPosition, FIELD_INTEGER, "spawnpos"), \
	DEFINE_KEYFIELD(m_eOpenDirection, FIELD_INTEGER, "opendir" ), \
	DEFINE_KEYFIELD(m_vecAxis, FIELD_VECTOR, "axis"), \
	DEFINE_KEYFIELD(m_flDistance, FIELD_FLOAT, "distance"), \
	DEFINE_KEYFIELD( m_angRotationAjar, FIELD_VECTOR, "ajarangles" ), \
	DEFINE_FIELD( m_angRotationClosed, FIELD_VECTOR ), \
	DEFINE_FIELD( m_angRotationOpenForward, FIELD_VECTOR ), \
	DEFINE_FIELD( m_angRotationOpenBack, FIELD_VECTOR ), \
	DEFINE_FIELD( m_angGoal, FIELD_VECTOR ), \
	DEFINE_FIELD( m_hDoorBlocker, FIELD_EHANDLE ), \
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetRotationDistance", InputSetRotationDistance ),

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
CBaseDoorRotating<ENTITY_CLASS>::~CBaseDoorRotating( void )
{
	// Remove our door blocker entity
	if ( m_hDoorBlocker != NULL )
	{
		UTIL_Remove( m_hDoorBlocker );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &mins1 - 
//			&maxs1 - 
//			&mins2 - 
//			&maxs2 - 
//			*destMins - 
//			*destMaxs - 
//-----------------------------------------------------------------------------
void UTIL_ComputeAABBForBounds( const Vector &mins1, const Vector &maxs1, const Vector &mins2, const Vector &maxs2, Vector *destMins, Vector *destMaxs )
{
	// Find the minimum extents
	(*destMins)[0] = min( mins1[0], mins2[0] );
	(*destMins)[1] = min( mins1[1], mins2[1] );
	(*destMins)[2] = min( mins1[2], mins2[2] );

	// Find the maximum extents
	(*destMaxs)[0] = max( maxs1[0], maxs2[0] );
	(*destMaxs)[1] = max( maxs1[1], maxs2[1] );
	(*destMaxs)[2] = max( maxs1[2], maxs2[2] );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::Spawn()
{
	// Doors are built closed, so save the current angles as the closed angles.
	m_angRotationClosed = GetLocalAngles();

	// The axis of rotation must be along the z axis for now.
	// NOTE: If you change this, be sure to change IsMovementReversed to account for it!
	m_vecAxis = Vector(0, 0, 1);

#if 1
	if ( HasSpawnFlags( SF_DOOR_ROTATE_ROLL ) )
		m_vecAxis = Vector(1, 0, 0);
	else if ( HasSpawnFlags( SF_DOOR_ROTATE_PITCH ) )
		m_vecAxis = Vector(0, 1, 0);
#endif

	CalcOpenAngles();

	// Call this last! It relies on stuff we calculated above.
	BaseClass::Spawn();

	// Figure out our volumes of movement as this door opens
#if 1
	// BUG! GetLocalAngles() is not "closed" angles if the door spawns open!
	// You can see this by setting g_debug_doors=1
	CalculateDoorVolume( m_angRotationClosed, m_angRotationOpenForward, &m_vecForwardBoundsMin, &m_vecForwardBoundsMax );
	CalculateDoorVolume( m_angRotationClosed, m_angRotationOpenBack, &m_vecBackBoundsMin, &m_vecBackBoundsMax );
#else
	CalculateDoorVolume( GetLocalAngles(), m_angRotationOpenForward, &m_vecForwardBoundsMin, &m_vecForwardBoundsMax );
	CalculateDoorVolume( GetLocalAngles(), m_angRotationOpenBack, &m_vecBackBoundsMin, &m_vecBackBoundsMax );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Setup the m_angRotationOpenForward and m_angRotationOpenBack variables based on
//			the m_flDistance variable. Also restricts m_flDistance > 0.
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::CalcOpenAngles()
{
	// HACK: convert the axis of rotation to dPitch dYaw dRoll
	Vector vecMoveDir(m_vecAxis.y, m_vecAxis.z, m_vecAxis.x); 

	if (m_flDistance == 0)
	{
		m_flDistance = 90;
	}
	m_flDistance = fabs(m_flDistance);

	// Calculate our orientation when we are fully open.
	m_angRotationOpenForward.x = m_angRotationClosed.x - (vecMoveDir.x * m_flDistance);
	m_angRotationOpenForward.y = m_angRotationClosed.y - (vecMoveDir.y * m_flDistance);
	m_angRotationOpenForward.z = m_angRotationClosed.z - (vecMoveDir.z * m_flDistance);

	m_angRotationOpenBack.x = m_angRotationClosed.x + (vecMoveDir.x * m_flDistance);
	m_angRotationOpenBack.y = m_angRotationClosed.y + (vecMoveDir.y * m_flDistance);
	m_angRotationOpenBack.z = m_angRotationClosed.z + (vecMoveDir.z * m_flDistance);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
doorCheck_e CBaseDoorRotating<ENTITY_CLASS>::GetOpenState( void )
{
	return ( m_angGoal == m_angRotationOpenForward ) ? DOOR_CHECK_FORWARD : DOOR_CHECK_BACKWARD;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::OnDoorOpened( void )
{
	if ( m_hDoorBlocker != NULL )
	{
		// Allow passage through this blocker while open
		m_hDoorBlocker->AddSolidFlags( FSOLID_NOT_SOLID );

		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( GetAbsOrigin(), m_hDoorBlocker->CollisionProp()->OBBMins(), m_hDoorBlocker->CollisionProp()->OBBMaxs(), 0, 255, 0, true, 1.0f );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::OnDoorClosed( void )
{
	if ( m_hDoorBlocker != NULL )
	{
		// Destroy the blocker that was preventing NPCs from getting in our way.
		UTIL_Remove( m_hDoorBlocker );
		
		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( GetAbsOrigin(), m_hDoorBlocker->CollisionProp()->OBBMins(), m_hDoorBlocker->CollisionProp()->OBBMaxs(), 0, 255, 0, true, 1.0f );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether the way is clear for the door to close.
// Input  : state - Which sides to check, forward, backward, or both.
// Output : Returns true if the door can close, false if the way is blocked.
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
bool CBaseDoorRotating<ENTITY_CLASS>::DoorCanClose( bool bAutoClose )
{
	if ( GetMaster() != NULL )
		return GetMaster()->DoorCanClose( bAutoClose );
	
	// Check all slaves
	if ( HasSlaves() )
	{
		int	numDoors = m_hDoorList.Count();

		CBaseDoorRotating *pLinkedDoor = NULL;

		// Check all links as well
		for ( int i = 0; i < numDoors; i++ )
		{
			pLinkedDoor = dynamic_cast<CBaseDoorRotating *>((CBaseDoor *)m_hDoorList[i]);

			if ( pLinkedDoor != NULL )
			{
				if ( !pLinkedDoor->CheckDoorClear( bAutoClose ? DOOR_CHECK_FULL : pLinkedDoor->GetOpenState() ) )
					return false;
			}
		}
	}
	
	// See if our path of movement is clear to allow us to shut
	return CheckDoorClear( bAutoClose ? DOOR_CHECK_FULL : GetOpenState() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : closedAngles - 
//			openAngles - 
//			*destMins - 
//			*destMaxs - 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::CalculateDoorVolume( QAngle closedAngles, QAngle openAngles, Vector *destMins, Vector *destMaxs )
{
	// Save our current angles and move to our start angles
	QAngle	saveAngles = GetLocalAngles();
	SetLocalAngles( closedAngles );

	// Find our AABB at the closed state
	Vector	closedMins, closedMaxs;
	GetWorldSpaceAABB( &closedMins, &closedMaxs );
	
	SetLocalAngles( openAngles );

	// Find our AABB at the open state
	Vector	openMins, openMaxs;
	GetWorldSpaceAABB( &openMins, &openMaxs );

	// Reset our angles to our starting angles
	SetLocalAngles( saveAngles );

	// Find the minimum extents
	UTIL_ComputeAABBForBounds( closedMins, closedMaxs, openMins, openMaxs, destMins, destMaxs );
	
	// Move this back into local space
	*destMins -= GetAbsOrigin();
	*destMaxs -= GetAbsOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::OnRestore( void )
{
	BaseClass::OnRestore();

	// Figure out our volumes of movement as this door opens
#if 1
	// BUG! GetLocalAngles() is not "closed" angles if the door is open!
	// You can see this by setting g_debug_doors=1
	CalculateDoorVolume( m_angRotationClosed, m_angRotationOpenForward, &m_vecForwardBoundsMin, &m_vecForwardBoundsMax );
	CalculateDoorVolume( m_angRotationClosed, m_angRotationOpenBack, &m_vecBackBoundsMin, &m_vecBackBoundsMax );
#else
	CalculateDoorVolume( GetLocalAngles(), m_angRotationOpenForward, &m_vecForwardBoundsMin, &m_vecForwardBoundsMax );
	CalculateDoorVolume( GetLocalAngles(), m_angRotationOpenBack, &m_vecBackBoundsMin, &m_vecBackBoundsMax );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : forward - 
//			mask - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
bool CBaseDoorRotating<ENTITY_CLASS>::CheckDoorClear( doorCheck_e state )
{
	Vector moveMins;
	Vector moveMaxs;

	switch ( state )
	{
	case DOOR_CHECK_FORWARD:
		moveMins = m_vecForwardBoundsMin;
		moveMaxs = m_vecForwardBoundsMax;
		break;

	case DOOR_CHECK_BACKWARD:
		moveMins = m_vecBackBoundsMin;
		moveMaxs = m_vecBackBoundsMax;
		break;

	default:
	case DOOR_CHECK_FULL:
		UTIL_ComputeAABBForBounds( m_vecForwardBoundsMin, m_vecForwardBoundsMax, m_vecBackBoundsMin, m_vecBackBoundsMax, &moveMins, &moveMaxs );
		break;
	}

	// Look for blocking entities, ignoring ourselves and the entity that opened us.
	trace_t	tr;
	TraceHull_Door( this, GetAbsOrigin(), GetAbsOrigin(), moveMins, moveMaxs, MASK_SOLID, GetActivator(), COLLISION_GROUP_NONE, &tr );
	if ( tr.allsolid || tr.startsolid )
	{
		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( GetAbsOrigin(), moveMins, moveMaxs, 255, 0, 0, true, 10.0f );

			if ( tr.m_pEnt )
			{
				NDebugOverlay::Box( tr.m_pEnt->GetAbsOrigin(), tr.m_pEnt->CollisionProp()->OBBMins(), tr.m_pEnt->CollisionProp()->OBBMaxs(), 220, 220, 0, true, 10.0f );
			}
		}

		return false;
	}

	if ( g_debug_doors.GetBool() )
	{
		NDebugOverlay::Box( GetAbsOrigin(), moveMins, moveMaxs, 0, 255, 0, true, 10.0f );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Puts the door in its appropriate position for spawning.
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::DoorTeleportToSpawnPosition()
{
	// We have to call this after we call the base Spawn because it requires
	// that the model already be set.
	// BUG! Moved this from Spawn() othewise a door that spawns open is in the wrong position.
	if ( IsMovementReversed() )
	{
		::V_swap( m_angRotationOpenForward, m_angRotationOpenBack );
	}

	QAngle angSpawn;

	// The Start Open spawnflag trumps the choices field
	// BUG! Made sure door opens in the proper direction (used to always be DOOR_SPAWN_OPEN_FORWARD.
	if ( HasSpawnFlags( SF_DOOR_START_OPEN_OBSOLETE ) )
	{
		switch ( m_eOpenDirection )
		{
		case DOOR_ROTATING_OPEN_BOTH_WAYS:
		case DOOR_ROTATING_OPEN_FORWARD:
			m_eSpawnPosition = DOOR_SPAWN_OPEN_FORWARD;
			break;
		case DOOR_ROTATING_OPEN_BACKWARD:
			m_eSpawnPosition = DOOR_SPAWN_OPEN_BACK;
			break;
		}
	}

	if ( ( m_eSpawnPosition == DOOR_SPAWN_OPEN_FORWARD ) )
	{
		angSpawn = m_angRotationOpenForward;
		SetDoorState( DOOR_STATE_OPEN );
	}
	else if ( m_eSpawnPosition == DOOR_SPAWN_OPEN_BACK )
	{
		angSpawn = m_angRotationOpenBack;
		SetDoorState( DOOR_STATE_OPEN );
	}
	else if ( m_eSpawnPosition == DOOR_SPAWN_CLOSED )
	{
		angSpawn = m_angRotationClosed;
		SetDoorState( DOOR_STATE_CLOSED );
	}
	else if ( m_eSpawnPosition == DOOR_SPAWN_AJAR )
	{
		angSpawn = m_angRotationAjar;
		SetDoorState( DOOR_STATE_AJAR );
	}
	else
	{
		// Bogus spawn position setting!
		Assert( false );
		angSpawn = m_angRotationClosed;
		SetDoorState( DOOR_STATE_CLOSED );
	}

	SetLocalAngles( angSpawn );

	// Doesn't relink; that's done in Spawn.
}


//-----------------------------------------------------------------------------
// Purpose: After rotating, set angle to exact final angle, call "move done" function.
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::MoveDone()
{
	SetLocalAngles(m_angGoal);
	SetLocalAngularVelocity(vec3_angle);
	SetMoveDoneTime(-1);
	BaseClass::MoveDone();
}


//-----------------------------------------------------------------------------
// Purpose: Calculate m_vecVelocity and m_flNextThink to reach vecDest from
//			GetLocalOrigin() traveling at flSpeed. Just like LinearMove, but rotational.
// Input  : vecDestAngle - 
//			flSpeed - 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::AngularMove(const QAngle &vecDestAngle, float flSpeed)
{
	ASSERTSZ(flSpeed != 0, "AngularMove:  no speed is defined!");
	
	m_angGoal = vecDestAngle;

	// Already there?
	if (vecDestAngle == GetLocalAngles())
	{
		MoveDone();
		return;
	}
	
	// Set destdelta to the vector needed to move.
	QAngle vecDestDelta = vecDestAngle - GetLocalAngles();
	
	// Divide by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// Call MoveDone when destination angles are reached.
	SetMoveDoneTime(flTravelTime);

	// Scale the destdelta vector by the time spent traveling to get velocity.
	SetLocalAngularVelocity(vecDestDelta * (1.0 / flTravelTime));
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::BeginOpening(CBaseEntity *pOpenAwayFrom)
{
	// Determine the direction to open.
	QAngle angOpen = m_angRotationOpenForward;
	doorCheck_e eDirCheck = DOOR_CHECK_FORWARD;

	if ( m_eOpenDirection == DOOR_ROTATING_OPEN_FORWARD )
	{
		eDirCheck	= DOOR_CHECK_FORWARD;
		angOpen		= m_angRotationOpenForward;
	}
	else if ( m_eOpenDirection == DOOR_ROTATING_OPEN_BACKWARD )
	{
		eDirCheck	= DOOR_CHECK_BACKWARD;
		angOpen		= m_angRotationOpenBack;
	}
	else // Can open either direction, test to see which is appropriate
	{
		if (pOpenAwayFrom != NULL)
		{
#if 1
			// support brush-based doors which always face east
			Vector vecForwardDoor;
			Vector centerF = GetAbsOrigin() + (m_vecForwardBoundsMax + m_vecForwardBoundsMin) * 0.5f;
			Vector centerB = GetAbsOrigin() + (m_vecBackBoundsMax + m_vecBackBoundsMin) * 0.5f;
			vecForwardDoor = centerF - centerB;
			vecForwardDoor.NormalizeInPlace();

			if (vecForwardDoor.Dot(pOpenAwayFrom->GetAbsOrigin()) > vecForwardDoor.Dot(GetAbsOrigin()))
			{
				angOpen = m_angRotationOpenBack;
				eDirCheck = DOOR_CHECK_BACKWARD;
			}
#else
			Vector vecForwardDoor;
			GetVectors(&vecForwardDoor, NULL, NULL);

			if (vecForwardDoor.Dot(pOpenAwayFrom->GetAbsOrigin()) > vecForwardDoor.Dot(GetAbsOrigin()))
			{
				angOpen = m_angRotationOpenBack;
				eDirCheck = DOOR_CHECK_BACKWARD;
			}
#endif
		}

		// If player is opening us and we're opening away from them, and we'll be
		// blocked if we open away from them, open toward them.
		if (IsPlayerOpening() && (pOpenAwayFrom && pOpenAwayFrom->IsPlayer()) && !CheckDoorClear(eDirCheck))
		{
			if (eDirCheck == DOOR_CHECK_FORWARD)
			{
				angOpen = m_angRotationOpenBack;
				eDirCheck = DOOR_CHECK_BACKWARD;
			}
			else
			{
				angOpen = m_angRotationOpenForward;
				eDirCheck = DOOR_CHECK_FORWARD;
			}
		}
	}

	// Create the door blocker
	Vector mins, maxs;
	if ( eDirCheck == DOOR_CHECK_FORWARD )
	{
		mins = m_vecForwardBoundsMin;
		maxs = m_vecForwardBoundsMax;
	}
	else
	{
		mins = m_vecBackBoundsMin;
		maxs = m_vecBackBoundsMax;		
	}

	if ( m_hDoorBlocker != NULL )
	{
		UTIL_Remove( m_hDoorBlocker );
	}

	// Create a blocking entity to keep random entities out of our movement path
	m_hDoorBlocker = CEntityBlocker::Create( GetAbsOrigin(), mins, maxs, pOpenAwayFrom, false );
	
	Vector	volumeCenter = ((mins+maxs) * 0.5f) + GetAbsOrigin();

	// Ignoring the Z
	float volumeRadius = max( fabs(mins.x), maxs.x );
	volumeRadius = max( volumeRadius, max( fabs(mins.y), maxs.y ) );

	// Debug
	if ( g_debug_doors.GetBool() )
	{
		NDebugOverlay::Cross3D( volumeCenter, -Vector(volumeRadius,volumeRadius,volumeRadius), Vector(volumeRadius,volumeRadius,volumeRadius), 255, 0, 0, true, 1.0f );
	}

	// Make respectful entities move away from our path
	if( !HasSpawnFlags(SF_DOOR_SILENT_TO_NPCS) )
	{
		CSoundEnt::InsertSound( SOUND_MOVE_AWAY, volumeCenter, volumeRadius, 0.5f, pOpenAwayFrom );
	}

	// Do final setup
	if ( m_hDoorBlocker != NULL )
	{
		// Only block NPCs
		m_hDoorBlocker->SetCollisionGroup( COLLISION_GROUP_DOOR_BLOCKER );

		// If we hit something while opening, just stay unsolid until we try again
		if ( CheckDoorClear( eDirCheck ) == false )
		{
			m_hDoorBlocker->AddSolidFlags( FSOLID_NOT_SOLID );
		}

		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( GetAbsOrigin(), m_hDoorBlocker->CollisionProp()->OBBMins(), m_hDoorBlocker->CollisionProp()->OBBMaxs(), 255, 0, 0, true, 1.0f );
		}
	}

	AngularMove(angOpen, m_flSpeed);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::BeginClosing( void )
{
	if ( m_hDoorBlocker != NULL )
	{
		// Become solid again unless we're already being blocked
		if ( CheckDoorClear( GetOpenState() )  )
		{
			m_hDoorBlocker->RemoveSolidFlags( FSOLID_NOT_SOLID );
		}
		
		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( GetAbsOrigin(), m_hDoorBlocker->CollisionProp()->OBBMins(), m_hDoorBlocker->CollisionProp()->OBBMaxs(), 255, 0, 0, true, 1.0f );
		}
	}

	AngularMove(m_angRotationClosed, m_flSpeed);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::DoorStop( void )
{
	SetLocalAngularVelocity( vec3_angle );
	SetMoveDoneTime( -1 );
}

//-----------------------------------------------------------------------------
// Purpose: Restart a door moving that was temporarily paused
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::DoorResume( void )
{
	// Restart our angular movement
	AngularMove( m_angGoal, m_flSpeed );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecMoveDir - 
//			opendata - 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::GetNPCOpenData(CAI_BaseNPC *pNPC, opendata_t &opendata)
{
#if 1
	// support brush-based doors which always face east
	// FIXME: assumes door opens 90 degrees in both directions
	Vector vecForward;
	Vector centerF = GetAbsOrigin() + (m_vecForwardBoundsMax + m_vecForwardBoundsMin) * 0.5f;
	Vector centerB = GetAbsOrigin() + (m_vecBackBoundsMax + m_vecBackBoundsMin) * 0.5f;
	vecForward = centerF - centerB;
	vecForward.NormalizeInPlace();

	//
	// Figure out where the NPC should stand to open this door,
	// and what direction they should face.
	//
	Vector vecNPCOrigin = pNPC->GetAbsOrigin();
	float flNPCSize = 30;

	if (pNPC->GetAbsOrigin().Dot(vecForward) > GetAbsOrigin().Dot(vecForward))
	{
		// In front of the door relative to the door's forward vector.
		opendata.vecStandPos = centerF;
		if ( m_eOpenDirection == DOOR_ROTATING_OPEN_FORWARD )
			opendata.vecStandPos += vecForward * ((m_vecForwardBoundsMax.x -  m_vecForwardBoundsMin.x) * 0.5f + flNPCSize / 2);
		opendata.vecFaceDir = -vecForward;
	}
	else
	{
		// Behind the door relative to the door's forward vector.
		opendata.vecStandPos = centerB;
		if ( m_eOpenDirection == DOOR_ROTATING_OPEN_BACKWARD )
			opendata.vecStandPos -= vecForward * ((m_vecForwardBoundsMax.x -  m_vecForwardBoundsMin.x) * 0.5f + flNPCSize / 2);
		opendata.vecFaceDir = vecForward;
	}
	opendata.vecStandPos.z = GetAbsOrigin().z - m_vecForwardBoundsMin.z;

	opendata.eActivity = ACT_OPEN_DOOR;
#else
	// dvs: TODO: finalize open position, direction, activity
	Vector vecForward;
	Vector vecRight;
	AngleVectors(GetAbsAngles(), &vecForward, &vecRight, NULL);

	//
	// Figure out where the NPC should stand to open this door,
	// and what direction they should face.
	//
	opendata.vecStandPos = GetAbsOrigin() - (vecRight * 24);
	opendata.vecStandPos.z -= 54;
	Vector vecNPCOrigin = pNPC->GetAbsOrigin();

	if (pNPC->GetAbsOrigin().Dot(vecForward) > GetAbsOrigin().Dot(vecForward))
	{
		// In front of the door relative to the door's forward vector.
		opendata.vecStandPos += vecForward * 64;
		opendata.vecFaceDir = -vecForward;
	}
	else
	{
		// Behind the door relative to the door's forward vector.
		opendata.vecStandPos -= vecForward * 64;
		opendata.vecFaceDir = vecForward;
	}

	opendata.eActivity = ACT_OPEN_DOOR;
#endif
}


//-----------------------------------------------------------------------------
// Purpose: Returns how long it will take this door to open.
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
float CBaseDoorRotating<ENTITY_CLASS>::GetOpenInterval()
{
	// set destdelta to the vector needed to move
	QAngle vecDestDelta = m_angRotationOpenForward - GetLocalAngles();
	
	// divide by speed to get time to reach dest
	return vecDestDelta.Length() / m_flSpeed;
}


//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
int CBaseDoorRotating<ENTITY_CLASS>::DrawDebugTextOverlays(void) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		char tempstr[512];
		Q_snprintf(tempstr, sizeof(tempstr),"Avelocity: %.2f %.2f %.2f", GetLocalAngularVelocity().x,  GetLocalAngularVelocity().y,  GetLocalAngularVelocity().z);
		EntityText( text_offset, tempstr, 0);
		text_offset++;

		if ( IsDoorOpen() )
		{
			Q_strncpy(tempstr, "DOOR STATE: OPEN", sizeof(tempstr));
		}
		else if ( IsDoorClosed() )
		{
			Q_strncpy(tempstr, "DOOR STATE: CLOSED", sizeof(tempstr));
		}
		else if ( IsDoorOpening() )
		{
			Q_strncpy(tempstr, "DOOR STATE: OPENING", sizeof(tempstr));
		}
		else if ( IsDoorClosing() )
		{
			Q_strncpy(tempstr, "DOOR STATE: CLOSING", sizeof(tempstr));
		}
		else if ( IsDoorAjar() )
		{
			Q_strncpy(tempstr, "DOOR STATE: AJAR", sizeof(tempstr));
		}
		EntityText( text_offset, tempstr, 0);
		text_offset++;
	}
#if 0
	Vector forward, right, up;
	GetDoorVectors( &forward, &right, &up );
	forward = GetAbsOrigin() + forward * 20;
	right = GetAbsOrigin() - right * 20; // Left is positive
	NDebugOverlay::Line( GetAbsOrigin(), forward, 255, 0, 0, true, 0 );
	NDebugOverlay::Line( GetAbsOrigin(), right, 0, 255, 0, true, 0 );
#endif
	return text_offset;
}

//-----------------------------------------------------------------------------
// Purpose: Change this door's distance (in degrees) between open and closed
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorRotating<ENTITY_CLASS>::InputSetRotationDistance( inputdata_t &inputdata )
{
	m_flDistance = inputdata.value.Float();
	
	// Recalculate our open volume
	CalcOpenAngles();

#if 1
	if ( IsMovementReversed() )
	{
		::V_swap( m_angRotationOpenForward, m_angRotationOpenBack );
	}

	// BUG! GetLocalAngles() is not "closed" angles if the door spawns open!
	// You can see this by setting g_debug_doors=1
	CalculateDoorVolume( m_angRotationClosed, m_angRotationOpenForward, &m_vecForwardBoundsMin, &m_vecForwardBoundsMax );
	CalculateDoorVolume( m_angRotationClosed, m_angRotationOpenBack, &m_vecBackBoundsMin, &m_vecBackBoundsMax );
#else
	CalculateDoorVolume( GetLocalAngles(), m_angRotationOpenForward, &m_vecForwardBoundsMin, &m_vecForwardBoundsMax );
	CalculateDoorVolume( GetLocalAngles(), m_angRotationOpenBack, &m_vecBackBoundsMin, &m_vecBackBoundsMax );
#endif
}

//=============================================================================================================
// prop_door_rotating
//=============================================================================================================

#define DOOR_HARDWARE_GROUP 1

typedef CBaseDoor< CDynamicProp > CPropDoorRotatingBaseBaseClass;
typedef CBaseDoorRotating< CDynamicProp > CPropDoorRotatingBaseClass;

class CPropDoorRotating : public CPropDoorRotatingBaseClass
{
public:
	DECLARE_CLASS( CPropDoorRotating, CPropDoorRotatingBaseClass );
	DECLARE_DATADESC();

	void Spawn( void );

	bool TestCollision( const Ray_t &ray, unsigned int mask, trace_t& trace );

	void HandleAnimEvent(animevent_t *pEvent);

	void CalcDoorSounds( void );
	virtual bool IsMovementReversed( void );

	void OnUseLocked( void );
	void OnUseUnlocked( void );

	int m_nHardwareType;
};

BEGIN_DATADESC( CPropDoorRotatingBaseBaseClass )
	BASE_DOOR_DATADESC
END_DATADESC()

BEGIN_DATADESC( CPropDoorRotatingBaseClass )
	BASE_DOOR_ROTATING_DATADESC
END_DATADESC()

BEGIN_DATADESC( CPropDoorRotating )
	DEFINE_KEYFIELD(m_nHardwareType, FIELD_INTEGER, "hardware"),
END_DATADESC()

LINK_ENTITY_TO_CLASS(prop_door_rotating, CPropDoorRotating);

//-----------------------------------------------------------------------------
void CPropDoorRotating::Spawn(void)
{
	BaseClass::Spawn();

	AddSolidFlags( FSOLID_CUSTOMRAYTEST | FSOLID_CUSTOMBOXTEST );

	DisableAutoFade();

	RemoveFlag(FL_STATICPROP);

	SetBodygroup( DOOR_HARDWARE_GROUP, m_nHardwareType );
	if ((m_nHardwareType == 0) && (!HasSpawnFlags(SF_DOOR_LOCKED)))
	{
		// Doors with no hardware must always be locked.
		DevWarning(1, "Unlocked prop_door '%s' at (%.0f %.0f %.0f) has no hardware. All openable doors must have hardware!\n", GetDebugName(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
	}

	if ( !PropDataOverrodeBlockLOS() )
	{
		CalculateBlockLOS();
	}
}

//-----------------------------------------------------------------------------
bool CPropDoorRotating::TestCollision( const Ray_t &ray, unsigned int mask, trace_t& trace )
{
	if ( !VPhysicsGetObject() )
		return false;

	CStudioHdr *pStudioHdr = GetModelPtr( );
	if (!pStudioHdr)
		return false;

	physcollision->TraceBox( ray, VPhysicsGetObject()->GetCollide(), GetAbsOrigin(), GetAbsAngles(), &trace );

	if ( trace.DidHit() )
	{
		trace.surface.surfaceProps = VPhysicsGetObject()->GetMaterialIndex();
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void CPropDoorRotating::HandleAnimEvent(animevent_t *pEvent)
{
	// Opening is called here via an animation event if the open sequence has one,
	// otherwise it is called immediately when the open sequence is set.
	if ( pEvent->event == AE_DOOR_OPEN )
	{
		DoorActivate();
	}
}

// Only overwrite str1 if it's NULL_STRING.
#define ASSIGN_STRING_IF_NULL( str1, str2 ) \
	if ( ( str1 ) == NULL_STRING ) { ( str1 ) = ( str2 ); }

//-----------------------------------------------------------------------------
void CPropDoorRotating::CalcDoorSounds( void )
{
	ErrorIfNot( GetModel() != NULL, ( "prop_door with no model at %.2f %.2f %.2f\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z ) );

	string_t strSoundOpen = NULL_STRING;
	string_t strSoundClose = NULL_STRING;
	string_t strSoundMoving = NULL_STRING;
	string_t strSoundLocked = NULL_STRING;
	string_t strSoundUnlocked = NULL_STRING;

	bool bFoundSkin = false;
	// Otherwise, use the sounds specified by the model keyvalues. These are looked up
	// based on skin and hardware.
	KeyValues *modelKeyValues = new KeyValues("");
	if ( modelKeyValues->LoadFromBuffer( modelinfo->GetModelName( GetModel() ), modelinfo->GetModelKeyValueText( GetModel() ) ) )
	{
		KeyValues *pkvDoorSounds = modelKeyValues->FindKey("door_options");
		if ( pkvDoorSounds )
		{
			// Open / close / move sounds are looked up by skin index.
			char szSkin[80];
			int skin = m_nSkin;
			Q_snprintf( szSkin, sizeof( szSkin ), "skin%d", skin );
			KeyValues *pkvSkinData = pkvDoorSounds->FindKey( szSkin );
			if ( pkvSkinData )
			{
				strSoundOpen = AllocPooledString( pkvSkinData->GetString( "open" ) );
				strSoundClose = AllocPooledString( pkvSkinData->GetString( "close" ) );
				strSoundMoving = AllocPooledString( pkvSkinData->GetString( "move" ) );
				const char *pSurfaceprop = pkvSkinData->GetString( "surfaceprop" );
				if ( pSurfaceprop && VPhysicsGetObject() )
				{
					bFoundSkin = true;
					VPhysicsGetObject()->SetMaterialIndex( physprops->GetSurfaceIndex( pSurfaceprop ) );
				}
			}

			// Locked / unlocked sounds are looked up by hardware index.
			char szHardware[80];
			Q_snprintf( szHardware, sizeof( szHardware ), "hardware%d", m_nHardwareType );
			KeyValues *pkvHardwareData = pkvDoorSounds->FindKey( szHardware );
			if ( pkvHardwareData )
			{
				strSoundLocked = AllocPooledString( pkvHardwareData->GetString( "locked" ) );
				strSoundUnlocked = AllocPooledString( pkvHardwareData->GetString( "unlocked" ) );
			}

			// If any sounds were missing, try the "defaults" block.
			if ( ( strSoundOpen == NULL_STRING ) || ( strSoundClose == NULL_STRING ) || ( strSoundMoving == NULL_STRING ) ||
				 ( strSoundLocked == NULL_STRING ) || ( strSoundUnlocked == NULL_STRING ) )
			{
				KeyValues *pkvDefaults = pkvDoorSounds->FindKey( "defaults" );
				if ( pkvDefaults )
				{
					ASSIGN_STRING_IF_NULL( strSoundOpen, AllocPooledString( pkvDefaults->GetString( "open" ) ) );
					ASSIGN_STRING_IF_NULL( strSoundClose, AllocPooledString( pkvDefaults->GetString( "close" ) ) );
					ASSIGN_STRING_IF_NULL( strSoundMoving, AllocPooledString( pkvDefaults->GetString( "move" ) ) );
					ASSIGN_STRING_IF_NULL( strSoundLocked, AllocPooledString( pkvDefaults->GetString( "locked" ) ) );
					ASSIGN_STRING_IF_NULL( strSoundUnlocked, AllocPooledString( pkvDefaults->GetString( "unlocked" ) ) );
					// NOTE: No default needed for surfaceprop, it's set by the model
				}
			}
		}
	}
	modelKeyValues->deleteThis();
	modelKeyValues = NULL;
	if ( !bFoundSkin && VPhysicsGetObject() )
	{
		Warning( "%s has Door model (%s) with no door_options! Verify that SKIN is valid, and has a corresponding options block in the model QC file\n", GetDebugName(), modelinfo->GetModelName( GetModel() ) );
		VPhysicsGetObject()->SetMaterialIndex( physprops->GetSurfaceIndex("wood") );
	}

	// Any sound data members that are already filled out were specified as level designer overrides,
	// so they should not be overwritten.
	ASSIGN_STRING_IF_NULL( m_SoundOpen, strSoundOpen );
	ASSIGN_STRING_IF_NULL( m_SoundClose, strSoundClose );
	ASSIGN_STRING_IF_NULL( m_SoundMoving, strSoundMoving );
	ASSIGN_STRING_IF_NULL( m_ls.sLockedSound, strSoundLocked );
	ASSIGN_STRING_IF_NULL( m_ls.sUnlockedSound, strSoundUnlocked );

	// Make sure we have real, precachable sound names in all cases.
	UTIL_ValidateSoundName( m_SoundMoving, "DoorSound.Null" );
	UTIL_ValidateSoundName( m_SoundOpen, "DoorSound.Null" );
	UTIL_ValidateSoundName( m_SoundClose, "DoorSound.Null" );
	UTIL_ValidateSoundName( m_ls.sLockedSound, "DoorSound.Null" );
	UTIL_ValidateSoundName( m_ls.sUnlockedSound, "DoorSound.Null" );

	PrecacheScriptSound( STRING( m_SoundMoving ) );
	PrecacheScriptSound( STRING( m_SoundOpen ) );
	PrecacheScriptSound( STRING( m_SoundClose ) );
	PrecacheScriptSound( STRING( m_ls.sLockedSound ) );
	PrecacheScriptSound( STRING( m_ls.sUnlockedSound ) );
}

//-----------------------------------------------------------------------------
// Figures out whether the door's hinge is on its left or its right.
// Assumes:
// - that the door is hinged through its origin.
// - that the origin is at one edge of the door (revolving doors will give
//   a random answer)
// - that the hinge axis lies along the z axis
//-----------------------------------------------------------------------------
bool CPropDoorRotating::IsMovementReversed( void )
{
	//
	// Find the point farthest from the hinge in 2D.
	//
	Vector vecMins;
	Vector vecMaxs;
	CollisionProp()->WorldSpaceAABB( &vecMins, &vecMaxs );

	vecMins -= GetAbsOrigin();
	vecMaxs -= GetAbsOrigin();

	// Throw out z -- we only care about 2D distance.
	// NOTE: if we allow for arbitrary hinge axes, this needs to change
	vecMins.z = vecMaxs.z = 0;

	Vector vecPointCheck;
	if ( vecMins.LengthSqr() > vecMaxs.LengthSqr() )
	{
		vecPointCheck = vecMins;
	}
	else
	{
		vecPointCheck = vecMaxs;
	}

	//
	// See if the projection of that point lies along our right vector.
	// If it does, the door is hinged on its left.
	//
	Vector vecRight;
	GetVectors( NULL, &vecRight, NULL );
	float flDot = DotProduct( vecPointCheck, vecRight );

	return ( flDot > 0 );
}

//-----------------------------------------------------------------------------
void CPropDoorRotating::OnUseLocked( void )
{
	PlayLockSounds(this, &m_ls, TRUE, FALSE);
	PropSetSequence(SelectWeightedSequence((Activity)ACT_DOOR_LOCKED));
}

//-----------------------------------------------------------------------------
void CPropDoorRotating::OnUseUnlocked( void )
{
	PlayLockSounds(this, &m_ls, FALSE, FALSE);
	int nSequence = SelectWeightedSequence((Activity)ACT_DOOR_OPEN);
	PropSetSequence(nSequence);

	if ((nSequence == -1) || !HasAnimEvent(nSequence, AE_DOOR_OPEN))
	{
		// No open anim event, we need to open the door here.
		DoorActivate();
	}
}

//=============================================================================================================
// func_door_rotating
//=============================================================================================================

// When this is defined, it is assumed that all func_door_rotatings use the original flags/keyvalues.
// When defined, old-style keyvalues are converted silently to the newer prop_door code.
#define FUNC_DOOR_ROTATING_COMPAT

typedef CBaseDoor< CBaseEntity > CFuncDoorRotatingBaseBaseClass;
typedef CBaseDoorRotating< CBaseEntity > CFuncDoorRotatingBaseClass;

class CFuncDoorRotating : public CFuncDoorRotatingBaseClass
{
public:
	DECLARE_CLASS( CFuncDoorRotating, CFuncDoorRotatingBaseClass );
	DECLARE_DATADESC();

	void Spawn( void );
	virtual void CalcDoorSounds( void );

	virtual bool IsMovementReversed( void );

#ifdef FUNC_DOOR_ROTATING_COMPAT
	bool KeyValue( const char *szKeyName, const char *szValue );
#endif
};

BEGIN_DATADESC( CFuncDoorRotatingBaseBaseClass )
	BASE_DOOR_DATADESC
END_DATADESC()

BEGIN_DATADESC( CFuncDoorRotatingBaseClass )
	BASE_DOOR_ROTATING_DATADESC
END_DATADESC()

BEGIN_DATADESC( CFuncDoorRotating )
END_DATADESC()

LINK_ENTITY_TO_CLASS( func_door_rotating, CFuncDoorRotating );

//-----------------------------------------------------------------------------
void CFuncDoorRotating::Spawn( void )
{
#ifdef FUNC_DOOR_ROTATING_COMPAT
	if ( HasSpawnFlags( SF_DOOR_ONEWAY ) )
		KeyValue( "opendir", "2" );
	if ( HasSpawnFlags( SF_DOOR_NO_AUTO_RETURN ) )
		AddSpawnFlags( SF_DOOR_USE_CLOSES );
	if ( !HasSpawnFlags( SF_DOOR_PUSE ) )
		AddSpawnFlags( SF_DOOR_IGNORE_USE );
#endif

	// Do this before BaseClass::Spawn() so CalculateDoorVolume works.
	SetModel( STRING( GetModelName() ) );

	BaseClass::Spawn();

	if ( GetMoveParent() && GetRootMoveParent()->GetSolid() == SOLID_BSP )
	{
		SetSolid( SOLID_BSP );
	}

	if ( HasSpawnFlags(SF_DOOR_PASSABLE) )
	{
		//normal door
		AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );
		AddSolidFlags( FSOLID_NOT_SOLID );
	}

	if ( HasSpawnFlags( SF_DOOR_NONSOLID_TO_PLAYER ) )
	{
		SetCollisionGroup( COLLISION_GROUP_PASSABLE_DOOR );
	}
#if 0 // FIXME: can't prop_doors ignore debris?
	if ( m_bIgnoreDebris )
	{
		// both of these flags want to set the collision group and 
		// there isn't a combo group
		Assert( !HasSpawnFlags( SF_DOOR_NONSOLID_TO_PLAYER ) );
		if ( HasSpawnFlags( SF_DOOR_NONSOLID_TO_PLAYER ) )
		{
			Warning("Door %s with conflicting collision settings, removing ignoredebris\n", GetDebugName() );
		}
		else
		{
			SetCollisionGroup( COLLISION_GROUP_INTERACTIVE );
		}
	}
#endif

//	CreateVPhysics();
}

//-----------------------------------------------------------------------------
bool CFuncDoorRotating::IsMovementReversed( void )
{
	return HasSpawnFlags( SF_DOOR_ROTATE_BACKWARDS );
}

#ifdef FUNC_DOOR_ROTATING_COMPAT
//-----------------------------------------------------------------------------
bool CFuncDoorRotating::KeyValue( const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "wait") )
	{
		float wait = Q_atof(szValue);
		if ( wait == 0 )
			wait = -1;
		CFmtStr fmt("%g", wait);
		return BaseClass::KeyValue( "returndelay", fmt.Access() );
	}
	else if (FStrEq(szKeyName, "noise1"))
	{
		return BaseClass::KeyValue( "soundmoveoverride", szValue );
	}
	else if (FStrEq(szKeyName, "noise2"))
	{
		BaseClass::KeyValue( "soundcloseoverride", szValue );
		return BaseClass::KeyValue( "soundopenoverride", szValue );
	}
	else if (FStrEq(szKeyName, "locked_sound"))
	{
		return BaseClass::KeyValue( "soundlockedoverride", szValue );
	}
	else if (FStrEq(szKeyName, "unlocked_sound"))
	{
		return BaseClass::KeyValue( "soundunlockedoverride", szValue );
	}
	else 
		return BaseClass::KeyValue( szKeyName, szValue );

	return true;
}
#endif

//-----------------------------------------------------------------------------
void CFuncDoorRotating::CalcDoorSounds( void )
{
	// from doors.cpp
	UTIL_ValidateSoundName( m_SoundMoving,		"RotDoorSound.DefaultMove" );
	UTIL_ValidateSoundName( m_SoundOpen,		"RotDoorSound.DefaultArrive" );
	UTIL_ValidateSoundName( m_SoundClose,		"RotDoorSound.DefaultArrive" );
	UTIL_ValidateSoundName( m_ls.sLockedSound,	"RotDoorSound.DefaultLocked" );
	UTIL_ValidateSoundName( m_ls.sUnlockedSound,"DoorSound.Null" );

	PrecacheScriptSound( STRING( m_SoundMoving ) );
	PrecacheScriptSound( STRING( m_SoundOpen ) );
	PrecacheScriptSound( STRING( m_SoundClose ) );
	PrecacheScriptSound( STRING( m_ls.sLockedSound ) );
	PrecacheScriptSound( STRING( m_ls.sUnlockedSound ) );
}

//=============================================================================================================
// Base linear door
//=============================================================================================================

template< class ENTITY_CLASS >
class CBaseDoorLinear : public CBaseDoor< ENTITY_CLASS >
{
public:
	DECLARE_CLASS( CBaseDoorLinear, CBaseDoor< ENTITY_CLASS > );
	DECLARE_DATADESC();

	void Spawn( void );

	virtual void GetNPCOpenData( CAI_BaseNPC *pNPC, opendata_t &opendata );
	virtual float GetOpenInterval( void );

	virtual void DoorTeleportToSpawnPosition( void );
	void MoveDone( void );

	void BeginOpening( CBaseEntity *pOpenAwayFrom );
	void BeginClosing( void );

	void DoorResume( void );
	void DoorStop( void );

	// from subs.cpp
	void LinearMove( const Vector &vecDest, float flSpeed );

	bool DoorCanClose( bool bAutoClose );

	Vector m_vecMoveDir;
	float m_flLip;
	Vector m_vecPositionOpen;
	Vector m_vecPositionClosed;
	Vector m_vecGoal;
};

#define BASE_DOOR_LINEAR_DATADESC \
	DEFINE_KEYFIELD( m_flLip, FIELD_FLOAT, "lip" ), \
	DEFINE_KEYFIELD( m_vecMoveDir, FIELD_VECTOR, "movedir" ), \
	DEFINE_FIELD( m_vecPositionOpen, FIELD_POSITION_VECTOR ), \
	DEFINE_FIELD( m_vecPositionClosed, FIELD_POSITION_VECTOR ), \
	DEFINE_FIELD( m_vecGoal, FIELD_POSITION_VECTOR ),

//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorLinear<ENTITY_CLASS>::Spawn( void )
{
	// Convert movedir from angles to a vector
	QAngle angMoveDir = QAngle( m_vecMoveDir.x, m_vecMoveDir.y, m_vecMoveDir.z );
	AngleVectors( angMoveDir, &m_vecMoveDir );

	m_vecPositionClosed = GetLocalOrigin();

	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	Vector vecOBB = CollisionProp()->OBBSize();
	vecOBB -= Vector( 2, 2, 2 );
	m_vecPositionOpen = m_vecPositionClosed + (m_vecMoveDir * (DotProductAbs( m_vecMoveDir, vecOBB ) - m_flLip));

	// Do this after calculating the open/closed positions so DoorTeleportToSpawnPosition() works
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorLinear<ENTITY_CLASS>::DoorTeleportToSpawnPosition( void )
{
	if ( HasSpawnFlags( SF_DOOR_START_OPEN_OBSOLETE ) )
	{
		UTIL_SetOrigin( this, m_vecPositionOpen );
		SetDoorState( DOOR_STATE_OPEN );
	}
	else
	{
		SetDoorState( DOOR_STATE_CLOSED );
	}
}

//-----------------------------------------------------------------------------
// Purpose: After moving, set origin to exact final position, call "move done" function.
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorLinear<ENTITY_CLASS>::MoveDone( void )
{
	SetLocalOrigin( m_vecGoal );
	SetLocalVelocity( vec3_origin );
	SetMoveDoneTime( -1 );
	BaseClass::MoveDone();
}

//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorLinear<ENTITY_CLASS>::GetNPCOpenData(CAI_BaseNPC *pNPC, opendata_t &opendata)
{
	ASSERT( pNPC != NULL );
	if ( pNPC == NULL ) return;

	// FIXME: Need to figure front/back of door and stand there.
	// TEMP: just stand where the NPC already is.
	opendata.vecStandPos = pNPC->GetAbsOrigin();
	AngleVectors( pNPC->GetAbsAngles(), &opendata.vecFaceDir );
	opendata.eActivity = ACT_OPEN_DOOR;
}

//-----------------------------------------------------------------------------
// Purpose: Returns how long it will take this door to open.
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
float CBaseDoorLinear<ENTITY_CLASS>::GetOpenInterval()
{
	// set destdelta to the vector needed to move
	Vector vecDestDelta = m_vecPositionOpen - m_vecPositionClosed;
	
	// divide by speed to get time to reach dest
	return vecDestDelta.Length() / m_flSpeed;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorLinear<ENTITY_CLASS>::BeginOpening( CBaseEntity *pOpenAwayFrom )
{
	LinearMove( m_vecPositionOpen, m_flSpeed );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorLinear<ENTITY_CLASS>::BeginClosing( void )
{
	LinearMove( m_vecPositionClosed, m_flSpeed );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorLinear<ENTITY_CLASS>::DoorStop( void )
{
	SetLocalVelocity( vec3_origin );
	SetMoveDoneTime( -1 );
}

//-----------------------------------------------------------------------------
// Purpose: Restart a door moving that was temporarily paused
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorLinear<ENTITY_CLASS>::DoorResume( void )
{
	// Restart our movement
	LinearMove( m_vecGoal, m_flSpeed );
}

//-----------------------------------------------------------------------------
// Purpose: Calculate m_vecVelocity and m_flNextThink to reach vecDest from
//			GetOrigin() traveling at flSpeed.
// Input  : Vector	vecDest - 
//			flSpeed - 
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
void CBaseDoorLinear<ENTITY_CLASS>::LinearMove( const Vector &vecDest, float flSpeed )
{
	ASSERTSZ(flSpeed != 0, "LinearMove:  no speed is defined!");
	
	m_vecGoal = vecDest;

	// Already there?
	if ( vecDest == GetLocalOrigin() )
	{
		MoveDone();
		return;
	}
		
	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - GetLocalOrigin();
	
	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set m_flNextThink to trigger a call to LinearMoveDone when dest is reached
	SetMoveDoneTime( flTravelTime );

	// scale the destdelta vector by the time spent traveling to get velocity
	SetLocalVelocity( vecDestDelta / flTravelTime );
}

//-----------------------------------------------------------------------------
// Custom trace filter for linear doors
//-----------------------------------------------------------------------------

class CTraceFilterDoorLinear : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterDoorLinear );
	
	CTraceFilterDoorLinear( const IHandleEntity *pDoor, const IHandleEntity *passentity, int collisionGroup )
		: m_pDoor(pDoor), m_pPassEnt(passentity), m_collisionGroup(collisionGroup)
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !StandardFilterRules( pHandleEntity, contentsMask ) )
			return false;

		if ( !PassServerEntityFilter( pHandleEntity, m_pDoor ) )
			return false;

		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;

		// Don't test if the game code tells us we should ignore this collision...
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );
		
		if ( pEntity )
		{
			if ( !pEntity->ShouldCollide( m_collisionGroup, contentsMask ) )
				return false;
			
			if ( !g_pGameRules->ShouldCollide( m_collisionGroup, pEntity->GetCollisionGroup() ) )
				return false;

			// Brush model bboxes are expanded by 1 in all directions reporting collisions when there are none,
			// blocking doors.
			if ( ( pEntity->GetModel() != NULL ) && ( modelinfo->GetModelType( pEntity->GetModel() ) == mod_brush ) )
			{
				Vector mins, maxs;
				pEntity->CollisionProp()->WorldSpaceAABB( &mins, &maxs );

				// Subtract 1 from each dimension because the engine expands bboxes by 1 in all directions making the size too big
				mins += Vector(1,1,1);
				maxs -= Vector(1,1,1);

				if ( !IsBoxIntersectingBox( mins, maxs, m_mins, m_maxs ) )
					return false;
			}
		}

		return true;
	}

	Vector m_mins, m_maxs;
private:

	const IHandleEntity *m_pDoor;
	const IHandleEntity *m_pPassEnt;
	int m_collisionGroup;
};

template< class ENTITY_CLASS>
inline void TraceHull_DoorLinear( const CBaseDoor<ENTITY_CLASS> *pDoor, const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &hullMin, 
					 const Vector &hullMax,	unsigned int mask, const CBaseEntity *ignore, 
					 int collisionGroup, trace_t *ptr )
{
	Ray_t ray;
	ray.Init( vecAbsStart, vecAbsEnd, hullMin, hullMax );
	CTraceFilterDoorLinear traceFilter( pDoor, ignore, collisionGroup );

	traceFilter.m_mins = hullMin; // this only works when vecAbsStart==vecAbsEnd
	traceFilter.m_maxs = hullMax;

	enginetrace->TraceRay( ray, mask, &traceFilter, ptr );
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether the way is clear for the door to close.
// Input  : bAutoClose - TRUE if the door is auto-closing.
// Output : Returns true if the door can close, false if the way is blocked.
//-----------------------------------------------------------------------------
template< class ENTITY_CLASS >
bool CBaseDoorLinear<ENTITY_CLASS>::DoorCanClose( bool bAutoClose )
{
	Vector mins, maxs;
	mins = CollisionProp()->OBBMins();
	maxs = CollisionProp()->OBBMaxs();

	// Subtract 1 from each dimension because the engine expands bboxes by 1 in all directions making the size too big
	mins += Vector(1,1,1);
	maxs -= Vector(1,1,1);

	// Look for blocking entities, ignoring ourselves and the entity that opened us.
	trace_t	tr;
	TraceHull_DoorLinear( this, m_vecPositionClosed, m_vecPositionClosed, mins, maxs, MASK_SOLID, GetActivator(), COLLISION_GROUP_NONE, &tr );
	if ( tr.allsolid || tr.startsolid )
	{
		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( GetAbsOrigin(), mins, maxs, 255, 0, 0, true, 10.0f );

			if ( tr.m_pEnt )
			{
				NDebugOverlay::Box( tr.m_pEnt->GetAbsOrigin(), tr.m_pEnt->CollisionProp()->OBBMins(), tr.m_pEnt->CollisionProp()->OBBMaxs(), 220, 220, 0, true, 10.0f );
			}
		}

		return false;
	}

	if ( g_debug_doors.GetBool() )
	{
		NDebugOverlay::Box( GetAbsOrigin(), mins, maxs, 0, 255, 0, true, 10.0f );
	}

	return true;
}

//=============================================================================================================
// func_door
//=============================================================================================================

// When this is defined, it is assumed that all func_doors use the original flags/keyvalues.
// When defined, old-style keyvalues are converted silently to the newer prop_door code.
#define FUNC_DOOR_COMPAT

typedef CBaseDoorLinear< CBaseEntity > CFuncDoorBaseClass;

class CFuncDoor : public CFuncDoorBaseClass
{
public:
	DECLARE_CLASS( CFuncDoor, CFuncDoorBaseClass );
	DECLARE_DATADESC();

	void Spawn( void );
	virtual void CalcDoorSounds( void );

#ifdef FUNC_DOOR_COMPAT
	bool KeyValue( const char *szKeyName, const char *szValue );
#endif
};

BEGIN_DATADESC( CFuncDoorBaseClass )
	BASE_DOOR_LINEAR_DATADESC
END_DATADESC()

BEGIN_DATADESC( CFuncDoor )
END_DATADESC()

LINK_ENTITY_TO_CLASS( func_door, CFuncDoor );

//-----------------------------------------------------------------------------
void CFuncDoor::Spawn( void )
{
#ifdef FUNC_DOOR_COMPAT
	if ( HasSpawnFlags( SF_DOOR_NO_AUTO_RETURN ) )
		AddSpawnFlags( SF_DOOR_USE_CLOSES );
	if ( !HasSpawnFlags( SF_DOOR_PUSE ) )
		AddSpawnFlags( SF_DOOR_IGNORE_USE );
#endif

	SetModel( STRING( GetModelName() ) );

	if ( GetMoveParent() && GetRootMoveParent()->GetSolid() == SOLID_BSP )
	{
		SetSolid( SOLID_BSP );
	}

	BaseClass::Spawn();

	if ( HasSpawnFlags(SF_DOOR_PASSABLE) )
	{
		//normal door
		AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );
		AddSolidFlags( FSOLID_NOT_SOLID );
	}

	if ( HasSpawnFlags( SF_DOOR_NONSOLID_TO_PLAYER ) )
	{
		SetCollisionGroup( COLLISION_GROUP_PASSABLE_DOOR );
	}
#if 0 // FIXME: can't prop_doors ignore debris?
	if ( m_bIgnoreDebris )
	{
		// both of these flags want to set the collision group and 
		// there isn't a combo group
		Assert( !HasSpawnFlags( SF_DOOR_NONSOLID_TO_PLAYER ) );
		if ( HasSpawnFlags( SF_DOOR_NONSOLID_TO_PLAYER ) )
		{
			Warning("Door %s with conflicting collision settings, removing ignoredebris\n", GetDebugName() );
		}
		else
		{
			SetCollisionGroup( COLLISION_GROUP_INTERACTIVE );
		}
	}
#endif
}

#ifdef FUNC_DOOR_COMPAT
//-----------------------------------------------------------------------------
bool CFuncDoor::KeyValue( const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "wait") )
	{
		float wait = Q_atof(szValue);
		if ( wait == 0 )
			wait = -1;
		CFmtStr fmt("%g", wait);
		return BaseClass::KeyValue( "returndelay", fmt.Access() );
	}
	else if (FStrEq(szKeyName, "noise1"))
	{
		return BaseClass::KeyValue( "soundmoveoverride", szValue );
	}
	else if (FStrEq(szKeyName, "noise2"))
	{
		BaseClass::KeyValue( "soundcloseoverride", szValue );
		return BaseClass::KeyValue( "soundopenoverride", szValue );
	}
	else if (FStrEq(szKeyName, "locked_sound"))
	{
		return BaseClass::KeyValue( "soundlockedoverride", szValue );
	}
	else if (FStrEq(szKeyName, "unlocked_sound"))
	{
		return BaseClass::KeyValue( "soundunlockedoverride", szValue );
	}
	else 
		return BaseClass::KeyValue( szKeyName, szValue );

	return true;
}
#endif

//-----------------------------------------------------------------------------
void CFuncDoor::CalcDoorSounds( void )
{
	// from doors.cpp
	UTIL_ValidateSoundName( m_SoundMoving,		"DoorSound.DefaultMove" );
	UTIL_ValidateSoundName( m_SoundOpen,		"DoorSound.DefaultArrive" );
	UTIL_ValidateSoundName( m_SoundClose,		"DoorSound.DefaultArrive" );
	UTIL_ValidateSoundName( m_ls.sLockedSound,	"DoorSound.DefaultLocked" );
	UTIL_ValidateSoundName( m_ls.sUnlockedSound,"DoorSound.Null" );

	PrecacheScriptSound( STRING( m_SoundMoving ) );
	PrecacheScriptSound( STRING( m_SoundOpen ) );
	PrecacheScriptSound( STRING( m_SoundClose ) );
	PrecacheScriptSound( STRING( m_ls.sLockedSound ) );
	PrecacheScriptSound( STRING( m_ls.sUnlockedSound ) );
}
