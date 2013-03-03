/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CMoteurRecord.h
	Partie	: Moteur

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_GCMOTEURRECORD_
#define _H_GCMOTEURRECORD_

#include "gMachine.h"

#include "CMoteurPlay.h"
#include "CSoundRecord.h"

#include <Screen.h>
#include <Bitmap.h>

//---------------------------------------------------------------------------


//**********************************************************
class CMoteurRecord : public RThread, public CMoteurDocument
//**********************************************************
{
public:
	CMoteurRecord(RThread *parent, RPath &path);
	virtual ~CMoteurRecord(void);

	void run(void);

	void setDisplayType(EDisplayType type)	{ mDisplayType = type; }
	EDisplayType displayType(void) 					{ return mDisplayType; }


//----
protected:

	void	initFile(FILE * &f);

	void	createDisplay(void);
	void	closeDisplay(void);
	void	displayFrame(void);
	void	recordFrame(FILE *f, fp64 temps);
	void	grabFrame(void);
	void	drawMouse(void);
	ui32	diffFrame(void);

	void	initSound(void);
	void	closeSound(void);
	void	recordSoundFrame(FILE *f);

	rbool f_bg_write2(ui32  s, FILE *f);
	rbool f_bg_write4(ui32  s, FILE *f);
	
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
	CSoundRecord				mSoundRecord;
	CAdpcmCodec					mAdpcmCodec;

private:
	// be-specific

	color_space	checkColorSpace(); // returns current color space if supported

	screen_id		mScreenId;
	BBitmap		 *mScreenCapture;

}; // end of class defs for CMoteurRecord


//---------------------------------------------------------------------------

#endif // of _H_GCMOTEURRECORD_

// eoh
