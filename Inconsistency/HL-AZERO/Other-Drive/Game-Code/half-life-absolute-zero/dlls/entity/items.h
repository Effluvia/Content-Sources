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
#ifndef ITEMS_H
#define ITEMS_H

#include "cbase.h"  //MODDD - really used to depend on the caller to provide this? Why?

// NOTE - every sound played here or in items.cpp send 'FALSE' to using the sound sentence save system,
// as these sounds are all hard-precached by the player (ignores that setting).

class CItem : public CBaseEntity
{
public:
	void Spawn( void );
	CBaseEntity* Respawn( void );
	void EXPORT ItemTouch( CBaseEntity *pOther );
	void EXPORT Materialize( void );
	virtual BOOL MyTouch( CBasePlayer *pPlayer ) { return FALSE; };
};

#endif // ITEMS_H
