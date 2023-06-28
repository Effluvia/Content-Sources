
// GENERAL NOTE:
// To my knowledge, there isn't a way to send variable arguments (...) to another method that also takes
// variable arguments directly.  Just have to copy them to a new string with the parameters filled in and
// send that to the 2nd method.
// "In English please?"
// Say you have a method that's a wrapper for a more specific engine call for printouts, like 
// easyForcePrintLine, which calls g_engfuncs.pfnAlertMessage.  pfnAlertMessage can take a variable
// number of arguments, just like easyForcePrintLine.  But we have no way of saying, "give my arguments
// from easyForcePrintLine to pfnAlertMessage for filling in on its own", like if I call 
// easyForcePrintLine("some text to print %.2f %s", 46.3f, "text"),  you can't specify to give the format
// string and 2 extra parameters to pfnAlertMessage.  You have to write the format string & 2 params to 
// a temporary string first, and give that filled string  ("some text to print 46.3 text") to
// pfnAlertMessage.  This is done most often through  UTIL_VarArgsVA(szFmt, argptr ): fills a temp
// char buffer with the filled format string and returns a pointer to it to be printed out as one
// parameter by pfnAlertMessage in this case.

// Sending the standard name for the argument list, "argptr", without involving UTIL_VarArgsVA, will still
// compile, but will NOT WORK.  It is only a pointer to the arguments in memory, and any printout method
// won't know what to do with that.  You'll get garbage.

// And, most printout methods don't even work with ellipses anyway, so they would need UTIL_VarArgsVA
// to pre-fill the string even if sending variable arguments directly from method to method were possible.


#include "build_settings.h"

#ifdef CLIENT_DLL
//***************************CLIENT INCLUDES******************************
#include "util_printout.h"
#include "util_preprocessor.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


#else
//***************************SERVER INCLUDES******************************
#include "extdll.h"
#include "util_printout.h"
#include "util_preprocessor.h"

#include "enginecallback.h"
#include "progdefs.h"

#include "cbase.h"

//reference:
//http://stackoverflow.com/questions/7031116/how-to-create-function-like-printf-variable-argument
//http://stackoverflow.com/questions/3530771/passing-variable-arguments-to-another-function-that-accepts-a-variable-argument
//
EASY_CVAR_EXTERN_DEBUGONLY(hgruntPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(panthereyePrintout)
EASY_CVAR_EXTERN_DEBUGONLY(squadmonsterPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(gargantuaPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(barnaclePrintout)
EASY_CVAR_EXTERN_DEBUGONLY(houndeyePrintout)

extern int gmsgTextMsg;
extern int gmsgSayText;


#endif



EASY_CVAR_EXTERN(hiddenmem);

// no need to vastly overcomplicate includes if we just need this from other util files right now.
extern void convertIntToBinary(char* buffer, unsigned int arg, unsigned int binaryDigits);




/*
//alert types for reference:
typedef enum
{
	at_notice,
	at_console,		// same as at_notice, but forces a ConPrintf, not a message box
	at_aiconsole,	// same as at_console, but only shown if developer level is 2!
	at_warning,
	at_error,
	at_logged		// Server print to console ( only in multiplayer games ).
} ALERT_TYPE;
*/


//Some other client-server dependent settings.
#ifdef CLIENT_DLL
//is there a difference between printout methods pfnConsolePrint and Con_DPrintf besides pfnConsolePrint
//taking strictly one argument (the string to print) as opposed to a variable list of arguments to combine by itself like printf?
// AND still not clear on whether to use Con_Printf or Con_DPrintf.

#define DEFAULT_ENGINE_HANDLE gEngfuncs
#define DEFAULT_PRINTOUT_ID "CL: "

#else

#define DEFAULT_ENGINE_HANDLE g_engfuncs
#define DEFAULT_PRINTOUT_ID "SV: "

#endif


// Because the alert print method for the server needs to be told "at_console",
// just going to re-do these separately for the client and server, that's the only difference.
// Not screwing around with "comma or not" differences between parameters after or not,
// really, really done with that.

#ifdef CLIENT_DLL

#define PRINT_VA\
	va_list argptr;\
	va_start(argptr, szFmt);\
	DEFAULT_ENGINE_HANDLE.Con_Printf( UTIL_VarArgsVA(szFmt, argptr ) );\
	va_end(argptr);

#define PRINT_VA_DEVELOPER\
	va_list argptr;\
	va_start(argptr, szFmt);\
	DEFAULT_ENGINE_HANDLE.Con_DPrintf( UTIL_VarArgsVA(szFmt, argptr ) );\
	va_end(argptr);


#define PRINT_LINE_VA\
	char* bufRef;\
	va_list argptr;\
	va_start(argptr, szFmt);\
	bufRef = UTIL_VarArgsVA_ID(szFmt, argptr );\
	va_end(argptr);\
	strcat(bufRef, "\n");\
	DEFAULT_ENGINE_HANDLE.Con_Printf( bufRef );

#define PRINT_LINE_VA_DEVELOPER\
	char* bufRef;\
	va_list argptr;\
	va_start(argptr, szFmt);\
	bufRef = UTIL_VarArgsVA_ID(szFmt, argptr );\
	va_end(argptr);\
	strcat(bufRef, "\n");\
	DEFAULT_ENGINE_HANDLE.Con_DPrintf( bufRef );

#else

// nope, no good.  Don't send 'argptr' as a parameter at the end alone, that doesn't fill in all the
// variable args.  Have to put them into a string to send by UTIL_VarArgsVA.
// Seems a little stupid not to have a way of just pasting the variable args to methods that can
// support variable args already, but hey.  C/C++ be C/C++ sometimes.
/*
#define PRINT_VA\
	va_list argptr;\
	va_start(argptr, szFmt);\
	DEFAULT_ENGINE_HANDLE.pfnAlertMessage(at_console, szFmt, argptr );\
	va_end(argptr);
*/

// WARNING:  pfnServerPrint and pfnAlertMessage still only go to the server console (or the first player's
// console for single-player and non-dedicated servers for the player running the server).
// Use broadcast printouts to reach all players from the server, but this should rarely ever be needed.
#define PRINT_VA\
	va_list argptr;\
	va_start(argptr, szFmt);\
	DEFAULT_ENGINE_HANDLE.pfnServerPrint(UTIL_VarArgsVA(szFmt, argptr ) );\
	va_end(argptr);

#define PRINT_VA_DEVELOPER\
	va_list argptr;\
	va_start(argptr, szFmt);\
	DEFAULT_ENGINE_HANDLE.pfnAlertMessage(at_console, UTIL_VarArgsVA(szFmt, argptr ) );\
	va_end(argptr);

#define PRINT_LINE_VA\
	char* bufRef;\
	va_list argptr;\
	va_start(argptr, szFmt);\
	bufRef = UTIL_VarArgsVA_ID(szFmt, argptr );\
	va_end(argptr);\
	strcat(bufRef, "\n");\
	DEFAULT_ENGINE_HANDLE.pfnServerPrint(bufRef );

#define PRINT_LINE_VA_DEVELOPER\
	char* bufRef;\
	va_list argptr;\
	va_start(argptr, szFmt);\
	bufRef = UTIL_VarArgsVA_ID(szFmt, argptr );\
	va_end(argptr);\
	strcat(bufRef, "\n");\
	DEFAULT_ENGINE_HANDLE.pfnAlertMessage(at_console, bufRef );


/*
// no point anymore, server printing is only for getting past the "developer 1" requirement
// that 'alert' has.
#define PRINT_SERVER_CONSOLE_VA\
	va_list argptr;\
	va_start(argptr, szFmt);\
	g_engfuncs.pfnServerPrint( UTIL_VarArgsVA(szFmt, argptr ) );\
	va_end(argptr);

#define PRINT_SERVER_CONSOLE_LINE_VA\
	char* bufRef;\
	va_list argptr;\
	va_start(argptr, szFmt);\
	bufRef = UTIL_VarArgsVA_ID(szFmt, argptr );\
	va_end(argptr);\
	strcat(bufRef, "\n");\
	g_engfuncs.pfnServerPrint( bufRef );
*/



// oh no!  For a version that works without "developer 1", we have to combine all args sent and
// push that to the client as a message.    Thanks valve.
/*
#define PRINT_SERVER_TO_CLIENT_VA(arg_destEdict, arg_printType)\
	va_list argptr;\
	va_start(argptr, szFmt);\
	g_engfuncs.pfnClientPrintf(arg_destEdict, arg_printType, UTIL_VarArgsVA(szFmt, argptr ));\
	va_end(argptr);

#define PRINT_SERVER_TO_CLIENT_LINE_VA(arg_destEdict, arg_printType)\
	char* bufRef;\
	va_list argptr;\
	va_start(argptr, szFmt);\
	bufRef = UTIL_VarArgsVA_Custom(szFmt, "SV-CL: ", 7, argptr );\
	va_end(argptr);\
	strcat(bufRef, "\n");\
	g_engfuncs.pfnClientPrintf(arg_destEdict, arg_printType, bufRef );
*/

// woohoo, this server method works fine, the "developer 1" requirement is ok.
#define PRINT_SERVER_TO_CLIENT_VA_DEVELOPER(arg_destEdict, arg_printType)\
	va_list argptr;\
	va_start(argptr, szFmt);\
	g_engfuncs.pfnClientPrintf(arg_destEdict, arg_printType, UTIL_VarArgsVA(szFmt, argptr ));\
	va_end(argptr);

#define PRINT_SERVER_TO_CLIENT_LINE_VA_DEVELOPER(arg_destEdict, arg_printType)\
	char* bufRef;\
	va_list argptr;\
	va_start(argptr, szFmt);\
	bufRef = UTIL_VarArgsVA_Custom(szFmt, "SV-CL: ", 7, argptr );\
	va_end(argptr);\
	strcat(bufRef, "\n");\
	g_engfuncs.pfnClientPrintf(arg_destEdict, arg_printType, bufRef );






//CLIENT_PRINTF -> pfnClientPrintf
//ClientPrint ???


#endif// END OF Client/Serverside check



char* UTIL_VarArgs( char *szFmt, ... )
{
	va_list		argptr;
	static char arychr_buffer[1024];
	
	va_start (argptr, szFmt);
	vsprintf (arychr_buffer, szFmt,argptr);
	va_end (argptr);

	return arychr_buffer;
}

char* UTIL_VarArgs_ID( char *szFmt, ... )
{
	va_list		argptr;
	static char arychr_buffer[1024];
	char* currentEnd;

	//arychr_buffer[0] = '\0';
	currentEnd = ::strcpy(&arychr_buffer[0], DEFAULT_PRINTOUT_ID );
	
	va_start (argptr, szFmt);
	vsprintf (currentEnd, szFmt,argptr);
	va_end (argptr);

	return arychr_buffer;
}



/*
void UTIL_easyPrint(char* stringarg, int numbarg){

	char numstr[128];
	sprintf(numstr, stringarg, numbarg);
	ClientPrintAll( HUD_PRINTNOTIFY, numstr);

}
*/

//#define UTIL_easyPrint(a, ...) ClientPrintAll( HUD_PRINTNOTIFY, UTIL_VarArgs( a, ##__VA_ARGS__ );
//CLIENT_PRINTF( pEntity, print_console, UTIL_VarArgs( "\"fov\" is \"%d\"\n", (int)GetClassPtr((CBasePlayer *)pev)->m_iFOV ) );

//g_engfuncs.pfnClientPrintf( pEntity, print_console, UTIL_VarArgs( "\"fov\" is \"%d\"\n", (int)GetClassPtr((CBasePlayer *)pev)->m_iFOV ) );
//#define UTIL_easyPrintClient(edict, str, ...) g_engfuncs.pfnClientPrintf( edict, print_console, UTIL_VarArgs( str, ##__VA_ARGS__ ) );


//This is essentially "UTIL_VarArgs" that accepts a "va_list" argument instead.  How it is applied involes the sender (wherever the call to this
//method is made) having responsibility for having "va_start" before calling this method, and "va_end" afterwards.
char* UTIL_VarArgsVA( const char *szFmt, va_list argptr )
{
	//va_list		argptr;
	static char arychr_buffer[1024];
	
	//va_start (argptr, szFmt);
	vsprintf (arychr_buffer, szFmt, argptr);
	//va_end (argptr);

	return arychr_buffer;
}

char* UTIL_VarArgsVA_ID( const char *szFmt, va_list argptr )
{
	//va_list		argptr;
	static char arychr_buffer[1024];
	char* currentEnd;
	
	#if (defined(_DEBUG) || FORCE_PRINTOUT_PREFIX != 0)
		//put a client/server indicator, just a few letters in front to tell where this printout came from.
		//The final string will start after that.
		//arychr_buffer[0] = '\0';
		currentEnd = ::strcpy(&arychr_buffer[0], DEFAULT_PRINTOUT_ID);
		currentEnd = &arychr_buffer[4]; //??? WHY? 
		/*
		currentEnd = &arychr_buffer[0];
		arychr_buffer[0] = 'a';
		arychr_buffer[1] = 's';
		arychr_buffer[2] = 's';
		arychr_buffer[3] = '\0';
		currentEnd = &arychr_buffer[3];
		*/
		//currentEnd = &arychr_buffer[0+3];
	#else
		currentEnd = &arychr_buffer[0];  //just the start, nothing special.
	#endif

	//va_start (argptr, szFmt);
	vsprintf (currentEnd, szFmt,argptr);
	//va_end (argptr);

	return arychr_buffer;
}

char* UTIL_VarArgsVA_Custom( const char *szFmt, const char* szPrefix, const int iPrefixLength, va_list argptr )
{
	//va_list		argptr;
	static char arychr_buffer[1024];
	char* currentEnd;

	#if (defined(_DEBUG) || FORCE_PRINTOUT_PREFIX != 0)
		currentEnd = ::strcpy(&arychr_buffer[0], szPrefix);
		currentEnd = &arychr_buffer[iPrefixLength];  //???? WHY
	#else
		currentEnd = &arychr_buffer[0];  //just the start, nothing special.
	#endif
	//va_start (argptr, szFmt);
	vsprintf (currentEnd, szFmt,argptr);
	//va_end (argptr);
	return arychr_buffer;
}










// NOTE - only do these if we're VS6. 
// We have a new way for after VS6.
// NOPE.  Dropping that idea!
//#ifdef IS_VS6
//MODDD: added for debugging convenience.  The same idea could be used
//in other client-side methods, I imagine (not sure about server-side).
//See my glock code for info about serverside-printouts.

void easyPrint(char *szFmt, ... )
{
	PRINT_VA_DEVELOPER
}
void easyForcePrint(char *szFmt, ... )
{
	PRINT_VA
}

void easyPrintStarter(void)
{
#if (defined(_DEBUG) || FORCE_PRINTOUT_PREFIX != 0)
#ifdef CLIENT_DLL
		DEFAULT_ENGINE_HANDLE.Con_DPrintf(DEFAULT_PRINTOUT_ID);
#else
		DEFAULT_ENGINE_HANDLE.pfnAlertMessage(at_console, DEFAULT_PRINTOUT_ID);
#endif
#endif
}
void easyForcePrintStarter(void)
{
#if (defined(_DEBUG) || FORCE_PRINTOUT_PREFIX != 0)
#ifdef CLIENT_DLL
	DEFAULT_ENGINE_HANDLE.Con_Printf(DEFAULT_PRINTOUT_ID);
#else
	DEFAULT_ENGINE_HANDLE.pfnServerPrint(DEFAULT_PRINTOUT_ID);
#endif
#endif
}

void easyPrintLine(char *szFmt, ... )
{
	PRINT_LINE_VA_DEVELOPER
}
void easyForcePrintLine(char *szFmt, ... )
{
	PRINT_LINE_VA
}


//PrintLine methods without arguments. That means, just print a new line.
void easyPrintLine(void)
{
#ifdef CLIENT_DLL
		DEFAULT_ENGINE_HANDLE.Con_DPrintf("\n");
#else
		DEFAULT_ENGINE_HANDLE.pfnAlertMessage(at_console, "\n");
#endif
}
void easyForcePrintLine(void)
{
#ifdef CLIENT_DLL
	DEFAULT_ENGINE_HANDLE.Con_Printf("\n");
#else
	DEFAULT_ENGINE_HANDLE.pfnServerPrint("\n");
#endif
}
//#endif //for the VS6 check, got canned.


	



// NOTE - outside the "#ifdef" checks below to go on the client & server.
// Prints the converted int to standard output (easyForcePrintLine) without giving a reference to the binary string to the caller.
void printIntAsBinary(unsigned int arg, unsigned int binaryDigits) {
	char binaryBuffer[33];
	convertIntToBinary(binaryBuffer, arg, binaryDigits);
	easyForcePrint(binaryBuffer);
}//END OF printIntAsBinary

// Same as above but add a newline character.
void printLineIntAsBinary(unsigned int arg, unsigned int binaryDigits) {
	char binaryBuffer[33];
	convertIntToBinary(binaryBuffer, arg, binaryDigits);
	easyForcePrintLine(binaryBuffer);
}//END OF printIntAsBinary


#ifdef CLIENT_DLL

#else
// Prints the converted int to standard output (easyForcePrintLine) without giving a reference to the binary string to the caller.
void printIntAsBinaryClient(edict_t* pEntity, unsigned int arg, unsigned int binaryDigits) {
	char binaryBuffer[33];
	convertIntToBinary(binaryBuffer, arg, binaryDigits);
	easyForcePrintClient(pEntity, binaryBuffer);
}//END OF printIntAsBinary

// Same as above but add a newline character.
void printLineIntAsBinaryClient(edict_t* pEntity, unsigned int arg, unsigned int binaryDigits) {
	char binaryBuffer[33];
	convertIntToBinary(binaryBuffer, arg, binaryDigits);
	easyForcePrintLineClient(pEntity, binaryBuffer);
}//END OF printIntAsBinary
#endif









//#ifndef CLIENT_DLL

void UTIL_printLineVector(const Vector& theVector){
	easyPrintLine("vector: (%.2f, %.2f, %.2f)", theVector.x, theVector.y, theVector.z);
}
void UTIL_printLineVector(const float vX, const float vY, const float vZ){
	easyPrintLine("vector: (%.2f, %.2f, %.2f)", vX, vY, vZ);
}
void UTIL_printLineVector(char* printLabel, const Vector& theVector){
	easyPrintLine("%s: (%.2f, %.2f, %.2f)", printLabel, theVector.x, theVector.y, theVector.z);
}
void UTIL_printLineVector(char* printLabel, const float vX, const float vY, const float vZ){
	easyPrintLine("%s: (%.2f, %.2f, %.2f)", printLabel, vX, vY, vZ);
}
void UTIL_printVector(const Vector& theVector){
	easyPrint("(%.2f, %.2f, %.2f) ", theVector.x, theVector.y, theVector.z);
}
void UTIL_printVector(const float vX, const float vY, const float vZ){
	easyPrint("(%.2f, %.2f, %.2f) ", vX, vY, vZ);
}
void UTIL_printVector(char* printLabel, const Vector& theVector){
	easyPrint("%s: (%.2f, %.2f, %.2f) ", printLabel, theVector.x, theVector.y, theVector.z);
}
void UTIL_printVector(char* printLabel, const float vX, const float vY, const float vZ){
	easyPrint("%s: (%.2f, %.2f, %.2f) ", printLabel, vX, vY, vZ);
}

void UTIL_forcePrintLineVector(const Vector& theVector) {
	easyForcePrintLine("vector: (%.2f, %.2f, %.2f)", theVector.x, theVector.y, theVector.z);
}
void UTIL_forcePrintLineVector(const float vX, const float vY, const float vZ) {
	easyForcePrintLine("vector: (%.2f, %.2f, %.2f)", vX, vY, vZ);
}
void UTIL_forcePrintLineVector(char* printLabel, const Vector& theVector) {
	easyForcePrintLine("%s: (%.2f, %.2f, %.2f)", printLabel, theVector.x, theVector.y, theVector.z);
}
void UTIL_forcePrintLineVector(char* printLabel, const float vX, const float vY, const float vZ) {
	easyForcePrintLine("%s: (%.2f, %.2f, %.2f)", printLabel, vX, vY, vZ);
}
void UTIL_forcePrintVector(const Vector& theVector) {
	easyForcePrint("(%.2f, %.2f, %.2f) ", theVector.x, theVector.y, theVector.z);
}
void UTIL_forcePrintVector(const float vX, const float vY, const float vZ) {
	easyForcePrint("(%.2f, %.2f, %.2f) ", vX, vY, vZ);
}
void UTIL_forcePrintVector(char* printLabel, const Vector& theVector) {
	easyForcePrint("%s: (%.2f, %.2f, %.2f) ", printLabel, theVector.x, theVector.y, theVector.z);
}
void UTIL_forcePrintVector(char* printLabel, const float vX, const float vY, const float vZ) {
	easyForcePrint("%s: (%.2f, %.2f, %.2f) ", printLabel, vX, vY, vZ);
}

//#endif //CLIENT_DLL check




#ifdef CLIENT_DLL

#else

void UTIL_printLineVectorClient(edict_t* pEntity, const Vector& theVector) {
	easyPrintLine("vector: (%.2f, %.2f, %.2f)", theVector.x, theVector.y, theVector.z);
}
void UTIL_printLineVectorClient(edict_t* pEntity, const float vX, const float vY, const float vZ) {
	easyPrintLine("vector: (%.2f, %.2f, %.2f)", vX, vY, vZ);
}
void UTIL_printLineVectorClient(edict_t* pEntity, char* printLabel, const Vector& theVector) {
	easyPrintLine("%s: (%.2f, %.2f, %.2f)", printLabel, theVector.x, theVector.y, theVector.z);
}
void UTIL_printLineVectorClient(edict_t* pEntity, char* printLabel, const float vX, const float vY, const float vZ) {
	easyPrintLine("%s: (%.2f, %.2f, %.2f)", printLabel, vX, vY, vZ);
}
void UTIL_printVectorClient(edict_t* pEntity, const Vector& theVector) {
	easyPrint("(%.2f, %.2f, %.2f) ", theVector.x, theVector.y, theVector.z);
}
void UTIL_printVectorClient(edict_t* pEntity, const float vX, const float vY, const float vZ) {
	easyPrint("(%.2f, %.2f, %.2f) ", vX, vY, vZ);
}
void UTIL_printVectorClient(edict_t* pEntity, char* printLabel, const Vector& theVector) {
	easyPrint("%s: (%.2f, %.2f, %.2f) ", printLabel, theVector.x, theVector.y, theVector.z);
}
void UTIL_printVectorClient(edict_t* pEntity, char* printLabel, const float vX, const float vY, const float vZ) {
	easyPrint("%s: (%.2f, %.2f, %.2f) ", printLabel, vX, vY, vZ);
}


void UTIL_forcePrintLineVectorClient(edict_t* pEntity, const Vector& theVector) {
	easyForcePrintLine("vector: (%.2f, %.2f, %.2f)", theVector.x, theVector.y, theVector.z);
}
void UTIL_forcePrintLineVectorClient(edict_t* pEntity, const float vX, const float vY, const float vZ) {
	easyForcePrintLine("vector: (%.2f, %.2f, %.2f)", vX, vY, vZ);
}
void UTIL_forcePrintLineVectorClient(edict_t* pEntity, char* printLabel, const Vector& theVector) {
	easyForcePrintLine("%s: (%.2f, %.2f, %.2f)", printLabel, theVector.x, theVector.y, theVector.z);
}
void UTIL_forcePrintLineVectorClient(edict_t* pEntity, char* printLabel, const float vX, const float vY, const float vZ) {
	easyForcePrintLine("%s: (%.2f, %.2f, %.2f)", printLabel, vX, vY, vZ);
}
void UTIL_forcePrintVectorClient(edict_t* pEntity, const Vector& theVector) {
	easyForcePrint("(%.2f, %.2f, %.2f) ", theVector.x, theVector.y, theVector.z);
}
void UTIL_forcePrintVectorClient(edict_t* pEntity, const float vX, const float vY, const float vZ) {
	easyForcePrint("(%.2f, %.2f, %.2f) ", vX, vY, vZ);
}
void UTIL_forcePrintVectorClient(edict_t* pEntity, char* printLabel, const Vector& theVector) {
	easyForcePrint("%s: (%.2f, %.2f, %.2f) ", printLabel, theVector.x, theVector.y, theVector.z);
}
void UTIL_forcePrintVectorClient(edict_t* pEntity, char* printLabel, const float vX, const float vY, const float vZ) {
	easyForcePrint("%s: (%.2f, %.2f, %.2f) ", printLabel, vX, vY, vZ);
}
#endif



//Don't do anything! You cheap bastard.
void easyPrintLineDummy(char *szFmt, ...){
	//PRINT_LINE_VA
}







// Console command methods

#ifdef CLIENT_DLL

//MODDD - new console printout method.

//Also, NOTICE:::
//cl_util.h "prototypes" most methods in here so that they can be accessed mostly anywhere else in the client.
//MODDD TODO - Should they be moved to util_printout.h which cl_util.h could then call??


//necessary for virtual (continuous, unknown type / number of) arguments.
//#include "stdarg.h"  ???

/*
inline void UTIL_easyPrint(edict_t* pEdict, char *szFmt, ... ){
	va_list argptr;
	va_start(argptr, szFmt);
	gEngfuncs.Con_Printf("CONSTRUCTA\n");
	//g_engfuncs.pfnClientPrintf(pEdict, print_console, UTIL_VarArgsVA(szFmt, argptr ));
	va_end(argptr);
}

inline void UTIL_easyPrintLine(edict_t* pEdict, char *szFmt, ... ){
	va_list argptr;
	va_start(argptr, szFmt);
	gEngfuncs.Con_Printf("CONSTRUCTA\n");
	//g_engfuncs.pfnClientPrintf(pEdict, print_console, UTIL_VarArgs( "%s\n", UTIL_VarArgsVA(szFmt, argptr ) ) );
	va_end(argptr);
}
*/


// call gEngfuncs.pfnClientCmd and handle the fancy parameter stuff for me.
// Sends a message from the client to the server for the current player as though the user typed something
// into console (processed in dlls/client.cpp, ClientCommand method)
// Easy way to tell the serverside version of the player of any changes to client-specific settings that 
// are still important for the server (cl_fvox, or fov strangely enough)
void easyClientCommand(char* szFmt, ...){
	va_list argptr; va_start(argptr, szFmt);
	gEngfuncs.pfnClientCmd(UTIL_VarArgsVA(szFmt, argptr));
	va_end(argptr);
}//END OF easyClientCmd

void easyServerCommand(char* szFmt, ...){
	va_list argptr; va_start(argptr, szFmt);
	gEngfuncs.pfnServerCmd(UTIL_VarArgsVA(szFmt, argptr));
	va_end(argptr);
}//END OF easyServerCmd


#else

/*
// reference from engine/eiface.h.  Required for pfnClientPrintf calls.
typedef enum
{
	print_console,
	print_center,
	print_chat,
} PRINT_TYPE;
*/


//#ifdef IS_VS6
// This prints directly to a particular provided client from the server. This is a bit different
// from the two "ClientPrint" methods further below that can search titles.txt for something to
// plug in too.
void easyPrintClient(edict_t* pEdict, const char* szFmt, ... ){
	if (CVAR_GET_FLOAT("developer") >= 1) {
		PRINT_SERVER_TO_CLIENT_VA_DEVELOPER(pEdict, print_console)
	}
}
void easyForcePrintClient(edict_t* pEdict, const char* szFmt, ... ){
	PRINT_SERVER_TO_CLIENT_VA_DEVELOPER(pEdict, print_console)
	/*
	char* formattedString;
	va_list argptr;
	va_start(argptr, szFmt);
	formattedString = UTIL_VarArgsVA_Custom(szFmt, "SV-CL: ", 7, argptr);
	va_end(argptr);

	MESSAGE_BEGIN(MSG_ONE, gmsgPrintClient, NULL, pEdict);
		WRITE_STRING(formattedString);
	MESSAGE_END();
	*/
}

void easyPrintStarterClient(edict_t* pEdict) {
#if (defined(_DEBUG) || FORCE_PRINTOUT_PREFIX != 0)
	if (CVAR_GET_FLOAT("developer") >= 1) {
		g_engfuncs.pfnClientPrintf(pEdict, print_console, "SV-CL: ");
	}
#endif
}
void easyForcePrintStarterClient(edict_t* pEdict) {
#if (defined(_DEBUG) || FORCE_PRINTOUT_PREFIX != 0)
	g_engfuncs.pfnClientPrintf(pEdict, print_console, "SV-CL: ");
	/*
	MESSAGE_BEGIN(MSG_ONE, gmsgPrintClient, NULL, pEdict);
		WRITE_STRING("SV-CL: ");
	MESSAGE_END();
	*/
#endif
}

void easyPrintLineClient(edict_t* pEdict, const char* szFmt, ... ){
	if (CVAR_GET_FLOAT("developer") >= 1) {
		PRINT_SERVER_TO_CLIENT_LINE_VA_DEVELOPER(pEdict, print_console)
	}
}
void easyForcePrintLineClient(edict_t* pEdict, const char* szFmt, ... ){
	PRINT_SERVER_TO_CLIENT_LINE_VA_DEVELOPER(pEdict, print_console)
	/*
	char* formattedString;
	va_list argptr;
	va_start(argptr, szFmt);
	formattedString = UTIL_VarArgsVA_Custom(szFmt, "SV-CL: ", 7, argptr);
	va_end(argptr);
	strcat(formattedString, "\n");

	MESSAGE_BEGIN(MSG_ONE, gmsgPrintClient, NULL, pEdict);
		WRITE_STRING(formattedString);
	MESSAGE_END();
	*/
}
void easyPrintLineClient(edict_t* pEdict) {
	if (CVAR_GET_FLOAT("developer") >= 1) {
		g_engfuncs.pfnClientPrintf(pEdict, print_console, "\n");
	}
}
void easyForcePrintLineClient(edict_t* pEdict) {
	g_engfuncs.pfnClientPrintf(pEdict, print_console, "\n");
	/*
	MESSAGE_BEGIN(MSG_ONE, gmsgPrintClient, NULL, pEdict);
		WRITE_STRING("\n");
	MESSAGE_END();
	*/
}





// And for going to all client players from the server.
void easyPrintBroadcast(const char* szFmt, ...) {
	if (CVAR_GET_FLOAT("developer") >= 1) {
		char* formattedString;
		va_list argptr;
		va_start(argptr, szFmt);
		formattedString = UTIL_VarArgsVA_Custom(szFmt, "SV-CL: ", 7, argptr);
		va_end(argptr);

		MESSAGE_BEGIN(MSG_ALL, gmsgPrintClient);
			WRITE_STRING(formattedString);
		MESSAGE_END();
	}
}
void easyForcePrintBroadcast(const char* szFmt, ...) {
	char* formattedString;
	va_list argptr;
	va_start(argptr, szFmt);
	formattedString = UTIL_VarArgsVA_Custom(szFmt, "SV-CL: ", 7, argptr);
	va_end(argptr);

	MESSAGE_BEGIN(MSG_ALL, gmsgPrintClient);
		WRITE_STRING(formattedString);
	MESSAGE_END();
}

void easyPrintStarterBroadcast(void) {
#if (defined(_DEBUG) || FORCE_PRINTOUT_PREFIX != 0)
	if (CVAR_GET_FLOAT("developer") >= 1) {
		MESSAGE_BEGIN(MSG_ALL, gmsgPrintClient);
			WRITE_STRING("SV-CL: ");
		MESSAGE_END();
	}
#endif
}
void easyForcePrintStarterBroadcast(void) {
#if (defined(_DEBUG) || FORCE_PRINTOUT_PREFIX != 0)
	MESSAGE_BEGIN(MSG_ALL, gmsgPrintClient);
		WRITE_STRING("SV-CL: ");
	MESSAGE_END();
#endif
}

void easyPrintLineBroadcast(const char* szFmt, ...) {
	if (CVAR_GET_FLOAT("developer") >= 1) {
		char* formattedString;
		va_list argptr;
		va_start(argptr, szFmt);
		formattedString = UTIL_VarArgsVA_Custom(szFmt, "SV-CL: ", 7, argptr);
		va_end(argptr);
		strcat(formattedString, "\n");

		MESSAGE_BEGIN(MSG_ALL, gmsgPrintClient);
			WRITE_STRING(formattedString);
		MESSAGE_END();
	}
}
void easyForcePrintLineBroadcast(const char* szFmt, ...) {
	char* formattedString;
	va_list argptr;
	va_start(argptr, szFmt);
	formattedString = UTIL_VarArgsVA_Custom(szFmt, "SV-CL: ", 7, argptr);
	va_end(argptr);
	strcat(formattedString, "\n");

	MESSAGE_BEGIN(MSG_ALL, gmsgPrintClient);
		WRITE_STRING(formattedString);
	MESSAGE_END();
}
void easyPrintLineBroadcast(void) {
	if (CVAR_GET_FLOAT("developer") >= 1) {
		MESSAGE_BEGIN(MSG_ALL, gmsgPrintClient);
			WRITE_STRING("\n");
		MESSAGE_END();
	}
}
void easyForcePrintLineBroadcast(void) {
	MESSAGE_BEGIN(MSG_ALL, gmsgPrintClient);
		WRITE_STRING("\n");
	MESSAGE_END();
}













// AKA, "CLIENT_COMMAND"
// ALSO, this engine call already supports the "..." ellipses.  Just throwing that out there.
void easyClientCommand(edict_t* pEdict, char* szFmt, ...){
	va_list argptr; va_start(argptr, szFmt);
	g_engfuncs.pfnClientCommand(pEdict, UTIL_VarArgsVA(szFmt, argptr));
	//g_engfuncs.pfnClientCommand(pEdict, szFmt);
	va_end(argptr);
}

//AKA, "SERVER_COMMAND"
void easyServerCommand(char* szFmt, ...){
	va_list argptr; va_start(argptr, szFmt);
	g_engfuncs.pfnServerCommand(UTIL_VarArgsVA(szFmt, argptr));
	va_end(argptr);
}

//#endif //VS6 check








//Note that the below methods can search titles.txt for messages and plug things in after reading them. Best just leave this system be.
// RENAMED.  Was "ClientPrintAll" in the base SDK.
// As all other as-is console commands meant to be called directly don't start with "UTIL_", why should this one?
// Especially from "ClientPrint" not doing so.

// The "ClientPrint" methods work like easyPrintLineClient calls above (they refer to the server's pfnServerPrint
// method that leaves the rest up to the engine instead of interpreting this result in cl_dlls/text_message.cpp).
// Since the "ClientPrint" methods allow for custom interpretation on the client, that's what allows reading from
// titles.txt.
// For anything that doesn't need that feature, may as well use the easyPrintClient ones above, more flexible
// (does not have a limit of 4 paramters after the message string).
// See cl_dlls/text_message.cpp for the MsgFunc_TextMsg

// !!! IMPORTANT.
// If you don't need any text to be interpreted by clientside text files for its own substitutions
// (starts with #), there isn't a need to use the 4 parameters at all.

// They're also a little cumbersome, since the 4 parameters must be strings. Parsing numbers to strings
// can work, but why the extra effort when sprintf ends up doing that sooner or later anyway.
// Just use the usual "UTIL_VarArgs" stuff to put any parameters in the format string and just send
// that by itself.  That's likely what any other printout from server to client are doing in some way
// anyway.
// Example from dlls/multiplayer_gamerules.cpp that proves this point:
//     ClientPrintAll( HUD_PRINTNOTIFY,
//       UTIL_VarArgs(
//         "%s has joined the game\n", 
//         (pl->pev->netname&& STRING(pl->pev->netname)[0] != 0) ? STRING(pl->pev->netname) : "unconnected"
//       )
//     );

// ***AND SO, easyForcePrintClient AND easyPrintBroadcast/Force were born.
// The existing "easyPrintClient" was actually fine, but its engine call depends on developer being 
// non-zero.  So nope, may as well make that one inspired by ClientPrint below too.
// Removing the whole "enableModPrintouts" thing because that's just dumbly redundant with the 
// existing "developer" CVar.

/*
// For the "msg_dest" parameter, ClientPrint methods use one of these choices:
#define HUD_PRINTNOTIFY		1
#define HUD_PRINTCONSOLE	2
#define HUD_PRINTTALK		3
#define HUD_PRINTCENTER		4
*/

void ClientPrintAll( int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4 ){
	MESSAGE_BEGIN( MSG_ALL, gmsgTextMsg );
		WRITE_BYTE( msg_dest );
		WRITE_STRING( msg_name );
		if ( param1 )
			WRITE_STRING( param1 );
		if ( param2 )
			WRITE_STRING( param2 );
		if ( param3 )
			WRITE_STRING( param3 );
		if ( param4 )
			WRITE_STRING( param4 );
	MESSAGE_END();
}

void ClientPrint( entvars_t *pClient, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4 ){
	MESSAGE_BEGIN( MSG_ONE, gmsgTextMsg, NULL, pClient);
		WRITE_BYTE( msg_dest );
		WRITE_STRING( msg_name );
		if ( param1 )
			WRITE_STRING( param1 );
		if ( param2 )
			WRITE_STRING( param2 );
		if ( param3 )
			WRITE_STRING( param3 );
		if ( param4 )
			WRITE_STRING( param4 );
	MESSAGE_END();
}



// And also from dlls/util.cpp because why not.
// Not renaming because I give no shits about this.
void UTIL_SayText(const char* pText, CBaseEntity* pEntity)
{
	if (!pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, pEntity->edict());
	WRITE_BYTE(pEntity->entindex());
	WRITE_STRING(pText);
	MESSAGE_END();
}

void UTIL_SayTextAll(const char* pText, CBaseEntity* pEntity)
{
	MESSAGE_BEGIN(MSG_ALL, gmsgSayText, NULL);
	WRITE_BYTE(pEntity->entindex());
	WRITE_STRING(pText);
	MESSAGE_END();
}






//TEMP!!!   Comment out contents to stop much of the AI spam.
void easyPrintLineGroup1(char* szFmt, ...) {


}


void easyPrintLineGroup2(char* szFmt, ...) {

}

void easyPrintLineGroup3(char* szFmt, ...) {

}


void easyPrintLineGroup4(char* szFmt, ...) {

}

/*
	va_list argptr;
	va_start(argptr, szFmt);
	g_engfuncs.pfnServerPrint( UTIL_VarArgs( "%s\n", UTIL_VarArgsVA(szFmt, argptr ) )  );
	va_end(argptr);
*/


#endif //END OF server details




