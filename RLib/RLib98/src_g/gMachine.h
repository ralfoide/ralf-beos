/*****************************************************************************

	Projet	: Droids

	Fichier	: gMachine.h
	Partie	: Defs

	Auteur	: RM
	Date		: 
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_GMACHINE_
#define _H_GMACHINE_

// includes OS-independants


//---------------------------------------------------------------------------
// declaration du type de machine/compilo

#if __dest_os == __be_os

	#undef  RLIB_AMIGA
	#undef  RLIB_WIN32
	#define RLIB_BEOS

#elif defined(WIN32) && defined(_WINDOWS)

	#undef  RLIB_AMIGA
	#define RLIB_WIN32
	#undef  RLIB_BEOS

#else

	#error Your compiler or your machine is not supported by RLib

#endif


//---------------------------------------------------------------------------
// inclusion de fichiers specifiques

#ifdef RLIB_AMIGA
#include "amiga.h"
#endif

#ifdef RLIB_BEOS
#include "bTypeDefs.h"
#endif

#ifdef RLIB_WIN32
#include "win_headerlist.h"
#include "winnt4.h"
#endif

//---------------------------------------------------------------------------
// inclusion de fichiers generiques

#include "RInterface.h"

//---------------------------------------------------------------------------

#endif // of _H_GMACHINE_

// eoh
