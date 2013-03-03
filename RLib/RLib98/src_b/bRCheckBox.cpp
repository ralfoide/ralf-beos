/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRCheckBox.cpp
	Partie	: GUI kit

	Auteur	: RM
	Date		: 070499
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RCheckBox.h"
#include "RErrEx.h"
#include "gErrors.h"

//---------------------------------------------------------------------------


//***************************************************************************
RCheckBox::RCheckBox(RRect frame, sptr name, RPaneKey key, RThread *reply)
					:BCheckBox(frame, name, name, NULL),
					 RValuePane(key),
					 RLayerPane()
//***************************************************************************
{
	if (debug) printf("RCheckBox::RCheckBox -- frame\n");
	mReply = reply;
} // end of constructor for RCheckBox


//***************************************************************************
RCheckBox::RCheckBox(sptr name, RPaneKey key, RThread *reply)
					:BCheckBox(BRect(0,0,50,20), name, name, NULL),
					 RValuePane(key),
					 RLayerPane()
//***************************************************************************
{
	if (debug) printf("RCheckBox::~RCheckBox -- no_frame\n");
	mReply = reply;
} // end of constructor for RCheckBox


//***************************************************************************
RCheckBox::~RCheckBox(void)
//***************************************************************************
{
	if (debug) printf("RCheckBox::~RCheckBox -- this %p\n", this);
} // end of destructor for RCheckBox



//---------------------------------------------------------------------------
// inherited from RPane, RLayerPane and RValuePane

//***************************************************************************
void RCheckBox::attached(rbool _attached)
//***************************************************************************
{
	if (debug) printf("RCheckBox::attached -- rbool %d\n", _attached);
	RLayerPane::attached(_attached);
	RValuePane::attached(_attached);
	if (_attached && getKey() != K_NO_PANEKEY)
	{
		RValue val(EValInt);
		val.setInt(Value());
		setValue(val);
	}
} // end of attached for RCheckBox


//---------------------------------------------------------------------------
// inherited from RLayerPane


//***************************************************************************
rbool RCheckBox::getIdealSize(RRect &minRect, RRect &maxRect)
//***************************************************************************
{
	// getIdealSize from RLayerPane is pure abstract
	if (debug) printf("RCheckBox::getIdealSize\n");

	// Porting Alert :
	// you may want to fill mMinRect, mMaxRect, mHasIdealSize & mHasMaxRect
	// here we just rely on the default behavior for adjustSize() that
	// already does all this the right way...

	float width, height;
	GetPreferredSize(&width, &height);

	minRect.set(0,0,width-1, height-1);
	maxRect = minRect;

	return false;

} // end of attached for RCheckBox


//***************************************************************************
void RCheckBox::adjustSize(void)
//***************************************************************************
{
	// adjustSize is a callback used by layers to indicate to this pane that it
	// should adjust it's own size right now...
	if (debug) printf("RCheckBox::adjustSize\n");

	// ResizeToPreferred();
	RLayerPane::adjustSize();

} // end of adjustSize(void for RCheckBox


//---------------------------------------------------------------------------



//***************************************************************************
status_t RCheckBox::Invoke(BMessage *msg)
//***************************************************************************
{
	BCheckBox::Invoke(msg);

	if (getKey() != K_NO_PANEKEY)
	{
		RValue val(EValInt);
		val.setInt(Value());
		setValue(val);

		if (mReply) mReply->postMessage(mReply, getKey(), Value(), (vptr)this);
	}

	return B_NO_ERROR;
} // end of BeOS Invoke for RCheckBox

//---------------------------------------------------------------------------

#endif // RLIB_BEOS

// eoc
