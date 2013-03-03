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
#include "CAddonList.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

//---------------------------------------------------------------------------

extern CPulsarApp *gApplication;

//---------------------------------------------------------------------------


//***********************************************************
CAddonListView::CAddonListView(BRect frame,
																const char *name,
																const char *filename,
																uint32 resizeMask, 
																uint32 flags)
				 			:BView(frame, name, resizeMask, flags),
				 			 mFont(be_plain_font)
//***********************************************************
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	mNbAddons = 0;

	bounds = Bounds();

	mImage = NULL;
	mMouseInside = false;
	mChangedLine = true;

	BDirectory app(gApplication->getAppDir());
	BDirectory dir(&app, K_IMG_DIR);
	BPath path(&dir, filename);

	mImage = gApplication->getBitmap(path.Path());

	if (mImage) mImageSize = mImage->Bounds();
	else gApplication->loadBitmapError(filename);
	mDemie = (mImageSize.IntegerHeight()+1)/2;
	mDemieSize = mImageSize;
	mDemieSize.bottom = mDemie-1;

	BPath path2(&dir, K_IMG_POSITION);
	mPositionBitmap = gApplication->getBitmap(path2.Path());
	if (mPositionBitmap)
	{
		mPositionSize = mPositionBitmap->Bounds();
		mPositionX = (long)((mPositionSize.Width()+1)/3);
		mPositionY = (long)(mPositionSize.Height()+1);
	}

	mFont.SetSize(10);
	mFont.SetSpacing(B_BITMAP_SPACING);
	//mFont.SetFlags(B_DISABLE_ANTIALIASING);
	mFont.GetHeight(&mFontHeight);
	mFontOffset = /*mFontHeight.ascent+*/mFontHeight.descent+mFontHeight.leading+2;

	mMouseCoord.Set(-1,-1);
	mOverIndex = -1;
} // end of constructor for CAddonListView


//***********************************************************
void CAddonListView::MouseDown(BPoint where)
//***********************************************************
{
	if (mOverIndex>=0)
	{
		CFilter *filter = (CFilter *)mAddons.ItemAt(mOverIndex);
		if (filter)
		{
			BMessage *msg = new BMessage(K_MSG_FILTER_INFO);
			msg->AddPointer(K_NAME_FILTER_INFO, (void *)filter);

			int32 count;

			// get click count
			Window()->CurrentMessage()->FindInt32("clicks", &count);

			//snooze(1e5);
			BPoint p;
			uint32 buttons;
			GetMouse(&p, &buttons);
			
			if (count>1)
			{
				be_app->PostMessage(msg);
				delete msg;
			}
			else
			{
				BBitmap *map = filter->sFilter.icon;
				if (!map)
					DragMessage(msg, BRect(p.x-16,p.y-16,p.x+16,p.y+16));
				else
				{
					BBitmap *map2 = new BBitmap(map->Bounds(), map->ColorSpace());
					if (!map2)
						DragMessage(msg, BRect(p.x-16,p.y-16,p.x+16,p.y+16));
					else
					{
						memcpy(map2->Bits(), map->Bits(), map2->BitsLength());
						DragMessage(msg, map2, BPoint(16,16));
					}
				}
				delete msg;
			}
		}
	}
} // end of MouseDown for CAddonListView


//***********************************************************
void CAddonListView::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
//***********************************************************
{
	long old = mOverIndex;
	bool changed;
	bool state = (code == B_INSIDE_VIEW && a_message == NULL);
	changed = (state != mMouseInside);
	if (state)
	{
		mMouseCoord = where;
		int32 over = (int32)where.y/(int32)mDemie;
		changed = changed || (over != mOverIndex);
		mOverIndex = over;
	}
	else
	{
		mMouseCoord.Set(-1,-1);
		mOverIndex = -1;
	}

	mChangedLine = changed;
	if (changed)
	{
		mMouseInside = state;
		if (Window() && Window()->Lock())
		{
			BRect r = mDemieSize;
			r.OffsetBy(0,old*mDemie);
			drawItem(r, old);
			if (old != mOverIndex)
			{
				BRect r = mDemieSize; //r(0,0,bounds.right-1,mDemie-1);
				r.OffsetBy(0,mOverIndex*mDemie);
				drawItem(r, mOverIndex);
			}
			Window()->Unlock();
		}
	}
} // end of MouseMoved for CAddonListView


//***********************************************************
void CAddonListView::Draw(BRect upd)
//***********************************************************
{
BRect r;
int32 yj;
int32 index;

	if (!mImage)return;

	upd.InsetBy(-1,-1);

	r = mDemieSize;
	index = (int32)(upd.top/mDemie);
	r.OffsetBy(0,index*mDemie);
	yj = (int32)(upd.bottom);

	for(; r.top < yj; index++, r.OffsetBy(0,mDemie)) drawItem(r, index);

} // end of Draw for CAddonListView


//***********************************************************
void CAddonListView::drawItem(BRect itemRect, int32 index)
//***********************************************************
{
int decal = (index == mOverIndex);

	SetDrawingMode(B_OP_COPY);
	BRect r(mDemieSize);
	r.OffsetBy(0.0, mDemie * decal);
	if (mImage) DrawBitmap(mImage, r, itemRect);

	if (index < mNbAddons)
	{
		CFilter *filter = (CFilter *)mAddons.ItemAt(index);
		if (!filter) return;

		char *s = filter->sFilter.name;
		if (!s) s = "(unnamed)";
		SetFont(&mFont);
		SetHighColor(20,20,40);
		SetLowColor(255,255,255);
		float x,y;
		x = 3; //(itemRect.Width()-StringWidth(s))/2;
		y = itemRect.bottom-mFontOffset;
		MovePenTo(x,y);
		DrawString(s);

		BBitmap *map = filter->sFilter.icon;
		if (map)
		{
			BRect rect = itemRect;
			rect.OffsetBy(K_ICON_PX, K_ICON_PY);
			rect.right = rect.left+31;
			rect.bottom = rect.top+31;
			DrawBitmap(map, rect);
		}

		if (mPositionBitmap)
		{
			long index=2;
			if (filter->sFilter.position < kFilterPositionLast/4) index = 0;
			else if (filter->sFilter.position < kFilterPositionLast*3/4) index = 1;
			BRect src(0, 0, mPositionX-1, mPositionY-1);
			BRect dest = src;
			src.OffsetBy(index*mPositionX, 0);
			dest.OffsetBy(itemRect.right-mPositionX-2, itemRect.top+20);
			DrawBitmap(mPositionBitmap, src, dest);
		}
	}

} // end of drawItem for CAddonListView


//***************************************************************************
void CAddonListView::updateAddonList(BList &filterList)
//***************************************************************************
{
	if (!Window() || !Window()->Lock()) return;
	mAddons = filterList;	// HOOK bof...
	mNbAddons = mAddons.CountItems();
	if (debug) printf("mNbAddons %d\n", mNbAddons);
	
	BScrollView *scroll = cast_as(Parent(), BScrollView);
	if (scroll)
	{
		BScrollBar *bar = scroll->ScrollBar(B_VERTICAL);
		if (bar)
		{
			bounds = Bounds();
			float vmax=mDemie*mNbAddons;
			ResizeTo(bounds.Width(), (vmax > mDemie/2 ? vmax : bounds.Height()));
			bounds = Bounds();
			vmax=bounds.Height()-mDemie;
			if (vmax<0) vmax=bounds.Height();
			if (debug) printf("%d addons, bar range %f, height step %f\n", mNbAddons, mDemie*(mNbAddons+1), vmax);
			bar->SetRange(0, mDemie*(mNbAddons+1));
			bar->SetSteps(mDemie, vmax);
		}
	}
	Invalidate();
	Window()->Unlock();

} // end of listFilters for CAddonListView


//***************************************************************************
void CAddonListView::FrameResized(float new_width, float new_height)
//***************************************************************************
{
	if (debug) printf("FrameResized\n");
	bounds = Bounds();
	if (debug) bounds.PrintToStream();

	BScrollView *scroll = cast_as(Parent(), BScrollView);
	if (scroll)
	{
		BScrollBar *bar = scroll->ScrollBar(B_VERTICAL);
		if (bar)
		{
			if (debug) printf("bar range %f, height step %f\n", mDemie*(mNbAddons+1), bounds.Height()-mDemie);
			bar->SetRange(0, mDemie*(mNbAddons+1));
			bar->SetSteps(mDemie, bounds.Height()-mDemie);
		}
	}

} // end of FrameResized for CAddonListView

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

//---------------------------------------------------------------------------

// eoc
