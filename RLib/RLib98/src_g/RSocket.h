/************
*	RSocket.h	*
*************
*	Cross		*
************/

#ifdef RLIB_AMIGA
#include "aRSocket.h"
#endif

#ifdef RLIB_BEOS
#include "bRSocket.h"
#endif

#ifdef RLIB_WIN32
#include "wRSocket.h"
#endif

/* eof */