/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RLockInterface.h
	Partie	: RLib basics

	Auteur	: RM
	Date		: 073099
	Format	: tabs==2

	
*****************************************************************************/

#ifndef _H_RLOCKINTERFACE_
#define _H_RLOCKINTERFACE_

#include "gMachine.h"

//---------------------------------------------------------------------------

//******************
class RLockInterface
//******************
{
public:
	// public nested calls to lock/unlock
	// lock			: returns false if can not lock.
	// unlock		: unlock the previous lock (always should be paired).
	// isLocked	:	indicates weither the current lock is locked.
	// lockThrow: same as lock() but throws an exception in case of error.
	virtual rbool	lock(void) = 0;
	virtual void	unlock(void) = 0;
	virtual rbool	isLocked(void) const = 0;
	virtual	void	lockThrow(void) = 0;

	// implementation of non-nested calls
	virtual	rbool	protect(void) = 0;
	virtual void	unprotect(void) = 0;
}; // end of class defs for RLockInterface


//---------------------------------------------------------------------------


//***********************************
template< class T > class TStAutoLock
//***********************************
// on-stack lock/unlock of a lockable template class
// destructor is not virtual, class shall not be derived
// no default constructor -- on-stack classes must receive args when created
{
public:
				TStAutoLock(T *lock)	{ mLock = lock; if (mLock) mLock->lockThrow(); }
				TStAutoLock(T &lock)	{ mLock =&lock; if (mLock) mLock->lockThrow(); }
				~TStAutoLock(void)		{ if (mLock) mLock->unlock(); }

	rbool	isLocked(void) const	{ M_ASSERT(mLock); return mLock->IsLocked(); }
	void	unlock(void)					{ M_ASSERT(mLock); mLock->unlock(); mLock = NULL; }
	T *		getLock(void)					{ return mLock; }

protected:
	T *		mLock;

}; // end of class defs for TStAutoLock


//---------------------------------------------------------------------------
// specific AutoLock for RLockInterface objects

typedef TStAutoLock<RLockInterface> RStAutoLock;


//---------------------------------------------------------------------------

#endif // of _H_RLOCKINTERFACE_

// eoh
