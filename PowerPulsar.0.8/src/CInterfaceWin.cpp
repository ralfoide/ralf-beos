/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CInterfaceWin.cpp
	Partie	: Loader

	Auteur	: RM
	Date	: 140297
	Format	: tabs==4

*****************************************************************************/

#include "CInterfaceWin.h"
#include "CPulsarApp.h"
#include "CPulsarMsg.h"			// messages defines herein

#if K_USE_DATATYPE
#include "Datatypes.h"
#include "GetBitmap.h"		/// in /boot/develop/Datatypes152/headers/glue/...
#endif

#include <Path.h>

#include "CDBits.h"				// nice icons from DR8 CD Player
#include "p_trash.h"				// trash icon from DR9 Tracker

#define _DEFINE_MOVE_CURSOR_
#include "move_cursor.h"

#include "CFondPatternView.h"
#include "CButton.h"
#include "CAddonList.h"
#include "CSimpleTrack.h"

//---------------------------------------------------------------------------

extern CPulsarApp *gApplication;

//---------------------------------------------------------------------------


//*************************
class CTrash : public BView
//*************************
{
public:
				CTrash(BRect frame,
						char *title,
						long resize,
						long flags=B_WILL_DRAW);
virtual			~CTrash(void) { if (mImage) delete mImage; }

virtual	void	Draw(BRect upd);
virtual	void	MouseMoved(BPoint where, uint32 code, const BMessage *a_message);
virtual	void	MessageReceived(BMessage *message);

private:

	bool mHasDrop;
	BRect source;
	BRect bounds;
	BBitmap *mImage;
};



//****************************************
CTrash::CTrash(BRect frame,
				char *title,
				long resize,
				long flags)
	   :BView(frame, title, resize, flags)
//****************************************
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	//SetViewColor(200,200,200);

	M_MAKE_TRASH_BITMAP(mImage);
	mHasDrop = false;
	bounds = Bounds();
	source.Set(0,0,K_TRASH_SX-1,K_TRASH_SY-1);

} // end of constructor for CTrash


//**************************
void CTrash::Draw(BRect upd)
//**************************
{
	SetDrawingMode(B_OP_COPY);
	if (mHasDrop)	SetHighColor(255,0,51);
	else			SetHighColor(80,80,80);
	StrokeLine(BPoint(0,bounds.bottom), BPoint(0,0));
	StrokeLine(BPoint(bounds.right,0));
	if (mHasDrop)	SetHighColor(255,255,102);
	else			SetHighColor(245,245,245);
	StrokeLine(BPoint(bounds.right,bounds.bottom));
	StrokeLine(BPoint(1,bounds.bottom));
	if (mHasDrop)
	{
		SetHighColor(200,200,200);
		FillRect(BRect(1,1,K_TRASH_SX,K_TRASH_SY));
		SetDrawingMode(B_OP_BLEND);
	}
	if (mImage) DrawBitmap(mImage, BPoint(2,2));

} // end of Draw for CTrash


//*********************************************************************
void CTrash::MouseMoved(BPoint where, uint32 code, const BMessage *msg)
//*********************************************************************
{
	bool state;
	state = (code == B_INSIDE_VIEW
			&& msg && msg->what == K_MSG_FILTER_INFO
			&&  msg->HasInt32(K_NAME_FILTER_OVER));
	if (state != mHasDrop)
	{
		mHasDrop = state;
		Draw(bounds);
	}
} // end of MouseMoved for CTrash


//*****************************************
void CTrash::MessageReceived(BMessage *msg)
//*****************************************
{
	if (msg->what == K_MSG_FILTER_INFO
		&& msg->HasInt32(K_NAME_FILTER_OVER))
	{
		int32 over;
		msg->FindInt32(K_NAME_FILTER_OVER, &over);
		BMessage msg2(K_MSG_DEL_LINE_PARAM);
		msg2.AddInt32(K_NAME_FILTER_OVER, over);
		Window()->PostMessage(&msg2);
	}
} // end of MessageReceived for CTrash


//---------------------------------------------------------------------------
#pragma mark -

//***************************
class CCdTable : public BView
//***************************
{
public:
			CCdTable(BRect frame,
					 char *title,
					 const char *bitmapname,
					 long resize,
					 long flags=B_WILL_DRAW);
virtual			~CCdTable(void)
				{ /*if (mImage8) delete mImage8;
				  if (mImage32) delete mImage32; */
				  for(long i=0; i<10; i++) if (mTrackLed[i]) delete mTrackLed[i];
				}
		void	Draw(BRect upd);
		void	displayTrack(CCdButton *play);

private:

		BRect 	source;
		BRect 	bounds;
		BBitmap *mImage32;
		BBitmap *mImage8;
		uint32	 mSx, mSy;
	
		BBitmap *mTrackLed[10];
		int32	mLastTrackNum;
		bool	mLastCdPlaying;
};


//*****************************************
CCdTable::CCdTable(BRect frame,
				   char *title,
				   const char *bitmapname,
				   long resize,
				   long flags)
		:BView(frame, title, resize, flags)
//*****************************************
{
	//SetViewColor(B_TRANSPARENT_32_BIT);
	SetViewColor(200,200,200);

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
		source = mImage32->Bounds();
		mSx = (uint32)(source.Width()+1);
		mSy = (uint32)(source.Height()+1);
	}
	else gApplication->loadBitmapError(bitmapname);
	//mImage8 = gApplication->makeBitmap8Bit(mImage32);
	mImage8 = NULL;

	bounds = Bounds();

	for(long i=0; i<10; i++) mTrackLed[i] = NULL;

	// init mTrackLed[0..9]
	BRect rect(0, 0, LED_WIDTH, LED_HEIGHT);
	unsigned char *bits;
	#define M_SET_TRACK_LED(x) bits = led_ ## x;																		\
		mTrackLed[ x ] = new BBitmap(rect, B_COLOR_8_BIT);														\
		mTrackLed[ x ]->SetBits((char*)bits, mTrackLed[ x ]->BitsLength(), 0, B_COLOR_8_BIT)

	// ---

	M_SET_TRACK_LED(0);
	M_SET_TRACK_LED(1);
	M_SET_TRACK_LED(2);
	M_SET_TRACK_LED(3);
	M_SET_TRACK_LED(4);
	M_SET_TRACK_LED(5);
	M_SET_TRACK_LED(6);
	M_SET_TRACK_LED(7);
	M_SET_TRACK_LED(8);
	M_SET_TRACK_LED(9);

	#undef M_SET_TRACK_LED	

	mLastCdPlaying = false;
	mLastTrackNum = 0;
	
	// ---

} // end of constructor for CCdTable


//****************************
void CCdTable::Draw(BRect upd)
//****************************
{
	displayTrack(NULL);

/*
BScreen screen;
BBitmap *map;

	map = (screen.ColorSpace() == B_COLOR_8_BIT ? mImage8 : mImage32);
	if (!map || !upd.IsValid()) return;

	DrawBitmap(map, source, bounds);
*/
//	if (mImage32) DrawBitmap(mImage32, source, bounds);
//	displayTrack(NULL);

/*	
	SetDrawingMode(B_OP_COPY);
	SetHighColor(200,200,200);
	FillRect(upd);
*/
} // end of Draw for CCdTable


//---------------------------------------------------------------------------
#pragma mark -



//********************************************************
CInterfaceWin::CInterfaceWin(SWinPref &pref,
							const char *title, 
						 	window_type type,
						 	ulong flags,
							ulong workspace)
			 :CWindow(pref, title, type, flags, workspace)
#define inherited CWindow
//********************************************************
{
	mFond = NULL;
	mQuitBtn = NULL;
	mPlayBtn = NULL;
	mStopBtn = NULL;
	mCurrFps = NULL;

	mFullBtn = NULL;
	mFullStartBtn = NULL;
	mPreviewBtn = NULL;
	mCdBtn = NULL;
	mMicBtn = NULL;
	mAiffBtn = NULL;
	mMp3Btn = NULL;
	mModBtn = NULL;

	mCdTable = NULL;
	mCdPlay = NULL;
	mCdStop = NULL;
	mCdEject = NULL;
	mCdNext = NULL;
	mCdPrev = NULL;
	mCdIndex = NULL;

	mMenuBar = NULL;
	mMenuFile = NULL;
	mMenuEdit = NULL;
	mMenuStream = NULL;
	mMenuCd = NULL;
	mMenuView = NULL;

	mBpp = 8;
	mMustSaveAs = true;
	mLoadPanel = NULL;
	mSavePanel = NULL;
	mMyFileRefFilter = new CMyFileRefFilter;

} // end of constructor for CInterfaceWin


//*********************************
CInterfaceWin::~CInterfaceWin(void)
//*********************************
{
	if (debug) printf("CInterfaceWin::~CInterfaceWin -- destructor\n");
} // end of destructor for CInterfaceWin


//---------------------------------------------------------------------------


//****************************
bool CInterfaceWin::init(void)
//****************************
{
	BRect rect;
	BRect rbounds;

	// setup CWindow behavior
	mInitShow				= true;
	mQuitRequestSavePref	= true;
	mPostQuitApp			= true;
	mQuitRequestReturnVal	= false;


	// add buttons and others
	// quite boring stuff

	float min_h, max_h, min_v, max_v;
	GetSizeLimits(&min_h, &max_h, &min_v, &max_v);
	min_h = 512;
	min_v = 384;
	SetSizeLimits(min_h, max_h, min_v, max_v);


	int hmenu = createMenu();

	// ---

	rbounds = mBounds;
	rbounds.top = hmenu;
	mFond = new CFondPatternView(rbounds, "fond", B_FOLLOW_ALL, K_IMG_FOND);
	if (mFond) AddChild(mFond);
	if (!mFond) return false;

	// ---

	rbounds = mFond->Bounds();

	// 53*25 buttons

	rect.Set(10,10,10+53,10+25);
	mQuitBtn = new CButton(rect, "quit", "", K_IMG_BOUTON_QUIT, new BMessage(K_MSG_QUIT));
	if (mQuitBtn) mFond->AddChild(mQuitBtn);

	CButton *dummy;
	rect.OffsetBy(0,30);
	dummy = new CButton(rect, "about", "", K_IMG_BOUTON_ABOUT, new BMessage(K_MSG_ABOUT));
	if (dummy) mFond->AddChild(dummy);

	rect.OffsetBy(0,30);
	dummy = new CButton(rect, "info", "", K_IMG_BOUTON_INFOS, new BMessage(K_MSG_ABOUT2));
	if (dummy) mFond->AddChild(dummy);

	// 73*25 buttons

	rect.Set(70,10,70+73,10+25);
	dummy = new CButton(rect, "new", "", K_IMG_BOUTON_NEW, new BMessage(K_MSG_NEW));
	if (dummy) mFond->AddChild(dummy);

	rect.OffsetBy(0,30);
	dummy = new CButton(rect, "load", "", K_IMG_BOUTON_LOAD, new BMessage(K_MSG_LOAD));
	if (dummy) mFond->AddChild(dummy);

	rect.OffsetBy(0,30);
	dummy = new CButton(rect, "save", "", K_IMG_BOUTON_SAVE, new BMessage(K_MSG_SAVE));
	if (dummy) mFond->AddChild(dummy);

/*
	rect.OffsetBy(80,-30);
	dummy = new CButton(rect, "start", "Start", K_IMG_BOUTON_DEFAULT, new BMessage(K_MSG_START_BTN));
	if (dummy) mFond->AddChild(dummy);

	rect.OffsetBy(0,30);
	dummy = new CButton(rect, "stop", "Stop", K_IMG_BOUTON_DEFAULT, new BMessage(K_MSG_STOP_BTN));
	if (dummy) mFond->AddChild(dummy);

	rect.OffsetBy(0,30);
	dummy = new CButton(rect, "Full 32", "Full Screen 32", K_IMG_BOUTON_DEFAULT, new BMessage(K_MSG_FULLSCREEN32_BTN));
	if (dummy) mFond->AddChild(dummy);
*/

	rect.Set(150,10,150+K_INPUT_SX,10+K_INPUT_SY);
	mCdBtn = new CButton(rect, "CD", "", K_IMG_BOUTON_CD, new BMessage(K_MSG_CDINPUT_BTN));
	if (mCdBtn) mFond->AddChild(mCdBtn);
mCdBtn->SetEnabled(false);
	rect.OffsetBy(0,K_INPUT_SY);
	mMicBtn = new CButton(rect, "Mic", "", K_IMG_BOUTON_MIC, new BMessage(K_MSG_MICINPUT_BTN));
	if (mMicBtn) mFond->AddChild(mMicBtn);
mMicBtn->SetEnabled(false);
	rect.Set(150+K_INPUT_SX,10,150+2*K_INPUT_SX,10+K_INPUT_SY);
	mAiffBtn = new CButton(rect, "Aiff", "", K_IMG_BOUTON_AIFF, new BMessage(K_MSG_AIFFINPUT_BTN));
	if (mAiffBtn) mFond->AddChild(mAiffBtn);
mAiffBtn->SetEnabled(false);
	rect.OffsetBy(0,K_INPUT_SY);
	mMp3Btn = new CButton(rect, "Mp3", "", K_IMG_BOUTON_MP3, new BMessage(K_MSG_MP3INPUT_BTN));
	if (mMp3Btn) mFond->AddChild(mMp3Btn);
	mMp3Btn->SetEnabled(false);
mMp3Btn->SetEnabled(false);

	mCdBtn->SetValue(1);

	rect.Set(40+K_ADDON_SX, rbounds.bottom-K_PLAY_SY-10, 40+K_ADDON_SX+K_PLAY_SX, rbounds.bottom-10);
	mFullBtn = new CButton(rect, "Full Screen", "", K_IMG_BOUTON_FULL, new BMessage(K_MSG_FULLSCREEN_BTN), B_FOLLOW_BOTTOM|B_FOLLOW_LEFT);
	if (mFullBtn) mFond->AddChild(mFullBtn);
	rect.OffsetBy(K_PLAY_SX+5, 0);
	mFullStartBtn = new CButton(rect, "Full 32", "", K_IMG_BOUTON_START, new BMessage(K_MSG_FULLSCREEN_BTN), B_FOLLOW_BOTTOM|B_FOLLOW_LEFT);
	if (mFullStartBtn) mFond->AddChild(mFullStartBtn);
	if (mFullStartBtn) mFullStartBtn->SetEnabled(false);
	rect.OffsetBy(K_PLAY_SX+5, 0);
	mPreviewBtn = new CButton(rect, "Preview", "", K_IMG_BOUTON_PREVIEW, new BMessage(K_MSG_PREVIEW_BTN), B_FOLLOW_BOTTOM|B_FOLLOW_LEFT);
	if (mPreviewBtn) mFond->AddChild(mPreviewBtn);

	// ---

	float xt=40+K_ADDON_SX+K_PLAY_SX*4+20;
	rect.Set(xt,rbounds.bottom-10-K_TRASH_SY-2+1,xt+K_TRASH_SX+2-1,rbounds.bottom-10);
	CTrash *trash = new CTrash(rect, "trash", B_FOLLOW_BOTTOM|B_FOLLOW_LEFT);
	if (trash) mFond->AddChild(trash);
	
	// ---
	rect.Set(10, 20+3*30, 10+K_ADDON_SX-1, rbounds.bottom-10);
	mAddonList = new CAddonListView(rect, "addonlist", K_IMG_ADDONLISTITEM);
	if (!mAddonList) return false;
	BScrollView *scroll = new BScrollView("scrollfilter", mAddonList,
																					B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,
																					0, FALSE, TRUE,
																					B_PLAIN_BORDER); //B_NO_BORDER); //B_FANCY_BORDER - B_PLAIN_BORDER);
	if (scroll) mFond->AddChild(scroll);

	// ---

	rect.Set(40+K_ADDON_SX+100, 20+3*30-20, rbounds.right-20, 20+3*30-2);
	//rect.Set(40+K_ADDON_SX+100, 25+K_INPUT_SY*2, rbounds.right-20, 20+K_INPUT_SY*2+24);
	CTrackHeader *head = new CTrackHeader(rect, "track");
	if (head) mFond->AddChild(head);

	rect.Set(40+K_ADDON_SX, 20+3*30/*25+K_INPUT_SY*2+25*/, rbounds.right-20, rbounds.bottom-K_PLAY_SY-20-2*30);
	mTrack = new CTrackView(rect, "track");
	if (!mTrack) return false;
	scroll = new BScrollView("scrolltrack", mTrack,
							B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM,
							0, FALSE, TRUE,
							B_PLAIN_BORDER); //B_NO_BORDER); //B_FANCY_BORDER - B_PLAIN_BORDER);
	if (scroll) mFond->AddChild(scroll);

	// ---


	float xr = 40+K_ADDON_SX;
	float yr = rbounds.bottom-K_PLAY_SY-10;
	rect.Set(xr, yr-20-mFullBtn->Bounds().Height(), xr+K_PLAY_SX*4+10, yr-10);
	BView *back; // fond is img 483*108
	back = new BView(rect, "fondinfo", B_FOLLOW_BOTTOM|B_FOLLOW_LEFT, B_WILL_DRAW);
	if (!back) return false;
	back->SetViewColor(200,200,200);
	mFond->AddChild(back);

	rect.Set(3,3,105,K_PLAY_SY-5);
	BPopUpMenu *mMenuIdeal;
	mMenuIdeal = new BPopUpMenu("Ideal Frame Rate");
	if (!mMenuIdeal) return false;
	//mMenuIdeal->SetTargetForItems(be_app);
	BMessage *msg = new BMessage(K_MSG_IDEAL_RATE);
	msg->AddInt32("rate", 1);
	mMenuIdeal->AddItem(new BMenuItem("1 Hz",	msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 5);
	mMenuIdeal->AddItem(new BMenuItem("5 Hz",	msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 15);
	mMenuIdeal->AddItem(new BMenuItem("15 Hz",	msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 25);
	mMenuIdeal->AddItem(new BMenuItem("25 Hz", 	msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 30);
	BMenuItem *item = new BMenuItem("30 Hz", 	msg);
	//item->SetMarked(true);
	mMenuIdeal->AddItem(item);
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 42);
	item = new BMenuItem("42 Hz", 	msg);
	item->SetMarked(true);
	mMenuIdeal->AddItem(item);
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 60);
	mMenuIdeal->AddItem(new BMenuItem("60 Hz", 	msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 75);
	mMenuIdeal->AddItem(new BMenuItem("75 Hz", 	msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 110);
	mMenuIdeal->AddItem(new BMenuItem("110 Hz", msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 150);
	mMenuIdeal->AddItem(new BMenuItem("150 Hz", msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 220);
	mMenuIdeal->AddItem(new BMenuItem("220 Hz", msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 380);
	mMenuIdeal->AddItem(new BMenuItem("380 Hz", msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 603);
	mMenuIdeal->AddItem(new BMenuItem("603 Hz", msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 604);
	mMenuIdeal->AddItem(new BMenuItem("604 Hz", msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 4040);
	mMenuIdeal->AddItem(new BMenuItem("A4040 Hz",msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 44100);
	mMenuIdeal->AddItem(new BMenuItem("44100 Hz",msg));
	msg = new BMessage(K_MSG_IDEAL_RATE);		msg->AddInt32("rate", 68040);
	mMenuIdeal->AddItem(new BMenuItem("68040 Hz",msg));
	BMenuField *rate;
	rate = new BMenuField(rect, "ideal", "Refresh", mMenuIdeal);
	if (rate) rate->SetDivider(40);
	if (rate) back->AddChild(rate);
	
	rect.OffsetBy(110,0);
	rect.right = rect.left+125;
	mMenuBpp = new BPopUpMenu("Colors");
	if (!mMenuBpp) return false;
	item = new BMenuItem("8-Bits/Pixel", 	new BMessage(K_MSG_8_BPP));
	if (item) item->SetMarked(true);
	mMenuBpp->AddItem(item);
	item = new BMenuItem("16-Bits/Pixel", 	new BMessage(K_MSG_16_BPP));
	item->SetEnabled(false);
	mMenuBpp->AddItem(item);
	item = new BMenuItem("32-Bits/Pixel", 	new BMessage(K_MSG_32_BPP));
	//if (item) item->SetEnabled(false);
	mMenuBpp->AddItem(item);
	item = new BMenuItem("128-Bits/Pixel",	new BMessage(K_MSG_128_BPP));
	if (item) item->SetEnabled(false);
	mMenuBpp->AddItem(item);
	BMenuField *bpp;
	bpp = new BMenuField(rect, "bpp", "Colors", mMenuBpp);
	if (bpp) bpp->SetDivider(40);
	if (bpp) back->AddChild(bpp);

	rect.OffsetBy(130,0);
	BPopUpMenu *mMenuFftWin;
	mMenuFftWin = new BPopUpMenu("FFT");
	if (!mMenuFftWin) return false;
	for(int i=0; i<=K_NB_FFT_WINDOW; i++)
	{
		msg = new BMessage(K_MSG_FFT_WINDOW);
		if (!msg) continue;
		msg->AddInt32(K_NAME_CONFIG_INDEX, i);
		item = new BMenuItem(gFftWindowName[i], msg);
		if (!item) continue;
		item->SetTarget(gApplication);
		mMenuFftWin->AddItem(item);
		if (i==0) item->SetMarked(true);
	}
	BMenuField *fftwin;
	fftwin = new BMenuField(rect, "fftwin", "FFT Window", mMenuFftWin);
	if (fftwin) fftwin->SetDivider(60);
	if (fftwin) back->AddChild(fftwin);


	rect.OffsetBy(130,0);
	rect.right = K_PLAY_SX*3+10-3;
	mCurrFps = new BStringView(rect, "CurrFps", "fps : n/a", B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW | B_PULSE_NEEDED);
	if (mCurrFps) back->AddChild(mCurrFps);

	// ---

	createCdInterface(mFond);
	SetPulseRate(1e6);

	// ---

	return inherited::init();
} // end of init for CInterfaceWin


//*********************************
int CInterfaceWin::createMenu(void)
//*********************************
{
	BRect r = mBounds;
	BMenuItem *item;
	BMenu *mFileConfig;
	BMenu *mFftWindow;

	mMenuBar = new BMenuBar(r, "menuplan");
	if (!mMenuBar) throw("mem alloc plan menubar");

	mMenuFile 	= new BMenu("File");
	mMenuEdit 	= new BMenu("Edit");
	mMenuView 	= new BMenu("View");
	mMenuInput 	= new BMenu("Input");
	mMenuStream = new BMenu("Stream");
	mMenuCd 	= new BMenu("Cd");
	mMenuNetwork= new BMenu("Network");
	mMenuAddons = new BMenu("Add-ons");
	mFftWindow  = new BMenu("FFT Window");

	mFileConfig = new BMenu("Preconfigured");

	if (!mMenuFile || !mMenuEdit || !mMenuView || !mMenuInput || !mFileConfig || !mFftWindow
		 || !mMenuStream || !mMenuCd || !mMenuNetwork || !mMenuAddons) throw("mem alloc menu");
	mMenuBar->AddItem(mMenuFile);
	mMenuBar->AddItem(mMenuEdit);
	mMenuBar->AddItem(mMenuView);
	// mMenuBar->AddItem(mMenuInput); -- RM 021400
	mMenuBar->AddItem(mMenuCd);
	// mMenuBar->AddItem(mMenuNetwork);  -- RM 021400
	mMenuBar->AddItem(mMenuAddons);

	BMessage *msg;
	for(int i=0; i<10; i++)
	{
		msg = new BMessage(K_MSG_LOAD_CONFIG);
		msg->AddInt32(K_NAME_CONFIG_INDEX, i);
		char s[16];
		sprintf(s, "Config %d", i);
		item = new BMenuItem(s, msg, (int)('0'+i));
		item->SetTarget(gApplication);
		mFileConfig->AddItem(item);
	}

	for(int i=0; i<=K_NB_FFT_WINDOW; i++)
	{
		msg = new BMessage(K_MSG_FFT_WINDOW);
		msg->AddInt32(K_NAME_CONFIG_INDEX, i);
		item = new BMenuItem(gFftWindowName[i], msg);
		item->SetTarget(gApplication);
		mFftWindow->AddItem(item);
	}

	#define K_DUMMY 'dUmY'
	mMenuFile->AddItem(new BMenuItem("New", new BMessage(K_MSG_NEW), 'N'));
	item = new BMenuItem("Open" B_UTF8_ELLIPSIS, new BMessage(K_MSG_LOAD), 'O');
	mMenuFile->AddItem(item);
	item = new BMenuItem("Save" B_UTF8_ELLIPSIS, new BMessage(K_MSG_SAVE), 'S');
	mMenuFile->AddItem(item);
	item = new BMenuItem("Save As" B_UTF8_ELLIPSIS, new BMessage(K_MSG_SAVE_AS), 'S', B_SHIFT_KEY);
	mMenuFile->AddItem(item);
	mMenuFile->AddSeparatorItem();
	item = new BMenuItem("About Pulsar " B_UTF8_ELLIPSIS, new BMessage(K_MSG_ABOUT), 'A');
	mMenuFile->AddItem(item);
	item = new BMenuItem("More Info " B_UTF8_ELLIPSIS, new BMessage(K_MSG_ABOUT2), 'I');
	mMenuFile->AddItem(item);
	mMenuFile->AddSeparatorItem();
	item = new BMenuItem("Preferences" B_UTF8_ELLIPSIS, new BMessage(K_MSG_PREF), 'P', B_OPTION_KEY);
	item->SetTarget(gApplication);
	mMenuFile->AddItem(item);
	mMenuFile->AddSeparatorItem();
	mMenuFile->AddItem(mFileConfig);
	mMenuFile->AddSeparatorItem();
	item = new BMenuItem("Quit", new BMessage(K_MSG_QUIT), 'Q');
	mMenuFile->AddItem(item);

	item = new BMenuItem("Reload", new BMessage(K_MSG_RELOAD_ADDON), 'R', B_SHIFT_KEY);
	item->SetTarget(gApplication);
	mMenuAddons->AddItem(item);

	mMenuEdit->AddItem(new BMenuItem("Undo", new BMessage(K_MSG_UNDO), 'Z'));
	mMenuEdit->AddItem(new BMenuItem("Redo", new BMessage(K_MSG_REDO), 'Z', B_SHIFT_KEY));
	mMenuEdit->AddSeparatorItem();
	mMenuEdit->AddItem(new BMenuItem("Cut", new BMessage(K_MSG_CUT), 'X'));
	mMenuEdit->AddItem(new BMenuItem("Copy", new BMessage(K_MSG_COPY), 'C'));
	mMenuEdit->AddItem(new BMenuItem("Paste", new BMessage(K_MSG_PASTE), 'V'));
	mMenuEdit->AddItem(new BMenuItem("Delete", new BMessage(K_MSG_DELETE), B_DELETE));
	mMenuEdit->AddSeparatorItem();
	mMenuEdit->AddItem(new BMenuItem("Select All", new BMessage(K_MSG_SELECT_ALL), 'A'));
	mMenuEdit->AddItem(new BMenuItem("Deselect", new BMessage(K_MSG_DESELECT), 'A', B_SHIFT_KEY));

	item = new BMenuItem("Preview Display", new BMessage(K_MSG_PREVIEW_BTN), 'P');
	mMenuView->AddItem(item);
	item = new BMenuItem("Game Kit Screen", new BMessage(K_MSG_FULLSCREEN_BTN), 'F');
	mMenuView->AddItem(item);
	mMenuView->AddSeparatorItem();
	/* -- RM 021400
	item = new BMenuItem("FFT Display", new BMessage(K_MSG_FFT_CHECK));
	item->SetEnabled(false);
	mMenuView->AddItem(item);
	*/
	mMenuView->AddSeparatorItem();
	item = new BMenuItem("BeOS Audio Panel" B_UTF8_ELLIPSIS, new BMessage(K_MSG_SOUND_PANEL), 'B');
	mMenuView->AddItem(item);

	item = new BMenuItem("(debug/don't use!)", new BMessage(K_DUMMY));
	item->SetEnabled(false);
	mMenuStream->AddItem(item);
	item = new BMenuItem("Start Streaming", new BMessage(K_MSG_START_BTN));
	item->SetEnabled(false);
	mMenuStream->AddItem(item);
	item = new BMenuItem("Stop Streaming", new BMessage(K_MSG_STOP_BTN));
	item->SetEnabled(false);
	mMenuStream->AddItem(item);
	item = new BMenuItem("ADC Stream", new BMessage(K_MSG_ADC_BTN));
	item->SetEnabled(false);
	mMenuStream->AddItem(item);
	item = new BMenuItem("DAC Stream", new BMessage(K_MSG_DAC_BTN));
	item->SetEnabled(false);
	mMenuStream->AddItem(item);
	item = new BMenuItem("Beginning of Stream", new BMessage(K_MSG_START_STREAM_BTN));
	item->SetEnabled(false);
	mMenuStream->AddItem(item);
	item = new BMenuItem("End of Stream", new BMessage(K_MSG_END_STREAM_BTN));
	item->SetEnabled(false);
	mMenuStream->AddItem(item);

	item = new BMenuItem("CD", new BMessage(K_MSG_CDINPUT_BTN));
	mMenuInput->AddItem(item);
item->SetEnabled(false);
	item = new BMenuItem("Line-In", new BMessage(K_MSG_LINEIN_BTN));
	mMenuInput->AddItem(item);
item->SetEnabled(false);
	item = new BMenuItem("AIFF", new BMessage(K_MSG_AIFFINPUT_BTN));
	mMenuInput->AddItem(item);
item->SetEnabled(false);
	item = new BMenuItem("MIC", new BMessage(K_MSG_MICINPUT_BTN));
	mMenuInput->AddItem(item);
item->SetEnabled(false);
	item = new BMenuItem("MP3", new BMessage(K_MSG_MP3INPUT_BTN));
	item->SetEnabled(false);
	mMenuInput->AddItem(item);
	item = new BMenuItem("MODs", new BMessage(K_MSG_MODINPUT_BTN));
	item->SetEnabled(false);
	mMenuInput->AddItem(item);
	mMenuInput->AddSeparatorItem();
	mMenuInput->AddItem(mMenuStream);

	item = new BMenuItem("Play", new BMessage(K_MSG_CD_PLAY));
	mMenuCd->AddItem(item);
	item = new BMenuItem("Stop", new BMessage(K_MSG_CD_STOP));
	mMenuCd->AddItem(item);
	item = new BMenuItem("Prev", new BMessage(K_MSG_CD_PREV));
	mMenuCd->AddItem(item);
	item = new BMenuItem("Next", new BMessage(K_MSG_CD_NEXT));
	mMenuCd->AddItem(item);
	item = new BMenuItem("Eject", new BMessage(K_MSG_CD_EJECT));
	mMenuCd->AddItem(item);
	item = new BMenuItem("Pause", new BMessage(K_MSG_CD_PAUSE));
	mMenuCd->AddItem(item);

	item = new BMenuItem("Master Interface Controller", new BMessage(K_DUMMY));
	item->SetEnabled(false);
	mMenuNetwork->AddItem(item);
	item = new BMenuItem("Slave Renderer", new BMessage(K_DUMMY));
	item->SetEnabled(false);
	mMenuNetwork->AddItem(item);
	mMenuNetwork->AddSeparatorItem();
	item = new BMenuItem("Network Settings" B_UTF8_ELLIPSIS, new BMessage(K_DUMMY));
	item->SetEnabled(false);
	mMenuNetwork->AddItem(item);

	AddChild(mMenuBar);
	return (int)(mMenuBar->Frame().Height()+1);

} // end of createMenu for CInterfaceWin


//---------------------------------------------------------------------------




//***********************************************************************
void CInterfaceWin::DispatchMessage(BMessage *message, BHandler *handler)
//***********************************************************************
{
	if (message && message->what == B_PULSE && mCurrFps)
	{
		char s[64];
		sprintf(s, "fps : %5.2f Hz", gApplication->currentFps());
		if (Lock())
		{
			mCurrFps->SetText(s);
			if (mCdTable) mCdTable->displayTrack(mCdPlay);
			Unlock();
		}
	}
	else if (message && message->what == B_KEY_DOWN)
	{
		char byte;
		message->FindInt8("byte", (int8 *)&byte);
		if (byte == 'F' || byte == 'f' || byte == ' ') gApplication->buttonFullScreen(mBpp);
	}
	inherited::DispatchMessage(message, handler);
} // end of DispatchMessage for CInterfaceWin


//****************************************************
void CInterfaceWin::MessageReceived(BMessage *message)
//****************************************************
{
	//if(debug) { printf("CInterfaceWin::MessageReceived : \n  "); message->PrintToStream(); }

	switch(message->what)
	{
		case K_MSG_LOAD:
			askLoad();
			break;
		case K_MSG_SAVE:
			askSave();
			break;
		case K_MSG_SAVE_AS:
			askSaveAs();
			break;
		case K_LOAD_REQUESTED:
			doLoad(message);
			break;
		case B_SAVE_REQUESTED:
			doSaveAs(message);
			break;
		case K_MSG_SOUND_PANEL:
			be_app->PostMessage(K_MSG_SOUND_PANEL);
			break;
		case K_MSG_DEL_LINE_PARAM:
			if (mTrack) mTrack->MessageReceived(message);
			break;
		case K_MSG_NEW:
			if (Lock())
			{
				if (mTrack)
				{
					mTrack->makeEmptyList();
					mTrack->Invalidate();
				}
				Unlock();
			}
			break;
		case K_MSG_QUIT:
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		case K_MSG_8_BPP:
			mBpp = 8;
			mMenuBpp->ItemAt(K_MENU_ITEM_8_BPP)->SetMarked(true);
			mMenuBpp->ItemAt(K_MENU_ITEM_16_BPP)->SetMarked(false);
			break;
		/*case K_MSG_16_BPP:
			mBpp = 16;
			mMenuBpp->ItemAt(K_MENU_ITEM_8_BPP)->SetMarked(false);
			mMenuBpp->ItemAt(K_MENU_ITEM_16_BPP)->SetMarked(true);
			break;*/
		case K_MSG_32_BPP:
			mBpp = 32;
			mMenuBpp->ItemAt(K_MENU_ITEM_8_BPP)->SetMarked(false);
			mMenuBpp->ItemAt(K_MENU_ITEM_32_BPP)->SetMarked(true);
			break;
		case K_MSG_PREVIEW_BTN:
			gApplication->buttonPreview(mBpp);
			break;
		case K_MSG_FULLSCREEN_BTN:
			gApplication->buttonFullScreen(mBpp);
			break;
		case K_MSG_INTEFACE_BTN:
			gApplication->buttonSwitchInterface();
			break;
		case K_MSG_START_BTN:
			gApplication->buttonStartStream();
			break;
		case K_MSG_STOP_BTN:
			gApplication->buttonStopStream();
			break;
		case K_MSG_ADC_BTN:
			gApplication->buttonADC();
			break;
		case K_MSG_DAC_BTN:
			gApplication->buttonDAC();
			break;
		case K_MSG_START_STREAM_BTN:
			gApplication->buttonStreamStart();
			break;
		case K_MSG_END_STREAM_BTN:
			gApplication->buttonStreamEnd();
			break;
		case K_MSG_ABOUT2:
			be_app->PostMessage(K_MSG_ABOUT2);
			break;
		case K_MSG_ABOUT:
			be_app->PostMessage(B_ABOUT_REQUESTED);
			break;
		case K_MSG_CD_PLAY:
			gApplication->mAudioCD.play(0);
			break;
		case K_MSG_CD_STOP:
			gApplication->mAudioCD.stop();
			break;
		case K_MSG_CD_PREV:
			gApplication->mAudioCD.prevTrack();
			break;
		case K_MSG_CD_NEXT:
			gApplication->mAudioCD.nextTrack();
			break;
		case K_MSG_CD_EJECT:
			gApplication->mAudioCD.stop();
			gApplication->mAudioCD.eject();
			break;
		case K_MSG_CD_PAUSE:
			gApplication->mAudioCD.switchPause();
			break;
		case K_MSG_CDINPUT_BTN:
		case K_MSG_AIFFINPUT_BTN:
		case K_MSG_MICINPUT_BTN	:
			setInputMethod(message->what);
			break;
		case K_MSG_CD_DEVICE:
			int32 device;
			if (message->FindInt32("device", &device) >= B_NO_ERROR)
				gApplication->mAudioCD.selectDevice(device);
			break;
		case K_MSG_CD_TRACK:
			if (mCdTable) mCdTable->displayTrack(mCdPlay);
			break;
		case K_MSG_IDEAL_RATE:
			if (1)
			{
				int32 rate;
				message->FindInt32("rate", &rate);
				gApplication->setIdealFrameRate((bigtime_t)((double)1e6/(double)rate));
			}
			break;
		default:
			//if (debug)message->PrintToStream();
			inherited::MessageReceived(message);
			break;
	}

	//if(debug) printf("DONE --> CInterfaceWin::MessageReceived\n");

} // end of MessageReceived for CInterfaceWin


//*************************************
bool CInterfaceWin::QuitRequested(void)
//*************************************
{
	if (mTrack) mTrack->makeEmptyList();
	return inherited::QuitRequested();

} // end of QuitRequested for CInterfaceWin


/*
//*********************************************
void CInterfaceWin::WindowActivated(bool state)
//*********************************************
{
	//if (debug) printf("CInterfaceWin::WindowActivated\n");
	inherited::WindowActivated(state);

} // end of WindowActivated for CInterfaceWin


//*****************************************************************
void CInterfaceWin::WorkspacesChanged(uint32 old_ws, uint32 new_ws)
//*****************************************************************
{
	inherited::WorkspacesChanged(old_ws, new_ws);
} // end of WorkspacesChanged for CInterfaceWin


//**********************************************************
void CInterfaceWin::WorkspaceActivated(int32 ws, bool state)
//**********************************************************
{
	inherited::WorkspaceActivated(ws, state);
} // end of WorkspaceActivated for CInterfaceWin


//*****************************************************************
void CInterfaceWin::FrameResized(float new_width, float new_height)
//*****************************************************************
{
	bounds = Bounds();
	frame = Frame();
	inherited::FrameResized(new_width, new_height);
} // end of FrameResized for CInterfaceWin


//*****************************************
void CInterfaceWin::Minimize(bool minimize)
//*****************************************
{
	inherited::Minimize(minimize);
} // end of Minimize for CInterfaceWin


//******************************************************************************
void CInterfaceWin::Zoom(BPoint rec_position, float rec_width, float rec_height)
//******************************************************************************
{
	bounds = Bounds();
	frame = Frame();
	inherited::Zoom(rec_position, rec_width, rec_height);
} // end of Zoom for CInterfaceWin


//*********************************************************************
void CInterfaceWin::ScreenChanged(BRect screen_size, color_space depth)
//*********************************************************************
{
	inherited::ScreenChanged(screen_size, depth);
} // end of ScreenChanged for CInterfaceWin
*/
//---------------------------------------------------------------------------


//********************************************
void CInterfaceWin::setInputMethod(int32 code)
//********************************************
/*
#define K_MSG_CDINPUT_BTN		'cdIN'
#define K_MSG_AIFFINPUT_BTN		'aiIN'
#define K_MSG_MP3INPUT_BTN		'm3IN'
#define K_MSG_MODINPUT_BTN		'moIN'
#define K_MSG_MICINPUT_BTN		'miIN'
*/
{
	if (mCdBtn)		mCdBtn->SetValue	(code == K_MSG_CDINPUT_BTN);
	if (mAiffBtn)	mAiffBtn->SetValue	(code == K_MSG_AIFFINPUT_BTN);
	if (mMicBtn)	mMicBtn->SetValue	(code == K_MSG_MICINPUT_BTN);
	gApplication->setInputMethod(code);
} // end of setInputMethod for CInterfaceWin


//************************************************
void CInterfaceWin::listFilters(BList &filterList)
//************************************************
{
	if (mAddonList) mAddonList->updateAddonList(filterList);
} // end of listFilters for CInterfaceWin


//****************************************************
void CInterfaceWin::makeDefaultList(BList &filterList)
//****************************************************
{
	if (mTrack) mTrack->makeDefaultList(filterList);
} // end of makeDefaultList for CInterfaceWin


//************************************************
void CInterfaceWin::createCdInterface(BView *back)
//************************************************
{
BRect rect;

	if (!back) return;

	float x=150+2*K_INPUT_SX+10;
	rect.Set(x,10,x+10+6*K_CD_SX+10+40/*bounds.right-10*/,10+2*K_INPUT_SY+5);

	CCdTable *fond;
	// fond is img 483*108
	fond = new CCdTable(rect, "fondcd", K_IMG_BOUTON_FONDCD, B_FOLLOW_LEFT /*_RIGHT*/ | B_FOLLOW_TOP);
	mCdTable = fond;
	if (!fond) return;
	//fond->SetEnabled(false);
	back->AddChild(fond);

	rect.Set(10,3,10+K_CD_SX-1,3+K_CD_SY-1);
	mCdPlay = new CCdButton(rect, "cdplay", K_CD_PLAY,6, K_IMG_BOUTON_BTNCD, new BMessage(K_MSG_CD_PLAY));
	if (mCdPlay) fond->AddChild(mCdPlay);

	rect.OffsetBy(K_CD_SX,0);
	mCdStop = new CCdButton(rect, "cdstop", K_CD_STOP,6, K_IMG_BOUTON_BTNCD, new BMessage(K_MSG_CD_STOP));
	if (mCdStop) fond->AddChild(mCdStop);

	rect.OffsetBy(K_CD_SX,0);
	mCdPrev = new CCdButton(rect, "cdprev", K_CD_PREV,6, K_IMG_BOUTON_BTNCD, new BMessage(K_MSG_CD_PREV));
	if (mCdPrev) fond->AddChild(mCdPrev);

	rect.OffsetBy(K_CD_SX,0);
	mCdNext = new CCdButton(rect, "cdnext", K_CD_NEXT,6, K_IMG_BOUTON_BTNCD, new BMessage(K_MSG_CD_NEXT));
	if (mCdNext) fond->AddChild(mCdNext);

	rect.OffsetBy(K_CD_SX,0);
	CCdButton *but;
	but = new CCdButton(rect, "cdpause", K_CD_PAUSE,6, K_IMG_BOUTON_BTNCD, new BMessage(K_MSG_CD_PAUSE));
	if (but) fond->AddChild(but);

	rect.OffsetBy(K_CD_SX,0);
	but = new CCdButton(rect, "cdeject", K_CD_EJECT,6, K_IMG_BOUTON_BTNCD, new BMessage(K_MSG_CD_EJECT));
	if (but) fond->AddChild(but);

	BList *devlist = gApplication->mAudioCD.getList();
	if (!devlist) return;

	rect.Set(10,3+K_CD_SY+5,10+6*K_CD_SX-1,fond->Bounds().Width()-3);
	BPopUpMenu *mMenuCD;
	mMenuCD = new BPopUpMenu("CD Devices");
	if (!mMenuCD) return;

	for(long i=0, j=devlist->CountItems(); i<j; i++)
	{
		char *p = (char *)devlist->ItemAt(i);
		if (!p) mMenuCD->AddSeparatorItem();
		else
		{
			BMessage *msg = new BMessage(K_MSG_CD_DEVICE);
			msg->AddInt32("device", i);
			BMenuItem *item = new BMenuItem(p, msg);
			mMenuCD->AddItem(item);
			if (item && i==0) item->SetMarked(true);
		}
	}
	BMenuField *cddev;
	cddev = new BMenuField(rect, "cddev", "CD Devices", mMenuCD);
	if (cddev) cddev->SetDivider(60);
	if (cddev) fond->AddChild(cddev);

} // end of createCdInterface for CInterfaceWin




//******************************************
void CCdTable::displayTrack(CCdButton *play)
//******************************************
{
bool result;
bool playing=false;
int32 track=mLastTrackNum;

	if (play)
		result = gApplication->mAudioCD.getCachedPosition(playing, track);

	if (playing != mLastCdPlaying && play)
	{
		mLastCdPlaying = playing;
		play->SetValue(playing);
	}

	if (track != mLastTrackNum || !play)
	{
		if (play) mLastTrackNum = track;
		track = track % 100;
		BPoint p(10+6*K_CD_SX+10,5);
		DrawBitmap(mTrackLed[track / 10], p);
		p.x += LED_WIDTH+1;
		DrawBitmap(mTrackLed[track % 10], p);
		SetHighColor(150,150,150);
		p.Set(10+6*K_CD_SX+10,5);
		StrokeLine(BPoint(p.x-1,p.y+LED_HEIGHT+1), BPoint(p.x-1, p.y-1));
		StrokeLine(BPoint(p.x+LED_WIDTH*2+2,p.y-1));
		SetHighColor(240,240,240);
		StrokeLine(BPoint(p.x+LED_WIDTH*2+2,p.y+LED_HEIGHT+1));
		StrokeLine(BPoint(p.x,p.y+LED_HEIGHT+1));
	}

} // end of displayTrack for CCdTable


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -



//********************************************************
bool CMyFileRefFilter ::Filter(const entry_ref	*ref,
							   BNode			*node,
							   struct stat		*st,
							   const char		*mimetype)
//********************************************************
{
	return (strcmp(mimetype, K_APP_DOC_MIME) == 0 || strcmp(mimetype, K_APP_MIME) == 0 || node->IsDirectory());
} // end of Filter for CMyFileRefFilter


//*******************************
void CInterfaceWin::askLoad(void)
//*******************************
{
	if (!mLoadPanel)
	{
		BMessage *msg = new BMessage(K_LOAD_REQUESTED);
		BEntry entry;
		entry_ref ref;
		status_t err;
		BDirectory dir(gApplication->getAppDir());
		err = dir.FindEntry(".", &entry);
		if (err >= B_NO_ERROR) err = entry.GetRef(&ref);
		if (err >= B_NO_ERROR)
			mLoadPanel = new BFilePanel(B_OPEN_PANEL, NULL, &ref, B_FILE_NODE, false, msg, mMyFileRefFilter);
		if (msg) delete msg;
		if (!mLoadPanel) throw("Can't create a Load File Panel");
	}
	mLoadPanel->Show();
} // end of askLoad for CInterfaceWin


//*******************************
void CInterfaceWin::askSave(void)
//*******************************
{
	if (mMustSaveAs)
	{
		askSaveAs();
		return;
	}
	else
	{
		try
		{
			saveToFile(mLastSavePath);
		}
		catch(...)
		{
			BAlert *box = new BAlert(K_APP_NAME " Error",
									 K_APP_NAME " Error\nUnhandled Save Exception thrown,"
									 "\ncatched in Save Document",
									 "   OK   ");
			if(box) box->Go();
		}
	}

} // end of askSave for CInterfaceWin


//*********************************
void CInterfaceWin::askSaveAs(void)
//*********************************
{
	if (!mSavePanel)
	{
		// this panel will send a B_SAVE_REQUESTED message
		// with field "name" STRING and "directory" as REFS entry_ref
		BEntry entry;
		entry_ref ref;
		status_t err;
		BDirectory dir(gApplication->getAppDir());
		err = dir.FindEntry(".", &entry);
		if (err >= B_NO_ERROR) err = entry.GetRef(&ref);
		if (err >= B_NO_ERROR)
			mSavePanel = new BFilePanel(B_SAVE_PANEL, NULL, &ref, B_FILE_NODE, false, NULL, mMyFileRefFilter);
		if (!mSavePanel) throw("Can't create a Save File Panel");
	}
	mSavePanel->Show();
} // end of askSaveAs for CInterfaceWin



//***************************************
void CInterfaceWin::doLoad(BMessage *msg)
//***************************************
{
	msg->PrintToStream();

	try
	{
		BEntry entry;
		BPath localpath;
		entry_ref ref;
		status_t err;

		err = msg->FindRef("refs", 0, &ref);
		if (err >= B_NO_ERROR) err = entry.SetTo(&ref);
		if (err >= B_NO_ERROR) err = entry.GetPath(&localpath);
		if (err < B_NO_ERROR) throw("Can't load that file !");

		loadFromFile(localpath);
	}
	catch(...)
	{
		BAlert *box = new BAlert(K_APP_NAME " Error",
								 K_APP_NAME " Error\nUnhandled Load Exception thrown,"
								 "\ncatched in Load Document",
								 "   OK   ");
		if(box) box->Go();
	}

} // end of doLoad for CInterfaceWin


//*****************************************
void CInterfaceWin::doSaveAs(BMessage *msg)
//*****************************************
{
	entry_ref dir_ref;
	const char *name;
	status_t err;
	
	try
	{
		err = msg->FindString("name", &name);
		if (err < B_NO_ERROR) throw("Can't save with that name !");
		err = msg->FindRef("directory", &dir_ref);
		if (err < B_NO_ERROR) throw("Can't save with that directory !");

		BDirectory dir(&dir_ref);
		BFile fichier;
		BEntry entry;
		
		err = dir.CreateFile(name, &fichier);
		if (err < B_NO_ERROR) throw("Can't create file !");

		err = dir.FindEntry(name, &entry);
		if (err < B_NO_ERROR || !entry.Exists()) throw("Can't open file !");

		BPath localpath;
		err = entry.GetPath(&localpath);
		if (err < B_NO_ERROR) throw("Can't open file !");

		// -- eventually write into the file -- (!!)

		saveToFile(localpath);
		mLastSavePath = localpath;
		mMustSaveAs = false;

		// -- end of writing --
		
		BNodeInfo info(&fichier);
		info.SetType(K_APP_DOC_MIME);
		info.SetPreferredApp(K_APP_MIME);
		
	}
	catch(status_t err)
	{
		BAlert *box;
		char s2[256];
		sprintf(s2,	K_APP_NAME " Error\n"
					"Save Exception : %d (%08xh)\n"
					"(probably a BeOS error number)", (int32)err, (int32)err);
		box = new BAlert("Abcd - Error box", s2, "    OK    ", NULL, NULL,
						B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
		if (box) box->Go();
	}
	catch(char *s)
	{
		BAlert *box;
		char s2[1024];
		if (strlen(s) > 800) s[800] = '\0';	// ugly but safe... :-/
		sprintf(s2,	K_APP_NAME " Error\n"
					"Save Exception : %s\n", s);
		box = new BAlert(K_APP_NAME " Error", s2, "    OK    ", NULL, NULL,
						B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
		if (box) box->Go();
	}
	catch(...)
	{
		BAlert *box = new BAlert(K_APP_NAME " Error",
								K_APP_NAME " Error\nUnhandled Save Exception thrown,"
								"\ncatched in Save Document",
								"   OK   ");
		if(box) box->Go();
	}

} // end of doSaveAs for CInterfaceWin


//*****************************************
void CInterfaceWin::saveToFile(BPath &path)
//*****************************************
{
BList *list = NULL;

	if (!gSimpleTrack->lock()) throw("Can't access track!");
	try
	{
		FILE *f = fopen(path.Path(), "w");
		if (!f) throw("Can't open file !");
		
		// -- eventually write into the file -- (!!)
	
		fprintf(f,"# " K_APP_NAME "v.%d.%d.%d -- DO NOT EDIT BY HAND !!\n\n",
					K_MAJOR_PULSAR_VERSION, K_MINOR_PULSAR_VERSION, K_QUANTIC_PULSAR_VERSION);
	
		// -- fake document info
		fprintf(f,"@VERSION_DOC 1\n");
		fprintf(f,"@NAME_DOC %s\n", path.Leaf());
	
		// -- fake track info
		fprintf(f,"@NB_TRACK 1\n");
		fprintf(f,"@NAME_TRACK track 1\n");
		fprintf(f,"@DURATION_TRACK INF\n");
	
		// -- real info for each line, first number of lines (0=empty, else >=1)
		long nbline = gSimpleTrack->countLine();
		fprintf(f,"@NB_LINE %d\n", nbline);
	
		for(long index=0; index<nbline; index++)
		{
			fprintf(f,"@LINE %d\n", index);
			
			SLineParam *line = gSimpleTrack->getLine(index);
			ELineMode mode=line->mode;
			CFilter *filter = line->filter;
			if (!line || !filter || !filter->sFilter.name)
			{
				fprintf(f,"@EMPTY_LINE\n");
				continue;
			}
			
			fprintf(f,"@INFO_LINE %d %d %d %d -- mode start len repeat\n", (long)mode, line->start, line->len, line->repeat);
			fprintf(f,"@FILTER_LINE %s\n", filter->sFilter.name);
		}
	
		fprintf(f,"\n# End of File -- please do not edit by hand\n");
		
		// -- end of writing --
		
		fclose(f);
	}
	catch(...)
	{
		gSimpleTrack->unlock();
		throw;
	}

	gSimpleTrack->unlock();

} // end of saveToFile for CInterfaceWin


//*******************************************
void CInterfaceWin::loadFromFile(BPath &path)
//*******************************************
{
BList *list = NULL;
SLineParam *line=NULL;

	if (mTrack) mTrack->makeEmptyList();
	if (!gSimpleTrack->lock()) throw("Can't access track!");
	try
	{
		FILE *f = fopen(path.Path(), "r");
		if (!f) throw("Can't open file !");
		
		// -- read it
		
		while(!feof(f))
		{
			char buf[512];
			fgets(buf, 512, f);
			
			if (buf[0] != '@') continue;
			char *s;
			if ((s=strchr(buf, '\n')) != NULL) *s='\0';
			if ((s=strchr(buf, '\r')) != NULL) *s='\0';
	
			if (strncmp(buf, "@LINE ", 6) == 0)
			{
				if (!line) line = new SLineParam;
				if (!line) throw("Alloc error");
			}
			else if (strncmp(buf, "@INFO_LINE ", 11) == 0)
			{
				long mode=0;
				sscanf(buf+11, "%d %d %d %d", &mode, &line->start, &line->len, &line->repeat);
				line->mode = (ELineMode)mode;
			}
			else if (strncmp(buf, "@FILTER_LINE ", 13) == 0)
			{
				char *name = buf+13;
				long index=0;
				CFilter *filter;
				//if(debug)printf("LOAD  ###### load filter name '%s'\n", name);
				while((filter = gApplication->getNthFilter(index++)) != NULL)
				{
					if (!filter->sFilter.name) continue;
					if (strcmp(name, filter->sFilter.name) == 0)
					{
						line->filter = filter;
						break;
					}
				}
				
				if (line->filter)
				{
					gSimpleTrack->addLine(line);
					line = NULL;
				}
			}
		}
		
		if (line) delete line;
	
		// -- end of reading --
		
		fclose(f);
	}
	catch(...)
	{
		gSimpleTrack->unlock();
		mTrack->updateList();
		throw;
	}

	gSimpleTrack->unlock();
	mTrack->updateList();

} // end of loadFromFile for CInterfaceWin




//---------------------------------------------------------------------------

// eoc
