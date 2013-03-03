/*****************************************************************************

	Projet	: Pulsar

	Fichier	:	CInterfaceWin.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CADDONLISTVIEW_
#define _H_CADDONLISTVIEW_

#include "globals.h"
#include <Path.h>


//---------------------------------------------------------------------------

//*********************************
class CAddonListView : public BView
//*********************************
{
public:
	CAddonListView(BRect frame,
								const char *name, 
								const char *filename,
								//BMessage *message,
								uint32 resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM, 
								uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);

	~CAddonListView(void) {	if (mImage) delete mImage; }

	void updateAddonList(BList &filterList);

	virtual	void MouseDown(BPoint where);
	virtual	void MouseMoved(BPoint where, uint32 code, const BMessage *a_message);
	virtual	void FrameResized(float new_width, float new_height);
	// virtual	void			FrameMoved(BPoint new_position) { printf("FrameMoved\n");}
	virtual void Draw(BRect upd);

	void drawItem(BRect itemrect, int32 index);

private:

	bool mChangedLine;
	BRect bounds;
	BBitmap *mImage;
	BRect mImageSize;
	float mDemie;
	BRect mDemieSize;
	bool	mMouseInside;
	BPoint mMouseCoord;
	int32	mOverIndex;
	BFont	mFont;
	font_height mFontHeight;
	float mFontOffset;

	BBitmap *mPositionBitmap;
	BRect		mPositionSize;
	long			mPositionX, mPositionY;

	// addon info
	BList	mAddons;
	int32	mNbAddons;

};

//---------------------------------------------------------------------------

#endif // of _H_CADDONLISTVIEW_

// eoh

