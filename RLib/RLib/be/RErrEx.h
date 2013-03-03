/*****************************************************************************

	Projet	: RLib

	Fichier	: RErrEx.h
	Partie	: Errors

	Auteur	: RM
	Date	: 250798 (Win32)
	Date	: 150298
	Format	: tabs==4

	Error exception class.

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_RERREX_
#define _H_RERREX_


//--------------------------------------------------------

/** Error Exception class.
	Defines the kind of exception that are typically thrown and
	catched by RLib. An error is defined by either a numeric
	constant in the range kErrXXX or by its description string.
	This class is not designed to be derived.
*/


//**********
class RErrEx
//**********
{
public:

		/** Constructor defining an error from a kErrXXX constant.
			@param what	the error code, a constant from the enum kErrXXX
		*/

		RErrEx(si32 what)				{ mWhat = what; mStr = NULL; }

		/** Constructor defining an error by its description string.
			@param str	the description string
		*/

		RErrEx(sptr str)				{ mWhat = 0;    mStr = str;  }

		/** Constructor defining an error by both a constant and a
			description string
			@param what	the error code, a constant from the enum kErrXXX
			@param str	the description string
		*/

		RErrEx(si32 what, sptr  str)	{ mWhat = what; mStr = str;  }

		/** Constructor defining an error by its description and the
			code source (file & line number). Typically used by assertions.
			@param str	the description string, less than 1023 chars
			@param file	the filename, typically __FILE__
			@param line	the line number, typically __LINE__
		*/

		RErrEx(sptr str, sptr file, si32 line);

		/**	The destructor of RErrEx does nothing. It it not virtual
			since this class is not designed to be derived.
		*/

		~RErrEx(void)				{ }

		// --- utilities ---

		/// Get the what field of this error.

		si32	what(void) { return mWhat; }

		/// Get the string description of this error, if any.

		sptr	str (void) { return mStr;  }

//---
protected:

		si32 mWhat;
		sptr mStr;

		// A static string storage, used when building the
		// error description string with the file name and
		// line number.

static	char mStrPool[1024];

};



//--------------------------------------------------------

#endif // of _H_RERREX_
#endif // of RLIB_BEOS


/****************************************************************

	$Log: RErrEx.h,v $
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoh
