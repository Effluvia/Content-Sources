
//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: A base class for model-based doors. The exact movement required to
//			open or close the door is not dictated by this class, only that
//			the door has open, closed, opening, and closing states.
//
//			Doors must satisfy these requirements:
//
//			- Derived classes must support being opened by NPCs.
//			- Never autoclose in the face of a player.
//			- Never close into an NPC.
//
//=============================================================================//

#ifndef HOE_DOORS_H
#define HOE_DOORS_H
#ifdef _WIN32
#pragma once
#endif

//#include "props.h"
#include "locksounds.h"
#include "entityoutput.h"

extern ConVar g_debug_doors;

struct opendata_t
{
	Vector vecStandPos;		// Where the NPC should stand.
	Vector vecFaceDir;		// What direction the NPC should face.
	Activity eActivity;		// What activity the NPC should play.
};

class IDoorAccessor
{
public:
	void Init( CBaseEntity *pEnt )
	{
		m_pDoor = pEnt;
	}
	CBaseEntity *GetEntity( void ) const
	{
		return m_pDoor;
	}

	virtual bool IsDoorOpen() = 0;
	virtual bool IsDoorAjar() = 0;
	virtual bool IsDoorOpening() = 0;
	virtual bool IsDoorClosed() = 0;
	virtual bool IsDoorClosing() = 0;
	virtual bool IsDoorLocked( CBaseEntity *pActivator ) = 0;
	virtual bool IsDoorBlocked() const = 0;
	virtual bool IsNPCOpening(CAI_BaseNPC *pNPC) = 0;
	virtual bool IsPlayerOpening() = 0;
	virtual bool IsOpener(CBaseEntity *pEnt) = 0;

	virtual bool NPCOpenDoor(CAI_BaseNPC *pNPC) = 0;
	virtual bool TestCollision( const Ray_t &ray, unsigned int mask, trace_t& trace ) = 0;

	virtual bool DoorCanClose( bool bAutoClose ) = 0;
	virtual bool DoorCanOpen( void ) = 0;

	virtual void GetNPCOpenData(CAI_BaseNPC *pNPC, opendata_t &opendata) = 0;
	virtual float GetOpenInterval(void) = 0;

private:
	CBaseEntity *m_pDoor;
};

// IDoorAccessor *pDoorAccessor = 0;
// IDoor *pDoor = dynamic_cast< IDoor *>(pEnt);
// if ( pDoor != NULL )
//		 pDoorAccessor = pDoor->GetDoorAccessor();
class IDoor
{
public:
	virtual IDoorAccessor *GetDoorAccessor( void ) = 0;
};

#endif // HOE_DOORS_H
