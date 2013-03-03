/*****************************************************************************

	Projet	: RLib

	Fichier	: RDPrint.h
	Partie	: Error Display

	Auteur	: RM
	Date	: 032700 (Win32)
	Format	: tabs==4

*****************************************************************************/

#ifdef RLIB_WIN32

#ifndef _H_RDEBUGPRINTF_
#define _H_RDEBUGPRINTF_

#include <stdarg.h>

//---------------------------------------------------------------------------
// main definition

#ifndef DEBUG

	/** A macro to output a debug string.
		Output is done on a terminal (BeOS, Unix), in the Trace buffer (Visual C++)
		or the SIOUX console (MacOS).
	**/

	#define DPRINTF(x)	/* x */

#elif defined(TRACE)

	#define DPRINTF(x)	TRACE x

#else

	#define DPRINTF(x)	printf x

#endif


//---------------------------------------------------------------------------

#endif // of _H_RDEBUGPRINTF_
#endif // of RLIB_WIN32


/****************************************************************

	$Log: RDPrintf.h,v $
	Revision 1.2  2000/04/05 06:23:51  Ralf
	Comments
	
	Revision 1.1  2000/04/01 21:49:16  Ralf
	Using DRPINTF instead of DebugPrintfs
	
****************************************************************/

// eoh
