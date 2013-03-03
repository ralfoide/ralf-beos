/*****************************************************************************

	Projet	: Droids

	Fichier	:	bCSem.h
	Partie	: Encapsulation

	Auteur	: RM
	Date		: 120297
	Format	: tabs==2

	Remarque : pour liberer un semaphore sans guru-ter sur la miga, il suffit de
	faire le truc neuneu suivant :
		while(!sem.attemp()) sem.release(); sem.release();
	:-) a condition qu'un autre thread ne tente pas de le liberer en meme temps !

*****************************************************************************/

#ifndef _H_BCSEM_
#define _H_BCSEM_

#include "machine.h"


//---------------------------------------------------------------------------


//********
class CSem
//********
{
public:
	CSem(void);
	virtual ~CSem(void);

	BOOL init(void);		// init and create, count=1 (1 acquire autorise)
	void release(void);	// ne pas releaser une fois de trop sur la miga (Guru?)
	void acquire(void);	// acquisition bloquante
	BOOL attempt(void);	// acquisition non-bloquante. TRUE is acquiert, FALSE si deja pris.

//----
protected:

//----
	sem_id	mId;		// id of semaphore

}; // end of class defs for CSem


//---------------------------------------------------------------------------

#endif // of _H_BCSEM_

// eoh
