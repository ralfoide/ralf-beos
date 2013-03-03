/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CFontPatternView.cpp
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

#include "CFondPatternView.h"

//---------------------------------------------------------------------------

extern CPulsarApp *gApplication;

//---------------------------------------------------------------------------


//***********************************************************
CFondPatternView::CFondPatternView(BRect frame,
									char *title,
									long resize,
									const char *bitmapname,
									long flags)
	 			:BView(frame, title, resize, flags)
//***********************************************************
{
	SetViewColor(B_TRANSPARENT_32_BIT);

	mImage8 = NULL;
	mImage32 = NULL;
	mSx = 0;
	mSy = 0;

	BDirectory app(gApplication->getAppDir());
	BDirectory dir(&app, K_IMG_DIR);
	BPath path(&dir, bitmapname);
	mImage32 = gApplication->getBitmap(path.Path());

	if (mImage32)
	{
		BRect r = mImage32->Bounds();
		mSx = (uint32)(r.Width()+1);
		mSy = (uint32)(r.Height()+1);
	}
	else gApplication->loadBitmapError(bitmapname);
	mImage8 = gApplication->makeBitmap8Bit(mImage32);
	
	bounds = Bounds();
} // end of constructor for CFondPatternView


//***********************************************************
void CFondPatternView::Draw(BRect upd)
//***********************************************************
{
long ix,iy;
float x,y,fx;
float sx = mSx;
float sy = mSy;
float sx1 = 1/sx;
float sy1 = 1/sy;
BRect r = upd;
BBitmap *map;

	if (1)
	{
		BScreen screen;
		map = (screen.ColorSpace() == B_COLOR_8_BIT ? mImage8 : mImage32);
		if (!map || !upd.IsValid()) return;
	}

	if(1)
	{	
		BRegion region;
		GetClippingRegion(&region);

		upd.InsetBy(-1,-1);
		iy = (long)(r.top*sy1);
		ix = (long)(r.left*sx1);
		fx = ix*sx;
		for(y=iy*sy; y<r.bottom; y+=sy)
			for(x=fx; x<r.right; x+=sx)
				if (region.Intersects(BRect(x,y,x+sx-1,y+sy-1)))
					DrawBitmapAsync(map, BPoint(x,y));
		Sync();
	}
/*
	else
	{
		BRegion region;
		GetClippingRegion(&region);
	
		int32 n=region.CountRects();
		BRegion region2;
		for(int32 i=0; i<n; i++)
		{
			BRect r = region.RectAt(i);
			
			fx = sx * ((long)(r.top*sy1));
			y = sy * ((long)(r.left*sx1));
	
			for(; y<=r.bottom; y+=sy)
				for(x=fx; x<=r.right; x+=sx)
				{
					BRect r2(x,y,x+sx-1,y+sy-1);
					if (!region2.Intersects(r2))
					{
						region2.Include(r2);
						DrawBitmapAsync(map, r2.LeftTop());
			
						//	 SetHighColor(255,0,0);
						// StrokeRect(r2);
					}
				}
	
		}
		Sync();
	}
*/
} // end of Draw for CFondPatternView


//***********************************************************
void CFondPatternView::MouseDown(BPoint where)
//***********************************************************
{
int32 count;

	// get click count
	Window()->CurrentMessage()->FindInt32("clicks", &count);

	if (count == 1)
	{
	/*
		BPoint location;
		uint32 buttons;
		
		snooze(100e3);	// wait 10 ms... arbitrary HOOK use define
		GetMouse(&location, &buttons);
		if (!buttons)
	*/
		{
			Window()->Activate();
		}
	}
} // end of MouseDown for CFondPatternView


//***********************************************************
void CFondPatternView::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
//***********************************************************
{
/*	if (code == B_ENTERED_VIEW)
		M_SET_MOVE_CURSOR;
	else if (code == B_EXITED_VIEW)
		M_RESET_CURSOR;
	else if (code == B_INSIDE_VIEW)
	{
		BPoint location;
		uint32 buttons=1;

		GetMouse(&where, &buttons);
		ConvertToScreen(&where);
		while(buttons)
		{
			snooze(40000);
			GetMouse(&location, &buttons, TRUE);
			ConvertToScreen(&location);
			if (buttons && location != where)
			{
				BPoint location2 = location;
				location2 -= where;
				Window()->MoveBy(location2.x, location2.y);
				where = location;
			}
			else
			{
				Draw(bounds);
				//break;
			}
		}
	}
*/
} // end of MouseMoved for CFondPatternView


//---------------------------------------------------------------------------

// eoc
