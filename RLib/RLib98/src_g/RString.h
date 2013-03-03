/*****************************************************************************

	Projet	: Pegase

	Fichier	: RString.h
	Partie	: Portable Lib

	Auteur	: RM
	Date		: 180298
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RSTRING_
#define _H_RSTRING_

#include "gMachine.h"
#include "gMacros.h"

//------------------------------------------------------------


//***********
class RString
//***********
{
public:
	// will move to portable lib ('C Portable String')

	RString(void) { mString = new char[1]; M_ASSERT_PTR(mString); mString[0] = '\0'; }
	RString(const RString& s) { mString = NULL; use(s.mString); }
	RString(const sptr s) { mString = NULL; use((sptr)s); }
	const RString& operator=(sptr s) { use(s); return (*this); }

	virtual ~RString(void) { free(); }

	ui32	strlen(void) 		{ return (mString ? ::strlen(mString) : 0); }
	ui32	len(void)				{ return strlen(); }

	sptr data(void)				{ return mString; }
	operator sptr() const { return mString; }  // as a C string

protected:
	void	free(void) { if (mString) delete [] mString; mString = NULL; }
	void	use(sptr s) { free(); if (s) { mString = new char[::strlen(s)+1]; M_ASSERT_PTR(mString); strcpy(mString, s); } }

	sptr	mString;
};


//------------------------------------------------------------

#endif // of _H_RSTRING_

// eof
