/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRSem.h
	Partie	: Semaphore System Resource

	Auteur	: RM
	Date		: 120297 -- 050398
	Format	: tabs==2

	Remarque : pour liberer un semaphore sans guru-ter sur la miga, il suffit de
	faire le truc neuneu suivant :
		while(!sem.attemp()) sem.release(); sem.release();
	:-) a condition qu'un autre thread ne tente pas de le liberer en meme temps !

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_BRSEM_
#define _H_BRSEM_

#include "gMachine.h"
#include <OS.h>

#define _BRSEM_USE_INLINE_ inline

//---------------------------------------------------------------------------


//************************************
class RSem : virtual public RInterface
//************************************
{
public:
	RSem(void);
	virtual ~RSem(void);

	_BRSEM_USE_INLINE_ void release(void);		// ne pas releaser une fois de trop sur la miga (Guru?)
	_BRSEM_USE_INLINE_ void acquire(void);		// acquisition bloquante
	_BRSEM_USE_INLINE_ rbool attempt(void);		// acquisition non-bloquante. TRUE is acquiert, FALSE si deja pris.
	_BRSEM_USE_INLINE_ rbool acquireTimeout(double microseconds);		// acquisition bloquante avec timeout

//----
protected:

	void init(void);		// init and create, count=1 (1 acquire autorise)

//----
	sem_id	mId;		// id of semaphore

}; // end of class defs for RSem


//---------------------------------------------------------------------------
// inline implementations

#ifdef _BRSEM_USE_INLINE_

//***************************************************************************
_BRSEM_USE_INLINE_ void RSem::release(void)
//***************************************************************************
/*
	Libere le semaphore. Doit theoriquement balancer un appel a acquire()
	ou un appel reussi a attempt().
*/
{
	if (mId != -1) release_sem(mId);
} // end of sleep for RSem


//***************************************************************************
_BRSEM_USE_INLINE_ void RSem::acquire(void)
//***************************************************************************
/*
	Acquisition bloquante du semphore.
*/
{
	if (mId != -1) acquire_sem(mId);
} // end of acquire for RSem


//***************************************************************************
_BRSEM_USE_INLINE_ rbool RSem::attempt(void)
//***************************************************************************
/*
	Acquisition non-bloquante du semaphore.
	Renvoie :
	- TRUE si le semaphore etait libre et a donc ete pris,
	- FALSE si le semahpore etait pris et n'a pu l'etre de nouveau.
*/
{
status_t result;

	result = acquire_sem_etc(mId, 1, B_TIMEOUT, (bigtime_t)0);
	// renvoie B_WOULD_BLOCK ou B_TIME_OUT ou B_BAD_SEM_ID
	return (result == B_NO_ERROR);

} // end of attempt for RSem



//***************************************************************************
_BRSEM_USE_INLINE_ rbool RSem::acquireTimeout(double microseconds)
//***************************************************************************
/*
	Acquisition bloquante du semaphore avec timeout.
	Renvoie :
	- TRUE si le semaphore etait libre et a donc ete pris,
	- FALSE si le semahpore etait pris et n'a pu l'etre de nouveau.
*/
{
status_t result;

	result = acquire_sem_etc(mId, 1, B_TIMEOUT, (bigtime_t)microseconds);
	// renvoie B_WOULD_BLOCK ou B_TIME_OUT ou B_BAD_SEM_ID
	return (result == B_NO_ERROR);

} // end of acquireTimeout for RSem


#endif // of _BRSEM_USE_INLINE_

//---------------------------------------------------------------------------

#endif // of _H_BRSEM_

#endif // of RLIB_BEOS

// eoh
