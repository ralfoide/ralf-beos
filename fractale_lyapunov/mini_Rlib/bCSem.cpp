/*****************************************************************************

	Projet	: Droids

	Fichier	:	bCSems.cpp
	Partie	: Encapsulation

	Auteur	: RM
	Date		: 120297
	Format	: tabs==2

*****************************************************************************/

#include <stdio.h>
#include "machine.h"
#include "gCSem.h"

//---------------------------------------------------------------------------



//***************************************************************************
CSem::CSem(void)
//***************************************************************************
{
	mId = -1;	// valeur d'invalidite

} // end of constructor for CSem


//***************************************************************************
CSem::~CSem(void)
//***************************************************************************
/*
*/
{
	if (mId != -1) delete_sem(mId);
	mId = -1;

} // end of destructor for CSem


//---------------------------------------------------------------------------


//***************************************************************************
BOOL CSem::init(void)
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
		sprintf(nom, "bCSem_%ld", index);
	}

	mId = create_sem(1, nom);
	if (mId < B_NO_ERROR)
	{
		mId = -1;
		return FALSE;
	}
	else
		return TRUE;
} // end of init for CSem



//---------------------------------------------------------------------------


//***************************************************************************
void CSem::release(void)
//***************************************************************************
/*
	Libere le semaphore. Doit theoriquement balancer un appel a acquire()
	ou un appel reussi a attempt().
*/
{
	if (mId != -1) release_sem(mId);
} // end of sleep for CSem


//***************************************************************************
void CSem::acquire(void)
//***************************************************************************
/*
	Acquisition bloquante du semphore.
*/
{
	if (mId != -1) acquire_sem(mId);
} // end of acquire for CSem


//***************************************************************************
BOOL CSem::attempt(void)
//***************************************************************************
/*
	Acquisition non-bloquante du semaphore.
	Renvoie :
	- TRUE si le semaphore etait libre et a donc ete pris,
	- FALSE si le semahpore etait pris et n'a pu l'etre de nouveau.
*/
{
long result;

	result = acquire_sem_etc(mId, 1, B_TIMEOUT, (bigtime_t)0);
	// renvoie B_WOULD_BLOCK ou B_TIME_OUT ou B_BAD_SEM_ID
	return (result == B_NO_ERROR);

} // end of attempt for CSem



//---------------------------------------------------------------------------

// eoc
