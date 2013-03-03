/*****************************************************************************

	Projet	: RLib

	Fichier	: RLib.h
	Partie	: Defs

	Auteur	: RM
	Date	: ?????? for BeOS and Win32
			  140699 for MacOS
			  010899 added test routines
			  160100 reorganization
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_RLIB_
#define _H_RLIB_

// some custom check

#ifdef _H_RTYPEDEFS_
	#error RTypeDefs.h MUST NOT be included BEFORE RLib.h !!
#endif

// includes OS-independants

// this call makes sure BeOS defs are setup
#include <stdio.h>

//---------------------------------------------------------------------------
// declaration du type de machine/compilo

#undef  RLIB_AMIGA
#undef  RLIB_WIN32
#undef	RLIB_MACOS
#undef	RLIB_BEOS

#if defined(__dest_os)
	#error toto
#endif

#if defined(B_BEOS_VERSION) && B_BEOS_VERSION >= B_BEOS_VERSION_5 && ( defined(__INTEL__) || defined(__POWERPC__) )

	/// Main switch indicating that the BeOS version of RLib is being compiled
	// And we made sure it was for PowerPC or Intel but nothing else.

	#define RLIB_BEOS

#elif defined(WIN32) && defined(_WINDOWS)

	/// Main switch indicating that the Win32 version of RLib is being compiled

	#define RLIB_WIN32

	// decide if you want to use MFC or the simple Win32 API
	// to use MFC, #define the following, else #undef it.

	/// Main switch indicating that RLib for Win32 uses MFC

	#define	RLIB_WIN32_USES_MFC
	//#undef	RLIB_WIN32_USES_MFC

#elif defined(__POWERPC__) && defined(__MWERKS__) && !defined(__BEOS__)

	/// Main switch indicating that the MacOS version of RLib is being compiled

	#define	RLIB_MACOS

#elif 0

	/// Main switch indicating that the AmigaOS version of RLib is being compiled

	#define	RLIB_AMIGA

#endif

// Check that at least one platform be defined
#ifndef RLIB_AMIGA
	#ifndef RLIB_WIN32
		#ifndef	RLIB_MACOS
			#ifndef	RLIB_BEOS
				#error Your compiler or your machine is not supported by RLib
			#endif
		#endif
	#endif
#endif


//---------------------------------------------------------------------------
// some RLib-wide generic defines

/// Switch to enable or disable the test routines for low & basic service classes

#define RLIB_TEST_ROUTINES

//---------------------------------------------------------------------------
// inclusion de fichiers specifiques

#ifdef RLIB_AMIGA
	// add here
#endif

#ifdef RLIB_BEOS
	// add here
#endif

#ifdef RLIB_WIN32
	#include "StdAfx.h"
#endif

#ifdef RLIB_MACOS
	// add here
	#include <PP_Prefix.h>
#endif

//---------------------------------------------------------------------------
// inclusion de fichiers generiques

#include "RTypeDefs.h"
#include "RInterface.h"
#include "RDPrintf.h"

//---------------------------------------------------------------------------

#endif // of _H_RLIB_


/****************************************************************

	$Log: RLib.h,v $
	Revision 1.2  2000/04/01 21:50:57  Ralf
	Using DRPINTF instead of DebugPrintfs
	
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoh
