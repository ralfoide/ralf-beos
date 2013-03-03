/*****************************************************************************

	Projet	: Portable Lib

	Fichier:	RErrEx.cpp
	Partie	: Errors

	Auteur	: RM
	Date		: 150298
	Format	: tabs==2

	Error exception class.

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "gErrors.h"

//--------------------------------------------------------

char RErrEx::mStrPoll[1024] = "";

//--------------------------------------------------------


//**********************************************
RErrEx::RErrEx(char *str, char *file, long line)
//**********************************************
{
	sprintf(mStrPoll, "PTR NULL !\nProbable malloc error\n"
										"File %s, Line %ld\nOffending : '%s'", file, line, str);
	mStr = mStrPoll;
	mWhat = kErrAssertPtr;
} // end of constructor for RErrEx



//--------------------------------------------------------

#endif // of RLIB_BEOS

// eoh
