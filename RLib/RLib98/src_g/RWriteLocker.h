/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RWriteLocker.cpp
	Partie	: Semaphores

	Auteur	: RM
	Date		: 050398
	Format	: tabs==2

	-----------------------------------------------------------------------------
	
	 RWriteLocker.h
	
	 This class implements a lock which permits multiple simultaneous reader
	 threads but guarantees exclusive access to writer threads.
	
	 The algorithm is adapted from "Operating Systems Concepts", second
	 edition, by James L. Perterson and Abraham Silberschatz.
	
	 By JOHN NORSTAD,
	 extracted from BetterEdit, Version 1.0 (for DR6). February 12, 1996.
	
	-----------------------------------------------------------------------------

*****************************************************************************/


#ifndef _H_CWRITELOCKER
#define _H_CWRITELOCKER

#include "gMachine.h"
#include "RSem.h"


//********************************************
class RWriteLocker : virtual public RInterface
//********************************************
{
public:
	RWriteLocker    (void);
	~RWriteLocker   (void);

	void ReadLock   (void);
	void ReadUnlock (void);
	void WriteLock  (void);
	void WriteUnlock(void);

protected:
	RSem entry;
	RSem write;
	si32 numReaders;
};

#endif // of _H_CWRITELOCKER

// eoh
