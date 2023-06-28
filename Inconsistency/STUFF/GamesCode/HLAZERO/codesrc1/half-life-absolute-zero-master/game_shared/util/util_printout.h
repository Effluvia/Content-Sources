
//MODDD - new.

// General place for printout methods, as being scattered all over the place can be irritating
// for something so commonly needed.
// Here all in one place to take advantage of as many engine methods as possible.
// There is also "cl_dlls/cl_util.h/ConsolePrint", but that is just a wrapper for
// gEngfuncs.pfnConsolePrint that does not even support extra parameters for filling
// in.  Just use 'easyPrint' / 'easyForcePrint' methods for giving fill-in paramers
// in the  same call, and they work client/serverside (call the right engine method).


#ifndef UTIL_PRINTOUT
#define UTIL_PRINTOUT

#include <stdarg.h>
#include "vector.h"


#ifdef CLIENT_DLL
	#include "wrect.h"
	#include "cl_dll.h"
	#include "cvardef.h"

#else
	#include "cdll_dll.h"
	#include "activity.h"
	#include "enginecallback.h"
	//extern enginefuncs_t g_engfuncs;
	class CBaseEntity;

#endif


//shared.
class Vector2D;
class Vector;



//Fun fact: "extern" is implied for method prototypes like this, whether the keyword itself is used or not.
//The counterpart is "static", which may be the default for variables instead? Don't know.
extern char	*UTIL_VarArgs( char *szFmt, ... );
extern char	*UTIL_VarArgs_ID( char *szFmt, ... );
extern char* UTIL_VarArgsVA( const char *szFmt, va_list argptr );
extern char* UTIL_VarArgsVA_ID( const char *szFmt, va_list argptr );
extern char* UTIL_VarArgsVA_Custom( const char *szFmt, const char* szPrefix, const int iPrefixLength, va_list argptr );



// INTERESTING NOTE.  C++ requires "##" to concatenate literals to macro parameters.
// "in English please?"
// it's why "string"##s1##"otherstring" works but "string"s1"otherstring" doesn't.
// Both approaches work fine in C though, no idea.



// ALSO,  VS6  C++ doesn't appear to even support the "..." macro required for
// doing this all in the preprocessor to work.  OH WELL.
// Not doing this.  Breaks a surprising number of other places for various
// reasons.  Not worth it.

//#ifdef IS_VS6
	extern void easyPrint(char* szFmt, ...);
	extern void easyForcePrint(char* szFmt, ...);
	extern void easyPrintStarter(void);
	extern void easyForcePrintStarter(void);
	extern void easyPrintLine(char* szFmt, ...);
	extern void easyForcePrintLine(char* szFmt, ...);
	extern void easyPrintLine(void);
	extern void easyForcePrintLine(void);
//#else
/*
	// yay, we can do it the easy way.
	#ifdef CLIENT_DLL
		#define easyPrint_starter if(EASY_CVAR_GET(enableModPrintouts)!=0){gEngfuncs.Con_Printf("CL");}
		#define easyPrint(s1, ...) if(EASY_CVAR_GET(enableModPrintouts)!=0){gEngfuncs.Con_Printf(s1, ##__VA_ARGS__);}
		#define easyPrintLine(s1, ...) if(EASY_CVAR_GET(enableModPrintouts)!=0){gEngfuncs.Con_Printf("CL: "##s1##"\n", ##__VA_ARGS__);}
		#define easyForcePrint_starter gEngfuncs.Con_Printf("CL");
		#define easyForcePrint(s1, ...) gEngfuncs.Con_Printf(s1, ##__VA_ARGS__);
		#define easyForcePrintLine(s1, ...) gEngfuncs.Con_Printf("CL: "##s1##"\n", ##__VA_ARGS__);
	#else
		#define easyPrint_starter if(EASY_CVAR_GET(enableModPrintouts)!=0){g_engfuncs.pfnAlertMessage(ALERT_TYPE::at_console, "SV");}
		#define easyPrint(s1, ...) if(EASY_CVAR_GET(enableModPrintouts)!=0){g_engfuncs.pfnAlertMessage(ALERT_TYPE::at_console, s1, ##__VA_ARGS__);}
		#define easyPrintLine(s1, ...) if(EASY_CVAR_GET(enableModPrintouts)!=0){g_engfuncs.pfnAlertMessage(ALERT_TYPE::at_console, "SV: "##s1##"\n", ##__VA_ARGS__);}
		#define easyForcePrint_starter g_engfuncs.pfnAlertMessage(ALERT_TYPE::at_console, "SV");
		#define easyForcePrint(s1, ...) g_engfuncs.pfnAlertMessage(ALERT_TYPE::at_console, s1, ##__VA_ARGS__);
		#define easyForcePrintLine(s1, ...) g_engfuncs.pfnAlertMessage(ALERT_TYPE::at_console, "SV: " ## s1 ## "\n", ##__VA_ARGS__);
	#endif
*/
//#endif



extern void printIntAsBinary(unsigned int arg, unsigned int binaryDigits);
extern void printLineIntAsBinary(unsigned int arg, unsigned int binaryDigits);
#ifdef CLIENT_DLL

#else
extern void printIntAsBinaryClient(edict_t* pEntity, unsigned int arg, unsigned int binaryDigits);
extern void printLineIntAsBinaryClient(edict_t* pEntity, unsigned int arg, unsigned int binaryDigits);
#endif

extern void UTIL_printLineVector(const Vector& theVector);
extern void UTIL_printLineVector(const float vX, const float vY, const float vZ);
extern void UTIL_printLineVector(char* printLabel, const Vector& theVector);
extern void UTIL_printLineVector(char* printLabel, const float vX, const float vY, const float vZ);
extern void UTIL_printVector(const Vector& theVector);
extern void UTIL_printVector(const float vX, const float vY, const float vZ);
extern void UTIL_printVector(char* printLabel, const Vector& theVector);
extern void UTIL_printVector(char* printLabel, const float vX, const float vY, const float vZ);

extern void UTIL_forcePrintLineVector(const Vector& theVector);
extern void UTIL_forcePrintLineVector(const float vX, const float vY, const float vZ);
extern void UTIL_forcePrintLineVector(char* printLabel, const Vector& theVector);
extern void UTIL_forcePrintLineVector(char* printLabel, const float vX, const float vY, const float vZ);
extern void UTIL_forcePrintVector(const Vector& theVector);
extern void UTIL_forcePrintVector(const float vX, const float vY, const float vZ);
extern void UTIL_forcePrintVector(char* printLabel, const Vector& theVector);
extern void UTIL_forcePrintVector(char* printLabel, const float vX, const float vY, const float vZ);


#ifdef CLIENT_DLL

#else
extern void UTIL_printLineVectorClient(edict_t* pEntity, const Vector& theVector);
extern void UTIL_printLineVectorClient(edict_t* pEntity, const float vX, const float vY, const float vZ);
extern void UTIL_printLineVectorClient(edict_t* pEntity, char* printLabel, const Vector& theVector);
extern void UTIL_printLineVectorClient(edict_t* pEntity, char* printLabel, const float vX, const float vY, const float vZ);
extern void UTIL_printVectorClient(edict_t* pEntity, const Vector& theVector);
extern void UTIL_printVectorClient(edict_t* pEntity, const float vX, const float vY, const float vZ);
extern void UTIL_printVectorClient(edict_t* pEntity, char* printLabel, const Vector& theVector);
extern void UTIL_printVectorClient(edict_t* pEntity, char* printLabel, const float vX, const float vY, const float vZ);

extern void UTIL_forcePrintLineVectorClient(edict_t* pEntity, const Vector& theVector);
extern void UTIL_forcePrintLineVectorClient(edict_t* pEntity, const float vX, const float vY, const float vZ);
extern void UTIL_forcePrintLineVectorClient(edict_t* pEntity, char* printLabel, const Vector& theVector);
extern void UTIL_forcePrintLineVectorClient(edict_t* pEntity, char* printLabel, const float vX, const float vY, const float vZ);
extern void UTIL_forcePrintVectorClient(edict_t* pEntity, const Vector& theVector);
extern void UTIL_forcePrintVectorClient(edict_t* pEntity, const float vX, const float vY, const float vZ);
extern void UTIL_forcePrintVectorClient(edict_t* pEntity, char* printLabel, const Vector& theVector);
extern void UTIL_forcePrintVectorClient(edict_t* pEntity, char* printLabel, const float vX, const float vY, const float vZ);
#endif

extern void easyPrintLineDummy(char *szFmt, ...);




#ifdef CLIENT_DLL
	
	// Message from the client to the server.
	extern void easyClientCommand(char* szFmt, ...);
	// Not sure the difference?  Maybe some other server place that doesn't send the current client calling for the message,
	// as this is still called from clientside.  Few examples of the engine call inside this in the as-is script.
	extern void easyServerCommand(char* szFmt, ...);
	

#else
	// Few printout methods exclusive to serverside moved here for sanity.

	// prints a message to each client. Can be filtered by the built-in strings from titles.txt.
	extern void ClientPrintAll( int msg_dest, const char *msg_name, const char *param1 = NULL, const char *param2 = NULL, const char *param3 = NULL, const char *param4 = NULL );
	inline void CenterPrintAll( const char *msg_name, const char *param1 = NULL, const char *param2 = NULL, const char *param3 = NULL, const char *param4 = NULL ) 
	{
		ClientPrintAll( HUD_PRINTCENTER, msg_name, param1, param2, param3, param4 );
	}
	// prints messages through the HUD
	extern void ClientPrint( entvars_t* pClient, int msg_dest, const char *msg_name, const char *param1 = NULL, const char *param2 = NULL, const char *param3 = NULL, const char *param4 = NULL );


	// prints a message to the HUD say (chat)
	extern void UTIL_SayText(const char* pText, CBaseEntity* pEntity);
	extern void UTIL_SayTextAll(const char* pText, CBaseEntity* pEntity);
	


	//#ifdef IS_VS6
		extern void easyPrintClient(edict_t* pEdict, const char* szFmt, ...);
		extern void easyForcePrintClient(edict_t* pEdict, const char* szFmt, ...);
		extern void easyPrintStarterClient(edict_t* pEdict);
		extern void easyForcePrintStarterClient(edict_t* pEdict);
		extern void easyPrintLineClient(edict_t* pEdict, const char* szFmt, ...);
		extern void easyForcePrintLineClient(edict_t* pEdict, const char* szFmt, ...);
		extern void easyPrintLineClient(edict_t* pEdict);
		extern void easyForcePrintLineClient(edict_t* pEdict);

		extern void easyPrintBroadcast(const char* szFmt, ...);
		extern void easyForcePrintBroadcast(const char* szFmt, ...);
		extern void easyPrintStarterBroadcast(void);
		extern void easyForcePrintStarterBroadcast(void);
		extern void easyPrintLineBroadcast(const char* szFmt, ...);
		extern void easyForcePrintLineBroadcast(const char* szFmt, ...);
		extern void easyPrintLineBroadcast(void);
		extern void easyForcePrintLineBroadcast(void);

		extern void easyClientCommand(edict_t* pEdict, char* szFmt, ...);
		extern void easyServerCommand(char* szFmt, ...);

	//#else
		/*
		#define easyPrintClient_starter(arg_destEdict) if(EASY_CVAR_GET(enableModPrintouts)!=0){g_engfuncs.pfnClientPrintf(arg_destEdict, "SV");}
		#define easyPrintClient(arg_destEdict, s1, ...) if(EASY_CVAR_GET(enableModPrintouts)!=0){g_engfuncs.pfnClientPrintf(arg_destEdict, s1, ##__VA_ARGS__);}
		#define easyPrintLineClient(arg_destEdict, s1, ...) if(EASY_CVAR_GET(enableModPrintouts)!=0){g_engfuncs.pfnClientPrintf(arg_destEdict, "SV: "##s1##"\n", ##__VA_ARGS__);}
		#define easyForcePrintClient_starter(arg_destEdict) g_engfuncs.pfnClientPrintf(arg_destEdict, "SV");
		#define easyForcePrintClient(arg_destEdict, s1, ...) g_engfuncs.pfnClientPrintf(arg_destEdict, s1, ##__VA_ARGS__);
		#define easyForcePrintLineClient(arg_destEdict, s1, ...) g_engfuncs.pfnClientPrintf(arg_destEdict, "SV: "##s1##"\n", ##__VA_ARGS__);
		
		#define easyPrintServer_starter(arg_destEdict) if(EASY_CVAR_GET(enableModPrintouts)!=0){g_engfuncs.pfnServerPrint("SV");}
		#define easyPrintServer(arg_destEdict, s1, ...) if(EASY_CVAR_GET(enableModPrintouts)!=0){g_engfuncs.pfnServerPrint(s1, ##__VA_ARGS__);}
		#define easyPrintLineServer(arg_destEdict, s1, ...) if(EASY_CVAR_GET(enableModPrintouts)!=0){g_engfuncs.pfnServerPrint("SV: "##s1##"\n", ##__VA_ARGS__);}
		#define easyForcePrintServer_starter(arg_destEdict) g_engfuncs.pfnServerPrint("SV");
		#define easyForcePrintServer(arg_destEdict, s1, ...) g_engfuncs.pfnServerPrint(s1, ##__VA_ARGS__);
		#define easyForcePrintLineServer(arg_destEdict, s1, ...) g_engfuncs.pfnServerPrint("SV: "##s1##"\n", ##__VA_ARGS__);
		*/
	//#endif


	// Why have separate 'pfnAlertMessage' and 'pfnServerPrint'  engine methods that both
	// go to the server (or first player if making a server in-game, that is, non-dedicated)?
	// AGGG.   Absolutely mind-boggling.
	// Going to just implement a "Broadcast" that does something like ClientPrintAll, since the 
	// four parameters that don't need to be used there (for interpreting by titles.txt and must
	// be strings) are just tedious compared to every other printout method.
	// Say hello to easyPrintBroadcast.


	//TEMP!
	extern void easyPrintLineGroup1(char* format, ...);
	extern void easyPrintLineGroup2(char* format, ...);
	extern void easyPrintLineGroup3(char* format, ...);
	extern void easyPrintLineGroup4(char* format, ...);

#endif //END OF main cl_dlls/Server check






#endif//END OF UTIL_PRINTOUT


