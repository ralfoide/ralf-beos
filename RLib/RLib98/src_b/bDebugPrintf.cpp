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

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "bDebugPrintf.h"
#include <stdio.h>

//---------------------------------------------------------------------------
// prototype declaration
#ifndef DEBUG
void bDebugPrintf(const char *format, ...);
#endif


//***************************************************************************
void bDebugPrintf(const char *format, ...)
//***************************************************************************
/*
	This is never called directly. Instead, the DPRINTF macro MUST be used.
	The DPRINTF macro may be disabled by the main DEBUG define, see "defines.h"

	This is pure ANSI-C and should be portable.
	Eventually, vsprintf or vfprintf may be used.
*/
{
va_list	ap;

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
} // end of bDebugPrintf for CClasse


//---------------------------------------------------------------------------

#endif // of RLIB_BEOS

// eoc
