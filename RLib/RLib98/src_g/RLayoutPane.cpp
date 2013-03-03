/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RLayoutPane.cpp
	Partie	: GUI kit

	Auteur	: RM
	Date		: 110798
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#include "RLayoutPane.h"
#include "RErrEx.h"
#include "gErrors.h"

//---------------------------------------------------------------------------


//***************************************************************************
RLayoutPane::RLayoutPane(void)
						:RLayerPane()
//***************************************************************************
{
	mLayoutMethod = NULL;
} // end of constructor for RLayoutPane


//***************************************************************************
RLayoutPane::RLayoutPane(RLayoutInterface *method)
						:RLayerPane()
//***************************************************************************
{
	mLayoutMethod = method;
} // end of constructor for RLayoutPane


//***************************************************************************
RLayoutPane::~RLayoutPane(void)
//***************************************************************************
{
} // end of destructor for RLayoutPane


//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// virtuals from RLayerPane

//***************************************************************************
rbool RLayoutPane::getIdealSize(RRect &minRect, RRect &maxRect)
//***************************************************************************
{
	// inherited method is pure virtual...
	// RLayable::getIdealSize(minRect, maxRect);

	if (!mHasIdealSize)
	{
		if (mLayoutMethod) mLayoutMethod->adjustPane(*this);

		mMinRect = bounds();
		mMaxRect = mMinRect;
		mHasIdealSize = true;
	}

	return false;

} // end of getIdealSize for RLayoutPane


//***************************************************************************
void RLayoutPane::adjustSize(void)
//***************************************************************************
{
	RLayerPane::adjustSize();

} // end of adjustSize for RLayoutPane


//---------------------------------------------------------------------------


//***************************************************************************
void RLayoutPane::setLayout(RLayoutInterface *method)
//***************************************************************************
{
	mLayoutMethod = method;
	if (method)
	{
		mHasIdealSize = false;
		adjustSize();
	}
} // end of setLayout for RLayoutPane


//---------------------------------------------------------------------------
// virtual from RPaneInterface/RPane


//***************************************************************************
void RLayoutPane::attached(rbool _attached)
//***************************************************************************
{
	RLayerPane::attached(_attached);
	if (_attached) adjustSize();

} // end of attached for RLayoutPane


//---------------------------------------------------------------------------

// eoc
