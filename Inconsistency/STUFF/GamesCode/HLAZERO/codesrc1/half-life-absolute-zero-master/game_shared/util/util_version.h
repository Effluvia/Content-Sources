
// Few helper methods for giving the user the version of the mod.
// Almost always assumes some version of Visual Studio is being used.

#ifndef UTIL_VERSION_H
#define UTIL_VERSION_H

void writeVersionInfo(char* aryChr, int maxLength);
void writeDateInfo(char* aryChr, int maxLength);
//const char* getDate(void);
void getDate(char* aryChr);
void getDateALT(char* aryChr, char* aryDateSrc);
const char* determineVisualStudioVersion(void);

#endif //UTIL_VERSION_H
