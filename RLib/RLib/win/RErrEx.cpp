/*****************************************************************************

	Projet	: RLib

	Fichier	: RErrEx.cpp
	Partie	: Errors

	Auteur	: RM
	Date	: 250798 (Win32)
	Date	: 150298
	Format	: tabs==4

	Error exception class.

*****************************************************************************/

#include "RLib.h"

#ifdef RLIB_WIN32

#include "RErrors.h"

//--------------------------------------------------------

char RErrEx::mStrPool[1024] = "";

//--------------------------------------------------------


//********************************************
RErrEx::RErrEx(sptr str, sptr file, si32 line)
//********************************************
{
	sprintf(mStrPool,
			"Assert Exception !\n"
			"File %s, Line %d\nOffending : '%s'",
			 file, line, str);

	mStr  = mStrPool;
	mWhat = kErrAssertPtr;
} // end of constructor for RErrEx



//--------------------------------------------------------

#endif // of RLIB_WIN32


/****************************************************************

	$Log: RErrEx.cpp,v $
	Revision 1.2  2000/04/05 06:24:53  Ralf
	Using RErrors.h
	
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoh
