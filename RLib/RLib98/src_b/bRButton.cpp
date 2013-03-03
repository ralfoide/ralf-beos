/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRButton.cpp
	Partie	: GUI kit

	Auteur	: RM
	Date		: 120798
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RButton.h"
#include "RErrEx.h"
#include "gErrors.h"

//---------------------------------------------------------------------------


//***************************************************************************
RButton::RButton(RRect frame, sptr name, RPaneKey key,
								 rbool isDefault, RThread *reply)
				:BButton(frame, name, name, NULL),
				 RValuePane(key),
				 RLayerPane()
//***************************************************************************
{
	if (debug) printf("RButton::RButton -- frame\n");
	mReply = reply;
	MakeDefault(isDefault);
} // end of constructor for RButton


//***************************************************************************
RButton::RButton(sptr name, RPaneKey key,
								 rbool isDefault, RThread *reply)
				:BButton(BRect(0,0,50,20), name, name, NULL),
				 RValuePane(key),
				 RLayerPane()
//***************************************************************************
{
	if (debug) printf("RButton::~RButton -- no_frame\n");
	mReply = reply;
	MakeDefault(isDefault);
} // end of constructor for RButton


//***************************************************************************
RButton::~RButton(void)
//***************************************************************************
{
	if (debug) printf("RButton::~RButton -- this %p\n", this);
} // end of destructor for RButton



//---------------------------------------------------------------------------
// inherited from RPane, RLayerPane and RValuePane

//***************************************************************************
void RButton::attached(rbool _attached)
//***************************************************************************
{
	if (debug) printf("RButton::attached -- rbool %d\n", _attached);
	RLayerPane::attached(_attached);
	RValuePane::attached(_attached);
	if (_attached && getKey() != K_NO_PANEKEY)
	{
		RValue val(EValInt);
		val.setInt(Value());
		setValue(val);
	}
} // end of attached for RButton


//---------------------------------------------------------------------------
// inherited from RLayerPane


//***************************************************************************
rbool RButton::getIdealSize(RRect &minRect, RRect &maxRect)
//***************************************************************************
{
	// getIdealSize from RLayerPane is pure abstract
	if (debug) printf("RButton::getIdealSize\n");

	// Porting Alert :
	// you may want to fill mMinRect, mMaxRect, mHasIdealSize & mHasMaxRect
	// here we just rely on the default behavior for adjustSize() that
	// already does all this the right way...

	float width, height;
	GetPreferredSize(&width, &height);

	minRect.set(0,0,width-1, height-1);
	maxRect = minRect;

	return false;

} // end of attached for RButton


//***************************************************************************
void RButton::adjustSize(void)
//***************************************************************************
{
	// adjustSize is a callback used by layers to indicate to this pane that it
	// should adjust it's own size right now...
	if (debug) printf("RButton::adjustSize\n");

	// ResizeToPreferred();
	RLayerPane::adjustSize();

} // end of adjustSize(void for RButton


//---------------------------------------------------------------------------



//***************************************************************************
status_t RButton::Invoke(BMessage *msg)
//***************************************************************************
{
	BButton::Invoke(msg);

	if (getKey() != K_NO_PANEKEY)
	{
		RValue val(EValInt);
		val.setInt(Value());
		setValue(val);

		if (mReply) mReply->postMessage(mReply, getKey(), Value(), (vptr)this);
	}

	return B_NO_ERROR;
} // end of BeOS Invoke for RButton

//---------------------------------------------------------------------------

#endif // RLIB_BEOS

// eoc
