/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RWriteLocker.cpp
	Partie	: Semaphore

	Auteur	: RM
	Date		: 050398
	Format	: tabs==2

*****************************************************************************/

#include "RWriteLocker.h"

//***********************************************************
RWriteLocker::RWriteLocker(void)
					   :numReaders(0)
//***********************************************************
{
}


//***********************************************************
RWriteLocker::~RWriteLocker(void)
//***********************************************************
{
	// RSem are automatically deleted
}


//***********************************************************
void RWriteLocker::ReadLock(void)
//***********************************************************
{
	entry.acquire();
	#ifdef RLIB_BEOS
		if (atomic_add(&numReaders, 1) == 0) write.acquire();
	#elif defined(RLIB_WIN32)
		// HOOK RM : check for lighter atomic add/delete ops under Win32
		mNumLock.acquire();
		si32 old = numReaders;
		numReaders++;
		mNumLock.release();
		if (old == 0) write.acquire();
	#endif
	entry.release();
}


//***********************************************************
void RWriteLocker::ReadUnlock(void)
//***********************************************************
{
	#ifdef RLIB_BEOS
		if (atomic_add(&numReaders, -1) == 1) write.release();
	#elif defined(RLIB_WIN32)
		// HOOK RM : check for lighter atomic add/delete ops under Win32
		mNumLock.acquire();
		si32 old = numReaders;
		numReaders--;
		mNumLock.release();
		if (old == 1) write.release();
	#endif
}


//***********************************************************
void RWriteLocker::WriteLock(void)
//***********************************************************
{
	entry.acquire();
	write.acquire();
	entry.release();
}


//***********************************************************
void RWriteLocker::WriteUnlock(void)
//***********************************************************
{
	write.release();
}

// eoc
