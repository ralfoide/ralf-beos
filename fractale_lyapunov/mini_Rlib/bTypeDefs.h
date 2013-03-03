/*****************************************************************************

	Projet	: Droids

	Fichier	:	bTypeDefs.h
	Partie	: Defs

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_bTypeDefs
#define _H_bTypeDefs


//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <Be.h>

//---------------------------------------------------------------------------
// We compile for BeOS R4, not R3

#undef B_BEOS_VERSION_3
#ifndef B_BEOS_VERSION_4
	#define	B_BEOS_VERSION_4	4
#endif

//---------------------------------------------------------------------------

#define CHAR char

#define BYTE int8
#define UBYTE uint8

#define WORD int16
#define UWORD uint16

#define LONG int32
#define ULONG uint32

#define DOUBLE double

// bool, true and false sont definis dans CW/Be
#ifndef BOOL
#define BOOL bool
#endif
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

#define APTR void *
#define STRPTR CHAR *

// PFV : Pointer on Function returning Void -- definit dans Be/SupportDefs.h
#ifndef PFV
#ifdef B_PFV
#define PFV B_PFV
#else
typedef void (*PFV)(void);
#endif
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

//---------------------------------------------------------------------------

#endif // of _H_bTypeDefs

// eoh
