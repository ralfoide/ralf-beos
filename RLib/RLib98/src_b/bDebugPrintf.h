/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bDebugPrint.cpp
	Partie	: Error Management

	Auteur	: RM
	Date		: 160497 -- 050398
	Format	: tabs==2

	NOTE : this stuff is ANSI-C compatible but any implementation may
	override this in order to print or send info to specific devices.

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_BDEBUGPRINTF_
#define _H_BDEBUGPRINTF_

#include <stdarg.h>
#include "gMachine.h"
#include "gDebugPrintf.h"

//---------------------------------------------------------------------------
// main definition

#ifndef DEBUG

	#define DPRINTF bDummyDebugPrintf

	// this is NOT optimal.
	inline void bDummyDebugPrintf(const char *format, ...) { }

#else

	#define DPRINTF	bDebugPrintf
	// prototype of debug printf
	void bDebugPrintf(const char *format, ...);

#endif



//---------------------------------------------------------------------------

#endif // of _H_BDEBUGPRINTF_

#endif // of RLIB_BEOS

// eoh
