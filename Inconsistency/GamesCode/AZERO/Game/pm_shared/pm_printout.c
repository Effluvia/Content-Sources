
#include "external_lib_include.h"
#include "build_settings.h"

#include <assert.h>
#include "mathlib.h"
#include "const.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_shared.h"
#include "pm_movevars.h"
#include "pm_debug.h"
#include <stdio.h>  // NULL
#include <math.h>   // sqrt
#include <string.h> // strcpy
#include <ctype.h>  // isspace

// MODDD - TODO.  Support something like "easyForcePrintLine" for here?
// Has to be re-done since there are new print methods (pmove->Con_DPrintf).
// Then again this isn't touched much.

// And should Con_Printf or Con_DPrintf be used?? I forget the difference, if there was one.


extern playermove_t* pmove;



#ifdef CLIENT_DLL
#define DEFAULT_PRINTOUT_C_ID "CL: "
#else
#define DEFAULT_PRINTOUT_C_ID "SV: "
#endif




char* UTIL_VarArgsVA_C(const char* szFmt, va_list argptr)
{
	static char arychr_buffer[1024];
	vsprintf(arychr_buffer, szFmt, argptr);
	return arychr_buffer;
}
char* UTIL_VarArgsVA_C_ID(const char* szFmt, va_list argptr)
{
	static char arychr_buffer[1024];
	char* currentEnd;
#if (defined(_DEBUG) || FORCE_PRINTOUT_PREFIX != 0)
	currentEnd = strcpy(&arychr_buffer[0], DEFAULT_PRINTOUT_C_ID);
	currentEnd = &arychr_buffer[4];
#else
	currentEnd = &arychr_buffer[0];  //just the start, nothing special.
#endif
	vsprintf(currentEnd, szFmt, argptr);
	return arychr_buffer;
}



#define PRINT_VA_C\
	va_list argptr;\
	va_start(argptr, szFmt);\
	pmove->Con_DPrintf( UTIL_VarArgsVA_C(szFmt, argptr ) );\
	va_end(argptr);

#define PRINT_VA_C_LINE\
	char* bufRef;\
	va_list argptr;\
	va_start(argptr, szFmt);\
	bufRef = UTIL_VarArgsVA_C_ID(szFmt, argptr );\
	strcat(bufRef, "\n");\
	pmove->Con_DPrintf( bufRef );\
	va_end(argptr);





void easyForcePrint(char* szFmt, ...) {
	PRINT_VA_C
}
void easyForcePrintStarter(void) {
#if (defined(_DEBUG) || FORCE_PRINTOUT_PREFIX != 0)
	pmove->Con_DPrintf(DEFAULT_PRINTOUT_C_ID);
#endif
}
void easyForcePrintLine(char* szFmt, ...) {
	PRINT_VA_C_LINE
}
/*
void easyForcePrintLine(void) {
	pmove->Con_DPrintf("\n");
}
*/

