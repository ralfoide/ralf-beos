/*****************************************************************************

	Projet	: Droids

	Fichier	:	gDebugPrintf.h
	Partie	: Defs

	Auteur	: RM
	Date		: 160497
	Format	: tabs==2

	Stuff here may be disabled by DEBUG in "defines.h"

*****************************************************************************/

#ifndef _H_GDEBUGPRINTF_
#define _H_GDEBUGPRINTF_

#include "gMachine.h"
#include <stdarg.h>

//---------------------------------------------------------------------------

#ifdef RLIB_AMIGA
#include "aDebugPrintf.h"
#endif

#ifdef RLIB_BEOS
#include "bDebugPrintf.h"
#endif

#ifdef RLIB_WIN32
#include "wDebugPrintf.h"
#endif


//---------------------------------------------------------------------------

#endif // of _H_GDEBUGPRINTF_

// eoh
