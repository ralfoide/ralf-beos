/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RLock.h
	Partie	: RLib basics

	Auteur	: RM
	Date		: 073099
	Format	: tabs==2

	This class provide a default implementation of the RLockInterface.
	It's a typicall template to build implementation-specific locks.

	You then only need to implement protect() and unprotect() given the
	specific nature of your lock and use the generic lock/unlock/isLocked
	methods.

	Example of implementation-specific details :
	- use of a BLocker object,
	- use of a critical section or semaphore,
	- use of Mac Toolbox HLock/HUnlock routine to deal with Handles.

	The RLock class is not abstract. By itself, even if it locks nothing,
	it asserts that the lock/unlock calls are correctly paired.

*****************************************************************************/

#ifndef _H_RLOCK_
#define _H_RLOCK_

#include "gMachine.h"
#include "gMacros.h"
#include "gErrors.h"
#include "RLockInterface.h"

//---------------------------------------------------------------------------

//*****************************************
class RLock : virtual public RLockInterface
//*****************************************
{
public:
								RLock(void)			{ mLockCount = 0 ;}
	virtual				~RLock(void)		{ M_ASSERT(!isLocked()); }

	virtual rbool	lock(void);
	virtual	void	lockThrow(void);
	virtual void	unlock(void)		{ M_ASSERT(isLocked()); mLockCount--; if (!mLockCount) unprotect(); }
	virtual rbool	isLocked(void) const { return mLockCount > 0; }

	virtual	rbool	protect(void)		{ return true; }
	virtual void	unprotect(void)	{ }


protected:
	si32		mLockCount;
}; // end of class defs for RLock


//---------------------------------------------------------------------------
// inline implementation

//****************************
inline rbool RLock::lock(void)
//****************************
{
	try
	{
		lockThrow();
		return true;
	}
	catch(...)
	{
	}

	return false;
}


//********************************
inline void RLock::lockThrow(void)
//********************************
{
	if (mLockCount || protect())
	{
		mLockCount++;
		return;
	}
	M_THROW(kErrLockFailed);
}


//---------------------------------------------------------------------------

#endif // of _H_RLOCK_

// eoh
