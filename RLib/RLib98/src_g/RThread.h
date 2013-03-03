/***************
*	RThread.h	*
***************/

#ifdef RLIB_AMIGA
#include "aRThread.h"
#endif

#ifdef RLIB_BEOS
#include "bRThread.h"
#endif

#ifdef RLIB_WIN32
#include "wRThread.h"
#endif

/* eof */