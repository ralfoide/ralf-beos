/*****************************************************************************

	Projet	: Portable lib

	Fichier	: RDate.h
	Partie	: Types

	Auteur	: RM
	Date	: 180298
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_RDATE_
#define _H_RDATE_

#include <time.h>		// <sys/time.h>

//------------------------------------------------------------

#define K_DATE_LEN 16

//------------------------------------------------------------

/// A base-type representing a date in ASCII format (y2k compliant)

//*********
class RDate
//*********
{
public:
		RDate(void)					{ Zero();				}
		RDate(const RDate& s)		{ use((sptr)s.mDate);	}
		RDate(const sptr s)			{ use((sptr)s);			}

virtual	~RDate(void) { }

inline	const	RDate&	operator=(RDate &d)	{ use((sptr)d.mDate); return (*this);	}
inline	const	RDate&	operator=(sptr s)	{ use(s); return (*this);				}
inline	const	RDate&	operator=(time_t t)	{ Set(t); return (*this);				}

inline	void	Zero(void)			{ strcpy(mDate, "00000000000000");		}
inline	void	Set(time_t t)		{ strftime(mDate, K_DATE_LEN-1, "%d%m%Y%H%M%S", localtime(&t)); }
		time_t	Get(void) const;

inline	ui32	Strlen(void) 		{ return K_DATE_LEN-1;	}
inline	ui32	Len(void)			{ return Strlen();		}

		operator sptr() const		{ return (sptr)mDate;	}	// as a C string
		operator time_t() const		{ return Get();			}	// as a time_t

protected:

		void	use(sptr s)			{ Zero(); if (s) { strncpy(mDate, s, K_DATE_LEN-1); } }
		char	mDate[K_DATE_LEN];
		time_t	mT;
};


//------------------------------------------------------------

#endif // of _H_RDATE_

// eof
