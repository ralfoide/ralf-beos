/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RLayerPane.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 040798
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#include "RLayerPane.h"
#include "RErrEx.h"
#include "gErrors.h"

//---------------------------------------------------------------------------


//***************************************************************************
RLayerPane::RLayerPane(ELayerHConstraint hConstraint,
													 ELayerVConstraint vConstraint)
						 :RPane()
//***************************************************************************
{
	mHConstraint = hConstraint;
	mVConstraint = vConstraint;

	mHasIdealSize = false;
	mHasMaxRect = false;
} // end of constructor for RLayerPane


//***************************************************************************
RLayerPane::~RLayerPane(void)
//***************************************************************************
{
} // end of destructor for RLayerPane


//---------------------------------------------------------------------------



//***************************************************************************
void RLayerPane::setConstraint(ELayerHConstraint hConstraint,
																 ELayerVConstraint vConstraint)
//***************************************************************************
{
	mHConstraint = hConstraint;
	mVConstraint = vConstraint;
	
} // end of attached for RLayerPane


//***************************************************************************
void RLayerPane::getConstraint(ELayerHConstraint &hConstraint,
															 ELayerVConstraint &vConstraint)
//***************************************************************************
{
	hConstraint = mHConstraint;
	vConstraint = mVConstraint;
} // end of attached for RLayerPane



//---------------------------------------------------------------------------


//***************************************************************************
void RLayerPane::adjustSize(void)
//***************************************************************************
{
	if (!mHasIdealSize)
	{
		mHasMaxRect = getIdealSize(mMinRect, mMaxRect);
		mHasIdealSize = true;
	}

	//moveTo(mMinRect.left, mMinRect.top);
	resizeTo(mMinRect.width(), mMinRect.height());

} // end of adjustSize for RLayerPane


//---------------------------------------------------------------------------

// eoc
