/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CPulsarApp.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CPULSARAPP_
#define _H_CPULSARAPP_

#include "PP_Be.h"		// for R4b4 x86

#include "CAudioCD.h"
#include "globals.h"
#include "CFilter.h"
#include "CSimpleTrack.h"
#include "CPulsarMsg.h"
#include "CScreen.h"
#include "CPreference.h"

#include <MediaDefs.h>

#define K_EXT ".tif"	// HOOK RM 161198 -- ".png"

#define K_FOND_FFT_FILENAME "img_pulsar/fondfft" K_EXT
#define K_ADDON_DIR_NAME    "add-ons"
#define K_IMG_DIR_NAME			"img"


#define K_APP_SUB_MIME	"x-vnd.ralf.powerpulsar"
#define K_APP_MIME 			"application/" ## K_APP_SUB_MIME
#define K_APP_DOC_MIME	"application/x-vnd.ralf.powerpulsar-octet"
#define K_APP_NAME			"PowerPulsar"
#define K_MAJOR_PULSAR_VERSION		0
#define K_MINOR_PULSAR_VERSION		8
#define K_QUANTIC_PULSAR_VERSION	1

//---------------------------------------------------------------------------

extern CSimpleTrack				*gSimpleTrack;				// defined in CPulsarApp.cpp
extern BTranslatorRoster	*gTranslationRoster;	// defined in CPulsarApp.cpp

//---------------------------------------------------------------------------
class CFilledWin;
class CInterfaceWin;
class CFilterInfoWin;
class CSplatchWin;
class CPrefWin;
class CScreen;
class CMatroxScreen;
class SoundConsumer;


//*********************************
class CBlackWindow : public BWindow
//*********************************
{
public:
				CBlackWindow(BRect frame,
										 const char *title, 
										 window_type type,
										 ulong flags,
										 ulong workspace = B_CURRENT_WORKSPACE);
virtual	~CBlackWindow(void);

				bool	init(void);

virtual	bool	QuitRequested(void);
virtual void	DispatchMessage(BMessage *message, BHandler *handler);
virtual	void	WindowActivated(bool state);

private:
	BView *mBlackView;
};


//---------------------------------------------------------------------------

//**************
enum EGkSupState
//**************
{
	kGkSupVolume	= 1,		// must be a bit field
	kGkSupCd			= 2
};


//---------------------------------------------------------------------------


//****************
struct SPulsarPref
//****************
{
	EScreenMemory			mMemory;
	EScreenBuffering	mOffscreenBuffering;
	EScreenBuffering	mOnBoardBuffering;
	bool							mUmaxHack;
	bool							mMatroxHack;
	bool							mLoadDefaultConfig;
	bool							mDisplayFrameRate;
	bool							mDisplayGkSup;
};



//---------------------------------------------------------------------------


//************************************
class CPulsarApp : public BApplication
//************************************
{
public:
							CPulsarApp(void);
virtual				~CPulsarApp(void);

virtual	void	AboutRequested();
virtual void	ReadyToRun(void);
virtual bool	QuitRequested(void);
virtual void	MessageReceived(BMessage *message);
virtual void	RefsReceived(BMessage *a_message);

				bool	init(void);

				// --- methodes interface
	
				// comme BView::KeyDown
				bool	touches(const char *bytes, int32 numBytes, BMessage *msg=NULL);
				bool	handleMessageAddons(BMessage *msg);

				void	buttonADC(void)					{ mUseDAC = false;											}
				void	buttonDAC(void)					{ mUseDAC = true;												}
				void	buttonStreamStart(void)	{ mUseStreamEnd = false;								}
				void	buttonStreamEnd(void)		{ mUseStreamEnd = true;									}
				void	buttonStartStream(void)	{ mButtonAudio = true;  stateStream();	}
				void	buttonStopStream(void)	{ mButtonAudio = false; stateStream();	}
				void	buttonFullScreen(uint32 bpp);
				void	buttonPreview(uint32 bpp);
				void	buttonSwitchInterface(void);
				void	buttonUpdatePulsar(BCheckBox *check);
				void	buttonUpdateFilled(BCheckBox *check);

				BSubscriber * audio(void)			{return mAudio;}
				BBitmap			* fondFft(void) 	{return mFondFft;}
				CFilter			* filterRef(void)	{return &mFilterReference;}
				void		  		setFilterRef(int32 sx, int32 sy,
																	 uint8 *screen,
																	 int32 bpr, EColorSpace mode,
																	 BWindowScreen *game_kit);
				void					setFullScreenEnable(bool state)
																											{ mGameKitDrawingEnabled=state; }
				double				idealFrameRate		(void				)	{return mIdealFrameRate;				}
				void					setIdealFrameRate	(bigtime_t t)	{mIdealFrameRate = t;						}
				double				currentFps				(void				)	{return mFrameInfo.lastFps;			}
				void					setCurrentFps			(double t		)	{mFrameInfo.lastFps = t;				}
const		char 				* getAppDir					(void				)	{return mAppDir.Path();					}
				BBitmap			* getAppIcon				(void				)	{return &mLargeAppIcon;		}
				BBitmap			* makeBitmap8Bit		(BBitmap *b	)	{return mFilterReference.makeBitmap8Bit(b);	  }
				CFilter			* getNthFilter			(long index)	{return (CFilter *)mFilterList.ItemAt(index); }
				void		  		setInputMethod		(int32 input);
				BBitmap			* getBitmap					(const char *path);
				void 		  		loadBitmapError		(const char *file);

				// --- membres publiques
			
				SPulsarPref		mPulsarPref;
				CAudioCD			mAudioCD;					// access to CD
				CPreference		mPreference;
		
				//CSimpleTrack	mSimpleTrack;	// acces to tracks

private:
				CInit mInit;
		
				void	dynamicAddOnReload(void);
		
				void	clearPref(void);
				void	loadPref(void);
				void	savePref(void);
		
				void	stateStream(void);
				void	startStream(void);
				void	stopStream(void);
			
				void	getFirstPhysicalInput(media_node &input);
				void	makeRecordConnection(const media_node &input);
static	void	recordProcessFunc(void * cookie, bigtime_t timestamp,
														    void * data, size_t size,
														    const media_raw_audio_format & format);
static	void	recordNotifyFunc(void * cookie, int32 code, ...);
	
				bool	initDatatypeLib(void);
				bool	initTranslationKit(void);
				bool	loadBitmap(void);
				void	loadFilterAddons(void);
				void	unloadFilterAddons(void);
				void	prepareFilterAddons(void);
				void	terminateFilterAddons(void);
				void	processFrameFilterAddons(void);
	
				void	prepareOneFilterAddon(int32 index, bool *first=NULL);
				void	terminateOneFilterAddon(int32 index, bool continueRunning=false);
	
static	int32	pulsarThreadEntry(void *data);
				void	pulsarThread(void);
	
				void	applyFftWindow(void);
				void	setFftWindow(int32 index);
	
				bool	initGkSup(void);
				void	drawGkSup(void);
	
				void	loadConfig(uint32 index);

				// ---
			
				bool	mMatroxHack;
				bool	mIsLoading;
			
				BADCStream 		*mAdcStream;
				BDACStream		*mDacStream;
				BSubscriber 	*mAudio;
				bool					mUseDAC;
				bool					mUseStreamEnd;
			
				SoundConsumer	*mSoundRecorder;
				BMediaRoster 	*mMediaRoster;
				media_node		mAudioInputNode;
				media_output	mAudioOutput;
				media_input		mRecInput;


				// state of play
				bool			mIsRunning;
				bool			mButtonAudio;
				bool			mUpdateFft;
				bool			mUpdateFilled;
				bool			mGameKitRunning;
				bool			mGameKitDrawingEnabled;
				bool			mInvertGameKitScreen;		// only active in game kit mode -- see touches(), 'I' key
				uint32		mBpp;
				bool			isRunning(void) {return mIsRunning; }
	
				EColorSpace getColorSpace(void) { return (mBpp != 8 ? kRgb32Bit : kColor8Bit); }

				CFilledWin			*mFilledWindow;
				CInterfaceWin		*mInterfaceWindow;
				CFilterInfoWin 	*mFilterInfoWindow;
				CSplatchWin			*mSplatchWindow, *mInfoWindow;
				CPrefWin				*mPrefWindow;
				CScreen					*mFullScreen;
				CMatroxScreen		*mMatroxScreen;
				CBlackWindow		*mBlackWindow;

				BPath						mAppDir;
				BPath						mAddonDir;
				BPath						mImgDir;
				BBitmap 				*mFondFft;
				BBitmap					mLargeAppIcon;
		
				BList 					mFilterList;	// a list of ptr on new SFilterInfo
				CFilter 				mFilterReference;	// reference info for filters
				SFrameInfo			mFrameInfo;

				// -- infos for Pulsar Thread
				bool						mPulsarThreadMustQuit;
				thread_id				mPulsarThread;
				sem_id					mPulsarFrameSem;
				bigtime_t				mIdealFrameRate;
		
				// -- stuff for the GK-sup interface
				BBitmap *				mGkControlSon;
				BBitmap *				mGkControlCD;
				BRect						mGkControlSonBounds;
				BRect						mGkControlCdBounds;
			
				uint32					mGkSupState;		// ORed EGkSupState constants
				uint32					mGkSupVolume;
				uint32					mGkSupCdTrack;
				bigtime_t				mGkSupVolEndTime;
				bigtime_t				mGkSupCdEndTime;
		
				// big hack : list of SLineParam
				//BList	*mLine;
				//BList mLineCopy;
				CSimpleTrack	*	mSimpleTrack;
		
				int32					 mInputMethod; // K_MSG_xxINPUT from CPulsarWin.h
		
				// Translation Kit
				BTranslatorRoster *mTranslationKit;

}; // end of class defs for CPulsarApp


//---------------------------------------------------------------------------

#endif // of _H_CPULSARAPP_

// eoh
