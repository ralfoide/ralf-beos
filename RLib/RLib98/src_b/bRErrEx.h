/*****************************************************************************

	Projet	: Portable Lib

	Fichier:	RErrEx.h
	Partie	: Errors

	Auteur	: RM
	Date		: 150298
	Format	: tabs==2

	Error exception class.

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_RERREX_
#define _H_RERREX_

#include <SupportDefs.h>		// for status_t

//--------------------------------------------------------


//**********
class RErrEx
//**********
{
public:
	RErrEx(status_t what)								{ mWhat = what; mStr = NULL; }
	RErrEx(char *   str)								{ mWhat = 0;    mStr = str;  }
	RErrEx(status_t what, char *   str)	{ mWhat = what; mStr = str;  }
	RErrEx(char *str, char *file, long line);
  virtual ~RErrEx(void)			{ }

	status_t	what(void)		{ return mWhat; }
	char		* str(void)			{ return mStr;  }

//---
protected:

	status_t mWhat;
	char		 *mStr;

	static char mStrPoll[1024];
};



//--------------------------------------------------------

#endif // of _H_RERREX_

#endif // of RLIB_BEOS

// eoh
