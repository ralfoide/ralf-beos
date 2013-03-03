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
#endif

#include <Path.h>

#include "CSimpleTrack.h"

//---------------------------------------------------------------------------

extern CPulsarApp *gApplication;

static long sMapModeToImg[kLineModeMax] = {3,0,1,2};
static ELineMode sMapImgToMode[kLineModeMax] = {kLineModeOn, kLineModeRepeat, kLineModeHand, kLineModeOff};

//---------------------------------------------------------------------------


//***********************************************************
CTrackHeader::CTrackHeader(BRect frame,
										const char *name,
										uint32 resizeMask, 
										uint32 flags)
				 			:BView(frame, name, resizeMask, flags),
				 			 mFont(be_plain_font)
//***********************************************************
{
	//SetViewColor(255,255,255);
	SetViewColor(B_TRANSPARENT_32_BIT);
	bounds = Bounds();
	mFont.SetSize(9);
	mFont.SetSpacing(B_BITMAP_SPACING);
	mFont.GetHeight(&mFontHeight);
	mFontOffset = /*mFontHeight.ascent+*/mFontHeight.descent+mFontHeight.leading+2;
	mTimeScale = 200/60e3;	// pixels per milliseconds

} // end of constructor for CTrackHeader


//***********************************************************
void CTrackHeader::FrameResized(float new_width, float new_height)
//***********************************************************
{
	BRect oldbounds = bounds;
	bounds = Bounds();

	if (oldbounds.right > bounds.right)
		oldbounds.Set(bounds.right-2, bounds.top, bounds.right, bounds.bottom);
	else
		oldbounds.Set(oldbounds.right-2, bounds.top, bounds.right, bounds.bottom);

	if (Window()->Lock())
	{
		Draw(oldbounds);
		Window()->Unlock();
	}

} // end of FrameResized for CTrackHeader


//***********************************************************
void CTrackHeader::Draw(BRect upd)
//***********************************************************
{
float x;
int32 t;

	SetDrawingMode(B_OP_COPY);

	SetHighColor(255,255,255);
	FillRect(BRect(upd.left, bounds.top, upd.right, bounds.bottom));

	SetHighColor(0,0,0);
	SetLowColor(255,255,255);
	SetFont(&mFont);

	for(x=mTimeScale*10e3; x<upd.right; x+=mTimeScale*10e3)
		StrokeLine(BPoint(x,bounds.bottom), BPoint(x,bounds.bottom-3));

	for(t=1, x=mTimeScale*60e3; x<upd.right; x+=mTimeScale*60e3, t+=1)
	{
		//StrokeLine(BPoint(x,bounds.bottom), BPoint(x,bounds.bottom-5));
		char s[64];
		sprintf(s, "%d mn", t);
		int w = (int)StringWidth(s);
		float y = bounds.bottom-mFontOffset-4;
		SetHighColor(0,0,0);
		MovePenTo(x-w/2, y);
		DrawString(s);

		y = bounds.bottom-6;
		SetHighColor(0,102,203);
		FillTriangle(BPoint(x,y+3), BPoint(x+3,y), BPoint(x-3,y));
		SetHighColor(100,100,100);
		StrokeTriangle(BPoint(x,y+4), BPoint(x+4,y-1), BPoint(x-4,y-1));
	}

	SetHighColor(0,0,0);
	MovePenTo(10,mFontHeight.ascent+mFontHeight.descent+2);
	DrawString("Project Track 1");

	SetHighColor(0,0,0);
	StrokeRect(bounds);

} // end of Draw for CTrackHeader


//---------------------------------------------------------------------------
#pragma mark -


//***********************************************************
CTrackView::CTrackView(BRect frame,
										const char *name,
										uint32 resizeMask, 
										uint32 flags)
				 			:BView(frame, name, resizeMask, flags),
				 			 mFont(be_plain_font)
//***********************************************************
{
	//SetViewColor(255,255,255);
	SetViewColor(B_TRANSPARENT_32_BIT);
	mNbLine=0;

	bounds = Bounds();

	mChangedLine = true;
	mMouseInside = false;
	mIsDropTarget = false;

	mFont.SetSize(9);
	mFont.SetSpacing(B_BITMAP_SPACING);
	//mFont.SetFlags(B_DISABLE_ANTIALIASING);
	mFont.GetHeight(&mFontHeight);
	mFontOffset = /*mFontHeight.ascent+*/mFontHeight.descent+mFontHeight.leading+2;

	BDirectory app(gApplication->getAppDir());
	BDirectory dir(&app, K_IMG_DIR);
	BPath path2(&dir, K_IMG_POSITION);
	mPositionBitmap = gApplication->getBitmap(path2.Path());
	if (mPositionBitmap)
	{
		mPositionSize = mPositionBitmap->Bounds();
		mPositionX = (long)(mPositionSize.Width()+1)/3;
		mPositionY = (long)mPositionSize.Height()+1;
	}

	BPath path3(&dir, K_IMG_MODE);
	mModeBitmap = gApplication->getBitmap(path3.Path());
	if (mModeBitmap)
	{
		mModeSize = mModeBitmap->Bounds();
		mModeX = (long)(mModeSize.Width() +1)/kLineModeMax;
		mModeY = (long)(mModeSize.Height()+1)/kLineModeMax;
	}


	mMouseCoord.Set(-1,-1);
	mOverIndex = -1;
	mOverInsertPoint = kOverInsertMiddle;

	mLineSize = 24;
	mColumn = 100;
	mTimeScale = 200/60e3;	// pixels per milliseconds

} // end of constructor for CTrackView


//***********************************************************
void CTrackView::MouseDown(BPoint where)
//***********************************************************
{
SLineParam *line;
CFilter *filter=NULL;

	if (mOverIndex<0 || mOverIndex>mNbLine) return;
	if (!gSimpleTrack->lock()) return;
	line = gSimpleTrack->getLine(mOverIndex);
	if (line) filter = line->filter;
	gSimpleTrack->unlock();
	if (!filter) return;

	float x=where.x;
	const double seuil=4.0;
	BPoint location;
	uint32 buttons;
	int32 count;

	// rectangle for drawItem for this line
	BRect r(1,0,bounds.right-1,mLineSize-1);
	r.OffsetBy(0,mOverIndex*mLineSize);

	// get click count
	Window()->CurrentMessage()->FindInt32("clicks", &count);

	GetMouse(&location, &buttons);

	if (x < mColumn-3-mModeX && count==1 && buttons)
	{
		// --- drag from the Filter Name

		BMessage msg(K_MSG_FILTER_INFO);
		msg.AddPointer(K_NAME_FILTER_INFO, (void *)filter);
		msg.AddPointer(K_NAME_FILTER_LINE, (void *)line);
		msg.AddInt32  (K_NAME_FILTER_OVER, mOverIndex);

		BPoint p(location);
		DragMessage(&msg, BRect(p.x-mColumn/2,p.y-mLineSize/2,p.x+mColumn/2,p.y+mLineSize/2));
	}
	else if (x >= mColumn-2-mModeX && x < mColumn && count==1)
	{
		// -- change the mode of the line
		if (gSimpleTrack->lock())
		{
			line->mode = (ELineMode)(((long)line->mode + 1) % (long)kLineModeMax);
			if (line->mode == kLineModeHand) // not supported yet !!
				line->mode = (ELineMode)(((long)line->mode + 1) % (long)kLineModeMax);
			gSimpleTrack->unlock();
		}
		drawLine(r, mOverIndex, true);
	}
	else 
	{
		// -- drag & move a key

		float xstart, xend, xrepeat;
		bool mode2, mode1;
		
		if (gSimpleTrack->lock())
		{
			xstart		= mColumn+2+line->start * mTimeScale;
			xend			= mColumn+2+(line->start+line->len) * mTimeScale;
			xrepeat	= mColumn+2+(line->start+line->repeat) * mTimeScale;
			mode2 =  (line->mode == kLineModeRepeat);
			mode1 =  (mode2 || line->mode == kLineModeOn);
			gSimpleTrack->unlock();
		}

		if (fabs(x-xrepeat) <= seuil && mode2)
		{
			GetMouse(&location, &buttons);
			while(buttons)
			{
				float x=location.x;
				if (x>xstart+3 && x>xend && x<bounds.right-5)
				{
					if (gSimpleTrack->lock())
					{
						line->repeat = (int32)((x-xstart)/mTimeScale);
						x	= mColumn+2+(line->start+line->repeat) * mTimeScale;
						gSimpleTrack->unlock();
					}
					if (x != xrepeat)
					{
						drawLine(r, mOverIndex, true);
						xrepeat=x;
					}
				}
				GetMouse(&location, &buttons);
			}
		}
		else if (fabs(x-xend) <= seuil && mode2)
		{
			GetMouse(&location, &buttons);
			while(buttons)
			{
				float x=location.x;
				if (x>xstart && x<xrepeat && x<bounds.right-5)
				{
					if (gSimpleTrack->lock())
					{
						line->len = (int32)((x-xstart)/mTimeScale);
						x = mColumn+2+(line->start+line->len) * mTimeScale;
						gSimpleTrack->unlock();
					}
					if (x != xend)
					{
						drawLine(r, mOverIndex, true);
						xend = x;
					}
				}
				GetMouse(&location, &buttons);
			}
		}
		else if (fabs(x-xstart) <= seuil && mode1)
		{
			GetMouse(&location, &buttons);
			while(buttons)
			{
				float x=location.x;
				if (x>=mColumn+2 && x<bounds.right-5)
				{
					if (gSimpleTrack->lock())
					{
						line->start = (int32)((x-mColumn-2.0)/mTimeScale);
						x = mColumn+2+line->start * mTimeScale;
						gSimpleTrack->unlock();
					}
					if (x != xstart)
					{
						drawLine(r, mOverIndex, true);
						xstart = x;
					}
				}
				GetMouse(&location, &buttons);
			}
		}
	}

} // end of MouseDown for CTrackView


//***********************************************************
void CTrackView::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
//***********************************************************
{
	bool changed;
	bool state = (code == B_INSIDE_VIEW);
	long old = mOverIndex;
	EOverInsertPoint old2 = mOverInsertPoint;
	changed = (state != mMouseInside);
	if (state)
	{
		mMouseCoord = where;
		int32 wy = (int32)where.y;
		mOverIndex = (int32)wy/mLineSize;
		int32 delta = (mOverIndex*mLineSize)-wy;
		if (delta < 5) mOverInsertPoint = kOverInsertBefore;
		else if (delta > mLineSize-5) mOverInsertPoint = kOverInsertAfter;
		else mOverInsertPoint = kOverInsertMiddle;
		changed = changed || (mOverIndex != old) || (old2 != mOverInsertPoint);
	}
	else
	{
		mMouseCoord.Set(-1,-1);
		mOverIndex = -1;
		mOverInsertPoint = kOverInsertMiddle;
	}

	mIsDropTarget = (a_message
								&& a_message->what == K_MSG_FILTER_INFO
								&& a_message->HasPointer(K_NAME_FILTER_INFO));

	mChangedLine = changed;
	if (changed)
	{
		mMouseInside = state;
		if (Window() && Window()->Lock())
		{
			BRect r(1,0,bounds.right-1,mLineSize-1);
			r.OffsetBy(0,old*mLineSize);
			drawLine(r, old);
			if (old != mOverIndex || old2 != mOverInsertPoint)
			{
				BRect r(1,0,bounds.right-1,mLineSize-1);
				r.OffsetBy(0,mOverIndex*mLineSize);
				drawLine(r, mOverIndex);
			}
			Window()->Unlock();
		}
	}

} // end of MouseMoved for CTrackView


//***********************************************************
void CTrackView::MessageReceived(BMessage *message)
//***********************************************************
{
long notify_line = -1;
CFilter *notify_filter = NULL;

	if (message->what == K_MSG_DEL_LINE_PARAM)
	{
		status_t err;
		int32 over;
		err = message->FindInt32(K_NAME_FILTER_OVER, &over);
		if (err >= B_NO_ERROR && over >= 0 && over < mNbLine)
		{
			BMessage reply;
			BMessage msg(K_MSG_TERMINATE_TRACK_LINE);
			msg.AddInt32(K_NAME_FILTER_OVER, over);
			BMessenger(gApplication).SendMessage(&msg, &reply);
		
			if (gSimpleTrack->lock())
			{
				gSimpleTrack->delLine(over);
				mNbLine = gSimpleTrack->countLine();
				gSimpleTrack->unlock();
			}
		}
		mIsDropTarget = false;
		Invalidate();
	}
	else if (message->WasDropped()
			&& message->what == K_MSG_FILTER_INFO
			&& message->HasPointer(K_NAME_FILTER_INFO))
	{
		// some filter info was dropped there
		BPoint where = message->DropPoint();
		ConvertFromScreen(&where);

		int32 wy = (int32)where.y;
		int32 over = wy/mLineSize;
		EOverInsertPoint point = kOverInsertMiddle;
		int32 delta = (over*mLineSize)-wy;
		if (delta < 5) point = kOverInsertBefore;
		else if (delta > mLineSize-5) point = kOverInsertAfter;

		if (over < 0 || over > mNbLine) return;
		
		SLineParam *line;
		status_t err;
		err = message->FindPointer(K_NAME_FILTER_LINE, (void **) &line);
		if (err >= B_NO_ERROR && is_kind_of(line, SLineParam))
		{
			if (gSimpleTrack->lock())
			{
				// this is a drag from the track view itself
				SLineParam *newline = gSimpleTrack->getLine(over);
				// modify existing line
				notify_line = over;
				if (newline) *newline = *line;
				else
				{
					// or add new one
					newline = new SLineParam;
					*newline = *line;
					gSimpleTrack->addLine(newline);
					mNbLine = gSimpleTrack->countLine();
					notify_line = mNbLine-1;
				}
				gSimpleTrack->unlock();
			}
			if (notify_line >= 0)
				message->FindPointer(K_NAME_FILTER_INFO, (void **) &notify_filter);
		}
		else
		{
			// this is a drag from the addon/filter list
			CFilter *filter;
			err = message->FindPointer(K_NAME_FILTER_INFO, (void **) &filter);
			if (err >= B_NO_ERROR
					&& is_kind_of(filter, CFilter)
					&& !is_instance_of(filter, CFilter))
			{
				if (gSimpleTrack->lock())
				{
					// this is a *valid* drag from the addon/filter list
					line = gSimpleTrack->getLine(over);
					notify_line = over;
					// modify existing line
					if (line) line->filter = filter;
					else
					{
						// or add new one
						line = new SLineParam;
						line->filter = filter;
						gSimpleTrack->addLine(line);
						mNbLine = gSimpleTrack->countLine();
						notify_line = mNbLine-1;
					}
					gSimpleTrack->unlock();
				}
				if (notify_line >= 0) notify_filter = filter;
			}
		}
		mIsDropTarget = false;
		Invalidate();
	}
	
	if (notify_line >= 0 && notify_filter)
	{
		// a new line was added. Notify the app that the main thread should consider
		// preparing this line for a start if currently in play mode
		BMessage msg(K_MSG_PREPARE_TRACK_LINE);
		msg.AddPointer(K_NAME_FILTER_INFO, (void *)notify_filter);
		msg.AddInt32(K_NAME_FILTER_OVER, notify_line);
		gApplication->PostMessage(&msg);
	}
	
} // end of MessageReceived for CTrackView


//***********************************************************
void CTrackView::FrameResized(float new_width, float new_height)
//***********************************************************
{
	BRect oldbounds = bounds;
	bounds = Bounds();

	if (oldbounds.right > bounds.right)
		oldbounds.Set(bounds.right-2, bounds.top, bounds.right, bounds.bottom);
	else
		oldbounds.Set(oldbounds.right-2, bounds.top, bounds.right, bounds.bottom);

	if (Window()->Lock())
	{
		Draw(oldbounds);
		Window()->Unlock();
	}

} // end of FrameResized for CTrackView



//***********************************************************
void CTrackView::Draw(BRect upd)
//***********************************************************
{
BRect r;
int32 yj;
int32 index;

	upd.InsetBy(-1,-1);

	r = upd;
	r.top = 0;
	r.bottom = (int32)mLineSize-1;
	index = (int32)(upd.top/mLineSize);
	r.OffsetBy(0,index*mLineSize);
	yj = (int32)upd.bottom;

	//SetFont(&mFont);
	BRect rect = upd;
	for(; r.top < yj; index++, r.OffsetBy(0,mLineSize))
	{
		rect.top = r.top;
		rect.bottom = r.bottom;
		drawLine(r, index, false, &rect);
	}

	//SetDrawingMode(B_OP_COPY);
	//SetHighColor(0,0,0);
	//StrokeRect(bounds);

} // end of Draw for CTrackView


//****************************************************************************
void CTrackView::drawLine(BRect itemRect, int32 index, bool erase, BRect *upd)
//****************************************************************************
{
float x;
rgb_color backnormal ={255,255,255};
rgb_color backjaune  ={255,255,203};
rgb_color backrouge  ={203,102,51};

	SetDrawingMode(B_OP_COPY);

	if(upd)
	{
		SetHighColor(255,255,255);
		FillRect(*upd);
	}

	SetLowColor(255,255,255);

	if (mChangedLine)
	{
		if (mMouseInside && mOverIndex == index)
		{
			if (mIsDropTarget && mOverIndex <= mNbLine && mOverInsertPoint == kOverInsertMiddle)
				backnormal=backrouge;
			else
				backnormal=backjaune;
		}
		SetHighColor(backnormal);
		BRect r=itemRect;
		r.right=mColumn;
		r.InsetBy(1,1);
		r.left-=2;
		r.top--;
		FillRect(r);

		if (mMouseInside && mOverIndex == index
				&& mIsDropTarget && mOverIndex <= mNbLine && mOverInsertPoint != kOverInsertMiddle)
		{
			SetHighColor(backrouge);
			if (mOverInsertPoint == kOverInsertBefore)
				r.bottom = r.top+5;
			else if (mOverInsertPoint == kOverInsertAfter)
				r.top = r.bottom-5;
			FillRect(r);
		}
	}

	SetHighColor(0,0,0);
	StrokeLine(itemRect.LeftBottom(), itemRect.RightBottom());
	StrokeLine(BPoint(mColumn,itemRect.top), BPoint(mColumn,itemRect.bottom));

	if (index<0 || index>=mNbLine) return;
	if (!gSimpleTrack->lock()) return;
	SLineParam *line = gSimpleTrack->getLine(index);
	if (!line)
	{
		gSimpleTrack->unlock();
		return;
	}

	ELineMode mode=line->mode;
	int32 start = line->start;
	int32 len = line->len;
	int32 repeat = line->repeat;

	EFilterPosition position = kFilterPositionAny;
	char *sn = NULL;
	if (line->filter)
	{
		position = line->filter->sFilter.position;
		sn = line->filter->sFilter.name;
	}
	if (!sn) sn = "(unnamed)";
	
	// no longer need to acces to Track information
	gSimpleTrack->unlock();

	// put name -- ideal place for BFont::GetTruncatedStrings() (should be cached)
	SetFont(&mFont);
	SetLowColor(backnormal);
	MovePenTo(3+mPositionX+5, itemRect.bottom-mFontOffset);
	DrawString(sn);

	if (mPositionBitmap)
	{
		long index=2;
		if (position < kFilterPositionLast/4) index = 0;
		else if (position < kFilterPositionLast*3/4) index = 1;
		BRect src(0, 0, mPositionX-1, mPositionY-1);
		BRect dest = src;
		src.OffsetBy(index*mPositionX, 0);
		dest.OffsetBy(3, itemRect.top+(mLineSize-mPositionY)/2);
		DrawBitmap(mPositionBitmap, src, dest);
	}

	if (mModeBitmap)
	{
		long index=sMapModeToImg[(long)mode];
		BRect src(0, 0, mModeX-1, mModeY-1);
		BRect dest=src;
		src.OffsetBy(index*mModeX, index*mModeY);
		dest.OffsetBy(mColumn-2-mModeX, itemRect.top+(mLineSize-mModeY)/2);
		DrawBitmap(mModeBitmap, src, dest);
	}

	if (erase)
	{
		SetHighColor(255,255,255);
		BRect r = itemRect;
		r.left=mColumn;
		r.InsetBy(1,1);
		FillRect(r);
	}

	SetLowColor(255,255,255);
	SetHighColor(0,0,0);
	for(long t=1, x=(long)(mColumn+mTimeScale*20e3); x<itemRect.right; x+=(long)(mTimeScale*20e3), t+=1)
		StrokeLine(BPoint(x,itemRect.top+1), BPoint(x,itemRect.bottom-1), B_MIXED_COLORS);

	if (mode == kLineModeOn || mode == kLineModeRepeat)
	{
		BRect r = itemRect;
		r.bottom -= 3;
		r.top = r.bottom-8;
		r.left = mColumn+2+start * mTimeScale;
		if (mode == kLineModeRepeat) r.right = mColumn+2+(start+len) * mTimeScale;
		else r.right = itemRect.right-1; // mode on
		SetHighColor(0,102,203);
		FillRect(r);
		float px=r.left;
		float py=r.top-3;
		// start triangle "|/"
		SetHighColor(0,102,203);
		FillTriangle(BPoint(px,py), BPoint(px+3,py-3), BPoint(px-0,py-3));
		SetHighColor(100,100,100);
		StrokeTriangle(BPoint(px,py+1), BPoint(px+4,py-4), BPoint(px-0,py-4));

		if (mode == kLineModeRepeat)
		{
			px = r.right;
			// stop triangle "\|"
			SetHighColor(0,102,203);
			FillTriangle(BPoint(px,py), BPoint(px+0,py-3), BPoint(px-3,py-3));
			SetHighColor(100,100,100);
			StrokeTriangle(BPoint(px,py+1), BPoint(px+0,py-4), BPoint(px-4,py-4));
	
			x=mColumn+2+(start+repeat) * mTimeScale;
			// repeat triangle "|/"
			SetHighColor(102,203,203);
			FillTriangle(BPoint(x,py), BPoint(x+3,py-3), BPoint(x-0,py-3));
			SetHighColor(100,100,100);
			StrokeTriangle(BPoint(x,py+1), BPoint(x+4,py-4), BPoint(x-0,py-4));
			SetHighColor(102,203,203);
			for(; x<itemRect.right; x+=repeat*mTimeScale)
			{
				r.left = x;
				r.right = r.left+len*mTimeScale;
				FillRect(r);
			}
		}
	}

} // end of drawLine for CTrackView


//***********************************************************
void CTrackView::makeDefaultList(BList &filterList)
//***********************************************************
{
	//if (debug) printf(" --> CTrackView::makeDefaultList\n");

	bool first=true;
	if (!Window() || !Window()->Lock()) return;

	makeEmptyList();
	long nb=filterList.CountItems();
	for(long i=0; i<nb; i++)
	{
		CFilter *filter = (CFilter *)filterList.ItemAt(i);
		if (!filter) continue;

		if (debug) printf("   parsing filter name %s\n", filter->sFilter.name);

		if (0 && filter->sFilter.name
				&& (    strncmp(filter->sFilter.name, "Smo", 3) == 0
						|| strncmp(filter->sFilter.name, "FFT", 3) == 0
						|| strncmp(filter->sFilter.name, "Sco", 3) == 0
						|| strncmp(filter->sFilter.name, "Sch", 3) == 0))
		{
			SLineParam *param = new SLineParam;
			param->filter = filter;
			if (first) {param->mode = kLineModeOn; first=false;}
			if (gSimpleTrack->lock())
			{
				gSimpleTrack->addLine(param);
				gSimpleTrack->unlock();
			}
			mNbLine++;
		}
	}

	Invalidate();
	Window()->Unlock();
} // end of makeDefaultList


//***********************************************************
void CTrackView::makeEmptyList(void)
//***********************************************************
{
	//if (debug) printf(" --> CTrackView::makeEmptyList\n");
	if (gSimpleTrack->lock())
	{
		//if (debug) printf(" --> track locked\n");
		mNbLine = 0;
		long nb = gSimpleTrack->countLine();
		//if (debug) printf(" --> nb lines track %d\n",nb);
		if (nb > 0)
			for(long i=nb-1; i>=0; i--)
			{
				SLineParam *line = gSimpleTrack->delLine(i);
				if (line) delete line;
			}
		gSimpleTrack->unlock();
		//if (debug) printf(" --> track unlocked\n");
	}
	//else
	//	if (debug) printf(" --> CAN'T LOCK TRACK ?\n");

	//if (debug) printf(" --> END\n");
	updateList();
} // end of makeEmptyList


//***********************************************************
void CTrackView::updateList(void)
//***********************************************************
{
	//if (debug) printf(" --> CTrackView::updateList\n");
	if (Window() && Window()->Lock())
	{
		bounds = Bounds();
		mNbLine = gSimpleTrack->countLine();
		Invalidate();
		Window()->Unlock();
	}
} // end of updateList







//---------------------------------------------------------------------------

// eoc
