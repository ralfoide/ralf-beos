/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRPane.cpp
	Partie	: Semaphore System Resource

	Auteur	: RM
	Date		: 270698
	Format	: tabs==2

	Leaf panes that reimplemented existing Be widgets should multiple inherit
	from the base class RPane and from their Be counterpart.

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RPane.h"
#include "RErrEx.h"
#include "gErrors.h"

//---------------------------------------------------------------------------



//***************************************************************************
RPane::RPane(void)
//***************************************************************************
{
	mWindow = NULL;
	mParent = NULL;
} // end of constructor for RPane


//***************************************************************************
RPane::~RPane(void)
//***************************************************************************
{
	// panes are views and must not be deleted when attached
	if (mParent) M_THROW(kErrAttached);
} // end of destructor for RPane



//---------------------------------------------------------------------------


//***************************************************************************
void RPane::addPane(RPane *p)
//***************************************************************************
{

	// if only RPanes-like should be added here
	// RPane *pane = M_CAST_AS(p, RPane);
	// if (!pane) M_THROW(kErrClassMismatch);

	mPaneList.addItem(p);
	p->mParent = this;
	p->mWindow = mWindow;

	// theorical Be BView call should go here : BView::AddChild(p);

	// attachement notification
	si32 n = p->countPane();
	for(n--; n>=0; n--)
	{
		RPane *p2 = p->paneAt(n);
		if (p2) p2->attached(true);
	}
	
} // end of addPane for RPane


//***************************************************************************
void RPane::removePane(RPane *p)
//***************************************************************************
{

	// if only RPanes-like should have been present here...
	// RPane *pane = M_CAST_AS(p, RPane);
	// if (!pane) M_THROW(kErrClassMismatch);

	// theorical Be BView call should go here : BView::RemoveChild(p);

	// de-attachement notification
	si32 n = p->countPane();
	for(n--; n>=0; n--)
	{
		RPane *p2 = p->paneAt(n);
		if (p2)
		{
			p2->attached(false);
			// p->removePane(p2); --- ???? HOOK RM 010998
		}
	}

	p->mParent = NULL;
	p->mWindow = NULL;
	mPaneList.removeItem(p);

} // end of removePane for RPane


//---------------------------------------------------------------------------

#endif // RLIB_BEOS

// eoc
