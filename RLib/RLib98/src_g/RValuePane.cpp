/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RValuePane.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 270698
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#include "RValuePane.h"
#include "RPaneDialog.h"
#include "RErrEx.h"
#include "gErrors.h"

//---------------------------------------------------------------------------


//***************************************************************************
RValuePane::RValuePane(RPaneKey key)
							:RPane()
//***************************************************************************
{
	mKey = key;
	mAssoc = NULL;
} // end of constructor for RValuePane


//***************************************************************************
RValuePane::~RValuePane(void)
//***************************************************************************
{
	removeAssoc();

} // end of destructor for RValuePane


//---------------------------------------------------------------------------



//***************************************************************************
void RValuePane::attached(rbool _attached)
//***************************************************************************
{
RValuePane *p = NULL;

	if (debug) printf("RValuePane::attached -- rbool %d\n", _attached);

	if (mParent) p = M_CAST_AS(mParent, RValuePane);

	if (_attached)
	{
		// set assoc from parent
		// if the parent is not a RValuePane (thus has no assoc),
		// rewind the hierarchy to find the first parent that is
		// a RValuePane or if the window is a RPaneDialog.
		
		RPane *p2 = mParent;
		while (p2 && p2->parent() && !p)
		{
			p2 = p2->parent();
			p = M_CAST_AS(p2, RValuePane);
		}

		// set assoc will register the key/value if any
		if (p) setAssoc(p->getAssoc());
		else if (window())
		{
			if (debug) printf("    RValuePane::attached -- get assoc from window\n");
			RPaneDialog *p3 = M_CAST_AS(window(), RPaneDialog);
			if (p3) setAssoc(p3->getAssoc());
		}
		
	}
	else
	{
		// remove the key from the assoc if any
		if (mAssoc && mKey != K_NO_PANEKEY) mAssoc->del(mKey);

		// then remove the assoc reference that the pane share with
		// its parent
		removeAssoc();
	}

	RPane::attached(_attached);
	
} // end of attached for RValuePane


//---------------------------------------------------------------------------


//***************************************************************************
void RValuePane::removeAssoc(void)
//***************************************************************************
{
RValuePane *p;
si32 n;

	n = countPane();
	for(n--; n>=0; n--)
	{
		RPane *p2 = paneAt(n);
		if (!p2) continue;
		p = M_CAST_AS(p2, RValuePane);
		if (p) p->removeAssoc();
	}

	mAssoc = NULL;

} // end of removeAssoc for RValuePane


//***************************************************************************
void RValuePane::setAssoc(RPaneAssoc *a)
//***************************************************************************
{
RValuePane *p;
si32 n;

	// recursively remove old assoc
	removeAssoc();

	// set the assoc for this instance
	mAssoc = a;

	// set the assoc for each child which is a RValuePane
	n = countPane();
	for(n--; n>=0; n--)
	{
		RPane *p2 = paneAt(n);
		if (!p2) continue;
		p = M_CAST_AS(p2, RValuePane);
		if (p) p->setAssoc(a);
	}

} // end of setAssoc for RValuePane


//***************************************************************************
void RValuePane::setValue(RValue &v)
//***************************************************************************
{
	if (mKey == K_NO_PANEKEY) M_THROW(kErrNoPaneKey);
	if (!mAssoc) M_THROW(kErrNoAssoc);
	mAssoc->set(mKey, v);

} // end of setValue for RValuePane


//***************************************************************************
RValue RValuePane::getValue(void)
//***************************************************************************
{
	if (mKey == K_NO_PANEKEY) M_THROW(kErrNoPaneKey);
	if (!mAssoc) M_THROW(kErrNoAssoc);
	return mAssoc->get(mKey);
} // end of getValue for RValuePane


//***************************************************************************
void RValuePane::getValue(RValue &v)
//***************************************************************************
{
	if (mKey == K_NO_PANEKEY) M_THROW(kErrNoPaneKey);
	if (!mAssoc) M_THROW(kErrNoAssoc);
	v = mAssoc->get(mKey);
} // end of getValue for RValuePane


//---------------------------------------------------------------------------

// eoc
