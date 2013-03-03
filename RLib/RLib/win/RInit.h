/*****************************************************************************

	Projet	: RLib

	Fichier	: RInit.h
	Partie	: Main app init

	Auteur	: RM
	Date	: 270798 (Win32)
	Date	: 160598
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_WINIT_
#define _H_WINIT_

#ifdef RLIB_WIN32

#include "RApp.h"


//---------------------------------------------------------------------------
/**	Global initialization routine for the library.
	Must be called by the main() routine with a pointer onto
	the application instance. This pointer can't be null.
*/

void initRLib(RApp *application);


//---------------------------------------------------------------------------

#endif // of RLIB_WIN32
#endif // of _H_WINIT_


/****************************************************************

	$Log: RInit.h,v $
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoh
