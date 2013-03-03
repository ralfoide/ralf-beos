/*****************************************************************************

	Projet	: Pulsar

	Fichier:	CFilterParamWin.h
	Date		:	200797

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CFILTERPARAMWIN_
#define _H_CFILTERPARAMWIN_

#include "globals.h"

//---------------------------------------------------------------------------

#define K_WINDOW_BORDER_WIDTH 5


//---------------------------------------------------------------------------

class CFilter;


//***********************************
class CFilterParamView : public BView
//***********************************
{
public:
	CFilterParamView(BRect frame,
										char *title,
										long resize,
										const char *bitmapname,
										long flags=B_WILL_DRAW);
	~CFilterParamView(void) { if (mImage8) delete mImage8;
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
class CFilterParamWin : public BWindow
//***********************************
{
public:
	CFilterParamWin(	BRect frame,
							const char *title, 
							window_type type,
							ulong flags,
							ulong workspace = B_CURRENT_WORKSPACE);
	virtual ~CFilterParamWin(void);

	bool init(void);
	void setFilter(CFilter *filter);

	virtual	void MessageReceived(BMessage *message);
	virtual 	bool QuitRequested(void);

private:

	CFilterParamView *mInfo;


}; // end of class defs for CFilterParamWin


//---------------------------------------------------------------------------

#endif // of _H_CFILTERPARAMWIN_

// eoh

