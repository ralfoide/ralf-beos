/*****************************************************************************

	Projet	: Portable Lib

	Fichier:	CErrEx.h
	Partie	: Errors

	Auteur	: RM
	Date		: 150298
	Format	: tabs==2

	Error 'what' definitions.
	Might come a OS-dependant file.

*****************************************************************************/

#ifndef _H_GERRORS_
#define _H_GERRORS_

#include "RErrEx.h"

//--------------------------------------------------------
// Error ranges (for RErrEx.what field)

#define K_ERROR_BASE	0x10000000
#define K_ERROR_MAX   0x20000000


//--------------------------------------------------------
// Errors commonly defined by the Portable Lib

enum
{
	kNoErr = 0,
	kErr   = K_ERROR_BASE+1,			// 1 - generic error (in general, a TBD failure)
	kErrAlloc,										// 2 - memory allocation failure (new)
	kErrAssertPtr,								// 3 - assertion failed
	kErrClassMismatch,						// 4 - when cast_as RTTI fails
	kErrInit,
	kErrUnsupported,
	kErrTranslationKit,
	kErrNotFound,

	// RThread
	kErrThreadAlreadyLaunched,		// 5 - RThread already launched

	// RSocket
	kErrSocket,										// 6 - invalid socket call (descriptor invalid typically)
	kErrSend,											// 7 - send (write) error  (actually not used)
	kErrReceive,									// 8 - receive (read) error(actually not used)
	kErrWouldBlock,								// 9 - a listen() call on server would block
	kErrAlreadyConnected,					// 10- socket opened twice
	kErrHostNotFound,							// 11- host not found

	// RPane
	kErrAttached,
	kErrNotAttached,
	kErrNoPaneKey,
	kErrNoAssoc,

	// RSoundPlay, RSoundRecord
	kErrNoSoundResource,					// can open device or allocate sound services

	// app-specific (to be re-integrated ?)

	// ...
	kLastErr											// xx- not to be used except for an array size eventually
};

#define kOK 			kNoErr
#define kOk 			kNoErr
#define kSuccess	kNoErr
#define kGood			kNoErr
#define kFine			kNoErr
#define kTopCool	kNoErr

//--------------------------------------------------------

#endif // of _H_GERRORS_

// eoh
