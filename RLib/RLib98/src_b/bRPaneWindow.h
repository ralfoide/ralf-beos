/*************************************************************************

	Projet	: Portable Lib

	Fichier	: bRPaneWindow.h
	Partie	: RWindow for displaying RPanes

	Auteur	: RM
	Date		: 050798
	Format	: tabs==2

*************************************************************************/

#ifndef _H_BRPANEWINDOW_
#define _H_BRPANEWINDOW_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RWindow.h"
#include "RPane.h"

//------------------------------------------------------------------------
// RPaneWindow class
//
// RPaneWindow implements the RPaneInterface, so that you can add/remove/count
// sub-panes as you would do with a normal pane.

//*******************************************************
class RPaneWindow : public RWindow, public RPaneInterface
//*******************************************************
{
public:
	RPaneWindow(RRect frame, sptr title, rbool show=true);
	virtual void deleteWindow(void);				// use this instead of ~RPaneWindow
	virtual ~RPaneWindow(void);							// never call this directly
	
	// services
	// TBDL...

	// the five next methods implements the RPaneInterface
	virtual void		addPane(RPane *p);
	virtual void		removePane(RPane *p);
	virtual RPane*	paneAt(si32 index)				{ return mPaneList.itemAt(index); }
	virtual si32		countPane(void)						{ return mPaneList.size();				}
	virtual void		attached(rbool _attached)	{ /* makes no sense for a RWindow-like */ }

	// --- variables, methodes protegees ---
protected:

	void		addPaneDirect(RPane *p);
	void		removePaneDirect(RPane *p);

	RPaneList	mPaneList;
	RPaneView	*mBaseView;

	// -- pure internal beos-specific stuff --
private:

}; // end of class RPaneWindow


//------------------------------------------------------------------------

#endif // of RLIB_BEOS
#endif // of _H_BRPANEWINDOW_

//------------------------------- eoh ------------------------------------

