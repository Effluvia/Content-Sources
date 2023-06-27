//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef ELIGHT_H
#define ELIGHT_H

#define MAX_MODEL_ENTITY_LIGHTS	8

struct elight_t
{
	int entindex;

	vec3_t origin;
	vec3_t color;
	float radius;

	vec3_t mins;
	vec3_t maxs;

	bool temporary;
};
#endif