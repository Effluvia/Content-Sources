#include "external_lib_include.h"

// what.
struct stat info;



float customRound(float arg) {
	float decimalOnly = arg - ((int)arg);

	if (arg >= 0) {
		//this... might be cheaper than some modulos operation?
		if (decimalOnly < 0.5) {
			//round down.
			return floor(arg);
		}
		else {
			//round up.
			return ceil(arg);
		}
	}
	else {
		if (decimalOnly < -0.5) {
			//round down.
			return floor(arg);
		}
		else {
			//round up.
			return ceil(arg);
		}
	}
}

