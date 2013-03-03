/*****************************************************************************

	Projet	: RLib

	Fichier	: RString.h
	Partie	: Portable Lib

	Auteur	: RM
	Date	: 061499 (MacOS)
	Date	: 300798 (operator+=)
	Date	: 180298
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_RSTRING_
#define _H_RSTRING_

#include "RMacros.h"
#include "RErrors.h"


#ifdef RLIB_MACOS
	#include <LString.h>
#endif


//------------------------------------------------------------


/// An ASCIZ string class

//***********
class RString
//***********
{
public:

			RString(void);
			RString(const RString& s)				{ mString = NULL; use(s.mString); }
			RString(const sptr s)					{ mString = NULL; use((sptr)s);	  }
			RString& operator=(const RString& s)	{ use(s.mString); return (*this); }
			RString& operator=(sptr s)				{ use(s);		  return (*this); }
			RString& operator=(const sptr s)		{ use((sptr)s);	  return (*this); }
		
			~RString(void)	{ _free(); }		// destructor is explicitly NOT virtual here

	ui32	Strlen(void)	{ return mStrLen; }
	ui32	Len(void)		{ return mStrLen; }

	sptr	Data(void)		{ if (mString) return mString; else return (sptr) &mEmpty; }
	operator sptr() const	{ if (mString) return mString; else return (sptr) &mEmpty; }

	void	Cut(ui32 pos)	{ if (mString && pos < mStrLen) Cut(mString + pos); }
	void	Cut(sptr pos);

	void	SetData(sptr ptr, ui32 size);

	#ifdef RLIB_MACOS
		// modifies the content of the string itself
		// also in pascal strings maps '\n' to '\r'.
		void		c2p(void);
		void		p2c(void);

					RString(const LStr255 &str);
		void		LStr255toC(const LStr255 &str);
		operator	StringPtr()	{ if (mString) { c2p(); return (StringPtr)mString; }
								  else return (StringPtr) &mEmpty; }
	#endif

	// extra services

	RString& operator+=(RString &rhs)	{ return concat(rhs.mString);	}
	RString& operator+=(sptr rhs)		{ return concat(rhs);			}
	RString& operator+=(const sptr rhs)	{ return concat((sptr)rhs);		}

	// HACK ! operator[] doesn't return a const reference (which means
	// that affectations are meaningful). OTOH, affecting a zero byte
	// to a char will BREAK the strlen() return value.
	char&	operator[](ui32 index);

protected:

	void	_free(void);
	void	use(sptr s); 
	RString& concat(sptr rhs);

	// inner members

	sptr	mString;
	ui32	mStrLen;

	#ifdef RLIB_MACOS
		bool	mIsPascal;
	#endif

private:

	// returned in case of operator[] wrong index, instead of
	// throwing an exception. This is a kludge.
	static const char mEmpty;
};


//------------------------------------------------------------

#endif // of _H_RSTRING_

/****************************************************************

	$Log: RString.h,v $
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eof
