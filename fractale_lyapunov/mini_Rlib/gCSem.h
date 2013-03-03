/*****************************************************************************

	Projet	: Droids

	Fichier	:	gCSem.h
	Partie	: Encapsulation

	Auteur	: RM
	Date		: 120297
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_GCSEM_
#define _H_GCSEM_

#include "machine.h"

//---------------------------------------------------------------------------

#ifdef AMIGA
#include "aCSem.h"
#endif

#ifdef BEBOX
#include "bCSem.h"
#endif

#ifdef WINNT4
#include "wCSem.h"
#endif


//---------------------------------------------------------------------------

#endif // of _H_GCSEM_

// eoh
