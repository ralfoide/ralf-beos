/*****************************************************************************

	Projet	: Droids

	Fichier	:	gCThread.h
	Partie	: Encapsulation

	Auteur	: RM
	Date		: 120297
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_GCTHREAD_
#define _H_GCTHREAD_

#include "machine.h"

//---------------------------------------------------------------------------
// definition des priorite de threads

#define THREAD_PRI_IDLE			0
#define THREAD_PRI_NORMAL		1
#define THREAD_PRI_HIGH			2
#define THREAD_PRI_REALTIME	3

//---------------------------------------------------------------------------

#ifdef AMIGA
#include "aCThread.h"
#endif

#ifdef BEBOX
#include "bCThread.h"
#endif

#ifdef WINNT4
#include "wCThread.h"
#endif


//---------------------------------------------------------------------------

#endif // of _H_GCTHREAD_

// eoh
