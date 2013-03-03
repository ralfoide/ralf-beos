/*****************************************************************************

	Projet		: Pulsar

	Fichier	: CSplatchWin.cpp
	Partie		: Loader

	Auteur		: RM
	Date			: 140297
	Format		: tabs==2

*****************************************************************************/

#include "globals.h"
#include "CSplatchWin.h"
#include "CPulsarApp.h"
#include "CPulsarMsg.h"

#if K_USE_DATATYPE
#include "Datatypes.h"
#include "GetBitmap.h"		/// in /boot/develop/Datatypes152/headers/glue/...
#endif

#include <Path.h>

#include "move_cursor.h"

//---------------------------------------------------------------------------

#define K_EXT ".tif"	// HOOK RM 161198 -- ".png"
#define K_EXT2 ".tga"	// HOOK RM 161198 -- K_EXT

#define K_IMG_DIR					"img_pulsar"
#define K_IMG_BAR_NAME			"barrevertdef" K_EXT
#define K_IMG_SPLATCH_NAME	"power" K_EXT
#define K_IMG_INFOS_NAME		"info" K_EXT
#define K_IMG_SPLATCH_TEXT	"textabout" K_EXT2

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------


//*******************************
class CSplatchView : public BView
//*******************************
{
public:
	CSplatchView(BRect frame,
							 char *title,
							 long resize,
							 const char *bitmapname,
							 bool scrolltext=true,
							 long flags=B_WILL_DRAW|B_PULSE_NEEDED);
	~CSplatchView(void) { if (mImage32) delete mImage32; if (mImage8) delete mImage8; }
	void MouseMoved(BPoint where, uint32 code, const BMessage *a_message);
	void Draw(BRect upd);
	void adjustColorSpace(void);
	void Pulse(void);
	void getSize(uint32 &x, uint32 &y) { x=mSx; y=mSy; }
	void forceText(void) { mForceText = true; }
	void startLoading(void) { mIsLoading = true; }
	void endLoading(void) { mIsLoading = false; }

private:

	void makeScrollText(void);

	bool			mIsScrollText;
	bool			mIsLoading;
	bool 		mForceText;
	BRect		mBounds;
	BBitmap	*mScrollText;
	BBitmap *mImage, *mImage32, *mImage8;
	uint32	 mSx, mSy;
	int32		 mYOffsetScroll, mScrollSy;
	BRect		 mScrollBounds;
};



//***********************************************************
CSplatchView::CSplatchView(BRect frame,
														char *title,
														long resize,
														const char *bitmapname,
							 							bool scrolltext,
														long flags)
				 			:BView(frame, title, resize, flags)
//***********************************************************
{
	SetViewColor(B_TRANSPARENT_32_BIT);

	mScrollText = NULL;
	mImage = NULL;
	mImage8 = NULL;
	mImage32 = NULL;
	mSx = 0;
	mSy = 0;
	mYOffsetScroll = 0;
	mScrollSy = 0;
	mForceText = false;
	mBounds = Bounds();
	mIsScrollText = scrolltext;
	mIsLoading = true;

	BDirectory app(gApplication->getAppDir());
	BDirectory dir(&app, K_IMG_DIR);
	BPath path(&dir, bitmapname);
	mImage32 = gApplication->getBitmap(path.Path());

	if (mImage32)
	{
		BRect r = mImage32->Bounds();
		mSx = (uint32)r.Width();
		mSy = (uint32)r.Height();

		mImage8 = gApplication->makeBitmap8Bit(mImage32);
	}
	else gApplication->loadBitmapError(bitmapname);

	adjustColorSpace();
	if(scrolltext)makeScrollText();
} // end of constructor for CSplatchView


//***********************************************************
void CSplatchView::makeScrollText(void)
//***********************************************************
{
	// load text bitmap
	BDirectory app(gApplication->getAppDir());
	BDirectory dir(&app, K_IMG_DIR);
	BPath path(&dir, K_IMG_SPLATCH_TEXT);
	mScrollText = gApplication->getBitmap(path.Path());
	if (!mScrollText) { 	gApplication->loadBitmapError(K_IMG_SPLATCH_TEXT); return; }
	if (mScrollText->ColorSpace() != B_COLOR_8_BIT)
	{
		// make 8 bit bitmap from 32 bit fix one.
		// transform 255/255/255 into TRANSPARENT_8_BIT...
		BRect r = mScrollText->Bounds();
		BBitmap *map = new BBitmap(r, B_COLOR_8_BIT);
		if (!map)
		{
			delete mScrollText;
			mScrollText = NULL;
			return;
		}

		uchar *sourceBits = (uchar *)mScrollText->Bits();
		int32	  sourceLen = mScrollText->BitsLength();
		uchar *destBits = (uchar *)map->Bits();
		int32  destLen  = map->BitsLength();
		//long sourceBpr = source->BytesPerRow();

		color_map *cmap = (color_map *)system_colors();

		long i,j;
		for(i=0, j=0; i<sourceLen & j<destLen; j++, i+=4)
		{
			uchar b = *(sourceBits++);
			uchar g = *(sourceBits++);
			uchar r = *(sourceBits++);
			sourceBits++;

			if (b == 255 && g == 255 && r == 255)
			{
				*(destBits++) = B_TRANSPARENT_8_BIT;
			}
			else
			{
				ulong rgb15 = (  ((r & 0xf8) << 7) |    
 	      	  	        	((g & 0xf8) << 2) | 
   	      	  	      	((b & 0xf8) >> 3) );
				*(destBits++) = cmap->index_map[rgb15];
			}
		}

		delete mScrollText;
		mScrollText = map;
	}
	mScrollBounds = mScrollText->Bounds();
	if (debug) printf("mScrollBounds : "); mScrollBounds.PrintToStream();
	mYOffsetScroll = 0;
	mScrollSy = (int32)mScrollBounds.bottom /*-36*/;
}


//***********************************************************
void CSplatchView::adjustColorSpace(void)
//***********************************************************
{
	BBitmap *oldImage = mImage;
	BScreen screen(Window());

	//get_screen_info(&info);
	if (screen.ColorSpace() == B_COLOR_8_BIT) mImage = mImage8;
	else mImage = mImage32;

	if (Window()->Lock())
	{
		if (mImage != oldImage) Invalidate();
		Window()->Unlock();
	}
}

//***********************************************************
void CSplatchView::Draw(BRect upd)
//***********************************************************
{
	if (mImage)
	{
		SetDrawingMode(B_OP_COPY);
		DrawBitmap(mImage, BPoint(0.0,0.0));
	}
	if (mIsLoading)
	{
		BFont afont(be_plain_font);
		afont.SetSize(42);
		font_height h;
		afont.GetHeight(&h);
		SetDrawingMode(B_OP_OVER);
		afont.SetSpacing(B_BITMAP_SPACING);
		char s1[] = "Please Wait !";
		char s2[] = "Loading !";
		SetHighColor(240,0,0);
		SetFont(&afont);
		long l1 = (long)afont.StringWidth(s1);
		long l2 = (long)afont.StringWidth(s2);
		long h2 = mSy/2;
		long x2 = mSx/2;
		MovePenTo(x2-l1/2, h2-h.descent-h.leading);
		DrawString(s1);
		MovePenTo(x2-l2/2, h2+h.ascent);
		DrawString(s2);
	}
	else if (mScrollText)
	{
		if (mForceText)
		{
			SetDrawingMode(B_OP_OVER);
			BRect r(0,0,mScrollBounds.right,mScrollSy);
			BRect dest(r);
			dest.OffsetBy(5.0,0.0);
			DrawBitmap(mScrollText, r,dest);
		}
		else if (mYOffsetScroll < 2*mScrollSy)
		{
			BRect source;
			if (mYOffsetScroll > mScrollSy)
				source.Set(0,mYOffsetScroll-mScrollSy,mScrollBounds.right,mScrollSy);
			else
				source.Set(0,0,mScrollBounds.right,mYOffsetScroll);
			BRect dest(source);
			dest.OffsetBy(5.0,0.0);
			SetDrawingMode(B_OP_OVER);
			DrawBitmap(mScrollText, source, dest);
		}
	}
} // end of Draw for CSplatchView


//***********************************************************
void CSplatchView::Pulse(void)
//***********************************************************
{
	if (mScrollText && !mIsLoading)
	{
		if (mForceText)
		{
			SetDrawingMode(B_OP_OVER);
			DrawBitmap(mScrollText, BPoint(5.0,0.0));
		}
		else if (mYOffsetScroll++ > 4*mScrollSy)
		{
			mYOffsetScroll = 0;
			Draw(mBounds);
		}
		else if (mYOffsetScroll < 2*mScrollSy)
		{
			BRect source;
			if (mYOffsetScroll > mScrollSy)
			{
				//source.Set(0,mYOffsetScroll-mScrollSy,mScrollBounds.right,mScrollSy);
				source.Set(0,0,mScrollBounds.right,mYOffsetScroll-mScrollSy);
				source.OffsetBy(5.0,0.0);
				if (mImage) DrawBitmap(mImage, source, source);
			}
			else
			{
				source.Set(0,0,mScrollBounds.right,mYOffsetScroll);
				BRect dest(source);
				dest.OffsetBy(5.0,0.0);
				SetDrawingMode(B_OP_OVER);
				DrawBitmap(mScrollText, source, dest);
			}
		}
	}
} // end of Pulse for CSplatchView


//***********************************************************
void CSplatchView::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
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
			else break;
		}
	}
*/
} // end of MouseMoved for CSplatchView


//---------------------------------------------------------------------------
#pragma mark -


//*******************************
class CVertBarView : public BView
//*******************************
{
public:
	CVertBarView(BRect frame,
							 char *title,
							 long resize,
							 const char *bitmapname,
							 long flags=B_WILL_DRAW | B_PULSE_NEEDED);
	~CVertBarView(void) { if (mImage) delete mImage; }

	void Pulse(void);
	void Draw(BRect upd);

	void getSize(uint32 &x, uint32 &y) { x=mSx; y=mSy; }
	void FrameResized(float new_width, float new_height)
		{ bounds = Bounds(); viewH = (uint32)bounds.Height(); }

private:
	BBitmap *mImage;
	uint32	 mSx, mSy;
	int32		 mOffset;
	BRect		 bounds;
	uint32	 viewH;
};



//***********************************************************
CVertBarView::CVertBarView(BRect frame,
														char *title,
														long resize,
														const char *bitmapname,
														long flags)
				 			:BView(frame, title, resize, flags)
//***********************************************************
{
	SetViewColor(B_TRANSPARENT_32_BIT);

	mImage = NULL;
	mSx = 0;
	mSy = 0;
	mOffset = 0;

	BDirectory app(gApplication->getAppDir());
	BDirectory dir(&app, K_IMG_DIR);
	BPath path(&dir, bitmapname);
	mImage = gApplication->getBitmap(path.Path());

	if (mImage)
	{
		BRect r = mImage->Bounds();
		mSx = (uint32)r.Width();
		mSy = (uint32)r.Height();
	}
	else gApplication->loadBitmapError(bitmapname);

	bounds = Bounds();
	viewH = (uint32)bounds.Height();

} // end of constructor for CVertBarView


//***********************************************************
void CVertBarView::Draw(BRect upd)
//***********************************************************
{
	if (!mImage) return;

	float y=mSy;
	y=mOffset-y;
	DrawBitmap(mImage, BPoint(0.0,y));
	DrawBitmap(mImage, BPoint(0.0,mOffset));
	
} // end of Draw for CVertBarView


//***********************************************************
void CVertBarView::Pulse(void)
//***********************************************************
{
	mOffset-=1;
	if (mOffset<0) mOffset+=mSy;
	Draw(bounds);
	
} // end of Pulse for CVertBarView


//---------------------------------------------------------------------------
#pragma mark -


//***************************************************************************
CSplatchWin::CSplatchWin(SWinPref &pref,
											const char *title, 
											window_type type,
											ulong flags,
											ulong workspace)
						 :CWindow(pref, title, type, flags, workspace)
#define inherited CWindow
//***************************************************************************
{
	mImage = NULL;
	mBar = NULL;

} // end of constructor for CSplatchWin


//***************************************************************************
CSplatchWin::~CSplatchWin(void)
//***************************************************************************
{
	if (debug) printf("CSplatchWin::~CSplatchWin -- destructor\n");
} // end of destructor for CSplatchWin


//***************************************************************************
bool CSplatchWin::init(bool aboutbox)
//***************************************************************************
{
BRect b;
BRect rect;
uint32 xb,yb;
uint32 xs,ys;
//screen_info screen;
BScreen screen;

	// setup CWindow behavior
	mInitShow							= true;
	mQuitRequestSavePref		= true;
	mPostQuitApp					= false;
	mQuitRequestReturnVal	= false;
	mAutoSaveFramePref			= false;

	mIsLoading = true;

	b = Bounds();
	rect = b;

	mBar = new CVertBarView(rect, "bar", B_FOLLOW_NONE, K_IMG_BAR_NAME);
	if (!mBar) return false;
	mBar->getSize(xb,yb);

	rect.left = xb+1;
	mImage = new CSplatchView(rect, "splatch", B_FOLLOW_NONE, (aboutbox ? K_IMG_SPLATCH_NAME : K_IMG_INFOS_NAME), aboutbox);
	if (!mImage) return false;
	mImage->getSize(xs,ys);

	AddChild(mBar);
	AddChild(mImage);

	mBar->ResizeTo(xb,ys);
	mImage->ResizeTo(xs,ys);

	//get_screen_info(&screen);
	rect = screen.Frame();
	xs+=xb;		// xs * ys -> taille desiree de l'ecran
	#define YBUTTON 24
	ys += (aboutbox ? YBUTTON : 0);
	xb = (uint32)(rect.Width()-xs)/2;
	yb = (uint32)(rect.Height()-ys)/2;

	MoveTo(xb,yb);
	ResizeTo(xs,ys);
	
	if (aboutbox)
	{
		SetPulseRate(10000);
		rect.Set(-1,ys-YBUTTON,xs+1,ys);
		char s[64];
		sprintf(s, "More Info about " K_APP_NAME " v%d.%d", K_MAJOR_PULSAR_VERSION, K_MINOR_PULSAR_VERSION);
		if (K_QUANTIC_PULSAR_VERSION > 0)
		{
			char s2[16];
			sprintf(s2, ".%d", K_QUANTIC_PULSAR_VERSION);
			strcat(s, s2);
		}
		strcat(s, B_UTF8_ELLIPSIS);
		BButton *button = new BButton(rect, "moreinfo", s, new BMessage(K_MSG_ABOUT2));
		if (button)
		{
			AddChild(button);
			button->ResizeTo(xs+2, YBUTTON+1);
		}
		else ResizeTo(xs,ys-YBUTTON);
	}
	else
	{
		//SetPulseRate(10000);
		SetPulseRate(1e16);
		mInitShow = false;
	}
	return inherited::init();

} // end of init for CSplatchWin


//---------------------------------------------------------------------------


//***************************************************************************
void CSplatchWin::DispatchMessage(BMessage *message, BHandler *handler)
//***************************************************************************
{
	if (message->what == K_MSG_ABOUT2)
	{
		be_app->PostMessage(K_MSG_ABOUT2);
	}
	else if (!IsHidden() || message->what != B_PULSE)
	{
		// dispatch message.
		// Don't dispatch PULSE when window is NOT visible
		inherited::DispatchMessage(message, handler);
	}
} // end of DispatchMessage for CSplatchWin


//***************************************************************************
bool CSplatchWin::QuitRequested(void)
//***************************************************************************
{
	forceText();
	Hide();
	return inherited::QuitRequested();
} // end of QuitRequested for CSplatchWin


//***************************************************************************
void CSplatchWin::ScreenChanged(BRect screen_size, color_space depth)
//***************************************************************************
{
	mImage->adjustColorSpace();
	inherited::ScreenChanged(screen_size, depth);
} // end of ScreenChanged for CSplatchWin


//***************************************************************************
void CSplatchWin::WorkspacesChanged(uint32 old_ws, uint32 new_ws)
//***************************************************************************
{
	mImage->adjustColorSpace();
	inherited::WorkspacesChanged(old_ws, new_ws);
} // end of WorkspacesChanged for CSplatchWin


//***************************************************************************
void CSplatchWin::forceText(void)
//***************************************************************************
{
	if (mImage) mImage->forceText();
} // end of forceText for CSplatchWin


//***************************************************************************
void CSplatchWin::endLoading(void)
//***************************************************************************
{
	mIsLoading = false;
	if (Lock())
	{
		if (mImage) mImage->endLoading();
		if (mImage) mImage->Invalidate();
		Unlock();
	}
	
} // end of endLoading for CSplatchWin


//***************************************************************************
void CSplatchWin::startLoading(void)
//***************************************************************************
{
	mIsLoading = true;
	if (Lock())
	{
		if (mImage) mImage->startLoading();
		if (mImage) mImage->Invalidate();
		Unlock();
	}
} // end of endLoading for CSplatchWin


//---------------------------------------------------------------------------

// eoc
