/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CMoteurPlay.h
	Partie	: Moteur

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_GCMOTEURPLAY_
#define _H_GCMOTEURPLAY_

#include "gMachine.h"
#include "RThread.h"
#include "TRotateBuffer.h"
#include "RBitmap.h"
#include "RBlitWindow.h"
#include "TQList.h"
#include "RPath.h"

#include "CSoundPlay.h"
#include "CAudioCodec.h"

//---------------------------------------------------------------------------
// default definitionss

const fp64 K_FRAME_RATE			= 80000.0; // 40000.0 -- 25 Hz
const ui32 K_SX							= 640;
const ui32 K_SY							= 480;
const ui32 K_BITMAP_BUFFER	= 3;
const ui32 K_NB_SPLASH_IMG	= 5;

//---------------------------------------------------------------------------

#ifndef K_PI
	#define K_PI	3.141592654
#endif
#define K_2PI	(2.0*K_PI)
#define K_PI2	(K_PI/2.0)


//---------------------------------------------------------------------------
// kind of display "window".
// not all definitions make sense on all platforms.

enum EDisplayType
{
	kDisplayWindow = 0,		// default
	kDisplayGameKit,			// fullscreen BeOS
	kDisplayDirectDraw,		// fullscreen Win
	kDisplay3dfx,					// fullscreen Win (BeOS ?)
	kDisplayOpenGL,				// Be+Win, fullscreen ?
};

//---------------------------------------------------------------------------
// Messages (starting with 'M' for Moteur)

const ui32 K_MSG_CHANGE_DISPLAY = 'McDi';
const ui32 K_MSG_END_LOAD_RSRC	= 'MeLR';


//---------------------------------------------------------------------------
// file type

const ui32 K_FILE_HEADER_SIGNATURE = 'ViSh';
const ui32 K_FILE_HEADER_VERSION	 =      1;


//---------------------------------------------------------------------------

//***********************************************
class CMoteurDocument : virtual public RInterface
//***********************************************
{
public:
	CMoteurDocument(RThread *parent) { mParentThread = parent; mDocWindow = NULL; mDocThread = 0; }
	virtual ~CMoteurDocument(void) { mDocWindow = 0; mDocThread = 0; }

	RThread	*getParent(void)	{ return mParentThread; }
	RThread	*getThread(void)	{ return mDocThread; }
	RWindow	*getWindow(void)	{ return mDocWindow; }

protected:
	void setThread(RThread * thr)	{ mDocThread = thr; }
	void setWindow(RWindow * win)	{ mDocWindow = win; }
	RWindow *mDocWindow;
	RThread	*mDocThread;
	RThread *mParentThread;
};



//---------------------------------------------------------------------------



//********************************************************
class CMoteurPlay : public RThread, public CMoteurDocument
//********************************************************
{
public:
	CMoteurPlay(RThread *parent, RPath &path);
	virtual ~CMoteurPlay(void);

	void run(void);

	void setDisplayType(EDisplayType type)	{ mDisplayType = type; }
	EDisplayType displayType(void) 					{ return mDisplayType; }


//----
protected:

	void	initFile(FILE * &f);
	void	initSound(void);
	void	closeSound(void);

	void	createDisplay(void);
	void	closeDisplay(void);
	void	unpackFrame(void);
	void	displayFrame(void);

	ui32	playFrame(FILE *f);
	void	readSoundFrame(FILE *f);

	rbool f_bg_read4(ui32 &s, FILE *f);
	
	// generic
	EDisplayType	mDisplayType;
	ui32					mDisplaySx, mDisplaySy;
	RBlitWindow		*mDisplay;

	RBitmapBuffer	mOffscreen;
	RBitmap	*mCurrentImg;
	ui8			*mDiffBuffer;
	ui32		mDiffLen;

	// document
	RPath	mDocumentPath;

	// stats
	ui32 mFrameDiffTotal;
	ui32 mFrameDiffService;
	ui32 mFrameCount;

	// sound
	rbool	mUseSound;
	ui8	 *mMemoryPool;			// a large-enough buffer that should cover most situations
	ui32	mMemoryPoolSize;	// size in bytes

	SSharedSoundBuffer	mSharedSoundBuffer;
	CSoundPlay					mSoundPlay;
	CAdpcmCodec					mAdpcmCodec;

}; // end of class defs for CMoteurPlay


//---------------------------------------------------------------------------

#endif // of _H_GCMOTEURPLAY_

// eoh
