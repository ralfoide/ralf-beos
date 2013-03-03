/*****************************************************************************

	Projet	: RLib

	Fichier	: RRLibApp.h
	Partie	: Main app class

	Auteur	: RM
	Date		: 160598
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RRLIBAPP_
#define _H_RRLIBAPP_

#include "gMachine.h"
#include "gTypeDefs.h"
#include "RPath.h"


//---------------------------------------------------------------------------
// the generic part of the RApp is called RLibApp and inherited in bRApp.h

//************
class RRLibApp
//************
{
public:
	RRLibApp(void)						{ }
	virtual ~RRLibApp(void)		{ }

	// --- virtual callbacks ---

	virtual void aboutRequested(void) = 0;
	virtual void init(void) = 0;
	virtual void terminate(void) = 0;
	virtual void processMessage(ui32 msg, ui32 lparam, vptr data, ui32 *channel=NULL) = 0;
	virtual void openDocument(RPath &path, rbool last=true) = 0;

	// --- methodes interface

	// --- membres publiques

private:

}; // end of class defs for RRLibApp



//---------------------------------------------------------------------------

#endif // of _H_RRLIBAPP_

// eoh
