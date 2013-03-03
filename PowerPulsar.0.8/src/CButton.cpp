/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CInterfaceWin.cpp
	Partie	: Loader

	Auteur	: RM
	Date		: 140297
	Format	: tabs==2

*****************************************************************************/

#include "CInterfaceWin.h"
#include "CPulsarApp.h"
#include "CPulsarMsg.h"		// messages defines herein

#if K_USE_DATATYPE

#include "Datatypes.h"
#include "GetBitmap.h"		/// in /boot/develop/Datatypes152/headers/glue/...
#include <Path.h>

#endif

#include "CButton.h"


//---------------------------------------------------------------------------

extern CPulsarApp *gApplication;

//---------------------------------------------------------------------------


//***********************************************************
CCdButton::CCdButton(BRect frame,
							const char *name,
							long _index, long _nb, 
							const char *filename,
							BMessage *message,
							uint32 resizeMask, 
							uint32 flags)
				 			:BButton(frame, name, "", message, resizeMask, flags)
#define inherited BButton
//***********************************************************
{
	SetViewColor(B_TRANSPARENT_32_BIT);

	index = _index;
	nb = _nb;
	bounds = Bounds();

	mImage = NULL;

	BDirectory app(gApplication->getAppDir());
	BDirectory dir(&app, K_IMG_DIR);
	BPath path(&dir, filename);

	mImage = gApplication->getBitmap(path.Path());

	if (mImage) mImageSize = mImage->Bounds();
	else gApplication->loadBitmapError(filename);
	mQuart = (float)((int)((mImageSize.IntegerHeight()+1)/2));
	mLarg = (float)((int)((mImageSize.IntegerWidth()+1)/nb));
	mQuartSize = mImageSize;
	mQuartSize.right = mLarg*(index+1)-1;
	mQuartSize.left = mLarg*index;
	mQuartSize.bottom = mQuart-1;

} // end of constructor for CCdButton


//***********************************************************
void CCdButton::MouseDown(BPoint where)
//***********************************************************
{
	inherited::MouseDown(where);
} // end of MouseDown for CCdButton


//***********************************************************
void CCdButton::Draw(BRect upd)
//***********************************************************
{
int decal;
int enfonce=2;

	upd.InsetBy(-1,-1);

	if (!mImage)
	{
		inherited::Draw(upd);
		return;
	}

	if (Value() && IsEnabled()) decal = K_BTN_DEF_ON;
	else decal = K_BTN_DEF_OFF;

	SetDrawingMode(B_OP_COPY);
	BRect r(mQuartSize);
	r.OffsetBy(0.0, mQuart * decal);
	DrawBitmap(mImage, r, bounds);

} // end of Draw for CCdButton


//---------------------------------------------------------------------------
#pragma mark -


//***********************************************************
CButton::CButton(BRect frame,
							const char *name,
							const char *label, 
							const char *filename,
							BMessage *message,
							uint32 resizeMask, 
							uint32 flags)
				 			:BButton(frame, name, label, message, resizeMask, flags),
				 			 mFont(be_bold_font)
//***********************************************************
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	//SetViewColor(B_TRANSPARENT_32_BIT);

	bounds = Bounds();

	mImage = NULL;
	mMouseInside = false;

	BDirectory app(gApplication->getAppDir());
	BDirectory dir(&app, K_IMG_DIR);
	BPath path(&dir, filename);

	mImage = gApplication->getBitmap(path.Path());

	if (mImage) mImageSize = mImage->Bounds();
	else gApplication->loadBitmapError(filename);
	mQuart = (float)((int)((mImageSize.IntegerHeight()+1)/4));
	mQuartSize = mImageSize;
	mQuartSize.bottom = mQuart-1;

	mFont.SetSpacing(B_BITMAP_SPACING);
	mFont.SetFlags(B_DISABLE_ANTIALIASING);
	mFont.GetHeight(&mFontHeight);
	mFontOffset = mFontHeight.ascent+mFontHeight.descent+mFontHeight.leading;
	mFontOffset = (bounds.Height()-mFontOffset)/2+mFontHeight.ascent;

} // end of constructor for CButton


//***********************************************************
void CButton::MouseDown(BPoint where)
//***********************************************************
{
	//Window()->Activate();
	/*if (!mImage) */ inherited::MouseDown(where);
} // end of MouseDown for CButton


//***********************************************************
void CButton::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
//***********************************************************
{
	bool state = (code == B_INSIDE_VIEW);
	if (state != mMouseInside)
	{
		mMouseInside = state;
		if (Window() && Window()->Lock())
		{
			Draw(bounds);
			Window()->Unlock();
		}
	}
} // end of MouseMoved for CButton


//***********************************************************
void CButton::Draw(BRect upd)
//***********************************************************
{
int decal;
int enfonce=2;

	upd.InsetBy(-1,-1);

	if (!mImage)
	{
		inherited::Draw(upd);
		return;
	}

	if (!IsEnabled()) decal = K_BTN_DEF_DISA;
	else
	{
		BPoint location;
		uint32 buttons;
		GetMouse(&location, &buttons, false);
		if (Value()) decal = K_BTN_DEF_ON;
		else decal = K_BTN_DEF_OFF;
		if (!buttons && mMouseInside) decal = K_BTN_DEF_OVER;
		if (buttons && mMouseInside) enfonce=1;
	}

	SetDrawingMode(B_OP_COPY);
	BRect r(mQuartSize);
	r.OffsetBy(0.0, mQuart * decal);
	DrawBitmap(mImage, r, bounds);

	const char *s = Label();
	SetFont(&mFont);
	SetDrawingMode(B_OP_COPY);
	SetHighColor(0,0,0);
	float x,y;
	x = (bounds.Width()-StringWidth(s))/2-enfonce;
	y = mFontOffset-enfonce;
	MovePenTo(x+1,y+1);
	DrawString(s);
	SetHighColor(255,255,255,0);
	MovePenTo(x,y);
	DrawString(s);

} // end of Draw for CButton




//---------------------------------------------------------------------------

// eoc
