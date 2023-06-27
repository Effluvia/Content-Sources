
#include "util_version.h"
#include "version.h"
#include "build_settings.h"
#include "external_lib_include.h"



//write the constants from "version.h" to whatever buffer for display.
void writeVersionInfo(char* aryChr, int maxLength) {
	//char aryChr[128];
	aryChr[0] = '\0';   //safety.
	int mode = 0;
#ifdef _DEBUG
	const char* addon = BUILD_INFO_DEBUG;
#else
	const char* addon = BUILD_INFO_RELEASE;
#endif


#ifndef MODINFO_VS
	const char* addon2 = determineVisualStudioVersion();
#else
	const char* addon2 = MODINFO_VS;
#endif

	int readFrom = 0;
	for (int i = 0; i < maxLength - 1; i++) {
		if (mode == 0) {
			if (MODINFO_Version[i] == '\0') {
				//end found!
				mode = 1;
				readFrom = 0;

				aryChr[i] = '_';
				//i -= 1;   //can overwrite this empty place.

			}
			else {
				aryChr[i] = MODINFO_Version[i];
			}
		}
		else if (mode == 1) {
			//begin writing the extra.

			if (addon2[readFrom] == '\0') {
				//done!
				//aryChr[i] = '\0';
				//break;
				readFrom = 0;
				mode = 2;

				aryChr[i] = '_';
				//i -= 1;

			}
			else {
				aryChr[i] = addon2[readFrom];
				readFrom++;
			}
		}
		else if (mode == 2) {
			//begin writing the extra next.

			if (addon[readFrom] == '\0') {
				//done!
				aryChr[i] = '\0';
				break;
			}
			else {
				aryChr[i] = addon[readFrom];
				readFrom++;
			}
		}

	}//END OF for loop
	aryChr[maxLength - 1] = '\0';
}

void writeDateInfo(char* aryChr, int maxLength) {
	//yay.
	aryChr[0] = '\0';   //safety.


#ifndef MODINFO_Date
	char aryChrDATE[128];
	const char* toWrite = aryChrDATE;
	//const char* toWrite = getDate();
	getDate(aryChrDATE);
#else
	//const char* toWrite = MODINFO_Date;
	// !!! Not so fast!  Want a filler 0 digit for the 'day' portion on days under 10, not
	// the space.  Parse it.
	char aryChrDATE[128];
	const char* toWrite = aryChrDATE;
	// Test?
	//getDateALT(aryChrDATE, "Ass 25 1999");
	getDateALT(aryChrDATE, MODINFO_Date);

#endif

	int mode = 0;

	for (int i = 0; i < maxLength - 1; i++) {
		if (toWrite[i] == '\0') {
			//done!
			aryChr[i] = '\0';
			break;
		}
		else {
			aryChr[i] = toWrite[i];
		}
	}//END OF for loop
	aryChr[maxLength - 1] = '\0';
}




//THANK YOU,
//http://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
#include <ctime>
void getDate(char* aryChr) {
	time_t currentTime = time(0);
	struct tm* currentTimeData = localtime(&currentTime);

	const char* monthString;

	switch(currentTimeData->tm_mon){
	case  0:monthString = "Jan"; break;
	case  1:monthString = "Feb"; break;
	case  2:monthString = "Mar"; break;
	case  3:monthString = "Apr"; break;
	case  4:monthString = "May"; break;
	case  5:monthString = "Jun"; break;
	case  6:monthString = "Jul"; break;
	case  7:monthString = "Aug"; break;
	case  8:monthString = "Sep"; break;
	case  9:monthString = "Oct"; break;
	case 10:monthString = "Nov"; break;
	case 11:monthString = "Dec"; break;
	default:monthString = "???"; break; // whut
	}

	//char  aryChr[128];
	//sprintf(aryChr, "M:%d D:%d Y:%d", currentTimeData->tm_mon + 1, currentTimeData->tm_mday, currentTimeData->tm_year + 1900);
	sprintf(aryChr, "%s %02d %d", monthString, currentTimeData->tm_mday, currentTimeData->tm_year + 1900);
	//aryChr[127] = '\0';

	//return aryChr;
}


// Given the provided build date (aryDateSrc), parse it and get the days with a 0 filler digit (for days under 10).
// The date comes with filler spaces used instead.
void getDateALT(char* aryChr, char* aryDateSrc) {
	time_t currentTime = time(0);
	struct tm* currentTimeData = localtime(&currentTime);

	const char* monthString = &aryDateSrc[0];
	const char* dayString = &aryDateSrc[4];
	const char* yearString = &aryDateSrc[7];

	// Mmm dd yyyy
	strncpy(&aryChr[0], &monthString[0], 3);
	aryChr[3] = ' ';
	
	// INTERVENTION
	//strncpy(&aryChr[4], dayString, 2);
	if(dayString[0] >= '0' && dayString[0] <= '9' ){
		// numeric? ok, take it.
		aryChr[4] = dayString[0];
	}else{
		// Anything else (space)?  Filler 0
		aryChr[4] = '0';
	}
	// no interpretation needed for the one's place
	aryChr[5] = dayString[1];

	// Does the year end in a null-terminating character?  No idea
	aryChr[6] = ' ';
	strncpy(&aryChr[7], &yearString[0], 4);

}//getDateALT




//THANK YOU,
//http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008
//UPDATE:
//https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019

const char* determineVisualStudioVersion(void) {

#ifdef _MSC_VER
#if _MSC_VER == 1100
	return "vs5.0";
#elif _MSC_VER == 1200
	return "vs6.0";
#elif _MSC_VER == 1300
	return "vs7.0";
#elif _MSC_VER == 1310
	return "vs7.1";
#elif _MSC_VER == 1400
	return "vs8.0";
#elif _MSC_VER == 1500
	return "vs9.0";
#elif _MSC_VER == 1600
	return "vs10.0";
#elif _MSC_VER == 1700
	return "vs11.0";
#elif _MSC_VER == 1800
	return "vs12.0";
#elif _MSC_VER == 1900
	return "vs14.0";
#elif _MSC_VER == 1910
	return "vs15.0";
#elif _MSC_VER == 1911
	return "vs15.3";
#elif _MSC_VER == 1912
	return "vs15.5";
#elif _MSC_VER == 1913
	return "vs15.6";
#elif _MSC_VER == 1914
	return "vs15.7";
#elif _MSC_VER == 1915
	return "vs15.8";
#elif _MSC_VER == 1916
	return "vs15.9";
#elif _MSC_VER == 1920
	return "vs16.0"
#elif _MSC_VER == 1921
	return "vs16.1";
#elif _MSC_VER == 1922
	return "vs16.2";
#elif _MSC_VER == 1923
	return "vs16.3";
#elif _MSC_VER == 1924
	return "vs16.4";
#elif _MSC_VER == 1925
	return "vs16.5";
#elif _MSC_VER == 1926
	return "vs16.6";

// ***Add more versions as needed.
//#elif _MSC_VER == ####
//	return  "vs##.#"

#else
	// Just an unknown VS version in particular?
	return "UNKNOWN-MSC_VER_" + _MSC_VER;
#endif
#else
	//_MSC_VER not defined?

#ifdef _WIN32
	// ok, we're not windows.
	return "UNKNOWN-NOT_WINDOWS";
#else
	// windows, but still no MSC_VER ?
	// What environment is this?
	return "UNKNOWN-NO_MSC_VER";
#endif
#endif

}//END OF determineVisualStudioVersion

