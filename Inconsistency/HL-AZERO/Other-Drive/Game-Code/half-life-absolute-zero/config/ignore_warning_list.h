
// File to put a bunch of warning-ignores.
// No sense getting ten thousand warnings on full re-builds, how about they mean something?


#if (_MSC_VER <= 1200)
    #define IS_VS6
#endif




//oh.   uh-oh.
//Warning	C4003	not enough arguments for function - like macro invocation 'EASY_CVAR_HIDDEN_ACCESS_CLIENTONLY_DEBUGONLY'	hl	C : \Users\blue2\Desktop\SM\HLS1 AZ\dlls\client.cpp	4643
//Warning	C4003	not enough arguments for function - like macro invocation 'EASY_CVAR_UPDATE_SERVER_CLIENTONLY_DEBUGONLY'	hl	C : \Users\blue2\Desktop\SM\HLS1 AZ\dlls\util.cpp	5133




// From extdll.h, may as well go everywhere then.
// Pasted a few other places, especially across pm_shared and cl_dlls files, so why not
// apply everywhere.
#pragma warning(disable : 4244)		// int or float down-conversion
#pragma warning(disable : 4305)		// int or float data truncation
#pragma warning(disable : 4201)		// nameless struct/union
#pragma warning(disable : 4514)		// unreferenced inline function removed
#pragma warning(disable : 4100)		// unreferenced formal parameter











//    nope, that's a good one.
//    Warning	C4002	too many arguments for function - like macro invocation
//#pragma warning(disable : 4002)

// nope, that's a good one.
//    not enough arguments for function-like macro invocation 'X'
//#pragma warning(disable : 4003)

//    Keep this one too.
//    Warning	C4005	'ASSERT': macro redefinition
//#pragma warning(disable : 4005)



//////////
#pragma warning(disable : 4065)
#pragma warning(disable : 4091)
#pragma warning(disable : 4101)		// unreferenced local variable

// Keep this one.
//Warning	C4172	returning address of local variable or temporary: charBuffer
//#pragma warning(disable : 4172)

// a new one from utils/common/mathlib.c, a file that isn't compiled?
// eh, why not.
#pragma warning( disable : 4237 )

#pragma warning(disable : 4407)

// nope, a good one.  We want to know about bad formatting symbols.
//#pragma warning(disable : 4477)




// and this one.   Wait.    What is this?
//Warning	C4353	nonstandard extension used : constant 0 as function expression.Use '__noop' function intrinsic instead
//#pragma warning(disable : 4353)


//NOTICE - out of the 4001 - 4999 range?  Then it isn't for VS6.
#ifndef IS_VS6
///////////////////////////////////////////////////////////////


#pragma warning(disable : 6001)     // Using uninitialized memory
                                    // ...would be nice if it were a little smarter.
#pragma warning(disable : 6011)


// HMMmmmm.  Going to say ignore it, but keep it in mind when it could be helpful.
//    String 'X' might not be zero-terminated.
#pragma warning(disable : 6054)



//    "local declaration of 'X' hides previous declaration at line Y of Z.cpp"
//    ...what.  How is that different from 6246.  Regardless, keep this one.
//#pragma warning(disable : 6244)
// Nope!  That's a good one too,
//    'local declaraiton of X hides declaraiton of the same name in outer scope'.
//#pragma warning(disable : 6246)



// And keep that one.
//Warning	C6281	Incorrect order of operations : relational operators have higher precedence than bitwise operators.
//#pragma warning(disable : 6281) 

// Nope!  Keep this one.
//#pragma warning(disable : 6283)     // 'X' is allocated with array new[], but delted with scalar delete.


// Nah, keep it.
//     Redundant code : the left and right sub - expressions are identical
//#pragma warning(disable : 6287)


// it's a good one.  Something about using 'printf' when 'vprintf' was wanted.
//#pragma warning(disable : 6306)

//    incorrect operator: zero-valued flag cannot be tested with bitwise-and. Use an equality test to check for zero-valued flags.
// eh, keep it.
//#pragma warning(disable : 6313)

#pragma warning(disable : 6330)

// nah, this one's ok.
//#pragma warning(disable : 6326)   // Potential comparison of a constant with another constant.

// another good one...?  what the.  Why is this such a different error number from 4477.
//    'Mismatch on sign: 'unsigned int' passed as _Param_(X) when some signed type is required in call to 'fprintf'
//#pragma warning(disable : 6340)

#pragma warning(disable : 6385)
#pragma warning(disable : 6386)
#pragma warning(disable : 6387) // 'X' could be '0': this does not adhere to the specification for the function 'memcpy'
                                // .........deja vu??


// From other VS2019 warnings.  Anything that comes from the as-is SDK may as well be safely ignored.
#pragma warning(disable : 26400)
#pragma warning(disable : 26401)
#pragma warning(disable : 26408)  // avoid malloc() and free(), prefer the nothrow version of new tih delete (r.10).
#pragma warning(disable : 26409)
#pragma warning(disable : 26426)
#pragma warning(disable : 26429) // never tested for nullneess
#pragma warning(disable : 26430) // symbol 'X' is not tested for nullness on all paths
#pragma warning(disable : 26432)
#pragma warning(disable : 26433)

// ...ehhh keep this one.
//#pragma warning(disable : 26434) // Function 'X' hides a non-virtual function.

#pragma warning(disable : 26436)
#pragma warning(disable : 26437) // 'X' could be '0': this does not adhere to the specification for the function 'memcpy'
#pragma warning(disable : 26438)
#pragma warning(disable : 26440) // avoid 'goto'.
#pragma warning(disable : 26446)
//    "Consider using gsl::finally if final action is intended(gsl.util).hl"
//    can happen on 'goto' lines.
//    ????????????????????????????????????????????????????????
#pragma warning(disable : 26448)

//    The function is declared 'noexcept' but calls function 'ClearServerList()' which may throw exceptions
#pragma warning(disable : 26447)


#pragma warning(disable : 26451)
#pragma warning(disable : 26455)

// Nope, keep that!
//    The reference argument 'X' for function 'Y' can be marked as const
#pragma warning(disable : 26460)

#pragma warning(disable : 26461)
#pragma warning(disable : 26462)
#pragma warning(disable : 26466)
// "do not use c-style casts" ???
// this one's so rare for whatever reason that we'll keep it for now.
//#pragma warning(disable : 26475)

#pragma warning(disable : 26477)  // 'use nullptr'?  Yeaa right.
#pragma warning(disable : 26481)
#pragma warning(disable : 26482)
// ?????  Why did this even happen.
//     Value 175 is outside the bounds(0, 0) of variable
//#pragma warning(disable : 26483)
#pragma warning(disable : 26485)
#pragma warning(disable : 26486)
#pragma warning(disable : 26487)

// Do not dereference a potentially null pointer.  Keeping this one.
//#pragma warning(disable : 26488)

#pragma warning(disable : 26489)
#pragma warning(disable : 26492)
#pragma warning(disable : 26493)
#pragma warning(disable : 26494)
#pragma warning(disable : 26495)  // 'uninitialized'.  yea so?  We handle this.
#pragma warning(disable : 26496)
#pragma warning(disable : 26497)
#pragma warning(disable : 26812)
#pragma warning(disable : 26814)
#pragma warning(disable : 28251)

///////////////////////////////////////////////////////////////
#endif //IS_VS6
