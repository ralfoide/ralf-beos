/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RHandleLock.h
	Partie	: RLib MacOS basics

	Auteur	: RM
	Date		: 073099
	Format	: tabs==2

	Implementation of the RLock class to lock/unlock memory Handles.
	This implementation is valid only for the MacOS version of RLib,
	for other platforms it merely equals RLock.

	IMPORTANT ! Semantic & usage note :
	- though they derive from the same base classe, RHandleLock and
	  RSemLock are two different classes.
	- RSemLock locks something in order to prevent multiple shared access
	- RHandleLock locks some memory in order to prevent it from moving or
	  being swapped out during access time.

*****************************************************************************/

#ifndef _H_RHANDLELOCK_
#define _H_RHANDLELOCK_

#include "gMachine.h"
#include "RLock.h"

//---------------------------------------------------------------------------

#ifdef RLIB_MACOS

#include <MacTypes.h>

//******************************
class RHandleLock : public RLock
//******************************
{
public:
								RHandleLock(Handle h = nil)	{ mHandle = h ;}
	virtual				~RHandleLock(void)					{ }

					void	setHandle(Handle h) 				{ M_ASSERT(!isLocked()); mHandle = h; }
				Handle	handle(void) const					{ return mHandle; }
								operator Handle(void)	const	{ return mHandle; }

	virtual	rbool	protect(void)		{ M_ASSERT(mHandle); ::HLock(mHandle);   return (::MemError() == noErr); }
	virtual void	unprotect(void)	{ M_ASSERT(mHandle); ::HUnlock(mHandle); M_ASSERT_NOERR(::MemError());   }

protected:
	Handle				mHandle;

}; // end of class defs for RHandleLock

#else // of RLIB_MACOS

//******************************
class RHandleLock : public RLock
//******************************
{
public:
								RHandleLock(void* h = nil)	{ mHandle = h ;}
	virtual				~RHandleLock(void)					{ }

					void	setHandle(void* h) 					{ M_ASSERT(!isLocked()); mHandle = h; }
				 void*	handle(void) const					{ return mHandle; }
								operator void*(void)	const	{ return mHandle; }

protected:
	void*					mHandle;

}; // end of class defs for RHandleLock

#endif // of non-RLIB_MACOS

//---------------------------------------------------------------------------

#endif // of _H_RHANDLELOCK_

// eoh
