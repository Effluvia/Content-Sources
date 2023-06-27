
//MODDD - NEW FILE.
// Create "easyPrint" methods for pm_shared.c (or the C language in general).
// Obviously this would have conflicts with util_printout.h/.cpp, not that this
// should even come up (that's C++).


#ifndef PM_PRINTOUT_H
#define PM_PRINTOUT_H


// Redirect any "easyPrint" variants to the easyForcePrint ones.
// We don't have restrictions for forbidding some printouts as we
// can't check for any CVars here.
#define easyPrint easyForcePrint
#define easyPrintStarter easyForcePrintStarter
#define easyPrintLine easyForcePrintLine


extern void easyForcePrint(char* szFmt, ...);
extern void easyForcePrintStarter(void);
extern void easyForcePrintLine(char* szFmt, ...);


#endif //PM_PRINTOUT_H