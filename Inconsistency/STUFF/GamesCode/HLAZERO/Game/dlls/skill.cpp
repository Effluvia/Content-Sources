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
//=========================================================
// skill.cpp - code for skill level concerns
//=========================================================
#include "extdll.h"
#include "util.h"
#include "skill.h"


skilldata_t gSkillData;


//=========================================================
// take the name of a cvar, tack a digit for the skill level
// on, and return the value.of that Cvar 
//=========================================================
float GetSkillCvar( char *pName )
{
	int	iCount;
	float flValue;
	char szBuffer[ 64 ];
	
	//MODDD - 'gSkillData.iSkillLevel' replaced with 'g_iSkillLevel
	iCount = sprintf( szBuffer, "%s%d",pName, g_iSkillLevel );

	flValue = CVAR_GET_FLOAT ( szBuffer );

	//MODDD - no longer send a message on 0 values, these are valid for some now.
	//if ( flValue <= 0 )
	//{
	//	ALERT ( at_console, "\n\n** GetSkillCVar Got a zero for %s **\n\n", szBuffer );
	//}

	return flValue;
}

// MODDD - NEW.
// No difficulty number after expected.  Pretty plain.
float GetSkillCvarSingular(char* pName)
{
	float flValue;
	flValue = CVAR_GET_FLOAT(pName);

	//MODDD - no longer send a message on 0 values, these are valid for some now.
	//if (flValue <= 0)
	//{
	//	ALERT(at_console, "\n\n** GetSkillCVar Got a zero for %s **\n\n", pName);
	//}

	return flValue;
}