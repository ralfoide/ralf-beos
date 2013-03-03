/************
*	RPath.h	*
*************
*	Cross		*
************/

#ifdef RLIB_AMIGA
#include "aRPath.h"
#endif

#ifdef RLIB_BEOS
#include "bRPath.h"
#endif

#ifdef RLIB_WIN32
#include "wRPath.h"
#endif

/* eof */