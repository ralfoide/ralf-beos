/*****************************************************************************

	Projet	: Pulsar

	Fichier:	CFilterInfoWin.h
	Date		:	200797

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#include "CFilterInfoWin.h"
#include "CPulsarApp.h"

#if K_USE_DATATYPE
#include "Datatypes.h"
#include "GetBitmap.h"		/// in /boot/develop/Datatypes152/headers/glue/...
#endif

#include <Path.h>

//---------------------------------------------------------------------------

//#define K_EXT ".png"
#define K_EXT ".tif"	// HOOK RM 161198 -- ".png"

#define K_IMG_DIR						"img_pulsar"
#define K_IMG_FOND						"fondparam1" K_EXT

#define K_WIN_TOP		24
#define K_WIN_BORDER	 5

#define K_SETTING_OFFSET_X	(mSx+5)
#define K_SETTING_OFFSET_Y	(50+15*5+10)

//---------------------------------------------------------------------------

extern CPulsarApp *gApplication;

const char CFilterInfoView::infolabel[6][20] =
{
	"Version",
	"Author",
	"Info",
	"Class",
	"Mode",
	"Enabled"
};


//---------------------------------------------------------------------------


//***********************************************************
CFilterInfoView::CFilterInfoView(BRect frame,
														char *title,
														long resize,
														const char *bitmapname,
														long flags)
				 			:BView(frame, title, resize, flags),
				 			 mTitleFont(be_bold_font),
				 			 mNameFont(be_plain_font),
				 			 mTextFont(be_plain_font)
#define inherited BView
//***********************************************************
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	mFilter = NULL;
	mFilterRect.Set(0,0,0,0);
	mFilterView = NULL;

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

	mTitleFont.SetSize(18);
	mNameFont.SetSize(10);
	mTextFont.SetSize(10);
	//mPosText = mNameFont.StringWidth("Version");	// largest string

	maxInfolabelLen=0;
	for(long i=0; i<6; i++)
	{
		infolabelLen[i]=(long)(mNameFont.StringWidth(infolabel[i]));
		if (infolabelLen[i]>maxInfolabelLen) maxInfolabelLen = infolabelLen[i];
	}
	mPosText = maxInfolabelLen;

	bounds = Bounds();
	referenceBounds = bounds;
} // end of constructor for CFilterInfoView


//***********************************************************
void CFilterInfoView::Draw(BRect upd)
//***********************************************************
{
long iy;
float x,y;
float sy;
BRect r;
BScreen screen;
BBitmap *map;

	map = (screen.ColorSpace() == B_COLOR_8_BIT ? mImage8 : mImage32);
	if (!map || !upd.IsValid()) return;

	SetDrawingMode(B_OP_COPY);

	upd.InsetBy(-1,-1);
	r = upd;
	sy = mSy;
	iy = (long)(r.top/sy);
	for(y=iy*sy; y<r.bottom; y+=sy)
		DrawBitmapAsync(map, BPoint(0.0,y));
	Sync();

	r = upd;
	SetHighColor(255,255,255);
	r.left = mSx;
	if (r.IsValid()) FillRect(r);

	x = (mSx-36)/2;
	y = 5.0;
	BRect d(0,0,33,33);
	d.OffsetBy(x,y);
	SetHighColor(160,160,160);
	FillRect(d);
	SetHighColor(30,30,30);
	StrokeLine(BPoint(x+1,y+34), BPoint(x+1,y+1));
	StrokeLine(BPoint(x+34,y+1));
	SetHighColor(220,220,220);
	StrokeLine(BPoint(x+1,y+34), BPoint(x+34,y+34));
	StrokeLine(BPoint(x+34,y+1));

	if (mFilter && mFilter->sFilter.icon)
	{
		BRect s(0,0,31,31);
		BRect d(s);
		d.OffsetBy(x+2,y+2);
		DrawBitmap(mFilter->sFilter.icon, s,d);
	}

char *sm=NULL, *sa, *si, *sc;
char sv[32]="";
char so[64]="";

	if (mFilter) sm = mFilter->sFilter.name;

	//SetDrawingMode(B_OP_COPY);
	SetLowColor(255,255,255);
	SetHighColor(0,0,0);
	SetFont(&mTitleFont);
	MovePenTo(mSx+5, 20);
	DrawString(sm ? sm : "(unnamed filter)");

	SetHighColor(203,51,51);
	SetFont(&mNameFont);
	x = mSx+5;
	y = 50-15;
	for(long i=0; i<6; i++)
	{
		MovePenTo(x+maxInfolabelLen-infolabelLen[i], y+=15);
		DrawString(infolabel[i]);
	}
/*
	MovePenTo(x,y);			DrawString("Version");
	MovePenTo(x,y+=15);	DrawString("Author");
	MovePenTo(x,y+=15);	DrawString("Info");
	MovePenTo(x,y+=15);	DrawString("Class");
	MovePenTo(x,y+=15);	DrawString("Mode");
	MovePenTo(x,y+=15);	DrawString("Enabled");
*/
	if (mFilter)
	{
		sprintf(sv, "v %d.%d", mFilter->sFilter.majorVersion, mFilter->sFilter.minorVersion);
		sa = mFilter->sFilter.author;
		if (!sa) sa = "";
		si = mFilter->sFilter.info;
		if (!si) si = "";
		sc = (char *)class_name(mFilter);
		if (!sc) sc="";
		if (mFilter->sFilter.supportedMode == kColor8Bit)
			strcat(so, "8 bpp");
		else if (mFilter->sFilter.supportedMode == kRgb32Bit)
			strcat(so, "32 bpp");
		else if (mFilter->sFilter.supportedMode == (EColorSpace)(kColor8Bit | kRgb32Bit))
			strcat(so, "8 and 32 bpp");
		else
			strcat(so, "unknown bpp");
		if (mFilter->sFilter.eraseScreen)
			strcat(so, ", erase screen");

		SetHighColor(0,0,0);	
		SetFont(&mTextFont);
		x+= mPosText+10;
		y = 50;
		MovePenTo(x,y);			DrawString(sv);
		MovePenTo(x,y+=15);	DrawString(sa);
		MovePenTo(x,y+=15);	DrawString(si);
		MovePenTo(x,y+=15);	DrawString(sc);
		MovePenTo(x,y+=15);	DrawString(so);
		MovePenTo(x,y+=15);	DrawString(mFilter->sLoad.enabled ? "Yes" : "No");
	}

	if(!mFilterView)
	{
		x = K_SETTING_OFFSET_X;
		y = K_SETTING_OFFSET_Y;
		float x1=bounds.right-5;
		float y1=bounds.bottom-5;
		SetHighColor(30,30,30);
		StrokeLine(BPoint(x,y1), BPoint(x,y));
		StrokeLine(BPoint(x1,y));
		SetHighColor(220,220,220);
		StrokeLine(BPoint(x,y1), BPoint(x1,y1));
		StrokeLine(BPoint(x1,y));
		SetHighColor(180,180,180);
		FillRect(BRect(x+1,y+1,x1-1,y1-1));

		SetHighColor(0,0,0);
		SetLowColor(180,180,180);
		SetFont(&mTextFont);
		MovePenTo(x+5,y1-10);
		DrawString("(This place for free) -- " K_APP_NAME);
	}

} // end of Draw for CFilterInfoView


//***********************************************************
void CFilterInfoView::DetachedFromWindow(void)
//***********************************************************
{
	// remove old settings view if any
	if (mFilterView && mFilter)
	{
		mFilter->settingsClosed();
		mFilterView = NULL;
		mFilter = NULL;
	}
	inherited::DetachedFromWindow();
} // end of DetachedFromWindow for CFilterInfoView


//***********************************************************
void CFilterInfoView::setFilter(CFilter *filter)
//***********************************************************
{
	if (Window() && Window()->Lock())
	{
		// remove old settings view if any
		if (mFilterView && mFilter != filter)
		{
			RemoveChild(mFilterView);
			if (mFilter) mFilter->settingsClosed();
			mFilterView = NULL;
			mFilter = NULL;
		}

		// and add new one if any
		if (filter && mFilter != filter && filter->sFilter.hasSettings)
		{
			mFilterView = filter->settingsOpened();
			if (mFilterView)
			{
				mFilterRect = mFilterView->Bounds();
				int32 x = (int32)(mFilterRect.Width());
				int32 y = (int32)(mFilterRect.Height());
				if (x < referenceBounds.Width()) x = (int32)(referenceBounds.Width());
				if (y < referenceBounds.Height()) y = (int32)(referenceBounds.Height());
				mFilterView->ResizeTo(x,y);
				mFilterView->MoveTo(K_SETTING_OFFSET_X, K_SETTING_OFFSET_Y);
				Window()->ResizeTo(K_SETTING_OFFSET_X+x+K_WIN_BORDER*2,
												 K_SETTING_OFFSET_Y+y+K_WIN_TOP+K_WIN_BORDER);
				AddChild(mFilterView);
			}
		}

		// keep new filter
		mFilter = filter;

		Invalidate();
		Window()->Unlock();

		if (filter && Window()->IsHidden()) Window()->Show();
		else if (filter) Window()->Activate();
		else if (!filter && !Window()->IsHidden()) Window()->Hide();
	}
} // end of setFilter for CFilterInfoView


//---------------------------------------------------------------------------
#pragma mark -



//***************************************************************************
CFilterInfoWin::CFilterInfoWin(BRect frame,
											 		const char *title, 
											 		window_type type,
											 		ulong flags,
											 		ulong workspace)
						 :BWindow(frame, title, type, flags, workspace)
#undef inherited
#define inherited BWindow
//***************************************************************************
{
	mInfo = NULL;
} // end of constructor for CFilterInfoWin


//***************************************************************************
CFilterInfoWin::~CFilterInfoWin(void)
//***************************************************************************
{
} // end of destructor for CFilterInfoWin


//---------------------------------------------------------------------------


//***************************************************************************
bool CFilterInfoWin::init(void)
//***************************************************************************
{
BRect bounds=Bounds();

	if (debug) printf("CInterfaceWin::init\n");

	// add buttons and others

	mInfo = new CFilterInfoView(bounds, "infoview", B_FOLLOW_ALL, K_IMG_FOND);
	if (mInfo) AddChild(mInfo);
	if (!mInfo) return false;

	// -- set limits
	float min_h, max_h, min_v, max_v;
	GetSizeLimits(&min_h, &max_h, &min_v, &max_v);
	min_h = bounds.Width();
	min_v = bounds.Height();
	SetSizeLimits(min_h, max_h, min_v, max_v);

	//Show();
	Run();
	return true;
} // end of init for CFilterInfoWin


//---------------------------------------------------------------------------

//***************************************************************************
void CFilterInfoWin::MessageReceived(BMessage *message)
//***************************************************************************
{
	switch(message->what)
	{
		default:
			//message->PrintToStream();
			inherited::MessageReceived(message);
			break;
	}

} // end of MessageReceived for CFilterInfoWin


//***************************************************************************
bool CFilterInfoWin::QuitRequested(void)
//***************************************************************************
{
	Hide();
	return false;
} // end of QuitRequested for CFilterInfoWin


//---------------------------------------------------------------------------


//***********************************************************
void CFilterInfoWin::setFilter(CFilter *filter)
//***********************************************************
{
	if (mInfo) mInfo->setFilter(filter);
} // end of setFilter for CFilterInfoWin

//---------------------------------------------------------------------------

// eoc
