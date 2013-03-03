/******************
*	RServerSocket.h	*
*******************
*	Cross		*
************/

#ifdef RLIB_AMIGA
#include "aRServerSocket.h"
#endif

#ifdef RLIB_BEOS
#include "bRServerSocket.h"
#endif

#ifdef RLIB_WIN32
#include "wRServerSocket.h"
#endif

/* eof */