/*************************************************************************

	Projet	: Portable Lib

	Fichier	: bRPaneWindow.cpp
	Partie	: RWindow for displaying RPanes

	Auteur	: RM
	Date		: 050798
	Format	: tabs==2

*************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RPaneWindow.h"
//#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"

//------------------------------------------------------------------------
// internal messages


//------------------------------------------------------------------------


//************************************************************************
RPaneWindow::RPaneWindow(RRect frame, sptr title, rbool show)
        		:RWindow(frame, title, show),
        		 mBaseView(NULL)
//************************************************************************
{
	RWindow::init(show);

	// add a default gray background pane view.
	// this is implementation specific
	// (don't set the ptr right now, it's a kuldge so that addPane()
	//  attached the pane to the window, not to the base view itself)
	/* RPaneView *base = new RPaneView(frame, "panewindow_baseview", true);
	addPane(base);
	mBaseView = base; */
} // end of constructor for RPaneWindow


//************************************************************************
void RPaneWindow::deleteWindow(void)
//************************************************************************
{
	// destroy the window and it's associated BeOS thread
	RWindow::deleteWindow();
} // end of deleteWindow for RPaneWindow


//***************************************************************************
RPaneWindow::~RPaneWindow(void)
//***************************************************************************
{
	// never to be used directly under BeOS

} // end of destructor for RPaneWindow


//------------------------------------------------------------------------


//***************************************************************************
void RPaneWindow::addPane(RPane *p)
//***************************************************************************
{
	if (Lock())
	{
		if (mBaseView) mBaseView->addPane(p);
		else addPaneDirect(p);
		Unlock();
	}
} // end of addPane for RPaneWindow


//***************************************************************************
void RPaneWindow::removePane(RPane *p)
//***************************************************************************
{
	if (Lock())
	{
		if (mBaseView) mBaseView->removePane(p);
		else removePaneDirect(p);
		Unlock();
	}
} // end of removePane for RPaneWindow


//------------------------------------------------------------------------


//***************************************************************************
void RPaneWindow::addPaneDirect(RPane *p)
//***************************************************************************
{
	if (debug) printf("RPaneWindow::addPane -- %p\n", p);
	mPaneList.addItem(p);
	p->mParent = NULL;
	p->mWindow = this;

	if (debug) printf("RPaneWindow::addPane\n");

	// theorical Be BView call should go here : BView::AddChild(p);
	if (Lock())
	{
		BView *bview = M_CAST_AS(p, BView);
		if (debug) printf("   RPaneWindow::addPane ---> AddChild -- %p\n", bview);
		if (bview) AddChild(bview);
		Unlock();
	}

	// attachement notification
	si32 n = p->countPane();
	for(n--; n>=0; n--)
	{
		RPane *p2 = p->paneAt(n);
		if (p2) p2->attached(true);
	}
	p->attached(true);
	
} // end of addPaneDirect for RPaneWindow


//***************************************************************************
void RPaneWindow::removePaneDirect(RPane *p)
//***************************************************************************
{
	if (debug) printf("RPaneWindow::removePane -- %p\n", p);
	// theorical Be BView call should go here : BView::RemoveChild(p);
	if (Lock())
	{
		BView *bview = M_CAST_AS(p, BView);
		if (debug) printf("   RPaneWindow::removePane ---> RemoveChild -- %p\n", bview);
		if (bview) RemoveChild(bview);
		Unlock();
	}

	// de-attachement notification
	si32 n = p->countPane();
	p->attached(false);
	for(n--; n>=0; n--)
	{
		RPane *p2 = p->paneAt(n);
		if (p2)
		{
			p2->attached(false);
			//p->removePane(p2); -- ???? HOOK RM 010998
		}
	}

	p->mParent = NULL;
	p->mWindow = NULL;
	mPaneList.removeItem(p);

} // end of removePaneDirect for RPaneWindow


#endif // of RLIB_BEOS
//------------------------------- eoc ------------------------------------

