/*****************************************************************************

	Projet	: Droids

	Fichier	:	machine.h
	Partie	: Defs

	Auteur	: RM
	Date		: 
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_MACHINE_
#define _H_MACHINE_

// includes OS-independants


//---------------------------------------------------------------------------
// declaration du type de machine/compilo

#undef AMIGA
#undef WINNT4
#define BEBOX

//---------------------------------------------------------------------------
// inclusion de fichiers specifiques

#ifdef AMIGA
#include "amiga.h"
#endif

#ifdef BEBOX
#include "bTypeDefs.h"
#endif

#ifdef WINNT4
#include "winnt4.h"
#endif

//---------------------------------------------------------------------------
// inclusion de fichiers generiques

// #include "defines.h"

//---------------------------------------------------------------------------

#endif // of _H_MACHINE_

// eoh
