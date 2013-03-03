/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRSem.cpp
	Partie	: Semaphore System Resource

	Auteur	: RM
	Date		: 120297 -- 050398
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RSem.h"
#include "RErrEx.h"

//---------------------------------------------------------------------------



//***************************************************************************
RSem::RSem(void)
//***************************************************************************
{
	mId = -1;	// valeur d'invalidite
	init();
} // end of constructor for RSem


//***************************************************************************
RSem::~RSem(void)
//***************************************************************************
/*
*/
{
	if (mId != -1) delete_sem(mId);
	mId = -1;

} // end of destructor for RSem


//---------------------------------------------------------------------------


//***************************************************************************
void RSem::init(void)
//***************************************************************************
/*
	init and create a semaphore with an inital count of 1 (only one acquire
	can be performed without blocking).
*/
{
char nom[B_OS_NAME_LENGTH];

	if (1)
	{
		static long index=0;
		++index;
		sprintf(nom, "bRSem_%ld", index);
	}

	mId = create_sem(1, nom);
	if (mId < B_NO_ERROR)
	{
		mId = -1;
		throw RErrEx("RSem init fail");
	}

} // end of init for RSem



//---------------------------------------------------------------------------

#ifndef _BRSEM_USE_INLINE_

//***************************************************************************
void RSem::release(void)
//***************************************************************************
/*
	Libere le semaphore. Doit theoriquement balancer un appel a acquire()
	ou un appel reussi a attempt().
*/
{
	if (mId != -1) release_sem(mId);
} // end of sleep for RSem


//***************************************************************************
void RSem::acquire(void)
//***************************************************************************
/*
	Acquisition bloquante du semphore.
*/
{
	if (mId != -1) acquire_sem(mId);
} // end of acquire for RSem


//***************************************************************************
rbool RSem::attempt(void)
//***************************************************************************
/*
	Acquisition non-bloquante du semaphore.
	Renvoie :
	- TRUE si le semaphore etait libre et a donc ete pris,
	- FALSE si le semahpore etait pris et n'a pu l'etre de nouveau.
*/
{
status_t result;

	result = acquire_sem_etc(mId, 1, B_TIMEOUT, 0.0);
	// renvoie B_WOULD_BLOCK ou B_TIME_OUT ou B_BAD_SEM_ID
	return (result == B_NO_ERROR);

} // end of attempt for RSem


//***************************************************************************
rbool RSem::acquireTimeout(double microseconds)
//***************************************************************************
/*
	Acquisition bloquante du semaphore avec timeout.
	Renvoie :
	- TRUE si le semaphore etait libre et a donc ete pris,
	- FALSE si le semahpore etait pris et n'a pu l'etre de nouveau.
*/
{
status_t result;

	result = acquire_sem_etc(mId, 1, B_TIMEOUT, microseconds);
	// renvoie B_WOULD_BLOCK ou B_TIME_OUT ou B_BAD_SEM_ID
	return (result == B_NO_ERROR);

} // end of acquireTimeout for RSem




#endif // of _BRSEM_USE_INLINE_

//---------------------------------------------------------------------------

#endif // RLIB_BEOS

// eoc
