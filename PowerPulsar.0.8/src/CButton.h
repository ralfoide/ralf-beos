/*****************************************************************************

	Projet	: Pulsar

	Fichier	:	CInterfaceWin.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CBUTTON_
#define _H_CBUTTON_

#include "globals.h"
#include <Path.h>


//---------------------------------------------------------------------------


//****************************
class CButton : public BButton
//****************************
{
public:
	CButton(BRect frame,
					const char *name,
					const char *label, 
					const char *filename,
					BMessage *message,
					uint32 resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP, 
					uint32 flags = B_WILL_DRAW /*| B_NAVIGABLE*/);
	~CButton(void) {	if (mImage) delete mImage; }

	virtual	void MouseDown(BPoint where);
	virtual	void MouseMoved(BPoint where, uint32 code, const BMessage *a_message);
	virtual void Draw(BRect upd);

private:

	BRect bounds;
	BBitmap *mImage;
	BRect mImageSize;
	float mQuart;
	BRect mQuartSize;
	bool	mMouseInside;
	BFont	mFont;
	font_height mFontHeight;
	float mFontOffset;
};

//******************************
class CCdButton : public BButton
//******************************
{
public:
	CCdButton(BRect frame,
					const char *name,
					long index, long nb, 
					const char *filename,
					BMessage *message,
					uint32 resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP, 
					uint32 flags = B_WILL_DRAW /*| B_NAVIGABLE*/);
	~CCdButton(void) {	if (mImage) delete mImage; }

	virtual	void MouseDown(BPoint where);
	virtual void Draw(BRect upd);

private:

	long index;
	long nb;
	BRect bounds;
	BBitmap *mImage;
	BRect mImageSize;
	float mQuart;
	float mLarg;
	BRect mQuartSize;
};



//---------------------------------------------------------------------------

#endif // of _H_CBUTTON_

// eoh

