/***************
*	RThreadTranslator.h	*
***************/

#ifdef RLIB_AMIGA
#include "aRThreadTranslator.h"
#endif

#ifdef RLIB_BEOS
#include "bRThreadTranslator.h"
#endif

#ifdef RLIB_WIN32
#include "wRThreadTranslator.h"
#endif

/* eof */