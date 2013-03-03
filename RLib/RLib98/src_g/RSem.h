/************
*	CSem.h	*
*************
*	Cross		*
************/

#ifdef RLIB_AMIGA
#include "aRSem.h"
#endif

#ifdef RLIB_BEOS
#include "bRSem.h"
#endif

#ifdef RLIB_WIN32
#include "wRSem.h"
#endif

/* eof */