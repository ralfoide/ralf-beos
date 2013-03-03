#if 0
/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	Moteur Display/Record.h
	Partie	: Moteur

	Auteur	: RM
	Date		: 170798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_GCMOTEUR_
#define _H_GCMOTEUR_

#include "gMachine.h"
#include "RThread.h"
#include "TRotateBuffer.h"
#include "RBitmap.h"
#include "RBlitWindow.h"
#include "TQList.h"

#include "adpcm.h"

//---------------------------------------------------------------------------
// default definitionss

#define K_FRAME_RATE		80000.0 // 40000.0 -- 25 Hz
#define K_SX						640
#define K_SY						480
#define K_BITMAP_BUFFER	3
#define K_NB_SPLASH_IMG	5

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

#define K_MSG_CHANGE_DISPLAY	'McDi'
#define K_MSG_END_LOAD_RSRC		'MeLR'

//---------------------------------------------------------------------------
// hack sound

//*************************************
struct SSoundBuffer : public SQListItem
//*************************************
{
	SSoundBuffer(void) : SQListItem() { mBuffer=NULL; mNbSample=0; mNbCurrent=0; }
	SSoundBuffer(ui32 samples) : SQListItem() { mBuffer=new si16[samples]; mNbSample=samples; mNbCurrent=0; }
	~SSoundBuffer(void)	{ if (mBuffer) delete mBuffer; }
	si16	*mBuffer;
	ui32	mNbSample;
	ui32	mNbCurrent;
};

//---------------------------------------------------------------------------

//*****************************
class CMoteur : public RThread
//*****************************
{
public:
	CMoteur(void);
	virtual ~CMoteur(void);

	void init(void);
	void run(void);

	void setDisplayType(EDisplayType type)	{ mDisplayType = type; }
	EDisplayType displayType(void) 					{ return mDisplayType; }


//----
protected:

	void	initFile(FILE * &f, rbool play, char *name);
	void	createDisplay(void);
	void	closeDisplay(void);
	void	displayFrame(void);
	rbool	playFrame(FILE *f);
	void	recordFrame(FILE *f);
	void	grabFrame(void);
	void	drawMouse(void);
	ui32	diffFrame(void);
	void	unpackFrame(void);

	void	initSound(rbool play);
	void	closeSound(void);
	void	recordSoundFrame(FILE *f);
	void	readSoundFrame(FILE *f);

	rbool f_bg_write2(ui32  s, FILE *f);
	rbool f_bg_write4(ui32  s, FILE *f);
	rbool f_bg_read4 (ui32 &s, FILE *f);
	
	// generic
	EDisplayType	mDisplayType;
	ui32					mDisplaySx, mDisplaySy;
	RBlitWindow		*mDisplay;

	RBitmapBuffer	mOffscreen;
	RBitmap	*mPreviousImg;
	RBitmap	*mCurrentImg;
	ui8			*mDiffBuffer;
	ui32		mDiffLen;
	ui32		mMouseX, mMouseY;
	rbool		mMouseVisible;

	// stats
	ui32 mFrameDiffTotal;
	ui32 mFrameDiffService;
	ui32 mFrameCount;

	// sound
	rbool	mUseSound;
	TQList<SSoundBuffer *> mSoundBufList;
	RSem	mSoundBufSem;
	ui8	 *mMemoryPool;			// a large-enough buffer that should cover most situations
	ui32	mMemoryPoolSize;	// size in bytes

	struct adpcm_state	mAdpcmState;
 
private:
	// be-specific

	color_space	checkColorSpace();
	void	restoreColorSpace(color_space cs);

	screen_id		mScreenId;
	color_space	mOldColorSpace;

	BSubscriber	*mSubscriber;
	BADCStream	*mAdcStream;
	BDACStream	*mDacStream;

	static bool beRecordStream(void *arg, char *buf, size_t cnt, void *header);
	static bool bePlayStream(void *arg, char *buf, size_t cnt, void *header);

	SSoundBuffer *mCurrentPlayBack;

}; // end of class defs for CMoteur


//---------------------------------------------------------------------------

#endif // of _H_GCMOTEUR_

// eoh
#endif
