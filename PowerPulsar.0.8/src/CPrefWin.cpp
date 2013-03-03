/*****************************************************************************

	Projet	: Pulsar

	Fichier:	CPrefWin.h
	Date		:	251097

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#include "CPrefWin.h"
#include "CPulsarApp.h"

#if K_USE_DATATYPE
#include "Datatypes.h"
#include "GetBitmap.h"		/// in /boot/develop/Datatypes152/headers/glue/...
#endif

#include <Path.h>

//---------------------------------------------------------------------------

//#define K_EXT 							".png"
#define K_EXT ".tif"	// HOOK RM 161198 -- 							".png"

#define K_IMG_DIR						"img_pulsar"
#define K_IMG_FOND						"pref" K_EXT

#define K_WIN_TOP		24
#define K_WIN_BORDER	 5

#define	K_IMG_TITLE_BAR		48
#define	K_NB_SETTING_VIEW	2
#define	K_NB_SETTING_SX		72

#define	K_IMG_REPEAT_ZONE_Y	96
#define	K_IMG_REPEAT_ZONE_SY	16

#define	K_IMG_END_ZONE_Y			(96+16)
#define	K_IMG_END_ZONE_SY		16

//---------------------------------------------------------------------------

extern CPulsarApp *gApplication;

//---------------------------------------------------------------------------


//***********************************************************
CPrefVideoView::CPrefVideoView(BRect frame,
															 char *title,
															 long resize,
															 long flags)
				 			:BView(frame, title, resize, flags)
#define inherited BView
//***********************************************************
{
	SetViewColor(220,220,220);

	mOffscreenSingle = NULL;
	mOffscreenDouble = NULL;
	mOffscreenTripple = NULL;
	mOnBoardSingle = NULL;
	mOnBoardDouble = NULL;
	mOnBoardTripple = NULL;;
	mOnBoardMemory = NULL;
	mOffscreenMemory = NULL;

	mMatroxSupport = NULL;
	mUmaxSupport = NULL;

	uint32 w=(uint32)(frame.Width()-5);
	uint32 h=(uint32)(frame.Height()-2);
	BBox *box1 = new BBox(BRect(3,0,w,38), "Memory");
	BBox *box2 = new BBox(BRect(3,40,w/2-2,40+70), "OnBoard_Buffering");
	BBox *box3 = new BBox(BRect(w/2+2,40,w,40+70), "Offscreen_Buffering");
	box1->SetLabel("Memory");
	box2->SetLabel("OnBoard Buffering");
	box3->SetLabel("Offscreen Buffering");

	AddChild(box1);
	AddChild(box2);
	AddChild(box3);

	mOnBoardMemory = new BRadioButton(BRect(5,15,w/2,30), "onboard", "OnBoard",
																		new BMessage(K_MSG_PVV_DEF));
	mOffscreenMemory = new BRadioButton(BRect(w/2+2,15,w-7,30), "offscreen", "Offscreen",
																			new BMessage(K_MSG_PVV_DEF));
	mOffscreenSingle = new BRadioButton(BRect(5,15,w/2-7,30), "single", "Single",
																			new BMessage(K_MSG_PVV_DEF));
	mOffscreenDouble = new BRadioButton(BRect(5,30,w/2-7,45), "double", "Double",
																			new BMessage(K_MSG_PVV_DEF));
	mOffscreenTripple = new BRadioButton(BRect(5,45,w/2-7,60), "tripple", "Trippe",
																			new BMessage(K_MSG_PVV_DEF));
	mOnBoardSingle = new BRadioButton(BRect(5,15,w/2-7,30), "single", "Single",
																		new BMessage(K_MSG_PVV_DEF));
	mOnBoardDouble = new BRadioButton(BRect(5,30,w/2-7,45), "double", "Double",
																		new BMessage(K_MSG_PVV_DEF));
	mOnBoardTripple = new BRadioButton(BRect(5,45,w/2-7,60), "tripple", "Trippe",
																		 new BMessage(K_MSG_PVV_DEF));
	mOnBoardMemory->SetTarget(this);
	mOffscreenMemory->SetTarget(this);
	mOffscreenSingle->SetTarget(this);
	mOffscreenDouble->SetTarget(this);
	mOffscreenTripple->SetTarget(this);
	mOnBoardSingle->SetTarget(this);
	mOnBoardDouble->SetTarget(this);
	mOnBoardTripple->SetTarget(this);
	box1->AddChild(mOnBoardMemory);
	box1->AddChild(mOffscreenMemory);
	box2->AddChild(mOffscreenSingle);
	box2->AddChild(mOffscreenDouble);
	box2->AddChild(mOffscreenTripple);
	box3->AddChild(mOnBoardSingle);
	box3->AddChild(mOnBoardDouble);
	box3->AddChild(mOnBoardTripple);

	h = 40+75+16;
	mMatroxSupport = new BCheckBox(BRect(8,h-16,w/2-2,h), "matrox", "Matrox Support",
														 			new BMessage(K_MSG_PVV_DEF));
	mUmaxSupport = new BCheckBox(BRect(w/2+2,h-16,w,h), "umax", "Umax Support",
													 			new BMessage(K_MSG_PVV_DEF));
	mMatroxSupport->SetTarget(this);
	mUmaxSupport->SetTarget(this);

	AddChild(mMatroxSupport);
	AddChild(mUmaxSupport);

} // end of constructor for CPrefVideoView


//***********************************************************
void CPrefVideoView::MessageReceived(BMessage *msg)
//***********************************************************
{
	if (msg && msg->what == K_MSG_PVV_DEF && Window()->Lock())
	{
		getValue();
		Window()->Unlock();
	}
	else
		inherited::MessageReceived(msg);
} // end of MessageReceived for CPrefVideoView


//***********************************************************
void CPrefVideoView::setValue(void)
//***********************************************************
{
	#define M_CVV_SET(p,v) if (p) p->SetValue(v)

	M_CVV_SET(mOnBoardMemory,		gApplication->mPulsarPref.mMemory == kScreenMemoryOnBoard);
	M_CVV_SET(mOffscreenMemory,	gApplication->mPulsarPref.mMemory == kScreenMemoryOffscreen);
	M_CVV_SET(mOffscreenSingle,	gApplication->mPulsarPref.mOffscreenBuffering == kScreenBufferingSingle);
	M_CVV_SET(mOffscreenDouble,	gApplication->mPulsarPref.mOffscreenBuffering == kScreenBufferingDouble);
	M_CVV_SET(mOffscreenTripple,gApplication->mPulsarPref.mOffscreenBuffering == kScreenBufferingTripple);
	M_CVV_SET(mOnBoardSingle,		gApplication->mPulsarPref.mOnBoardBuffering == kScreenBufferingSingle);
	M_CVV_SET(mOnBoardDouble,		gApplication->mPulsarPref.mOnBoardBuffering == kScreenBufferingDouble);
	M_CVV_SET(mOnBoardTripple,	gApplication->mPulsarPref.mOnBoardBuffering == kScreenBufferingTripple);
	M_CVV_SET(mMatroxSupport,		gApplication->mPulsarPref.mMatroxHack);
	M_CVV_SET(mUmaxSupport,			gApplication->mPulsarPref.mUmaxHack);

} // end of setValue for CPrefVideoView


//***********************************************************
void CPrefVideoView::getValue(void)
//***********************************************************
{
	#define M_CVV_GET(v,p) if (p) v = p->Value()
	#define M_CVV_GET_2(v,p1,c1,c2) if (p1) v = (p1->Value() ? c1 : c2)
	#define M_CVV_GET_3(v,p1,p2,c1,c2,c3) if (p1 && p2) v = (p1->Value() ? c1 : (p2->Value() ? c2 : c3))

	M_CVV_GET_2(gApplication->mPulsarPref.mMemory,
							mOnBoardMemory,
							kScreenMemoryOnBoard,
							kScreenMemoryOffscreen);
	M_CVV_GET_3(gApplication->mPulsarPref.mOffscreenBuffering,
							mOffscreenSingle,
							mOffscreenDouble,
							kScreenBufferingSingle,
							kScreenBufferingDouble,
							kScreenBufferingTripple);
	M_CVV_GET_3(gApplication->mPulsarPref.mOnBoardBuffering,
							mOnBoardSingle,
							mOnBoardDouble,
							kScreenBufferingSingle,
							kScreenBufferingDouble,
							kScreenBufferingTripple);
	M_CVV_GET(gApplication->mPulsarPref.mMatroxHack,mMatroxSupport);
	M_CVV_GET(gApplication->mPulsarPref.mUmaxHack,	mUmaxSupport);

} // end of getValue for CPrefVideoView

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -


//***********************************************************
CPrefOtherView::CPrefOtherView(BRect frame,
															 char *title,
															 long resize,
															 long flags)
				 			:BView(frame, title, resize, flags)
//***********************************************************
{
	SetViewColor(220,220,220);
	
	mLoadDefaultConfig = NULL;
	mDisplayFrameRate  = NULL;
	mDisplayGkSup 		 = NULL;

	uint32 w=(uint32)(frame.Width()-5);
	mLoadDefaultConfig = new BCheckBox(BRect(5,5,w-5,20), "ldfc", "Load Default Config At Startup",
														 			new BMessage(K_MSG_POV_DEF));
	mDisplayFrameRate =  new BCheckBox(BRect(5,20,w-5,35), "dfr", "Display Frame Rate",
														 			new BMessage(K_MSG_POV_DEF));
	mDisplayGkSup = 		 new BCheckBox(BRect(5,35,w-5,50), "dgs", "Use Gk Sup Interface",
														 			new BMessage(K_MSG_POV_DEF));

	AddChild(mLoadDefaultConfig);
	AddChild(mDisplayFrameRate);
	AddChild(mDisplayGkSup);

} // end of constructor for CPrefOtherView


//***********************************************************
void CPrefOtherView::MessageReceived(BMessage *msg)
//***********************************************************
{
	if (msg && msg->what == K_MSG_POV_DEF && Window()->Lock())
	{
		getValue();
		Window()->Unlock();
	}
	else
		inherited::MessageReceived(msg);
} // end of MessageReceived for CPrefOtherView


//***********************************************************
void CPrefOtherView::setValue(void)
//***********************************************************
{
	#define M_COV_SET(p,v) if (p) p->SetValue(v)

	M_COV_SET(mLoadDefaultConfig,	gApplication->mPulsarPref.mLoadDefaultConfig);
	M_COV_SET(mDisplayFrameRate,	gApplication->mPulsarPref.mDisplayFrameRate);
	M_COV_SET(mDisplayGkSup,			gApplication->mPulsarPref.mDisplayGkSup);

} // end of setValue for CPrefOtherView


//***********************************************************
void CPrefOtherView::getValue(void)
//***********************************************************
{
	#define M_COV_GET(v,p) if (p) v = p->Value()

	M_COV_GET(gApplication->mPulsarPref.mLoadDefaultConfig,mLoadDefaultConfig);
	M_COV_GET(gApplication->mPulsarPref.mDisplayFrameRate, mDisplayFrameRate);
	M_COV_GET(gApplication->mPulsarPref.mDisplayGkSup,		 mDisplayGkSup);

} // end of getValue for CPrefOtherView

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -


//***********************************************************
CPrefView::CPrefView(BRect frame,
									char *title,
									long resize,
									const char *bitmapname,
									long flags)
				 			:BView(frame, title, resize, flags)
//***********************************************************
{
	SetViewColor(B_TRANSPARENT_32_BIT);

	mCurrentView = NULL;
	mCurrentSetting = 0;
	mNbSetting = K_NB_SETTING_VIEW;
	mSettingZoneSx = K_NB_SETTING_SX;
	mSettingZoneSy = K_IMG_TITLE_BAR;

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
		mSx = (uint32)r.Width()+1;
		mSy = (uint32)r.Height()+1;
	}
	else gApplication->loadBitmapError(bitmapname);
	mImage8 = gApplication->makeBitmap8Bit(mImage32);

	bounds = Bounds();
	referenceBounds = bounds;

	// create sub views
	settingBounds = bounds;
	settingBounds.InsetBy(14,14);
	settingBounds.left--;
	settingBounds.top = K_IMG_TITLE_BAR-11;

	// views
	CPrefVideoView *view1 = new CPrefVideoView(settingBounds, "pref_video", B_FOLLOW_ALL, B_WILL_DRAW);
	CPrefOtherView *view2 = new CPrefOtherView(settingBounds, "pref_others", B_FOLLOW_ALL, B_WILL_DRAW);

	mSetting.AddItem(view1);
	mSetting.AddItem(view2);

	mCurrentView = view1;
	AddChild(mCurrentView);

} // end of constructor for CPrefView


//***********************************************************
void CPrefView::MouseDown(BPoint where)
//***********************************************************
{
	if (where.y <= mSettingZoneSy)
	{
		uint32 newset = (uint32)where.x / mSettingZoneSx;
		if (newset != mCurrentSetting && newset < mNbSetting && Window()->Lock())
		{
			mCurrentSetting = newset;
			Draw(bounds);

			if (mCurrentView) RemoveChild(mCurrentView);
			mCurrentView = (BView *)mSetting.ItemAt(newset);
			if (mCurrentView) AddChild(mCurrentView);

			Window()->Unlock();
		}
	}
} // end of MouseDown for CPrefView


//***********************************************************
void CPrefView::Draw(BRect upd)
//***********************************************************
{
BRect r;
BBitmap *map;
BRegion region;

	GetClippingRegion(&region);	

	if(1)
	{
		BScreen screen;
		map = (screen.ColorSpace() == B_COLOR_8_BIT ? mImage8 : mImage32);
		if (!map || !upd.IsValid()) return;
	}

	SetDrawingMode(B_OP_COPY);

	// draw top

	r = bounds;
	r.bottom = K_IMG_TITLE_BAR-1;
	if (1)//region.Intersects(r))
	{
		BRect r2(r);
		r2.OffsetBy(0,mCurrentSetting * K_IMG_TITLE_BAR);
		DrawBitmap(map, r2, r);
	}

	// draw middle
	
	//r = bounds;
	//r.top = K_IMG_TITLE_BAR;
	//r.bottom -= K_IMG_END_ZONE_SY;
	if(1)
	{
		BRect r(0,K_IMG_TITLE_BAR,mSx-1,bounds.bottom-K_IMG_END_ZONE_SY);
		BRect r2(0, K_IMG_REPEAT_ZONE_Y, mSx-1, K_IMG_REPEAT_ZONE_Y+K_IMG_REPEAT_ZONE_SY-1);
		DrawBitmap(map, r2, r);
	}

	// draw bottom
	
	r = bounds;
	r.top = r.bottom-K_IMG_END_ZONE_SY;
	if(1)//region.Intersects(r))
	{
		BRect r2(r);
		r2.top = K_IMG_END_ZONE_Y;
		r2.bottom = mSy;
		DrawBitmap(map, r2, r);
	}


} // end of Draw for CPrefView



//***********************************************************
void CPrefView::MessageReceived(BMessage *msg)
//***********************************************************
{
	if (msg && msg->what == K_MSG_PVV_DEF && mCurrentView)
		mCurrentView->MessageReceived(msg);
	else
		inherited::MessageReceived(msg);
} // end of MessageReceived for CPrefView



//---------------------------------------------------------------------------
#pragma mark -



//***************************************************************************
CPrefWin::CPrefWin(SWinPref &pref,
								const char *title, 
								window_type type,
								ulong flags,
								ulong workspace)
				:CWindow(pref, title, type, flags, workspace)
#undef inherited
#define inherited CWindow
//***************************************************************************
{
	mView = NULL;
} // end of constructor for CPrefWin


//***************************************************************************
CPrefWin::~CPrefWin(void)
//***************************************************************************
{
} // end of destructor for CPrefWin


//---------------------------------------------------------------------------


//***************************************************************************
bool CPrefWin::init(void)
//***************************************************************************
{
BRect bounds=Bounds();

	printf("FLAG DEBUG %d",debug);
	printf("CPrefWin::init -- sans debug\n");

	// setup CWindow behavior
	mInitShow							= false;
	mQuitRequestSavePref		= true;
	mPostQuitApp					= false;
	mQuitRequestReturnVal	= false;

	// add buttons and others

	mView = new CPrefView(bounds, "prefview", B_FOLLOW_ALL, K_IMG_FOND);
	if (!mView) return false;
	AddChild(mView);

/*
	// -- set limits
	float min_h, max_h, min_v, max_v;
	GetSizeLimits(&min_h, &max_h, &min_v, &max_v);
	min_h = bounds.Width();
	min_v = bounds.Height();
	SetSizeLimits(min_h, max_h, min_v, max_v);
*/

	return inherited::init();
} // end of init for CPrefWin


//---------------------------------------------------------------------------

//***************************************************************************
bool CPrefWin::QuitRequested(void)
//***************************************************************************
{
	Hide();
	return inherited::QuitRequested();
} // end of QuitRequested for CPrefWin


//***********************************************************
void CPrefWin::MessageReceived(BMessage *msg)
//***********************************************************
{
	if (msg && msg->what == K_MSG_PVV_DEF && mView)
		mView->MessageReceived(msg);
	else
		inherited::MessageReceived(msg);
} // end of MessageReceived for CPrefWin



//---------------------------------------------------------------------------

// eoc
