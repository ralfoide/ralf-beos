/**************************************************************************

	Projet	: RLib

	Fichier	: RAtomic.h
	Partie	: atomic value service

	Auteur	: RM
	Date	: 020899
	Format	: tabs==4
	
	Note	: on MacOS/MWCW, requires include of DriverServicesLib
			  (from MacOS Support:Libraries:MacOS Common)

**************************************************************************/

#ifndef _H_RATOMIC_
#define _H_RATOMIC_

#include "RLib.h"

//-------------------------------------------------------------------------

/** An atomic integer counter, that can be incremented and decremented safely.
	The purpose of an atomic integer is for synchronization of a value accessed
	from several threads (BeOS) or an interruption (MacOS).
*/

//***********
class RAtomic
//***********
{
public:

				/** Initializes the RAtomic with a given value.
					@param initial	Optional starting value.
									0 is the default.
				*/

				RAtomic(si32 initial = 0)	{ mValue = initial; }

				/** Safely increments the counter.
					This operation is thread-safe or interrupt-safe.
					@return	The value of the counter before the increment
				*/

inline	si32	Inc(void);

				/** Safely decrements the counter.
					This operation is thread-safe or interrupt-safe.
					@return	The value of the counter before the decrement
				*/

inline	si32	Dec(void);
	
				/** Returns the current counter valuer.
					@return	The current value
				*/

inline	si32	Value(void)		const { return mValue; }

				/** Returns the current counter value.
					@return	The current value
				*/

inline			operator si32()	const { return mValue; }

protected:

volatile si32	mValue;

}; // end of RAtomic


//-------------------------------------------------------------------------
// inline implementation

#if defined(RLIB_MACOS)

	#include <DriverSynchronization.h>

	//*********************
	si32 RAtomic::Inc(void)
	//*********************
	{
		return ::IncrementAtomic((si32 *)&mValue);
	};

	//*********************
	si32 RAtomic::Dec(void)
	//*********************
	{
		return ::DecrementAtomic((si32 *)&mValue);
	};

#elif defined(RLIB_WIN32)

	//*********************
	si32 RAtomic::Inc(void)
	//*********************
	{
		return ::InterlockedIncrement((LONG *)&mValue);
	};

	//*********************
	si32 RAtomic::Dec(void)
	//*********************
	{
		return ::InterlockedDecrement((LONG *)&mValue);
	};

#elif defined(RLIB_BEOS)

	#error please define RAtomic

	//*********************
	si32 RAtomic::Inc(void)
	//*********************
	{
		return atomic_add(&mValue, 1);
	};

	//*********************
	si32 RAtomic::Dec(void)
	//*********************
	{
		return atomic_add(&mValue, -1);
	};

#else

	// generic, lazy & not reliable implementation

	//*********************
	si32 RAtomic::Inc(void)
	//*********************
	{
		return mValue++;
	};

	//*********************
	si32 RAtomic::Dec(void)
	//*********************
	{
		return mValue--;
	};

#endif


#endif // of _H_RATOMIC_



/****************************************************************

	$Log: RAtomic.h,v $
	Revision 1.2  2000/04/01 21:48:42  Ralf
	Implemented for Win32
	
	Revision 1.1  2000/03/13 08:55:13  Ralf
	First revision
	
****************************************************************/

//--------------------------------- eoh ----------------------------------
