//======== (C) Copyright 1999, 2000 Valve, L.L.C. All rights reserved. ========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#if !defined( CLIENT_COMMAND_H )
#define CLIENT_COMMAND_H
#ifdef _WIN32
#pragma once
#endif

typedef struct cmd_s
{
	float		senttime;
	float		receivedtime;

	float		frame_lerp;

	bool		processedfuncs;
	bool		heldback;

	int			sendsize;
} cmd_t;

#endif // CLIENT_COMMAND_H