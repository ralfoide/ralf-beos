/*****************************************************************************

	Projet	: RLib

	Fichier	: bRApp.h
	Partie	: Main app class

	Auteur	: RM
	Date		: 160598
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_BRAPP_
#define _H_BRAPP_

#include "gMachine.h"
#include "gTypeDefs.h"
#include "RRLibApp.h"
#include <Application.h>

//---------------------------------------------------------------------------


//***********************************************
class RApp : public BApplication, public RRLibApp
//***********************************************
{
public:
	RApp(sptr signature = "application/x-vnd.ralf-rlibapp")
		: BApplication(signature), RRLibApp() { }
	virtual ~RApp(void)		{ }

	// --- BApplication callbacks ---

	virtual void AboutRequested(void)		{ aboutRequested(); }
	virtual bool QuitRequested(void)		{ terminate(); return true; }
	virtual void MessageReceived(BMessage *message) { }
	virtual	void RefsReceived(BMessage *a_message);

	// --- RRLibApp callbacks ---

	virtual void init(void) = 0;
	virtual void terminate(void) = 0;
	virtual void aboutRequested(void)		{ /* post message to moteur ? */ };

	// important note : the behavior of RRLibapp::processMessage
	// this function is similar to moteurMsg() except that it filter
	// some very app-specific messages. All ignored messages go to the moteur.
	virtual void processMessage(ui32 msg, ui32 lparam, vptr data, ui32 *channel=NULL) { }

	// should be derived by the caller if he wants to be notified that the application
	// was open by double clicking a document or if a file was dropped on the application
	virtual void openDocument(RPath &path, rbool last=true) { }

	// --- methodes interface

	// --- membres publiques

private:

}; // end of class defs for RApp


//---------------------------------------------------------------------------

#endif // of _H_BRAPP_

// eoh
