//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Includes all the headers/declarations necessary to access the
//			engine interface
//
// $NoKeywords: $
//=============================================================================

#ifndef ENGINEINTERFACE_H
#define ENGINEINTERFACE_H

#ifdef _WIN32
#pragma once
#endif

// these stupid set of includes are required to use the cdll_int interface
#include "vector.h"
//#include "wrect.h"
#define IN_BUTTONS_H

// engine interface
#include "cdll_int.h"
#include "icvar.h"

#include "../zzz NEW/cdll_int_goldsrc.h"

// engine interface singleton accessor
extern IVEngineClient *engine;
extern class IGameUIFuncs *gameuifuncs;
extern class IEngineSound *enginesound;

//MODDD - don't use engine, use goldsrc wherever possible for equivalent calls
// or comment out.
extern cl_enginefunc_t* goldsrc_engine;



#endif // ENGINEINTERFACE_H
