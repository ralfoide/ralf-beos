/*****************************************************************************

	Projet	: Portable lib

	Fichier	: RDate.h
	Partie	: Types

	Auteur	: RM
	Date		: 180298
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RDATE_
#define _H_RDATE_

#include "gMachine.h"
#include <time.h>		// <sys/time.h>

//------------------------------------------------------------

#define K_DATE_LEN 16

//------------------------------------------------------------

//*************************************
class RDate : virtual public RInterface
//*************************************
{
public:
	RDate(void) { zero(); }
	RDate(const RDate& s) { use((sptr)s.mDate); }
	RDate(const sptr s)   { use((sptr)s); }
	const RDate& operator=(RDate &d)	{ use((sptr)d.mDate); return (*this); }
	const RDate& operator=(sptr s) 		{ use(s); return (*this); }
	const RDate& operator=(time_t t) 	{ set(t); return (*this); }

	virtual ~RDate(void) { }

	void		zero(void)			{ strcpy(mDate, "00000000000000"); }
	void		set(time_t t)		{ strftime(mDate, K_DATE_LEN-1, "%d%m%Y%H%M%S", localtime(&t)); }
	time_t	get(void) const;

	ui32	strlen(void) 		{ return K_DATE_LEN-1; }
	ui32	len(void)				{ return strlen(); }

	operator sptr() const 	{ return (sptr)mDate; }  // as a C string
	operator time_t() const { return get(); }  // as a time_t

protected:
	void	use(sptr s) { zero(); if (s) { strncpy(mDate, s, K_DATE_LEN-1); } }
	char	mDate[K_DATE_LEN];
	time_t	mT;
};


//------------------------------------------------------------

#endif // of _H_RDATE_

// eof
