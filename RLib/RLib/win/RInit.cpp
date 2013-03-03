/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	wInit.cpp
	Partie	: Init for W32

	Auteur	: RM
	Date		: 190998
	Format	: tabs==2

*****************************************************************************/

#include "RLib.h"

#ifdef RLIB_WIN32

//---------------------------------------------------------------------------

#include "RMacros.h"
#include "RApp.h"
#include "RInit.h"
#include "RExterns.h"

//---------------------------------------------------------------------------
// Application instance pointers
// casted to appropriate type for convenience.

RGenApp	*gRGenApp;	// used by Generic sources
RApp	*gRApp;		// used by platform-specific sources

//---------------------------------------------------------------------------


//**********************
void initRLib(RApp *app)
//**********************
{
	M_ASSERT_PTR(app);

	gRApp	= app;
	gRGenApp= (RGenApp *)app;

} // end of initRLib



//---------------------------------------------------------------------------

#endif // RLIB_WIN32


/****************************************************************

	$Log: RInit.cpp,v $
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoc
