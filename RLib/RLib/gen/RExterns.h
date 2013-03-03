/*****************************************************************************

	Projet	: RLib

	Fichier	: RExterns.h
	Partie	: Globals defs 

	Auteur	: RM
	Date	: 020598
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_REXTERNS_H_
#define _H_REXTERNS_H_

//---------------------------------------------------------------------------
// Application instance pointers
// casted to appropriate type for convenience.
// Pointers are located in RInit.cpp

#include "RGenApp.h"
#include "RApp.h"

/// Application instance pointer used by generic sources

extern RGenApp *gRGenApp;

/// Application instance pointer used by platform-specific sources

extern RApp		*gRApp;

//---------------------------------------------------------------------------

#endif // _H_REXTERNS_H_


/****************************************************************

	$Log: RExterns.h,v $
	Revision 1.1  2000/03/13 08:55:13  Ralf
	First revision
	
****************************************************************/

// eoh

