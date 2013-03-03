/*****************************************************************************

	Projet	: Pulsar

	Fichier:	CFilterInfoWin.h
	Date		:	200797

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CFILTERINFOWIN_
#define _H_CFILTERINFOWIN_

#include "globals.h"

//---------------------------------------------------------------------------

#define K_WINDOW_BORDER_WIDTH 5


//---------------------------------------------------------------------------

class CFilter;


//***********************************
class CFilterInfoView : public BView
//***********************************
{
public:
	CFilterInfoView(BRect frame,
										char *title,
										long resize,
										const char *bitmapname,
										long flags=B_WILL_DRAW);
	~CFilterInfoView(void) { if (mImage8) delete mImage8;
												if (mImage32) delete mImage32; }

	void setFilter(CFilter *filter);
	void Draw(BRect upd);
	void DetachedFromWindow(void);

private:

	CFilter	*mFilter;				// can be NULL
	BView		*mFilterView;		// NULL if no view or no filter
	BRect		mFilterRect;		// rect of view created by CFilter (BRect0/0/0/0 by default)

	BRect bounds;
	BRect referenceBounds;
	BBitmap *mImage32;
	BBitmap *mImage8;
	uint32	 mSx, mSy;
	uint32	 mPosText;

	BFont	mTitleFont;
	BFont	mNameFont;
	BFont	mTextFont;

	static const char infolabel[6][20];
	long infolabelLen[6];
	long maxInfolabelLen;

};


//---------------------------------------------------------------------------


//***********************************
class CFilterInfoWin : public BWindow
//***********************************
{
public:
	CFilterInfoWin(	BRect frame,
							const char *title, 
							window_type type,
							ulong flags,
							ulong workspace = B_CURRENT_WORKSPACE);
	virtual ~CFilterInfoWin(void);

	bool init(void);
	void setFilter(CFilter *filter);

	virtual	void MessageReceived(BMessage *message);
	virtual 	bool QuitRequested(void);

private:

	CFilterInfoView *mInfo;


}; // end of class defs for CFilterInfoWin


//---------------------------------------------------------------------------

#endif // of _H_CFILTERINFOWIN_

// eoh

