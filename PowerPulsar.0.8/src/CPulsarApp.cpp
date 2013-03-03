/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CPulsarApp.cpp
	Partie	: Loader

	Auteur	: RM
	Date		: 140297
	Format	: tabs==2

*****************************************************************************/

#include "RLib.h"
#include "TRArray.h"

bool _gDebug;	// TEMP HACK RM 050800 for RLib

#include "CPulsarApp.h"

#include "CPulsarMsg.h"
#include "CFilledWin.h"
#include "CInterfaceWin.h"
#include "CFilterInfoWin.h"
#include "CSplatchWin.h"
#include "CPrefWin.h"
#include "CScreen.h"
#include "CMatroxScreen.h"


#include "SoundConsumer.h"			// for BeOS R4.5+ sound

#include <Path.h>
#include <AppFileInfo.h>

#if K_USE_DATATYPE
	#include "Datatypes.h"
	#include "GetBitmap.h"		/// in /boot/develop/Datatypes152/headers/glue/...
	#include <DataIO.h>
	#include "BitmapStream.h"
#endif

#if defined(B_BEOS_VERSION_4) && defined(__INTEL__)	// -- HOOK RM 151198 -- R4b4
	#include "R3MediaDefs.h"
	#define B_MASTER_OUT R3_B_MASTER_OUT
	#define B_ADC_IN R3_B_ADC_IN
	#define B_CD_IN R3_B_CD_IN
	#define B_MIC_IN R3_B_MIC_IN
#endif

#define _B_ATAN_1

#include "fftlib.h"
#include "fastmath.h"

#include "chiffres.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

//---------------------------------------------------------------------------

#undef K_EXT
#define K_EXT ".tga"
#define K_GK_SUP_CONTROL_SON_IMG	"img_pulsar/controlson" K_EXT
#define K_GK_SUP_CONTROL_CD_IMG		"img_pulsar/cd_gk" K_EXT

//---------------------------------------------------------------------------

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

//---------------------------------------------------------------------------

CSimpleTrack		*gSimpleTrack = NULL;
BTranslatorRoster	*gTranslationRoster = NULL;

//---------------------------------------------------------------------------

//*********************************************************
CBlackWindow::CBlackWindow(BRect frame,
							const char *title, 
							window_type type,
							ulong flags,
							ulong workspace)
						: BWindow(frame, title, type, flags, workspace)
#define inherited BWindow
//*********************************************************
{
	mBlackView = NULL;
}


//*********************************
CBlackWindow :: ~CBlackWindow(void)
//*********************************
{
}


//***************************
bool CBlackWindow::init(void)
//***************************
{
	mBlackView = new BView(Bounds(), "black", B_FOLLOW_ALL, B_WILL_DRAW);
	if (!mBlackView) return false;
	mBlackView->SetViewColor(0,0,0);
	AddChild(mBlackView);
	Minimize(true);
	Show();
	return true;
}


//************************************
bool CBlackWindow::QuitRequested(void)
//************************************
{
	return false;
}


//**********************************************************************
void CBlackWindow::DispatchMessage(BMessage *message, BHandler *handler)
//**********************************************************************
{
	if (message && message->what == B_KEY_DOWN)
	{
		char byte;
		message->FindInt8("byte", (int8 *)&byte);
		if (byte == 'F' || byte == 'f' || byte == ' ' )
			gApplication->buttonFullScreen(8);
		else if (byte == 27)
			Hide();
	}
	inherited::DispatchMessage(message, handler);
}


//********************************************
void CBlackWindow::WindowActivated(bool state)
//********************************************
{
	if (mBlackView) mBlackView->MakeFocus();
}



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -


#undef inherited
#define inherited BApplication

//***************************************************************************
CPulsarApp::CPulsarApp()
		   :BApplication(K_APP_MIME),
				mLargeAppIcon(BRect(0,0,B_LARGE_ICON-1,B_LARGE_ICON-1), B_COLOR_8_BIT),
				mPreference(K_APP_SUB_MIME)
//***************************************************************************
{
	mAdcStream = NULL;
	mDacStream = NULL;
	mAudio = NULL;
	mUseDAC = false;
	mUseStreamEnd = false;
	mFilledWindow = NULL;
	mUpdateFft = false;
	mUpdateFilled = false;
	mFondFft = NULL;
	mInterfaceWindow = NULL;
	mFilterInfoWindow = NULL;
	mSplatchWindow = NULL;
	mFullScreen = NULL;
	mButtonAudio = false;
	mIsRunning = false;
	mPulsarThreadMustQuit = false;
	mPulsarThread = B_ERROR;
	mPulsarFrameSem = B_ERROR;
	mIdealFrameRate = (bigtime_t)(1e6/42.0);
	mGameKitRunning = false;
	mGameKitDrawingEnabled = false;
	mBpp = 8;
	mInfoWindow = NULL;
	mInputMethod = K_MSG_CDINPUT_BTN;
	mIsLoading = true;
	mSimpleTrack = NULL;
	mBlackWindow = NULL;
	mInvertGameKitScreen = false;
	mPrefWindow = NULL;
	mGkSupVolume = 0;
	mGkSupCdTrack = 1;
	mMatroxScreen = NULL;
	mTranslationKit = NULL;
	mSoundRecorder = NULL;
	mMediaRoster = NULL;

} // end of constructor for CPulsarApp


//***************************
CPulsarApp::~CPulsarApp(void)
//***************************
{
	if (debug) printf("CPulsarApp::~CPulsarApp -- destructor\n");
} // end of destructor for CPulsarApp


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -


//*************************
bool CPulsarApp::init(void)
//*************************
{
	app_info info;
	BEntry fichier, dummy;
	BDirectory dir, adir;
	status_t result;

	if (debug) printf("CPulsarApp::init\n");

	GetAppInfo(&info);
	result=fichier.SetTo(&info.ref);
	if (!result) result=fichier.GetParent(&dir);
	if (result) return false;

	result = dir.GetEntry(&dummy);
	if (!result) result = dummy.GetPath(&mAppDir);
	if (result) return false;

	// find add-ons directory ; if doesn't exist create else use application directory
	if (adir.SetTo(&dir, K_ADDON_DIR_NAME) < B_NO_ERROR)
		if (dir.CreateDirectory(K_ADDON_DIR_NAME, &adir) < B_NO_ERROR)
			adir = dir;

	result = adir.GetEntry(&dummy);
	if (!result) result = dummy.GetPath(&mAddonDir);
	if (result) return false;

	// find add-ons directory ; if doesn't exist create else use application directory
	if (adir.SetTo(&dir, K_IMG_DIR_NAME) < B_NO_ERROR)
		if (dir.CreateDirectory(K_IMG_DIR_NAME, &adir) < B_NO_ERROR)
			adir = dir;

	result = adir.GetEntry(&dummy);
	if (!result) result = dummy.GetPath(&mImgDir);
	if (result) return false;

	try
	{
		BFile fichier(&info.ref, B_READ_ONLY);
		BAppFileInfo appfile(&fichier);
		result = appfile.GetIcon(&mLargeAppIcon, B_LARGE_ICON);

#if K_USE_DATATYPE
		if (result < B_NO_ERROR)
		{
			if (debug) printf("no app file icon !!\n");
			BBitmap *bitmap = GetBitmap("BEOS:L:STD_ICON");
			if (debug) printf("bitmap %p\n", bitmap);
		}
#endif
	}
	catch(...)
	{
		if (debug) printf("can't BFile the app to extract icon\n");
		//return false;
	}

	if (!mAudioCD.init()) return false;

#if K_USE_DATATYPE
	if (!initDatatypeLib()) return false;
#endif
	if (!initTranslationKit()) return false;

	// init prefs stuff
	mPreference.load();
	clearPref();
	loadPref();

	mSimpleTrack = new CSimpleTrack();
	if (!mSimpleTrack) return false;
	gSimpleTrack = mSimpleTrack;

	// build part of the audio
	// use new audio stuff (BeOS R4.5+)
	mAudioOutput.source = media_source::null;
	mRecInput.destination = media_destination::null;

	mMediaRoster = BMediaRoster::Roster(&result);
	if (debug) printf("mMediaRoster %p -- err %ld\n", mMediaRoster, result);
	if (!mMediaRoster || result < B_OK) return false;

	result = mMediaRoster->GetAudioInput(&mAudioInputNode);
	if (result < B_OK) return false;

	mSoundRecorder = new SoundConsumer("PowerPulsar Recorder");
	result = mMediaRoster->RegisterNode(mSoundRecorder);
	if (debug) printf("mSoundRecorder %p -- err %ld\n", mSoundRecorder, result);
	if (result < B_OK) return false;

	if (!mMediaRoster)	// (<-- deactivated)
	{
		// BeOS PR2-R3 Old Media Kit
		mAdcStream = new BADCStream();
		mDacStream = new BDACStream();
		if (!mAdcStream || !mDacStream)
		{
			if (debug) printf("BADCStream %p and BDACStream %p created\n", mAdcStream, mDacStream);
			return false;
		}
	
		if(debug) printf("BADCStream and BDACStream created\n");
	}

	// first window to be build...
	SWinPref pref1;
	pref1.mFrame.Set(160,160,160+320,160+200);
	pref1.mBaseName = "mSplatchWindow";
	pref1.loadPref(mPreference);
	mSplatchWindow = new CSplatchWin(pref1,
																	 "Welcome to PowerPulsar !",
																	 B_TITLED_WINDOW, //B_BORDERED_WINDOW,
																	 B_NOT_ZOOMABLE | B_NOT_RESIZABLE /*| B_WILL_FLOAT*/);
	if (!mSplatchWindow || !mSplatchWindow->init()) return false;
	mSplatchWindow->Activate();


	// build the main graphic interface
	// this window starts shown
	BRect frame,frame2;
	if(1)
	{
		BScreen screen;
		frame = frame2 = screen.Frame();
	}
	if (frame2.Width()>660)
		frame2.InsetBy(20,20);
	else
		frame2.InsetBy(K_WINDOW_BORDER_WIDTH, K_WINDOW_BORDER_WIDTH);

	SWinPref pref2;
	pref2.mFrame = frame2;
	pref1.mBaseName = "mInterfaceWindow";
	pref2.loadPref(mPreference);
	mInterfaceWindow = new CInterfaceWin(pref2,																	
																			 "PowerPulsar Interface",
																			 B_TITLED_WINDOW /* B_DOCUMENT_WINDOW */,
																			 B_WILL_ACCEPT_FIRST_CLICK);
	if (!mInterfaceWindow|| !mInterfaceWindow->init()) return false;
	mSplatchWindow->Activate();
/*
	if(mInterfaceWindow->Lock())
	{
				mInterfaceWindow->Minimize(true);
				mInterfaceWindow->Unlock();
	}
*/
	// load primordial bitmaps
	if (!loadBitmap()) return false;
	if (!initGkSup()) return false;

	// build the info window...
	SWinPref pref3;
	pref3.mFrame.Set(160,160,160+320,160+200);
	pref3.mBaseName = "mInfoWindow";
	pref3.loadPref(mPreference);
	mInfoWindow = new CSplatchWin(pref3,
																"PowerPulsar Info",
																B_TITLED_WINDOW, //B_BORDERED_WINDOW,
																B_NOT_ZOOMABLE | B_NOT_RESIZABLE /*| B_WILL_FLOAT*/);
	if (!mInfoWindow || !mInfoWindow->init(false)) return false;
	mInfoWindow->endLoading();
	//mSplatchWindow->Activate();

	// build the preview window
	// this window starts runned but hidden
	BRect fillfram2 = frame2;
	BRect fillfram(600,100,600+640-1,100+480-1);
	fillfram.OffsetTo(max(2,(fillfram2.Width()-fillfram.Width())/2),
									max(2,(fillfram2.Height()-fillfram.Height())/2));
	mFilledWindow = new CFilledWin(fillfram,
																 "PowerPulsar Filled View",
																 B_TITLED_WINDOW,
																 B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_NOT_CLOSABLE);
	if (!mFilledWindow|| !mFilledWindow->init()) return false;
	//mSplatchWindow->Activate();

	// build the filter info window
	mFilterInfoWindow = new CFilterInfoWin(BRect(100,100,100+320,100+300),
																				 "Filter Info",
																				 B_TITLED_WINDOW /* B_DOCUMENT_WINDOW */,
																				 B_NOT_ZOOMABLE/* | B_NOT_RESIZABLE*/);
	if (!mFilterInfoWindow|| !mFilterInfoWindow->init()) return false;
	//	mInterfaceWindow->AddFloater(mFilterInfoWindow);

	// build the pref window
	SWinPref pref4;
	pref4.mFrame.Set(100,100,100+320,100+200);
	pref4.mBaseName = "mPrefWindow";
	pref4.loadPref(mPreference);
	mPrefWindow = new CPrefWin(pref4,
														 K_APP_NAME " Prefs",
														 B_TITLED_WINDOW /* B_DOCUMENT_WINDOW */,
														 B_NOT_ZOOMABLE | B_NOT_RESIZABLE);
	if (!mPrefWindow|| !mPrefWindow->init()) return false;
	//	mInterfaceWindow->AddFloater(mPrefWindow);

	// setup filter reference
	// build the filter info window
/*
	mBlackWindow = new CBlackWindow(frame,
									"Black Out Total 2 le retour",
									B_TITLED_WINDOW,
									B_NOT_ZOOMABLE);
	if (!mBlackWindow|| !mBlackWindow->init()) return false;
*/

	if (!gSrcCpyHigh || !gSrcCpyBass || !gFftHigh || !gFftBass) return false;
	mFrameInfo.rawHigh = gSrcCpyHigh;
	mFrameInfo.rawBass = gSrcCpyBass;
	mFrameInfo.fftHigh = gFftHigh;
	mFrameInfo.fftBass = gFftBass;
	mFilledWindow->getInfos(mFilterReference.sPrepare.sx,
													mFilterReference.sPrepare.sy,
													mFilterReference.sPrepare.mode,
													mFrameInfo.screen,
													mFilterReference.sPrepare.bpr);
	mFilterReference.sPrepare.sxy					= mFilterReference.sPrepare.bpr*mFilterReference.sPrepare.sy;
	mFilterReference.sPrepare.isGameKit		= false;
	mFilterReference.sLoad.dirApplication	= mAppDir;
	mFilterReference.sLoad.dirAddon				= mAddonDir;
	mFilterReference.sLoad.dirImg					= mImgDir;
//	mFilterReference.sLoad.mAddonSetting	= &mPreference;	[RM 060100 TBDL]

	// setup the reference palette
	{
		BScreen screen;
		const color_map *map = screen.ColorMap();
		for(int32 i=0; i<256; i++)
		{
			const rgb_color &c = map->color_list[i];
			mFilterReference.sPrepare.colorList[i] = c;
			uint32 u = (K_ALPHA_OPAQUE << K_ALPHA_OFFSET) | (c.red << K_RED_OFFSET) | (c.green << K_GREEN_OFFSET) | (c.blue << K_BLUE_OFFSET);
			mFilterReference.sPrepare.colorMap[i] = u;
		}
	}
	

	// load add-ons
	loadFilterAddons();
	if (debug) printf("listFilters\n");
	mInterfaceWindow->listFilters(mFilterList);
	if (debug) printf("makeDefaultList\n");
	mInterfaceWindow->makeDefaultList(mFilterList);

	if(mPulsarPref.mLoadDefaultConfig)
	{
		BMessage msg(K_MSG_LOAD_CONFIG);
		msg.AddInt32(K_NAME_CONFIG_INDEX, 0);
		gApplication->PostMessage(&msg);
	}

	mIsLoading = false;
	mSplatchWindow->endLoading();
	mSplatchWindow->Activate();

	//if (mBlackWindow) mBlackWindow->Show();
	return true;

} // end of init for CPulsarApp


//***********************************
void CPulsarApp::AboutRequested(void)
//***********************************
{
	if (mSplatchWindow && mSplatchWindow->Lock())
	{
		if (!mSplatchWindow->IsActive())
		{
			if (mSplatchWindow->IsHidden()) mSplatchWindow->Show();
			mSplatchWindow->Activate();
		}
		else mSplatchWindow->Hide();
		mSplatchWindow->Unlock();
	}
} // end of AboutRequested for CPulsarApp


//***************************************
bool CPulsarApp::initTranslationKit(void)
//***************************************
{
status_t err;

	mTranslationKit = new BTranslatorRoster;
	if (!mTranslationKit) return false;

	err = mTranslationKit->AddTranslators(NULL); // load default translators
	printf("CPulsarApp::initTranslationKit [NULL] == %d\n", err);

	BPath path(mAddonDir.Path(), "Translators");
	mTranslationKit->AddTranslators(path.Path());  // load additional translators
	printf("CPulsarApp::initTranslationKit [%s] == %d\n", path.Path(), err);

	gTranslationRoster = mTranslationKit;

	return true;
} // end of initTranslationKit for CPulsarApp


#if K_USE_DATATYPE
//************************************
bool CPulsarApp::initDatatypeLib(void)
//************************************
{
	if (debug) printf("CPulsarApp::initDatatypeLib\n");
	if (DATAVersion == NULL)
	{
		BAlert *alrt = new BAlert("", "libdatatypes.so is not installed!", "Quit");
		alrt->Go();
		return false;
	}

	long curv, minv;
	DATAVersion(curv, minv);
	if (debug) printf("data type version : curv %d, minv %d\n", curv, minv);
	if (curv < DATA_MIN_VERSION)
	{
		BAlert *alrt = new BAlert("", "libdatatypes.so is too old!", "Force", "Quit");
		alrt->SetShortcut(0, B_ESCAPE);
		if (alrt->Go()) return false;
	}

	if (minv > DATA_CURRENT_VERSION)
	{
		BAlert *alrt = new BAlert("", "libdatatypes.so is too new!?", "Force", "Quit");
		alrt->SetShortcut(0, B_ESCAPE);
		if (alrt->Go()) return false;
	}

	long err = DATAInit(K_APP_MIME);
	if (err)
	{
		char message[100];
		sprintf(message, "libdatatypes.so got error 0x%08x when initializing!", err);
		BAlert *alrt = new BAlert("", message, "Quit");
		alrt->Go();
		return false;
	}
	return true;
} // end of initDatatypeLib for CPulsarApp
#endif


//*******************************
void CPulsarApp::ReadyToRun(void)
//*******************************
{
	if (debug) printf("CPulsarApp::ReadyToRun\n");

} // end of ReadyToRun for CPulsarApp


//**********************************
bool CPulsarApp::QuitRequested(void)
//**********************************
{
	if (debug) printf("CPulsarApp::QuitRequested\n");
	// if (mIsLoading) return; -- can be dangerous...

	if (mSplatchWindow)
	{
		mSplatchWindow->forceText();
		mSplatchWindow->Show();
		mSplatchWindow->Activate();
	}

	Lock();

	// first try to close the stream... should be synchronous (?)
	if (debug) printf("stop stream - synchronous (?)\n");
	stopStream();

	// delete base sound classes
	delete mAdcStream;	mAdcStream = NULL;
	delete mDacStream;	mDacStream = NULL;
	if (mSoundRecorder) mSoundRecorder->Release(); mSoundRecorder = NULL;

	// unload add-ons
	unloadFilterAddons();

	// then close the windows
	if (debug) printf("close every window\n");
	if (mFilledWindow			&& mFilledWindow->Lock())			{	mFilledWindow->QuitRequested();			mFilledWindow->Quit();		}
	if (mFilterInfoWindow	&& mFilterInfoWindow->Lock())	{	mFilterInfoWindow->QuitRequested();	mFilterInfoWindow->Quit();	}
	if (mPrefWindow				&& mPrefWindow->Lock())				{	mPrefWindow->QuitRequested();				mPrefWindow->Quit();		}
	if (mInterfaceWindow	&& mInterfaceWindow->Lock())	{	mInterfaceWindow->QuitRequested();	mInterfaceWindow->Quit();	}
	if (mInfoWindow				&& mInfoWindow->Lock())				{	mInfoWindow->QuitRequested();				mInfoWindow->Quit();		}
	if (mSplatchWindow		&& mSplatchWindow->Lock())		{	mSplatchWindow->QuitRequested();		mSplatchWindow->Quit();		}

	mFilledWindow = NULL;
	mInterfaceWindow = NULL;
	mInfoWindow = NULL;
	mFilterInfoWindow = NULL;

	// save the pref stuff
	savePref();
	mPreference.save();

	// stop datatype a clean way...
	if (debug) printf("shutdown data types\n");
#if K_USE_DATATYPE
	DATAShutdown();
#endif

	if (mTranslationKit) delete mTranslationKit;

	Unlock();
	if (debug) printf("end of bapp\n");
	return true;	// accepte de mourrir

} // end of QuitRequested for CPulsarApp


//************************************************************************
bool CPulsarApp::touches(const char *bytes, int32 numBytes, BMessage *msg)
//************************************************************************
{
	bool playing;
	int32 track;

	// reject both nothing and UTF
	if (mIsLoading) return false;
	if (!bytes || numBytes != 1) return false;

	switch(bytes[0])
	{
		// cd stuff
		case 'p':	case 'P':	mAudioCD.play(0);				break;
		case 's':	case 'S':	mAudioCD.stop();				break;
		case 'e':	case 'E':	mAudioCD.eject();				break;
		case 'o':	case 'O':	mAudioCD.switchPause();	break;
		case 'v':	case 'V':
			if (mPulsarPref.mDisplayGkSup)
			{
				mGkSupState |= (uint32)kGkSupCd;
				mGkSupCdEndTime = (bigtime_t)(system_time()+4e6);			// HOOK USE CONSTANT
			}
			mAudioCD.prevTrack();
			mAudioCD.getPosition(playing, track);
			mInterfaceWindow->PostMessage(K_MSG_CD_TRACK);	// RM 021400
			mGkSupCdTrack = track;
			break;

		case 'n':	case 'N':
			if (mPulsarPref.mDisplayGkSup)
			{
				mGkSupState |= (uint32)kGkSupCd;
				mGkSupCdEndTime = (bigtime_t)(system_time()+4e6);			// HOOK USE CONSTANT
			}
			mAudioCD.nextTrack();
			mAudioCD.getPosition(playing, track);
			mInterfaceWindow->PostMessage(K_MSG_CD_TRACK);	// RM 021400
			mGkSupCdTrack = track;
			break;
		
		case ' ':
		case 'f': case 'F':
		case 'q': case 'Q':
		case 'x': case 'X':
		case 'z': case 'Z':
		case 21: case B_ESCAPE:
			if (mGameKitRunning)
			{
				mGameKitRunning = false;
				stateStream();
			}
			break;
		
		case '0':	case '1':	case '2':case '3': case '4':
		case '5':	case '6':	case '7':	case '8': case '9':
			mAudioCD.setVolume((bytes[0]-'0')*255.0/9.0);
			break;

		case 'i':
		case 'I':
			if (mIsRunning && mGameKitRunning)
				mInvertGameKitScreen = !mInvertGameKitScreen;
			break;

		case '-':
		case '+':
			if(1)
			{
				BDACStream stream;
				status_t err;
				float l_volume;
				float r_volume;
				bool enabled=false;

				if (mPulsarPref.mDisplayGkSup) 
				{
					mGkSupState |= (uint32)kGkSupVolume;
					mGkSupVolEndTime = (bigtime_t)(system_time()+2e6);			// HOOK USE CONSTANT
				}

				err = stream.GetVolume(B_MASTER_OUT, &l_volume, &r_volume, &enabled);
				if (err >= B_NO_ERROR && enabled)
				{
					if (r_volume > l_volume) l_volume = r_volume;
					if(bytes[0] == '-') l_volume -= 0.1;
					else l_volume += 0.1;
					if (l_volume < 0) l_volume = 0;
					if (l_volume > 1) l_volume = 1;
					if (debug) printf("Touches : B_MASTER_OUT volume set to %f\n", l_volume);
					stream.SetVolume(B_MASTER_OUT, l_volume, l_volume);
					mGkSupVolume = (uint32)(10*l_volume);
				}
			}
			break;

		case '.':
		case '/':
		case '*':
			if(1)
			{
				BDACStream stream;
				status_t err;
				float l_volume;
				float r_volume;
				bool enabled=false;
				err = stream.GetVolume(B_ADC_IN, &l_volume, &r_volume, &enabled);
				if (err >= B_NO_ERROR && enabled)
				{
					if (r_volume > l_volume) l_volume = r_volume;
					if(bytes[0] == '*') l_volume += 0.1;
					else l_volume -= 0.1;
					if (l_volume < 0) l_volume = 0;
					if (l_volume > 1) l_volume = 1;
					if (debug) printf("Touches : B_ADC_IN volume set to %f\n", l_volume);
					stream.SetVolume(B_ADC_IN, l_volume, l_volume);
				}
			}
			break;
		case B_FUNCTION_KEY:
			if (msg && msg->HasInt32("key"))
			{
				int32 a;
				msg->FindInt32("key", &a);
				loadConfig(((a-B_F1_KEY))%12);
			}
			break;

		default:
			return false;
	}

	return true;
} // end of touches for CPulsarApp


//**********************************************
void CPulsarApp::RefsReceived(BMessage *message)
//**********************************************
{
	if(debug) {printf("APP REFS RECEIVED\n"); message->PrintToStream(); }
	if (mInterfaceWindow) mInterfaceWindow->doLoad(message);
} // end of RefsReceived for CPulsarApp


//*************************************************
void CPulsarApp::MessageReceived(BMessage *message)
//*************************************************
{
status_t err;
CFilter *ptr;
int32 index;

#if 0
	if(debug)
	{
		printf("CPulsarApp::MessageReceived : \n  ");
		message->PrintToStream();
	}
#endif

	if (mIsLoading)
	{
		inherited::MessageReceived(message);
		if(debug) printf("STILL LOADING --> CPulsarApp::MessageReceived\n");
		return;
	}


	switch(message->what)
	{
		case K_MSG_LOAD_CONFIG:
			err = message->FindInt32(K_NAME_CONFIG_INDEX, &index);
			if (err >= B_NO_ERROR) loadConfig(index);
			break;
		case K_MSG_RELOAD_ADDON:
			dynamicAddOnReload();
			break;
		case K_MSG_LOAD:
			if (mInterfaceWindow) mInterfaceWindow->askLoad();
			break;
		case K_MSG_SAVE:
			if (mInterfaceWindow) mInterfaceWindow->askSave();
			break;
		case K_MSG_SAVE_AS:
			if (mInterfaceWindow) mInterfaceWindow->askSaveAs();
			break;
		case K_LOAD_REQUESTED:
			if (mInterfaceWindow) mInterfaceWindow->doLoad(message);
			break;
		case B_SAVE_REQUESTED:
			if (mInterfaceWindow) mInterfaceWindow->doSaveAs(message);
			break;
		case K_MSG_ABOUT2:
			if (mInfoWindow && mInfoWindow->Lock())
			{
				if (!mInfoWindow->IsActive())
				{
					if (mInfoWindow->IsHidden()) mInfoWindow->Show();
					mInfoWindow->Activate();
				}
				else
				{
					mInfoWindow->Hide();
				}
				mInfoWindow->Unlock();
			}
			break;
		case K_MSG_PREF:
			if (mPrefWindow && mPrefWindow->Lock())
			{
				if (!mPrefWindow->IsActive())
				{
					if (mPrefWindow->IsHidden()) mPrefWindow->Show();
					mPrefWindow->Activate();
				}
				else
				{
					mPrefWindow->Hide();
				}
				mPrefWindow->Unlock();
			}
			break;
		case K_MSG_FILTER_INFO:
			err = message->FindPointer(K_NAME_FILTER_INFO, (void **)&ptr);
			if (err >= B_NO_ERROR )
			{
				CFilter *ptr2 = cast_as(ptr, CFilter);
				mFilterInfoWindow->setFilter(ptr2);
			}
			break;
		case K_MSG_STATE_STREAM:
			stateStream();
			break;
		case K_MSG_START_STREAM:
			startStream();
			break;
		case K_MSG_STOP_STREAM:
			stopStream();
			break;
		case K_MSG_PREVIEW_BTN:
			buttonPreview(mBpp);
			break;
		case K_MSG_SOUND_PANEL:
			system("/boot/beos/preferences/Media &");
			break;
		case K_MSG_PREPARE_TRACK_LINE:
			if (mIsRunning)
			{
				err = message->FindInt32(K_NAME_FILTER_OVER, &index);
				if (err >= B_NO_ERROR && index>=0) prepareOneFilterAddon(index);
			}
			break;
		case K_MSG_TERMINATE_TRACK_LINE:
			if (mIsRunning)
			{
				err = message->FindInt32(K_NAME_FILTER_OVER, &index);
				if (err >= B_NO_ERROR && index>=0) terminateOneFilterAddon(index);
			}
			break;
		case K_MSG_FFT_WINDOW:
				err = message->FindInt32(K_NAME_FFT_WINDOW, &index);
				if (err >= B_NO_ERROR && index>=0) setFftWindow(index);	
		default:
			inherited::MessageReceived(message);
			break;
	}

	//if(debug) printf("DONE --> CPulsarApp::MessageReceived\n");

} // end of MessageReceived for CPulsarApp


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -


//********************************
void CPulsarApp::stateStream(void)
//********************************
{
	bool new_is_running = (mButtonAudio && (mUpdateFft || mUpdateFilled))|| mGameKitRunning;

	if (mIsLoading) return;
	if (debug) printf("stateStream : AU=%d -- FT=%d -- FI=%d -- GK=%d --> %d [%d] *** bpp = %d\n",
										mButtonAudio,mUpdateFft,mUpdateFilled,mGameKitRunning,
										new_is_running, mIsRunning, mBpp);

	//if (new_is_running && !mButtonAudio) mButtonAudio = true;

	if (new_is_running && !(mUpdateFft || mUpdateFilled)) mGameKitRunning = true;

	if (new_is_running && !mIsRunning)
	{
		if (mInputMethod == K_MSG_CDINPUT_BTN && !mAudioCD.isPlaying()) mAudioCD.play(0);
		startStream();
	}
	else if (!new_is_running && mIsRunning)
		stopStream();

	mIsRunning = new_is_running;
}


//********************************
void CPulsarApp::startStream(void)
//********************************
{
	bool matroxHack = mPulsarPref.mMatroxHack;

	try
	{
		if (debug) printf("CPulsarApp::startStream\n");
		if (isRunning() || mAudio || mFullScreen || mMatroxScreen) stopStream();

		mGkSupState = 0;

		// -- start sound

		if (mMediaRoster)	// R4.5:
		{
			// BeOS R4.5 Media Kit
			// Some code is from the SoundCapture sample code
			
			// hack for testing -- get the first physical input available
			// then	Hook up input (these functions throw a const char * in case of error)
			getFirstPhysicalInput(mAudioInputNode);
			makeRecordConnection(mAudioInputNode);

			//	And get it going...
			bigtime_t then = mSoundRecorder->TimeSource()->Now()+50000LL;
			mMediaRoster->StartNode(mSoundRecorder->Node(), then);
			//if (debug) printf("then = %ld\nis audio input node kind time source -> %s\n",
			//									then, (mAudioInputNode.kind & B_TIME_SOURCE) ? "YES" : "NO");
			if (mAudioInputNode.kind & B_TIME_SOURCE)
				mMediaRoster->StartNode(mAudioInputNode, mSoundRecorder->TimeSource()->RealTimeFor(then, 0));
			else
				mMediaRoster->StartNode(mAudioInputNode, then);

			if (debug) printf("CPulsarApp::mMediaRoster->StartNode -- ok\n");
			mButtonAudio = true;
		}
		else
		{
			// BeOS PR2-R3 Old Media Kit

			mAudio = new BSubscriber("PulsarSound");
			if (debug) printf("CPulsarApp::startStream -- new audio %p\n", mAudio);
			if (!mAudio) throw("Can't launch audio subscriber.");
		
			// HOOK RM 180798 ? pourquoi c'etait _dans_ mUseDAC ?
			if (mInputMethod == K_MSG_CDINPUT_BTN)
				mAdcStream->SetADCInput(B_CD_IN);
			else if (mInputMethod == K_MSG_MICINPUT_BTN)
			{
				mAdcStream->SetADCInput(B_MIC_IN);
				mAdcStream->BoostMic(true);
			}
	
			if (mUseDAC)
			{
	
				if (mAudio->Subscribe(mDacStream) < B_NO_ERROR) throw("Can't subscribe to DAC stream.");
			}
			else
				if (mAudio->Subscribe(mAdcStream) < B_NO_ERROR) throw("Can't subscribe to ADC stream.");
		
			if (mAudio->EnterStream(NULL, mUseStreamEnd, (void *)this, streamer_fftlib, NULL, TRUE) < B_NO_ERROR)
				throw("Can't enter audio stream.");
			if (debug) printf("CPulsarApp::startStream -- ok\n");
			mButtonAudio = true;
		}

		// -- start game kit, if necessary
		
		if (mGameKitRunning)
		{
			if (mBlackWindow && mBlackWindow->Lock()) 
			{
				mBlackWindow->Show();
				mBlackWindow->Activate();
				mBlackWindow->Unlock();
			}

			if (debug) printf("starting game kit\n");
			if (matroxHack)
			{
				mMatroxScreen = new CMatroxScreen("Matrox PowerPulsar", (mBpp != 8 ? B_32_BIT_640x480 : B_8_BIT_640x480));
				mFullScreen = NULL;
				if (!mMatroxScreen || !mMatroxScreen->init()) throw("Can't start Matrox window screen.");
				mMatroxScreen->ScreenConnected(true);
			}
			else
			{
				//mFullScreen = new CScreen("Full PowerPulsar", (mBpp != 8 ? B_32_BIT_640x480 : B_8_BIT_640x480));
				uint32 space = B_8_BIT_640x480;
				if (mBpp == 32) space = B_32_BIT_640x480;
				mFullScreen = new CScreen("Full PowerPulsar", space);
				if (!mFullScreen || !mFullScreen->init()) throw("Can't start Game Kit window screen.");
			}
						
			// now wait for the screen to setup up.
			// when a BWindowScreen is createsd, the first ScreenConnected
			// just spawn up a few time after.
			// to wait for this, the screen starts with a locked semaphore
			// which is released once the first ScreenConnected applies
			// successfully. Once errors, the semaphore is deleted.
			// The semaphore is created in init().
			sem_id id;
			if (matroxHack)
				id = mMatroxScreen->getInitCompleteSem();
			else
				id = mFullScreen->getInitCompleteSem();
			if (debug) printf("init complete sem %d\n", id);
			if (id < B_NO_ERROR) throw("Internal error : Game Kit screen has no init sem.");
			
			status_t err;
			if (debug) printf("wait for end of screen init\n");
			err = acquire_sem_etc(id, 1, B_TIMEOUT, (bigtime_t)(3*1e6));
			if (debug) printf("AFTER wait for end of screen init\n");
			if (err < B_NO_ERROR) throw("Game Kit : timeout while waiting for init completion.");
			if (debug) printf("END OK of screen init\n");

			if (!matroxHack)
			{
				mFullScreen->getInfos(mFilterReference.sPrepare.sx,
														  mFilterReference.sPrepare.sy,
														  mFilterReference.sPrepare.mode,
														  mFrameInfo.screen,
														  mFilterReference.sPrepare.bpr,
														  mFilterReference.sPrepare.maxVideoLine,
														  mFilterReference.sPrepare.pulsarVideoLine);
				mFilterReference.sPrepare.sxy = mFilterReference.sPrepare.bpr * mFilterReference.sPrepare.sy;
				mFilterReference.sPrepare.gameKitScreen = mFullScreen;
				mFilterReference.sPrepare.isGameKit = true;
				mFullScreen->getHooks(mFilterReference);
			}
			else
			{
				mMatroxScreen->getInfos(mFilterReference.sPrepare.sx,
																mFilterReference.sPrepare.sy,
																mFilterReference.sPrepare.mode,
																mFrameInfo.screen,
																mFilterReference.sPrepare.bpr,
																mFilterReference.sPrepare.maxVideoLine,
																mFilterReference.sPrepare.pulsarVideoLine);
				mFilterReference.sPrepare.sxy = mFilterReference.sPrepare.bpr * mFilterReference.sPrepare.sy;
				mFilterReference.sPrepare.gameKitScreen = NULL; // ************ HOOK HOOK !!!!!
				mFilterReference.sPrepare.isGameKit = true;
				mMatroxScreen->getHooks(mFilterReference);
			}
		}
		else if (mFilledWindow)
		{
			if (!mFilledWindow->setColorSpace(getColorSpace()))
				throw "Unable to change preview's color space !";
			mFilledWindow->getInfos(mFilterReference.sPrepare.sx,
															mFilterReference.sPrepare.sy,
															mFilterReference.sPrepare.mode,
															mFrameInfo.screen,
															mFilterReference.sPrepare.bpr);
			mFilterReference.sPrepare.sxy = mFilterReference.sPrepare.bpr * mFilterReference.sPrepare.sy;
			mFilterReference.sPrepare.isGameKit = false;
		}
		mFilterReference.sPrepare.mode	 = getColorSpace();

		// -- start main thread
		if (debug) printf("start main thread\n");
		mPulsarThread = spawn_thread(CPulsarApp::pulsarThreadEntry,
																 "pulsar_thread",
																 B_NORMAL_PRIORITY,
																 (void *)this);
		if (mPulsarThread < B_NO_ERROR) throw("Can't start PowerPulsar play thread.");
		mPulsarThreadMustQuit = false;

		mIsRunning = true;
		prepareFilterAddons();

		if (debug) printf("resume thread\n");
		resume_thread(mPulsarThread);
	}
	catch(const char *s)
	{
		if (isRunning() || /* R4.5: mAudio || */ mFullScreen || mMatroxScreen) stopStream();
		if(1)
		{
			BAlert *box;
			char s2[1024];
			sprintf(s2,	"PowerPulsar\n"
									"Error while initializing game kit.\n"
									"Parameters : au=%d, ft=%d, fi=%d, gk=%d, bpp=%d, run %d\n\n"
									"%s",
									mButtonAudio,mUpdateFft,mUpdateFilled,mGameKitRunning,mBpp,
									mIsRunning, s);

			box = new BAlert("PowerPulsar - Error box", s2, "    OK    ", NULL, NULL,
							B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
			if (box) box->Go();
		}
	}
	catch(...)
	{
		if (debug) printf("startStream : Unknown exception catched\n");
		if (isRunning() || /* R4.5: mAudio || */ mFullScreen || mMatroxScreen) stopStream();
	}

} // end of startStream for CPulsarApp


//*******************************
void CPulsarApp::stopStream(void)
//*******************************
{
	if (debug) printf("CPulsarApp::stopStream\n");

	// stop everything in the reverse order of the launch

	// -- first make sure no more add-on is working...

	if (mPulsarThread >= B_NO_ERROR)
	{
		mPulsarThreadMustQuit = true;
		if (mPulsarFrameSem >= B_NO_ERROR) delete_sem(mPulsarFrameSem);
		int32 value;
		wait_for_thread(mPulsarThread, &value);
		mPulsarThread = B_ERROR;
	}

	// -- then notify add-ons that everything is giving up

	terminateFilterAddons();

	// -- then stop the fullscreen, if any

	if (mFullScreen)
	{
		if (mFullScreen->Lock()) mFullScreen->Quit();
		mFullScreen = NULL;
		mGameKitRunning = false;

		mFilledWindow->getInfos(mFilterReference.sPrepare.sx,
														mFilterReference.sPrepare.sy,
														mFilterReference.sPrepare.mode,
														mFrameInfo.screen,
														mFilterReference.sPrepare.bpr);
		mFilterReference.sPrepare.sxy = mFilterReference.sPrepare.bpr * mFilterReference.sPrepare.sy;
		mFilterReference.sPrepare.isGameKit = false;
	}
	else if (mMatroxScreen)
	{
		if (mMatroxScreen->Lock()) mMatroxScreen->Quit();
		mMatroxScreen = NULL;
		mGameKitRunning = false;

		mFilledWindow->getInfos(mFilterReference.sPrepare.sx,
														mFilterReference.sPrepare.sy,
														mFilterReference.sPrepare.mode,
														mFrameInfo.screen,
														mFilterReference.sPrepare.bpr);
		mFilterReference.sPrepare.sxy = mFilterReference.sPrepare.bpr * mFilterReference.sPrepare.sy;
		mFilterReference.sPrepare.isGameKit = false;
	}

	if (mUpdateFilled && mFilledWindow)
	{
		mFilledWindow->Minimize(TRUE);
		mUpdateFilled = false;
	}

	// -- and finally remove the audio stream

	if (mAudio)
	{
		mAudio->ExitStream(TRUE);
		mAudio->Unsubscribe();
		delete mAudio;
		mAudio = NULL;
		mButtonAudio = false;
	}

	if (mMediaRoster)	// R4.5
	{
		// BeOS R4.5 Media Kit
		// Some code is from the SoundCapture sample code

		//	If we are the last connection, the Node will stop automatically since it
		//	has nowhere to send data to.
		status_t err;
		if (mRecInput.destination != media_destination::null)
		{
			err = mMediaRoster->StopNode(mRecInput.node, 0);
			if (debug) printf("mMediaRoster->StopNode -> err %d, %s\n", err, strerror(err));
			err = mMediaRoster->Disconnect(mAudioOutput.node.node, mAudioOutput.source, mRecInput.node.node, mRecInput.destination);
			if (debug) printf("mMediaRoster->Disconnect -> err %d, %s\n", err, strerror(err));
			mAudioOutput.source = media_source::null;
			mRecInput.destination = media_destination::null;
		}
		mSoundRecorder->SetHooks(0, 0, 0);

		mButtonAudio = false;
	}

	mIsRunning = false;

} // end of stopStream for CPulsarApp


//******************************************
void CPulsarApp::setInputMethod(int32 input)
//******************************************
/*
#define K_MSG_CDINPUT_BTN	'cdIN'
#define K_MSG_AIFFINPUT_BTN	'aiIN'
#define K_MSG_MP3INPUT_BTN	'm3IN'
#define K_MSG_MODINPUT_BTN	'moIN'
#define K_MSG_MICINPUT_BTN	'miIN'
*/
{
	mInputMethod = input;
	if (isRunning() && mAudio)
	{
		bool newdac = (mInputMethod != K_MSG_CDINPUT_BTN && mInputMethod != K_MSG_MICINPUT_BTN);
		if (newdac != mUseDAC)
		{
			mUseDAC = newdac;
			stopStream();
		}
		else if (mAdcStream)
		{
			if (mInputMethod == K_MSG_CDINPUT_BTN)
				mAdcStream->SetADCInput(B_CD_IN);
			else if (mInputMethod == K_MSG_MICINPUT_BTN)
			{
				mAdcStream->SetADCInput(B_MIC_IN);
				mAdcStream->BoostMic(true);
			}
		}
	}
} // end of setInputMethod for CPulsarApp


//*******************************************************
void CPulsarApp::getFirstPhysicalInput(media_node &input)
//*******************************************************
{
	live_node_info live[30];
	int32 live_count = 30;
	status_t err;
	err = mMediaRoster->GetLiveNodes(live, &live_count,
																	 NULL, //const media_format *hasInput
																	 NULL, //const media_format *hasOutput
																	 NULL, //const char *name
																	 0);   //uint64 nodeKinds
	if (err < B_OK)
	{
		printf("getFirstPhysicalInput::err = %ld -- %s\n", err, strerror(err));
		throw strerror(err);
	}

	for(int32 i=0; i<live_count; i++)
	{
		media_node &node = live[i].node;
		if (node.kind & B_PHYSICAL_INPUT)
		{
			if (debug) printf("getFirstPhysicalInput -> live node[%ld] -- live name = '%s'\n"
												"Node:\n\tid = %lx\n\tport = %ld\n\tkind = %lx\n",
												i, live[i].name, node.node, node.port, node.kind);
			input = node;
			return;
		}
	}

	if (debug) printf("getFirstPhysicalInput - B_PHYSICAL_INPUT NOT FOUND !!??!\n");
	throw "B_PHYSICAL_INPUT NOT FOUND";

} // end of getFirstPhysicalInput for CPulsarApp


//************************************************************
void CPulsarApp::makeRecordConnection(const media_node &input)
//************************************************************
{
	status_t err;
	int32 count = 0;

	//	Find an available output for the given input node.
	err = mMediaRoster->GetFreeOutputsFor(input, &mAudioOutput, 1, &count, B_MEDIA_RAW_AUDIO);
	if (err < B_OK)
	{
		if (debug) printf("makeRecordConnection::GetFreeOutputsFor err = %ld -- %s\n", err, strerror(err));
		throw strerror(err);
	}

	if (count < 1)
	{
		if (debug) printf("makeRecordConnection::GetFreeOutputsFor -> no free outputs\n");
		throw strerror(err);
	}

	//	Find an available input for our own Node. Note that we go through the
	//	MediaRoster; calling Media Kit methods directly on Nodes in our app is
	//	not OK (because synchronization happens in the service thread, not in
	//	the calling thread).
	// TODO: explain this
	err = mMediaRoster->GetFreeInputsFor(mSoundRecorder->Node(), &mRecInput, 1, &count, B_MEDIA_RAW_AUDIO);
	if (err < B_OK)
	{
		if (debug) printf("makeRecordConnection::GetFreeInputsFor err = %ld -- %s\n", err, strerror(err));
		throw strerror(err);
	}

	if (count < 1)
	{
		if (debug) printf("makeRecordConnection::GetFreeInputsFor -) no free inputs\n");
		throw strerror(err);
	}

	//	Find out what the time source of the input is.
	//	For most nodes, we just use the preferred time source (the DAC) for synchronization.
	//	However, nodes that record from an input need to synchronize to the audio input node
	//	instead for best results.
	//	MakeTimeSourceFor gives us a "clone" of the time source node that we can manipulate
	//	to our heart's content. When we're done with it, though, we need to call Release()
	//	on the time source node, so that it keeps an accurate reference count and can delete
	//	itself when it's no longer needed.
	// TODO: what about filters connected to audio input?
	media_node use_time_source;
	BTimeSource * tsobj = mMediaRoster->MakeTimeSourceFor(input);
	if (!tsobj)
	{
		if (debug) printf("makeRecordConnection::MakeTimeSourceFor - NULL\n");
		throw strerror(err);
	}

	//	Apply the time source in effect to our own Node.
	err = mMediaRoster->SetTimeSourceFor(mSoundRecorder->Node().node, tsobj->Node().node);
	if (err < B_OK)
	{
		if (debug) printf("makeRecordConnection::SetTimeSourceFor err = %ld -- %s\n", err, strerror(err));
		tsobj->Release();
		throw strerror(err);
	}

	//	Get a format, any format.
	media_format fmt;
	fmt.u.raw_audio = mAudioOutput.format.u.raw_audio;
	fmt.type = B_MEDIA_RAW_AUDIO;

	//	Tell the consumer where we want data to go.
	err = mSoundRecorder->SetHooks(recordProcessFunc, recordNotifyFunc, this);
	if (err < B_OK)
	{
		if (debug) printf("makeRecordConnection::SetHooks err = %ld -- %s\n", err, strerror(err));
		tsobj->Release();
		throw strerror(err);
	}

	//	Using the same structs for input and output is OK in BMediaRoster::Connect().
	err = mMediaRoster->Connect(mAudioOutput.source, mRecInput.destination, &fmt, &mAudioOutput, &mRecInput);
	if (err < B_OK)
	{
		if (debug) printf("makeRecordConnection::Connect err = %ld -- %s\n", err, strerror(err));
		tsobj->Release();
		mSoundRecorder->SetHooks(0, 0, 0);
		throw strerror(err);
	}

	//	Start the time source if it's not running.
	if (tsobj->Node() != input && !tsobj->IsRunning())
		mMediaRoster->StartNode(tsobj->Node(), BTimeSource::RealTime());

	tsobj->Release();	//	we're done with this time source instance!

	//	And get it going...
	bigtime_t then = mSoundRecorder->TimeSource()->Now()+50000LL;
	mMediaRoster->StartNode(mSoundRecorder->Node(), then);
	if (mAudioInputNode.kind & B_TIME_SOURCE)
		mMediaRoster->StartNode(mAudioInputNode, mSoundRecorder->TimeSource()->RealTimeFor(then, 0));
	else
		mMediaRoster->StartNode(mAudioInputNode, then);


} // end of makeRecordConnection for CPulsarApp


//**********************************************************************
void CPulsarApp::recordProcessFunc(void * cookie, bigtime_t timestamp, 
																	 void * data, size_t size,
																	 const media_raw_audio_format &format)
//**********************************************************************
{
	//	Callback called from the SoundConsumer when receiving buffers.

	//if (debug) printf("recordProcessFunc: data %p -- size %ld -- format 0x%04lx -- channels %ld\n",
	//					data, size, format.format, format.channel_count);

	#define _SI16 media_raw_audio_format::B_AUDIO_SHORT				// 0x02
	#define _FP32 media_raw_audio_format::B_AUDIO_FLOAT				// 0x24 (?!!)

	assert(format.channel_count);
	assert((format.format & _SI16) || (format.format & _FP32));
	
	if ((format.format & _SI16) == _SI16)
		streamer_fftlib(cookie, (char *)data, size, NULL);
	else if ((format.format & _FP32) == _FP32)
		streamer_fftlib_float((float *)data, size/(sizeof(float)*format.channel_count));

	#undef _FP32
	#undef _SI16
}


//***************************************************************
void CPulsarApp::recordNotifyFunc(void * cookie, int32 code, ...)
//***************************************************************
{
	if (debug) printf("recordNotifyFunc -- cookie %p -- code %d\n", cookie, code);
	// Tell the app we've stopped, if it doesn't already know.
	// if (code == B_WILL_STOP || code == B_NODE_DIES)
	if (code == B_NODE_DIES)
		be_app->PostMessage(K_MSG_STOP_STREAM);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -


//***************************************************
void CPulsarApp::buttonUpdatePulsar(BCheckBox *check)
//***************************************************
{
	if (mIsLoading) return;
	mUpdateFft = check->Value();
} // end of buttonUpdatePulsar for CPulsarApp


//***************************************************
void CPulsarApp::buttonUpdateFilled(BCheckBox *check)
//***************************************************
{
	if (mIsLoading) return;
	long val = check->Value();
	if (val)
	{
		stopStream();
		mBpp = 8;
		mUpdateFilled = true;
		//prepareFilterAddons();
		stateStream();
	}
	else
	{
		//terminateFilterAddons();
		stopStream();
		mUpdateFilled = false;
	}
} // end of buttonUpdateFilled for CPulsarApp




//******************************************
void CPulsarApp::buttonSwitchInterface(void)
//******************************************
{
	if (mIsLoading) return;
	if (mInterfaceWindow && mInterfaceWindow->Lock())
	{
		if (mInterfaceWindow->IsHidden())
			mInterfaceWindow->Show();
		else
			mInterfaceWindow->Hide();
		mInterfaceWindow->Unlock();
	}

} // end of switchInterface for CPulsarApp



//*******************************************
void CPulsarApp::buttonFullScreen(uint32 bpp)
//*******************************************
{
	if (mIsLoading) return;
	if (mGameKitRunning || isRunning()) stopStream();
	if (mFilledWindow) mFilledWindow->Minimize(TRUE);
	mUpdateFft=false;
	mUpdateFilled=false;
	/*if (bpp > 0)*/ mBpp = bpp;
	mGameKitRunning = true;
	stateStream();
} // end of buttonFullScreen for CPulsarApp


//****************************************
void CPulsarApp::buttonPreview(uint32 bpp)
//****************************************
{
	bool do_it = mGameKitRunning;
	if (mIsLoading) return;
	do_it = mGameKitRunning || !isRunning();
	if (mGameKitRunning || isRunning()) stopStream();
	if (mFilledWindow) mFilledWindow->Minimize(!do_it);
	if (do_it)
	{
		mBpp = bpp;
		mButtonAudio=true;
		mUpdateFft=false;
		mUpdateFilled=true;
		mGameKitRunning=false;
		stateStream();
	}
} // end of buttonPreview for CPulsarApp


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -


//************************************************
void CPulsarApp::loadBitmapError(const char *file)
//************************************************
{
	char s[1024];

	if (debug) printf("Couldn't load %s with the Translation Kit\n",file);

	sprintf(s, "\"PowerPulsar\" for BeOS"
						 "\nUnable to load %s file "
						 "with the Translation Kit. Make sure that you have "
						 "have installed the needed library (PNG, JPG, TGA) "
						 "and that the file exists, then retry.", file);

	BAlert *box;
	box = new BAlert("PowerPulsar - Error box", s, "Force", "Quit");
	if (box)
	{
		box->SetShortcut(0, B_ESCAPE);
		if (box->Go()) throw("Unable to load bitmaps, giving up. Check you have the img_pulsar/ directory "
												 "and that the Translation Kit's handlers are correctly installed.\n"
												 "Support : <raphael@powerpulsar.com>\n:-)");
	}

} // end of loadBitmapError for CPulsarApp


//***********************************************
BBitmap * CPulsarApp::getBitmap(const char *path)
//***********************************************
{
#if K_USE_DATATYPE	
	BFile file(path, B_READ_ONLY);
	if (file.InitCheck() < B_NO_ERROR || !file.Exists()) return NULL;
	BitmapStream output;
	uint32 ext=0;
	uint32 len=strlen(path);
	if(len > 4) strncpy((char *)&ext,&path[len-4],4);
	if (   (ext=='.png' && !DATATranslate(file, NULL, NULL, output, DATA_BITMAP, 'PNGg'))
		  || (ext=='.tga' && !DATATranslate(file, NULL, NULL, output, DATA_BITMAP, 'TGA '))
		  || (ext=='.jpg' && !DATATranslate(file, NULL, NULL, output, DATA_BITMAP, 'JPEG')))
	{
		BBitmap *ret = NULL;
		if (output.DetachBitmap(ret))
			return NULL;
		return ret;
	}
	else return GetBitmap(path);

#else

	BFile file(path, B_READ_ONLY); 
	BTranslatorRoster *roster = gTranslationRoster;
	if (!roster) roster = BTranslatorRoster::Default(); 
	BBitmapStream stream; 
	BBitmap *result = NULL; 
	if (roster->Translate(&file, NULL, NULL, &stream, B_TRANSLATOR_BITMAP) < B_OK) 
		return NULL; 
	stream.DetachBitmap(&result); 
	return result; 

#endif
}


//*******************************
bool CPulsarApp::loadBitmap(void)
//*******************************
{
	BPath path(getAppDir(),K_FOND_FFT_FILENAME);
	//if (!result) result=parent.FindEntry(K_SOUND_DIR_NAME, &dummy);
	//if (!result) result=mDirSon.SetTo(&dummy);

	if(debug)printf("loadBitmap\n");

	mFondFft = getBitmap(path.Path());
	if (mFondFft) return true;

	loadBitmapError(K_FOND_FFT_FILENAME);
	return false;

} // end of loadBitmap for CPulsarApp



//**************************************************************
void CPulsarApp::setFilterRef(int32 sx, int32 sy, uint8 *screen,
														  int32 bpr, EColorSpace mode,
														  BWindowScreen *game_kit)
//**************************************************************
{
	mFilterReference.sPrepare.sx						= sx;
	mFilterReference.sPrepare.sy						= sy;
	mFilterReference.sPrepare.sxy						= sy*bpr;
	mFrameInfo.screen												= screen;
	mFilterReference.sPrepare.bpr						= bpr;
	mFilterReference.sPrepare.mode					= mode;
	mFilterReference.sPrepare.isGameKit			= (game_kit != NULL);
	mFilterReference.sPrepare.gameKitScreen	= game_kit;

} // end of setFilterRef for CPulsarApp


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -


//*************************************
void CPulsarApp::loadFilterAddons(void)
//*************************************
{
	filterInitProto func = NULL;
	status_t err;
	uint32 index;
	CFilter *info = NULL;
	bool correct;
	entry_ref next_ref;
	TRArray<entry_ref> ref_array;

	debug=1;
	if (debug) printf("loadFilterAddons\n");

	BDirectory addonDir(mAddonDir.Path());
	if (debug) printf("number of entries %d\n", addonDir.CountEntries());
	while(addonDir.GetNextRef(&next_ref) >= B_NO_ERROR)
		ref_array.append(next_ref);

	if (debug) printf("number of paths %d\n", ref_array.size());
	
	for(int32 ref_index=0; ref_index<ref_array.size(); ref_index++)
	{
		BPath path;
		BEntry entry(& (ref_array[ref_index]));
		if (entry.InitCheck() != B_OK || !entry.Exists()) continue;
		err = entry.GetPath(&path);
		if (err < B_NO_ERROR)
		{
			if (debug) printf("  ERROR get path(%s) -> %08X - %s\n", ref_array[ref_index].name, err, strerror(err));
			continue;
		}
		if (debug) printf("  Addon path %s\n", path.Path());

		image_id iid = load_add_on(path.Path());
		if (iid < B_NO_ERROR) if (debug) printf("  %s was NOT an addon !!\n", path.Path());
		if (iid < B_NO_ERROR) continue;	// not an add-on - give up

	//if (debug) printf(" Doing get_image_symbol for iid %d\n", iid);

#ifdef __POWERPC__
		err = get_image_symbol(iid, "filterInit__FUl", B_SYMBOL_TYPE_TEXT, (void **)&func);
//#elif __INTEL__
#else

	err = get_image_symbol(iid, "filterInit", B_SYMBOL_TYPE_TEXT, (void **)&func);

	if(debug && err<B_NO_ERROR)
	{
		// liste symbols
		printf("___ Dumping SYMBOLS ___\n");
		status_t err = B_NO_ERROR;
		for(long index=0; err == B_NO_ERROR; index++)
		{
			char buf[256];
			int32 bufsize = 256;
			int32 sclass;
			void *ptr;
			printf(" ### SYMBOL[%02d] --> ", index);
			err = get_nth_image_symbol(iid, index, buf, &bufsize, &sclass, &ptr);
			if (err != B_NO_ERROR) printf("error %x [%d]\n", err,err);
			else printf("Name = %s [%d], Class = %d, ptr = %p\n", buf, bufsize, sclass, ptr);
		}
	}
	else printf("get_image_symbol --> err %d, func %p\n", err,func);

//#else
//	#error Not PowerPC nor Intel
#endif

		if (err < B_NO_ERROR) continue;	// not the right function - give up

		if (!func) continue;	// bad value for a func adr - give up

		if (debug) printf("new add-on %s	\n", path.Path());

		for(index = 0; ; index++)
		{
			info = (CFilter *)func(index);
			if (!info) break; // no more filters to declare

			// reject object if it is not derived from CFilter
			// it MUST be a derived class, in fact.
			if (!is_kind_of(info, CFilter) || is_instance_of(info, CFilter))
			{
				if (debug) printf("filter addd-on class was not good -- it was %s\n", class_name(info));
				delete info;
				break;
			}
			if (debug) printf("loading class %s at %p\n", class_name(info), info);

			correct = (info->sFilter.supportedMode & kColor8Bit)
					||(info->sFilter.supportedMode & kRgb32Bit);
			if (correct)
			{
				// set primordial info in structure...
				info->sLoad.iid									= iid;
				info->sLoad.addonEntry					= ref_array[ref_index];
				info->sLoad.dirApplication			= mFilterReference.sLoad.dirApplication;
				info->sLoad.dirAddon						= mFilterReference.sLoad.dirAddon;
				info->sLoad.dirImg							= mFilterReference.sLoad.dirImg;
				info->sLoad.majorPulsarVersion 	= K_MAJOR_PULSAR_VERSION;
				info->sLoad.minorPulsarVersion 	= K_MINOR_PULSAR_VERSION;
				info->sLoad.index								= index;
//				info->sLoad.mAddonSetting				= &mPreference;	[RM 060100 TBDL]

				info->sFilter.modelParam.AddRect("position", BRect(0,0,639,479));		// HOOK THIS IS UGLY !! -- 091097
				info->sFilter.modelParam.AddPoint("center",  BPoint(320,200));			// HOOK THIS IS UGLY !! -- 091097

				info->sLoad.enabled = info->load();
				if (info->sLoad.enabled)
				{

					// search the ordered position where to put the filter in the list
					EFilterPosition pos = info->sFilter.position;
					const float epsilon = 1e-2;
					if (pos <= kFilterPositionFirst+epsilon)
					{
						mFilterList.AddItem(info, 0);	// insert at beginning
					}
					else if (pos >= kFilterPositionLast-epsilon)
					{
						mFilterList.AddItem(info);		// insert at end
					}
					else
					{
						// insert in the middle
						int32 count = mFilterList.CountItems();
						int32 i;
						if (!mFilterList.HasItem(info)) mFilterList.AddItem(info); // insert at end
						else for(i=0; i<count; i++)
						{
							CFilter *filter = (CFilter *)mFilterList.ItemAt(i);
							if (filter && pos < filter->sFilter.position)
							{
								mFilterList.AddItem(info, i);
								break;
							}
						}
					}

					if (!info->sFilter.icon)
					{
						// create a new bitmap of the adequate size
						info->sFilter.icon = new BBitmap(mLargeAppIcon.Bounds(),
																						 mLargeAppIcon.ColorSpace());
						if (info->sFilter.icon)
						{
							// setup the icon with the one of the image file if any
							BNode noeud(&entry);
							BNodeInfo nodeinfo(&noeud);
							err = nodeinfo.GetIcon(info->sFilter.icon, B_LARGE_ICON);
	
							// else by default use PowerPulsar nice icon (thx Jim !!)
							if (err < B_NO_ERROR)
								info->sFilter.icon->SetBits(mLargeAppIcon.Bits(),
																					 mLargeAppIcon.BitsLength(),
																					 0, mLargeAppIcon.ColorSpace());
						}
					}

					if (debug) printf("Loaded filter :\n\tname %s\n\tauthor %s\n\tinfo %s\n",
														(info->sFilter.name ? info->sFilter.name : "no name"),
														(info->sFilter.author ? info->sFilter.author : "no author"),
														(info->sFilter.info ? info->sFilter.info : "no info"));
				} // endif if load enabled
			}
			else
			{
				if (debug) printf("Wrong color mode, class instance is rejected !\n");
				delete info;
			}
		} // for i in filter class instances
	} // for path_index

	if(1)
	{
		CFilter *first = (CFilter *)mFilterList.ItemAt(0);
		if (first) mFilterReference.sFilter.eraseScreen = first->sFilter.eraseScreen;
	}


} // end of loadFilterAddons for CPulsarApp


//***************************************
void CPulsarApp::unloadFilterAddons(void)
//***************************************
{
	CFilter *info;
	int32 i;

	if (debug) printf("unloadFilterAddons\n");

	while((i = mFilterList.CountItems()-1) >= 0)
	{
		info = (CFilter *)mFilterList.RemoveItem(i);
		if (!info) continue;
		info->unload();
		image_id iid = info->sLoad.iid;
		delete info;

		// search every other add-on with the same image id
		for(int32 nb=mFilterList.CountItems(); nb>0;)
		{
			info = (CFilter *)mFilterList.RemoveItem(--nb);
			if (info && info->sLoad.iid == iid)
			{
				info->unload();
				delete info;
			}
		}
		
		// really unload the addon
		unload_add_on(iid);
	}

} // end of unloadFilterAddons for CPulsarApp


//****************************************
void CPulsarApp::prepareFilterAddons(void)
//****************************************
{
	int32 i,nb;
	bool first=true;

	if (debug) printf("prepareFilterAddons\n");

	// reset frame counter
	mFrameInfo.frame = 0;

	if (!gSimpleTrack->lock()) return;
	nb = gSimpleTrack->countLine();
	gSimpleTrack->unlock();

	for(i=0; i<nb; i++) prepareOneFilterAddon(i, &first);

} // end of prepareFilterAddons for CPulsarApp


//**************************************************************
void CPulsarApp::prepareOneFilterAddon(int32 index, bool *first)
//**************************************************************
{
	CFilter *info;
	EColorSpace currentmode = mFilterReference.sPrepare.mode;

	if (debug) printf("prepareOneFilterAddon -- index %d\n",index);

	bigtime_t t = (bigtime_t)(system_time() / 1e3);	// in ms
	double current = t;

	if (!gSimpleTrack->lock()) return;
	SLineParam *line = gSimpleTrack->getLine(index);
	if (!line)
	{
		gSimpleTrack->unlock();
		return;
	}

	info = line->filter;

	if (!info || !info->sLoad.enabled || info->sLoad.running || (info->sFilter.supportedMode < currentmode))
	{
		gSimpleTrack->unlock();
		return;
	}

	line->current_start  = current + line->start;
	line->current_stop   = current + line->start + line->len;
	line->current_repeat = current + line->start + line->repeat;

	info->sPrepare.sx				= mFilterReference.sPrepare.sx;
	info->sPrepare.sy				= mFilterReference.sPrepare.sy;
	info->sPrepare.sxy			= mFilterReference.sPrepare.sxy;
	info->sPrepare.isGameKit= mFilterReference.sPrepare.isGameKit;
	info->sPrepare.mode			= mFilterReference.sPrepare.mode;
	info->sPrepare.bpr			= mFilterReference.sPrepare.bpr;
	info->sPrepare.mode			= mFilterReference.sPrepare.mode;
	info->sPrepare.param		= new BMessage(info->sFilter.modelParam);
	if (!info->sPrepare.param)
	{
		if (debug)
		{
			printf("### failed to create a copy of modelParam !!\n");
			info->sFilter.modelParam.PrintToStream();
		}
		gSimpleTrack->unlock();
		return;
	}

	info->sPrepare.gameKitScreen	= mFilterReference.sPrepare.gameKitScreen;
	info->sPrepare.maxVideoLine		= mFilterReference.sPrepare.maxVideoLine;
	info->sPrepare.pulsarVideoLine= mFilterReference.sPrepare.pulsarVideoLine;

	for(int32 i=0; i<256; i++)
	{
		info->sPrepare.colorList[i] = mFilterReference.sPrepare.colorList[i];
		info->sPrepare.colorMap[i]  = mFilterReference.sPrepare.colorMap[i];
	}

	info->sFrame = &mFrameInfo;

	if ((first && *first) || (!first && index==0))
	{
		mFilterReference.sFilter.eraseScreen = info->sFilter.eraseScreen;
		if (first) *first = false;
	}

	info->line_8				= mFilterReference.line_8;
	info->line_32				= mFilterReference.line_32;
	info->rect_8				= mFilterReference.rect_8;
	info->rect_32				= mFilterReference.rect_32;
	info->blit					= mFilterReference.blit;
	info->invert_rect_32= mFilterReference.invert_rect_32;

	// return true or false if accept to run
	info->sLoad.running = info->prepare();

	gSimpleTrack->unlock();

} // end of prepareOneFilterAddon for CPulsarApp


//******************************************
void CPulsarApp::terminateFilterAddons(void)
//******************************************
{
int32 nb = mFilterList.CountItems();
int32 i;

	if (debug) printf("terminateFilterAddons\n");

	for(i=0; i<nb; i++) terminateOneFilterAddon(i);

} // end of terminateFilterAddons for CPulsarApp


//*************************************************************************
void CPulsarApp::terminateOneFilterAddon(int32 index, bool continueRunning)
//*************************************************************************
{
CFilter *info;

	info = (CFilter *)mFilterList.ItemAt(index);
	if (!info || !info->sLoad.enabled || !info->sLoad.running) return;

	// deactivate the addons inside the simple track lock
	// (thus the playback loop is not currently accessing tracks)

	if (gSimpleTrack->lock())
	{
		int32 count=0;
		if (continueRunning)
		{
			// search if this if there is more than one line with this addon
			// if no and if the other line must continue running then the filter
			// must not be terminated now.
		
			int32 nb = gSimpleTrack->countLine();
			int32 i;
	
			for(i=0; i<nb; i++)
			{
				SLineParam *line = gSimpleTrack->getLine(i);
				if (line && line->filter == info) count++;
			}
		}

		// addon is ready to be deactivated
		if (count == 0)
			info->sLoad.running = false;

		gSimpleTrack->unlock();
	}

	// if addon was correctly deactivated, terminate it
	if (info->sLoad.running == false)
	{
		info->terminate();
		delete info->sPrepare.param;
		info->sPrepare.param = NULL;
		info->sFrame = NULL;
	}

} // end of terminateOneFilterAddon for CPulsarApp


//*********************************************
void CPulsarApp::processFrameFilterAddons(void)
//*********************************************
{
CFilter *info;
int32 i,nb;
bool is8 = (mBpp == 8);
static bigtime_t last_time = 0;

	if (!gSimpleTrack->lock()) return;
	nb = gSimpleTrack->countLine();
	gSimpleTrack->unlock();
	if (!nb) return;

	bigtime_t t = (bigtime_t)system_time();
	double fps = 1.0e6/(double)(t-last_time);
	last_time = t;
	mFrameInfo.lastFps = fps;

	double current = t/1e3;

	if (!mFilterReference.sFilter.eraseScreen)
	{
		bool screen_to_be_erase = true;

		if(mFilterReference.sPrepare.isGameKit)
		{
			long sx=mFilterReference.sPrepare.sx-1;
			long sy1=mFrameInfo.startingLine;
			long sy2=mFilterReference.sPrepare.sy-1+sy1;
			if (mFilterReference.sPrepare.mode == kColor8Bit && mFilterReference.rect_8)
			{
				mFilterReference.rect_8(0,sy1,sx,sy2,0);
				screen_to_be_erase=false;
			}
			else if (mFilterReference.sPrepare.mode == kRgb32Bit && mFilterReference.rect_32)
			{
				mFilterReference.rect_32(0,sy1,sx,sy2,0);
				screen_to_be_erase=false;
			}
		}

		if (screen_to_be_erase)
		{
			uchar *base=mFrameInfo.screen;
			int32 len=mFilterReference.sPrepare.sxy;

#ifdef __POWERPC__
			if (((long)base & 0x7) == 0 && (len & 0x7) == 0)
			{
					double *d = (double *)(base-8);	// pour utiliser lfdzu des le depart
					double zero;
					long *z = (long *)(&zero);
					*(z++) = 0;
					*z = 0;
					register double regzero = zero;
					for(len=len/8/4; len>0; len--)
					{
						*(++d) = regzero;
						*(++d) = regzero;
						*(++d) = regzero;
						*(++d) = regzero;
					}
			}
			else
#endif
				memset(base, 0, len);
		}
	}
	int32 f = mFrameInfo.frame++;
	mFrameInfo.currentTime = current;

	if (gSimpleTrack->lock())
	{
		for(i=0; i<nb; i++)
		{
			SLineParam *line = gSimpleTrack->getLine(i);
			if (!line) continue;
			ELineMode mode = line->mode;
			info = line->filter;
			if (!info || !info->sLoad.enabled || !info->sLoad.running || mode==kLineModeOff) continue;

			mFrameInfo.param = info->sPrepare.param;
			mFrameInfo.paramChanged = false;
			bool oldinblock = line->inBlock;
	
			if (mode == kLineModeRepeat && current >= line->current_repeat)
			{
				line->current_start		= current;
				line->current_stop		= 	current + line->len;
				line->current_repeat	= current + line->repeat;
			}
			if (current >= line->current_start && (mode == kLineModeOn || current <= line->current_stop))
			{
				if (oldinblock == false)
				{
					line->inBlock = true;
					mFrameInfo.blockStart = true;
					mFrameInfo.blockIndex = (++line->inBlock);
				}
				else
					mFrameInfo.blockStart = false;
				
				if (is8)
					info->processFrame8(mFrameInfo);
				else
					info->processFrame32(mFrameInfo);
			}
			else
				line->inBlock = false;
		}
		gSimpleTrack->unlock();
	}

	if (mInvertGameKitScreen && mFilterReference.sPrepare.isGameKit)
	{
		long sx=mFilterReference.sPrepare.sx-1;
		long sy1=mFrameInfo.startingLine;
		long sy2=mFilterReference.sPrepare.sy-1+sy1;
		if (mFilterReference.invert_rect_32)
			mFilterReference.invert_rect_32(0,sy1,sx,sy2);
		else
		{
			uchar *base=mFrameInfo.screen;
			int32 len=mFilterReference.sPrepare.sxy;

			if (((long)base & 0x3) == 0 && (len & 0x3) == 0)
			{
				uint32 *d1 = (uint32 *)(base-4*4);
				uint32 *d2 = d1+1;
				uint32 *d3 = d2+1;
				uint32 *d4 = d3+1;
				for(len=len/4/4; len>0; len--)
				{
					register uint32 r1,r2,r3,r4;
					d1 += 4;
					d2 += 4;
					d3 += 4;
					d4 += 4;
					r1=*d1;			r1 = ~r1;
					r2=*d2;			r2 = ~r2;
					r3=*d3;			r3 = ~r3;
					r4=*d4;			r4 = ~r4;
					*d1 = r1;
					*d2 = r2;
					*d3 = r3;
					*d4 = r4;
				}
			}
		}
	}

	if (mGkSupState) drawGkSup();

	if (mPulsarPref.mDisplayFrameRate)
		if (!is8)
		{
			int32 bpr=mFilterReference.sPrepare.bpr;
			uint8 *d=mFrameInfo.screen;
			long value=(long)fps;
			uchar *c=chiffres_bits;
			if (c && d && bpr)
			{
				uint32 b2[2] = { 0x0, 0xFFFFFFFF };
				long a;
				d+=(mFilterReference.sPrepare.sy-9)*bpr+(mFilterReference.sPrepare.sx-8)*4;
				for(long j=0; j<4; j++)
				{
					a = value%10;
					uint8 *src=(c+8*a);
					uint32 *dest=(uint32 *)d;
					uint32 destbpr = bpr/4 - 7;
					for(long i=0; i<8; i++)
					{
						*(dest++)= b2[ (*(src++)) & 0x01];
						*(dest++)= b2[ (*(src++)) & 0x01];
						*(dest++)= b2[ (*(src++)) & 0x01];
						*(dest++)= b2[ (*(src++)) & 0x01];
						*(dest++)= b2[ (*(src++)) & 0x01];
						*(dest++)= b2[ (*(src++)) & 0x01];
						*(dest++)= b2[ (*(src++)) & 0x01];
						*dest    = b2[ (* src   ) & 0x01];
						dest+=destbpr;
						src+=80-7;
					}
					d-=8*4;
					value = value/10;
				}
			}
		}
		else
		{
			int32 bpr=mFilterReference.sPrepare.bpr;
			uchar *d=mFrameInfo.screen;
			long value=(long)fps;
			uchar *c=chiffres_bits;
			//printf("chiffres %c, bpr %d, screen %p\n", c,bpr,d);
			if (c && d && bpr)
			{
				long a;
				d+=(mFilterReference.sPrepare.sy-9)*bpr+(mFilterReference.sPrepare.sx-8);
				for(long j=0; j<4; j++)
				{
					a = value%10;
					uint32 *src=(uint32 *)(c+8*a);
					uint32 *dest=(uint32 *)d;
					uint32 destbpr = bpr/4;
					for(long i=0; i<8; i++)
					{
						dest[0]=src[0];
						dest[1]=src[1];
						dest+=destbpr;
						src+=20;
					}
					d-=8;
					value = value/10;
				}
			}
		}

} // end of processFrameFilterAddons for CPulsarApp


//*************************************************
bool CPulsarApp::handleMessageAddons(BMessage *msg)
//*************************************************
// 'msg' comes from the window UI
{
	bool processed = false;
	// if performing, ask add-ons about it...

	if (isRunning() && gSimpleTrack && gSimpleTrack->lock())
	{
		int32 nb = gSimpleTrack->countLine();
		for(int32 i=0; i<nb && !processed; i++)
		{
			SLineParam *line = gSimpleTrack->getLine(i);
			if (!line) continue;
			ELineMode mode = line->mode;
			CFilter *info = line->filter;
			if (!info || !info->sLoad.enabled || !info->sLoad.running || mode == kLineModeOff) continue;

			processed = info->handleMessage(msg);
		}
		gSimpleTrack->unlock();
	}

	if (!processed && msg->what == B_KEY_DOWN)
	{
		char byte;
		msg->FindInt8("byte", (int8 *)&byte);
		processed = gApplication->touches(&byte, 1, msg);
	}

	return processed;
}



//---------------------------------------------------------------------------
#pragma mark -


//*********************************************
int32 CPulsarApp::pulsarThreadEntry(void *data)
//*********************************************
{
CPulsarApp *elthis = (CPulsarApp *)data;
	if (data) elthis->pulsarThread();
	return 0;
} // end of pulsarThreadEntry for CPulsarApp


//*********************************
void CPulsarApp::pulsarThread(void)
//*********************************
{
	status_t err;
	bigtime_t frametempo;
	bool matroxHack = mPulsarPref.mMatroxHack;

	try
	{
		// -- init
		mPulsarFrameSem = create_sem(0, "frame_sem");
		if (mPulsarFrameSem < B_NO_ERROR) throw(1);

		frametempo = (bigtime_t)(system_time()+mIdealFrameRate);
		while(!mPulsarThreadMustQuit)
		{
			// -- start loop frame
			
			frametempo = (bigtime_t)(system_time()-frametempo);
			if (frametempo < mIdealFrameRate)
				err = acquire_sem_etc(mPulsarFrameSem, 1, B_TIMEOUT,
															mIdealFrameRate-frametempo);
			if (mPulsarThreadMustQuit) break;
			frametempo = (bigtime_t)system_time();

			// -- get sound buffer copy
			lock_stream();
			memcpy(gSrcCpyHigh, gSourceHigh, sizeof(int32)*N);
			memcpy(gSrcCpyBass, gSourceBass, sizeof(int32)*N);
			unlock_stream();

			// -- compute fft (TBD:in separate thread on multiprocessors machines)
			if (1)
			{
				// first copy all int32 into the float bufs for the fft
				float *dh = gFftHigh;
				float *db = gFftBass;
				int32 *sh = gSrcCpyHigh;
				int32 *sb = gSrcCpyBass;

				for(long i=0; i<N; i++) // HOOK disassemble...
				{
					*(dh++) = *(sh++);
					*(db++) = *(sb++);
				}

				applyFftWindow();

				// use ffts or rffts
				rffts(gFftHigh, gRealFftMptr, 1, gRealUtbl);
				rffts(gFftBass, gRealFftMptr, 1, gRealUtbl);

				// suppress im part -- only the N/2 first points are used...
				dh = gFftHigh;
				db = gFftBass;
				float *dh2 = gFftHigh;
				float *db2 = gFftBass;
				for(long i=0; i<N/2; i++) // HOOK disassemble...
				{
					float r,i;
					r = *(dh2++);
					i = *(dh2++);
					*(dh++) = inl_b_sqrt(r*r+i*i);	// sqrt or inl_b_sqrt
					r = *(db2++);
					i = *(db2++);
					*(db++) = inl_b_sqrt(r*r+i*i); // sqrt or inl_b_sqrt
				}
				
				// now compute average for the fft
				dh2 = gFftHigh;
				db2 = gFftBass;
				*dh2 = 0;
				*db2 = 0;
				double sumH = 0.0, sumB = 0.0;
				double *dh3 = &mFrameInfo.meanFftHigh[0];
				double *db3 = &mFrameInfo.meanFftBass[0];
				for(long i=0; i<8; i++)
				{
					double sh=0.0, sb=0.0;
					for(long j=0; j<N/2/8; j++)
					{
						sh += *(dh2++);
						sb += *(db2++);
					}
					sumH += sh;
					sumB += sb;
					*(dh3++) = sh/(N/2/8);
					*(db3++) = sb/(N/2/8);
				}
				mFrameInfo.averageFftHigh = sumH/(N/2);
				mFrameInfo.averageFftBass = sumB/(N/2);
			}

			// -- render part...
			if (mGameKitRunning)
			{
				if (matroxHack)
					mMatroxScreen->lock_draw();
				else
					mFullScreen->lock_draw();
				if (mGameKitDrawingEnabled)
				{
					// -- call filters
					processFrameFilterAddons();
					if (matroxHack)
						mMatroxScreen->swap(mFrameInfo.screen, mFrameInfo.startingLine);
					else
						mFullScreen->swap(mFrameInfo.screen, mFrameInfo.startingLine);
				}
				if (matroxHack)
					mMatroxScreen->unlock_draw();
				else
					mFullScreen->unlock_draw();
			}
			else if (mUpdateFilled)
			{
				// -- call filters
				processFrameFilterAddons();

				// -- update monitor display
				mFilledWindow->forceRedraw();
			}
			
			
			// -- end loop frame
		}
	}
	catch(...)
	{
		stopStream();
	}
} // end of pulsarThread for CPulsarApp


//***********************************
void CPulsarApp::applyFftWindow(void)
//***********************************
{
	float *dh = gFftHigh;
	float *db = gFftBass;
	float *f  = gFftWindow;

	if (!f) return;
	for(long i=0; i<N; i++, f++) // HOOK disassemble...
	{
		*(dh++) *= *f;
		*(db++) *= *f;
	}
}


//****************************************
void CPulsarApp::setFftWindow(int32 index)
//****************************************
{
float *f  = gFftWindow;

	if (!index || index > K_NB_FFT_WINDOW) 
	{
		// no window, just set ptr to NULL so that the apply fft window cancel the operation
		gFftWindow = NULL;
	}
	else
	{
		gFftWindow = gFftPrecomputedWindow+N*2*(index-kFftWindowTriangle);
	}
}


//---------------------------------------------------------------------------
#pragma mark -

//******************************
bool CPulsarApp::initGkSup(void)
//******************************
{
	BPath path1(getAppDir(),K_GK_SUP_CONTROL_SON_IMG);
	BPath path2(getAppDir(),K_GK_SUP_CONTROL_CD_IMG);

	BBitmap *map1 = getBitmap(path1.Path());
	BBitmap *map2 = getBitmap(path2.Path());
	mGkControlSon = 	gApplication->makeBitmap8Bit(map1);
	mGkControlCD = gApplication->makeBitmap8Bit(map2);
	if (!mGkControlSon)
	{
		loadBitmapError(K_GK_SUP_CONTROL_SON_IMG);
		return false;
	}
	if (!mGkControlCD)
	{
		loadBitmapError(K_GK_SUP_CONTROL_CD_IMG);
		return false;
	}

	mGkControlSonBounds = mGkControlSon->Bounds();
	mGkControlCdBounds = mGkControlCD->Bounds();

	if (map1) delete map1;
	if (map2) delete map2;

	return true;

} // end of initGkSup



//******************************
void CPulsarApp::drawGkSup(void)
//******************************
{
	if (mBpp != 8) return;


	bigtime_t t = (bigtime_t)system_time();

	if (mGkSupState & (uint32)kGkSupVolume)
	{
		if (t > mGkSupVolEndTime) mGkSupState &= ~((uint32)kGkSupVolume);
		
		uint32 dbpr=mFilterReference.sPrepare.bpr;
		uint8 *d=mFrameInfo.screen;
		uint32	dsx = mFilterReference.sPrepare.sx;
		uint32	dsy = mFilterReference.sPrepare.sy;

		uint8 *s = (uint8 *)mGkControlSon->Bits();
		int32 sbpr = mGkControlSon->BytesPerRow();
		uint32 ssx = (uint32)(mGkControlSonBounds.Width()+1);
		uint32 ssy = (uint32)(mGkControlSonBounds.Height()+1);
		uint32 ssy1 = ssy-6-6;

		d += dbpr*(dsy-ssy1);
		uint8 chromaKey = (*s);

		int32 x,y;
		uint32 dbpr1 = dbpr-ssx;
		for(y=0; y<8; y++)
		{
			for(x=0; x<ssx; x++, d++, s++)
				if (*s != chromaKey) *d = *s;
			d += dbpr1;
		}

		uint32 *d2 = ((uint32 *)d)-1;
		uint32 *s2 = ((uint32 *)s)-1;
		uint32 dbpr2 = (dbpr-ssx)/4;
		for(; y<ssy1; y++)
		{
			// 24 pixels : 24/4=6
			*(++d2) = *(++s2);
			*(++d2) = *(++s2);
			*(++d2) = *(++s2);
			*(++d2) = *(++s2);
			*(++d2) = *(++s2);
			*(++d2) = *(++s2);
			d2 += dbpr2;
		}

		uint32 vol = mGkSupVolume;
		uint32 n;
		d2 = (uint32 *)(mFrameInfo.screen)+((dsy-4-10)*dbpr+4)/4;
		uint32 *s1 = (uint32 *)mGkControlSon->Bits();
		s2 = s1;
		s1 += (124*sbpr+4)/4;
		s2 += (130*sbpr+4)/4;
		uint32 dbpr3 = (dbpr-16)/4;
		uint32 dbpr4 = (10*dbpr)/4;
		for(n=0; n<10; n++)
		{
			uint32 *s = (n < vol ? s1 : s2);
			uint32 *d3 = d2;
			for(y=0; y<6; y++)
			{
				*(d3++) = *(s++);
				*(d3++) = *(s++);
				*(d3++) = *(s++);
				*(d3++) = *(s++);
				s += 2;
				d3 += dbpr3;
			}
			d2 -= dbpr4;
		}
	}

	if (mGkSupState & (uint32)kGkSupCd)
	{
		if (t > mGkSupCdEndTime) mGkSupState &= ~((uint32)kGkSupCd);
		
		uint32 dbpr=mFilterReference.sPrepare.bpr;
		uint8 *d=mFrameInfo.screen;
		uint32	dsx = mFilterReference.sPrepare.sx;
		uint32	dsy = mFilterReference.sPrepare.sy;

		uint8 *s = (uint8 *)mGkControlCD->Bits();
		int32 sbpr = mGkControlCD->BytesPerRow();

		d += dbpr*(dsy-124)+120;
		uint8 chromaKey = (*s);

		int32 x,y;
		for(y=0; y<46; y++)
		{
			for(x=0; x<128; x++, d++, s++)
				if (*s != chromaKey) *d = *s;
			d += dbpr-128;
		}

		int32 track = mGkSupCdTrack;
		if (track > 39) track = 39;
		uint32 dizaine = track/10;
		track = track%10;

		if (track > 0)
		{
			track--;
			uint32 ty = track/3;
			track = track%3;

			uint8 *s = (uint8 *)mGkControlCD->Bits();
			uint8 *d = (uint8 *)mFrameInfo.screen;
			s += ((ty*10+46)*sbpr+track*16);
			d += ((ty*10+dsy-124+7)*dbpr+track*16+50+120);
			
			for(y=0; y<10; y++)
			{
				memcpy(d,s,16);
				d += dbpr;
				s += sbpr;
			}
		}

		if (dizaine > 0)
		{
			dizaine--;
			
			uint8 *s = (uint8 *)mGkControlCD->Bits();
			uint8 *d = (uint8 *)mFrameInfo.screen;
			s += ((dizaine*10+46)*sbpr+3*16);
			d += ((dizaine*10+dsy-124+7)*dbpr+3*16+50+120);
			
			for(y=0; y<10; y++)
			{
				memcpy(d,s,16);
				d += dbpr;
				s += sbpr;
			}
		}
	}

} // end of drawGkSup


//---------------------------------------------------------------------------
#pragma mark -



//***************************************
void CPulsarApp::loadConfig(uint32 index)
//***************************************
{
	try
	{
		char filename[64];
		sprintf(filename, "config%d.pp", index);
	
		BPath path(getAppDir(), filename);
		BEntry entry(getAppDir(), filename);
		
		if (path.InitCheck() >= B_NO_ERROR && entry.InitCheck() >= B_NO_ERROR && entry.Exists())
		{
			BMessage reply;
			BMessenger(mInterfaceWindow).SendMessage(K_MSG_NEW, &reply);
			terminateFilterAddons();
			mInterfaceWindow->loadFromFile(path);
			if (mIsRunning) prepareFilterAddons();
		}
	}
	catch(...)
	{
		// silently ignore errors during playback...
	}

} // end of loadConfig


//******************************
void CPulsarApp::clearPref(void)
//******************************
{
	mPulsarPref.mMemory				= kScreenMemoryOffscreen;
	mPulsarPref.mOffscreenBuffering	= kScreenBufferingDouble;
	mPulsarPref.mOnBoardBuffering	= kScreenBufferingDouble;
	mPulsarPref.mUmaxHack			= false;
	mPulsarPref.mMatroxHack			= false;
	mPulsarPref.mLoadDefaultConfig	= true;
	mPulsarPref.mDisplayFrameRate	= true;
	mPulsarPref.mDisplayGkSup		= true;
} // end of clearPref


//*****************************
void CPulsarApp::loadPref(void)
//*****************************
{
	status_t err;
	int32 a1,a2,a3;

	BMessage message;
	if (!mPreference.getPref("CPulsarApp", message)) return;

	if (debug) printf("CPulsarApp::loadPref START\n");
	try
	{
		if (debug) printf("CPulsarApp::loadPref TRY\n");

		#define M_GET_PP_PREF_DATA(n,t,v) err = message.Find ## t (n, & v); if (err != B_OK) throw(err);

		M_GET_PP_PREF_DATA("pp_membuf", Int32, a1);
		M_GET_PP_PREF_DATA("pp_offbuf", Int32, a2);
		M_GET_PP_PREF_DATA("pp_obdbuf", Int32, a3);
		M_GET_PP_PREF_DATA("pp_umax"  , Bool,  mPulsarPref.mUmaxHack);
		M_GET_PP_PREF_DATA("pp_matrox", Bool,  mPulsarPref.mMatroxHack);
		M_GET_PP_PREF_DATA("pp_defcfg", Bool,  mPulsarPref.mLoadDefaultConfig);
		M_GET_PP_PREF_DATA("pp_framrt", Bool,  mPulsarPref.mDisplayFrameRate);
		M_GET_PP_PREF_DATA("pp_gksup",  Bool,  mPulsarPref.mDisplayGkSup);

		mPulsarPref.mMemory							= (EScreenMemory)a1;
		mPulsarPref.mOffscreenBuffering	= (EScreenBuffering)a2;
		mPulsarPref.mOnBoardBuffering		= (EScreenBuffering)a3;
	}
	catch(...)
	{
		if (debug) printf("CPulsarApp::loadPref CATCH\n");
		clearPref();
	}

	if (debug) printf("CPulsarApp::loadPref END\n");

	#undef M_GET_PP_PREF_DATA

} // end of loadPref


//*****************************
void CPulsarApp::savePref(void)
//*****************************
{
	int32 a1,a2,a3;
	BMessage message;

	#define M_SET_PP_PREF_DATA(n,t,v)	message.Add ## t(n, v);

	a1 = mPulsarPref.mMemory;
	a2 = mPulsarPref.mOffscreenBuffering;
	a3 = mPulsarPref.mOnBoardBuffering;

	M_SET_PP_PREF_DATA("pp_membuf", Int32, a1);
	M_SET_PP_PREF_DATA("pp_offbuf", Int32, a2);
	M_SET_PP_PREF_DATA("pp_obdbuf", Int32, a3);
	M_SET_PP_PREF_DATA("pp_umax"  , Bool,  mPulsarPref.mUmaxHack);
	M_SET_PP_PREF_DATA("pp_matrox", Bool,  mPulsarPref.mMatroxHack);
	M_SET_PP_PREF_DATA("pp_defcfg", Bool,  mPulsarPref.mLoadDefaultConfig);
	M_SET_PP_PREF_DATA("pp_framrt", Bool,  mPulsarPref.mDisplayFrameRate);
	M_SET_PP_PREF_DATA("pp_gksup",  Bool,  mPulsarPref.mDisplayGkSup);

	mPreference.setPref("CPulsarApp", message);

	#undef M_SET_PP_PREF_DATA

} // end of savePref


//---------------------------------------------------------------------------
#pragma mark -


//***************************************
void CPulsarApp::dynamicAddOnReload(void)
//***************************************
{
	BMessage reply;

	if (isRunning() || mAudio || mFullScreen || mMatroxScreen) stopStream();
	BMessenger(mInterfaceWindow).SendMessage(K_MSG_NEW, &reply);
	mFilterInfoWindow->setFilter(NULL);
	unloadFilterAddons();
	mSplatchWindow->Activate();
	mSplatchWindow->startLoading();
	loadFilterAddons();
	mInterfaceWindow->listFilters(mFilterList);
	mSplatchWindow->endLoading();

} // end of dynamicAddOnReload

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

// eoc
