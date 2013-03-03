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
	if (atomic_add(&numReaders, 1) == 0) write.acquire();
	entry.release();
}


//***********************************************************
void RWriteLocker::ReadUnlock(void)
//***********************************************************
{
	if (atomic_add(&numReaders, -1) == 1) write.release();
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
