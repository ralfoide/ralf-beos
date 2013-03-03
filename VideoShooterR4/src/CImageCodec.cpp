#if 0
/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CMoteur.cpp
	Partie	: Moteur Display/Record

	Auteur	: RM
	Date		: 170798
	Format	: tabs==2

*****************************************************************************/

#include "CMoteur.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"
#include "RResource.h"

#include <math.h>

//---------------------------------------------------------------------------

#define K_IMG_SPLASH_BASE	"nerdtitle"
#define K_IMG_EXT	".tga"

// hack
extern rbool gPlaybackMode;
extern char *gFilename;

//---------------------------------------------------------------------------


//***************************************************************************
CMoteur::CMoteur(void)
        :RThread()
//***************************************************************************
{
	mDiffBuffer = NULL;
	mDiffLen = 0;
	mDisplay = NULL;
	mDisplayType = kDisplayWindow;
	mDisplaySx = K_SX;
	mDisplaySy = K_SY;
	mPreviousImg = NULL;
	mCurrentImg  = NULL;
	mMouseX = 0;
	mMouseY = 0;
	mMouseVisible = false;
	mFrameDiffTotal = 0;
	mFrameDiffService = 0;
	mFrameCount = 0;
	mUseSound = true;
	mMemoryPool = NULL;
	mMemoryPoolSize = 0;
	mCurrentPlayBack = NULL;

	// be specific
	mOldColorSpace = B_NO_COLOR_SPACE;
	mScreenId = B_MAIN_SCREEN_ID;
	mSubscriber = NULL;
	mAdcStream = NULL;
	mDacStream = NULL;

	// init

	si32 n = K_BITMAP_BUFFER;

	printf("CMoteur::init -- moteur %p, name %s\n", this, class_name(this));

	mDiffBuffer = new ui8[K_SX*K_SY*2];
	M_ASSERT_PTR(mDiffBuffer);

	mPreviousImg = new RBitmap(K_SX, K_SY, kPixel256);
	mCurrentImg  = new RBitmap(K_SX, K_SY, kPixel256);
	M_ASSERT_PTR(mPreviousImg);
	M_ASSERT_PTR(mCurrentImg );

	mOffscreen.init(K_BITMAP_BUFFER);
  for(n--; n>=0; n--)
	{
		RBitmap *map = new RBitmap(K_SX, K_SY, kPixel256);
		M_ASSERT_PTR(map);
		map->erase();
		mOffscreen.initItemAt(n, map);
	}


} // end of constructor for CMoteur


//***************************************************************************
CMoteur::~CMoteur(void)
//***************************************************************************
{
	if (mCurrentImg ) delete mCurrentImg ; mCurrentImg  = NULL;
	if (mPreviousImg) delete mPreviousImg; mPreviousImg = NULL;
	if (mDiffBuffer ) delete mDiffBuffer ; mDiffBuffer  = NULL;

} // end of destructor for CMoteur


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteur::initFile(FILE * &f, rbool play, char *name)
//***************************************************************************
{
	if (play)
	{
		f = fopen(name, "rb");

		if (!f)
		{
			RPath path;
			gApplication->getAppPath(path);
			char s[1024];
			sprintf(s, "%s/%s", path.path(), name);
			if (debug) printf("Failed to open './%s', trying '%s'\n", name, s);
			f = fopen(s, "rb");
		}
	}
	else
		f = fopen(name, "wb");

	printf("File %s open in %s mode\n", name, (play ? "read" : "write"));
	M_ASSERT_PTR(f);

} // end of initFile for CMoteur


//---------------------------------------------------------------------------



//***************************************************************************
void CMoteur::run(void)
//***************************************************************************
{
ui32 msg;
ui32 msgParam;
vptr msgData;
fp64 temps, deltatemps, tempsInitial;
RSem frameRateSem;
rbool loop = true;
rbool playback = gPlaybackMode;
FILE *file = NULL;

	frameRateSem.acquire();	// rend le semaphore bloquant

	// the first time, the moteur must create a display and use it
	createDisplay();

	// check bitdepth
	if (!playback) mOldColorSpace = checkColorSpace();

	initFile(file, playback, gFilename);
	initSound(playback);
	tempsInitial = systemTime();

	try
	{
		while(loop)
		{
			temps = systemTime();
	
			//lecture message
			if (receiveMessage(msg, msgParam, msgData))
			{
				// process message
				switch(msg)
				{
					case K_MSG_RTHREAD_EXIT:
						loop = false;
						break;
					case K_MSG_CHANGE_DISPLAY:
						createDisplay();
						break;
				}
			}
	
			// process frame
			if (playback)
			{
				if (!playFrame(file)) selfPostMessage(K_MSG_RTHREAD_EXIT);
			}
			else
				recordFrame(file);

			// synchronize to frame rate
			deltatemps = temps+K_FRAME_RATE-systemTime();
			if (deltatemps > 0) frameRateSem.acquireTimeout(deltatemps);
		}
	}
	M_CATCH("Moteur Loop");

	tempsInitial = (systemTime() - tempsInitial)/1e6;
	if (debug) printf("\n\nStats : %d frames, %.2f seconds, fps = %.2f\n",
		mFrameCount, tempsInitial, (tempsInitial > K_EPSILON ? mFrameCount/tempsInitial : 0.0));

	closeSound();
	if (mOldColorSpace != B_NO_COLOR_SPACE)
	{
		// restore previous bit depth
		restoreColorSpace(mOldColorSpace);
	}
	closeDisplay();

	if (file) fclose(file);

	// ask application to quit and kill this thread
	gApplication->processMessage(K_MSG_RTHREAD_EXIT);

} // end of run for CMoteur


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteur::createDisplay(void)
//***************************************************************************
{
	if (debug) printf("CMoteur::createDisplay (type %d)\n", displayType());
	if (mDisplay) closeDisplay();

	// reject unsupported display option
	if (displayType() != kDisplayWindow) M_THROW(kErrUnsupported);

	RRect rect = RWindow::centerWindow(mDisplaySx, mDisplaySy);
	mDisplay = new RBlitWindow(rect, K_APP_NAME " " K_APP_VERS);
	M_ASSERT_PTR(mDisplay);
	// init and show window
	mDisplay->setQuitAppWhenWindowClosed(true);
	mDisplay->setBitmapBuffer(&mOffscreen);

	// HOOK TBLD : HACK HACK !!
	if (!gPlaybackMode) mDisplay->moveTo(630, 20);

} // end of createDisplay for CMoteur


//***************************************************************************
void CMoteur::closeDisplay(void)
//***************************************************************************
{
	if (debug) printf("CMoteur::closeDisplay\n");
	if (!mDisplay) return;
	mDisplay->deleteWindow();
	mDisplay = NULL;
} // end of closeDisplay for CMoteur


//---------------------------------------------------------------------------


//***************************************************************************
color_space CMoteur::checkColorSpace(void)
//***************************************************************************
{
BRect frame;
color_space cs;

	if(1)
	{
		BScreen screen(mScreenId);
		cs    = screen.ColorSpace();
		frame = screen.Frame();
	}

	if (debug) { printf("Screen : color space %04x, Frame ", (ui32)cs); frame.PrintToStream(); }

	if (cs != B_CMAP8)
	{
		BAlert *box;
	
		box = new BAlert(	K_APP_NAME " - About box",
											K_APP_NAME " " K_APP_VERS " for BeOS R3"
											"\nYour screen is not configured in 8-Bits/pixel "
											"color mode.\nThis parameter is set via the Screen "
											"Preference Panel.\n"
											"ScreenShooter largely better works in 8 bpp. The mode can be "
											"changed temporarily for you and set back when you'll exit the app.",
											"Change Mode", "Quit", NULL,
											B_WIDTH_FROM_WIDEST, B_INFO_ALERT);
	
		if (box && box->Go() == 0)
		{
			// change mode requested
			// select the appropriate constant depending on rez

			uint32 space;
			ui32 w = frame.IntegerWidth();
			if      (w >= 1600-1)	space = B_8_BIT_1600x1200;
			else if (w >= 1280-1) space = B_8_BIT_1280x1024;
			else if (w >= 1152-1) space = B_8_BIT_1152x900;
			else if (w >= 1024-1) space = B_8_BIT_1024x768;
			else if (w >= 800-1)  space = B_8_BIT_800x600;
			else 									space = B_8_BIT_640x480;

			if (debug) printf("Screen changed to index space %08x\n", space);

			if (set_screen_space(0 /*mScreenId*/, space, false) == B_OK)
			{
				// return and remember old color space
				return cs;
			}

			// failure !!
			(new BAlert("", "BeOS R3 doesn't allow programmatical change of this color space yet.",
									"Quit !", NULL, NULL, B_WIDTH_FROM_WIDEST, B_INFO_ALERT))->Go();
			
		}

		// quit requested
		selfPostMessage(K_MSG_RTHREAD_EXIT);
	}

	return B_NO_COLOR_SPACE;	// notify no change was made

} // end of checkColorSpace for CMoteur


//***************************************************************************
void CMoteur::restoreColorSpace(color_space cs)
//***************************************************************************
{
BRect frame;

	if(1)
	{
		frame = BScreen(mScreenId).Frame();
	}

	uint32 space;
	ui32 w = frame.IntegerWidth();
	if      (w >= 1600-1)	space = B_8_BIT_1600x1200;
	else if (w >= 1280-1) space = B_8_BIT_1280x1024;
	else if (w >= 1152-1) space = B_8_BIT_1152x900;
	else if (w >= 1024-1) space = B_8_BIT_1024x768;
	else if (w >= 800-1)  space = B_8_BIT_800x600;
	else 									space = B_8_BIT_640x480;

	if (space == B_8_BIT_1152x900)
		switch(cs)
		{
			case B_RGB16: space = B_16_BIT_1152x900;
			case B_RGB32: space = B_32_BIT_1152x900;
			default : return;
		}
	else switch(cs)
	{
		case B_RGB15: space *= B_15_BIT_640x480; break;
		case B_RGB16: space *= B_16_BIT_640x480; break;
		case B_RGB32: space *= B_32_BIT_640x480; break;
		default: return;
	}

	if (debug) printf("Screen restored to index space %08x\n", space);

	set_screen_space(0 /*mScreenId*/, space, false);

} // end of restoreColorSpace for CMoteur


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteur::displayFrame(void)
//***************************************************************************
{
RBitmap *offscreen;

	offscreen = mOffscreen.acquireWrite();
	M_ASSERT_PTR(offscreen);

	RBitmap *src = mCurrentImg;
	RRect srcRect = src->bounds();
	RRect destRect(srcRect);
	offscreen->add(*src, srcRect, destRect);

	mOffscreen.releaseWrite();

} // end of displayFrame for CMoteur


//---------------------------------------------------------------------------


//***************************************************************************
rbool CMoteur::f_bg_write2(ui32 s, FILE *f)
//***************************************************************************
{
ui8 a,b;
	a=s;
	b=s>>8;
	return fwrite(&b, 1, 1, f) && fwrite(&a, 1, 1, f);
}

//***************************************************************************
rbool CMoteur::f_bg_write4(ui32 s, FILE *f)
//***************************************************************************
{
ui8 a,b,c,d;
	a=s;
	b=s>>8;
	c=s>>16;
	d=s>>24;
	return fwrite(&d, 1, 1, f) && fwrite(&c, 1, 1, f) && fwrite(&b, 1, 1, f) && fwrite(&a, 1, 1, f);
}

//***************************************************************************
rbool CMoteur::f_bg_read4(ui32 &s, FILE *f)
//***************************************************************************
{
ui8 a,b,c,d;
	if (fread(&d, 1, 1, f) && fread(&c, 1, 1, f) && fread(&b, 1, 1, f) && fread(&a, 1, 1, f))
	{
		s = a | (b << 8) | (c << 16) | (d << 24);
		return true;
	}
	else return false;
}


//---------------------------------------------------------------------------


//***************************************************************************
rbool CMoteur::playFrame(FILE *f)
//***************************************************************************
{
ui32 type;
ui32 frameCount;
ui32 len;
rbool process=true;

	while(process && f_bg_read4(type, f))
	{
		switch(type)
		{
			// video delta frame
			case 'DIFF':
				f_bg_read4(frameCount, f);	// read & ignore
				f_bg_read4(len, f);

				if(len)
				{
					if (debug) printf("Play Frame <%5d> -- len %8d\n", frameCount, len);
					// decode delta pack into current frame
					fread(mDiffBuffer, 1, len, f);
					mDiffLen = len;
					unpackFrame();
					displayFrame();
				}
			
				process = false;
				break;

			// video null frame
			case 'RALF':
				f_bg_read4(frameCount, f);	// read & ignore
				process = false;
				break;

			// sound buffer in ADPCM format
			case 'APCM':
				f_bg_read4(frameCount, f);	// read & ignore
				readSoundFrame(f);
				break;

			default:
				if (debug) printf("PLAY FRAME unknow type code %.4s\n", (char *)&type);
				return false;
		}
	}

	mFrameCount++;
	return true;

} // end of playFrame for CMoteur


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteur::recordFrame(FILE *f)
//***************************************************************************
{
ui32 diff;
ui32 frameCount;

	
	frameCount = mFrameCount;

	if (mUseSound) recordSoundFrame(f);

	grabFrame();
	drawMouse();
	diff = diffFrame();
	
	if (diff)
	{
		// save diff frame -- move to function
		if(1)
		{
			f_bg_write4('DIFF', f);
			f_bg_write4(frameCount, f);
			f_bg_write4(mDiffLen, f);
			fwrite(mDiffBuffer, 1, mDiffLen, f);
			if (debug) fflush(f);
		}

		// display if frame is different from previous one
		displayFrame();

		mFrameDiffTotal += mDiffLen;					// saved data
		mFrameDiffService += (mDiffLen-diff);	// services codes (rle extra)
	}
	else
	{
		// save empty frame (synchro) -- move to function
		if(1)
		{
			f_bg_write4('RALF', f);
			f_bg_write4(frameCount, f);
		}

		mFrameDiffTotal += 8;					// saved data
		mFrameDiffService += 8;				// services codes (frame extra)
	}

	if (debug && frameCount%50==0)
		printf("Frame : total %8d <+%7d> -- RLE %8d <+%7d> -- ratio %7.2lf\n",
			mFrameDiffTotal, mDiffLen, mFrameDiffService, diff, (mFrameDiffTotal ? 100.0*mFrameDiffService/mFrameDiffTotal : 0.0));

	// swap frames
	RBitmap *p = mPreviousImg;
	mPreviousImg = mCurrentImg;
	mCurrentImg = p;

	mFrameCount++;
	
} // end of recordFrame for CMoteur


//***************************************************************************
void CMoteur::grabFrame(void)
//***************************************************************************
{
	BScreen screen(mScreenId);
	uint32 sbpr = screen.BytesPerRow();
	uint32 ssy  = screen.Frame().Height();
	ui8 *source = (ui8 *)screen.BaseAddress();
	ui8 *dest   = mCurrentImg->data();
	ui32 dbpr   = mCurrentImg->dataBpr();
	ui32 dsy = min(K_SY, ssy);
	ui32 bpr = min(sbpr, dbpr);

	for(; dsy>0; dsy--)
	{
		memcpy(dest, source, bpr);
		source += sbpr;
		dest += dbpr;
	}

	mDisplay->getScreenMouseLoc(mMouseX, mMouseY);
	mMouseVisible = true;

} // end of grabFrame for CMoteur


//***************************************************************************
void CMoteur::drawMouse(void)
//***************************************************************************
{
const ui8 red = 42;
ui8 *dest = mCurrentImg->data();
ui32 bpr  = mCurrentImg->dataBpr();
si32 x,y;

	x = mMouseX;
	y = mMouseY;
	if (mMouseVisible && x < K_SX && y < K_SY)
	{
		ui8* p = dest+bpr*y+x;
		ui8* v = p;
		ui8* d = p;
		p -= 1;
		v -= bpr;
		
		si32 c;
		rbool p2, v2;
		p2 = (y+1<K_SY);
		v2 = (x+1<K_SX);
		for(c=0; c<10; c++, x++, y++)
		{
			if (x>=0 && x<K_SX) { if (p2) p[bpr] = red; *p = red; }		p++;
			if (y>=0 && y<K_SX) { if (v2) v[1] = red; *v = red; }			v+=bpr; 
			if (x>=0 && x<K_SX && y>=0 && y<K_SX) *d = red;						d+=bpr+1;
		}
	}


} // end of drawMouse for CMoteur


//***************************************************************************
ui32 CMoteur::diffFrame(void)
//***************************************************************************
// simple RLE encoding
// return true if images are different
{
ui8 *pold = mPreviousImg->data();
ui8 *pnew = mCurrentImg->data();
ui8 *diff = mDiffBuffer;
ui8 *diffcount;
ui32 diffBytes = 0;
ui32 sxy = mCurrentImg->dataSxy();
ui32 count;
rbool forcediff;

	//if (debug) printf("CMoteur::diffFrame -- pold %p pnew %p diff %p\n", pold, pnew, diff);
	while(sxy)
	{
		//if (debug) printf("%6d", sxy);
		// find identical-rung
		forcediff = false;
		for(count=0; count<16384-2+3 && sxy && *pold == *pnew; count++, pold++, pnew++, sxy--) /* nop */;
		if (count>64-1+3)
		{
			// write identical count if at least 3 and max 16384-1+3
			ui8 a,b;
			count -= 3;
			a = count & 0x3F;
			b = count>>6;
			*(diff++) = 128+64+a;
			*(diff++) = b;
		}
		else if (count>2)
		{
			// write identical count if at least 3 and max 64-1+3
			*(diff++) = 128+count-3;
		}
		else if (count)
		{
			// force a diff-rung
			forcediff = true;
			pold -= count;
			pnew -= count;
			sxy += count;
		}
		
		// find difference-rung
		diffcount = diff++;
		for(count=0; count<127 && sxy && ((forcediff && count<3) || *pold != *pnew); count++, pold++, sxy--) *(diff++)=*(pnew++);
		if (count > 0)
		{
			// write a diff rung (count is from 1->128, saved 0->127)
			diffBytes += count;
			*diffcount = count-1;
		}
		else diff = diffcount;
	}

	mDiffLen = diff-mDiffBuffer;
	//if (debug) printf("\ndiff len %d\n", mDiffLen);
	return diffBytes;

} // end of diffFrame for CMoteur


//***************************************************************************
void CMoteur::unpackFrame(void)
//***************************************************************************
// simple RLE encoding
// return true if images are different
{
ui8 *pnew = mCurrentImg->data();
ui8 *diff = mDiffBuffer;
ui32 diffBytes = 0;
si32 sxy = mCurrentImg->dataSxy();
ui32 count;

	while(sxy>0)
	{
		// get count
		count = *(diff++);

		if (count >= 192)
		{
			// unpack identical-rung with 14-bit counter
			count += 3-128-64+( *(diff++) << 6 );
			pnew += count;
			sxy -= count;
		}
		else if (count >= 128)
		{
			// unpack identical-rung with 6-bit counter
			count += 3-128;
			pnew += count;
			sxy -= count;
		}
		else
		{
			// unpack difference-rung
			for(count++; count && sxy; count--, sxy--) *(pnew++) = *(diff++);
		}
	}

} // end of unpackFrame for CMoteur


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteur::initSound(rbool play)
//***************************************************************************
{
	// HACK HACK !

	mAdpcmState.valprev = 0;
	mAdpcmState.index = 0;

	mCurrentPlayBack = NULL;

	mAdcStream = new BADCStream();
	mDacStream = new BDACStream();
	if (!mAdcStream || !mDacStream) mUseSound = false;
	if (debug) printf("mAdcStream %p -- mDacStream %p -- use sound %d\n", mAdcStream, mDacStream, mUseSound);
	if (!mUseSound) return;

	mSubscriber = new BSubscriber("VideoShooterSound");

	// adjust the memory pool used for compression/decompression
	// the memory pool contains enough space for one buffer of the Media Kit
	size_t bufferSize;
	int32 bufferCount;
	bool isRunning;
	int32 subscriberCount;
	if (mDacStream->GetStreamParameters(&bufferSize, &bufferCount, &isRunning, &subscriberCount) >= B_NO_ERROR)
	{
		mMemoryPoolSize = bufferSize;
		mMemoryPool = new ui8[bufferSize];
		M_ASSERT_PTR(mMemoryPool);
		if (debug) printf("Size of Memory Pool : %d bytes, ptr %p\n", mMemoryPoolSize, mMemoryPool);
	}

	printf("\nMODE SOUND : %s\n\n", (play ? "DAC --> PLAYBACK" : "ADC --> RECORD"));

	if (play)
	{
		mDacStream->SetSamplingRate(44100.0);
		if (mSubscriber->Subscribe(mDacStream) < B_NO_ERROR)
		{
			mUseSound = false;
			if (debug) printf("Can't subscribe to DAC stream.\n");
		}
		else if (mSubscriber->EnterStream(NULL, true, (void *)this, CMoteur::bePlayStream, NULL, TRUE) < B_NO_ERROR)
		{
			mUseSound = false;
			if (debug) printf("Can't enter audio DAC stream.\n");
		}
	}
	else // record
	{
		mAdcStream->SetSamplingRate(44100.0);
		mAdcStream->SetADCInput(B_MIC_IN);
		mAdcStream->BoostMic(false);
		if (mSubscriber->Subscribe(mAdcStream) < B_NO_ERROR)
		{
			mUseSound = false;
			if (debug) printf("Can't subscribe to ADC stream.\n");
		}
		else if (mSubscriber->EnterStream(NULL, true, (void *)this, CMoteur::beRecordStream, NULL, TRUE) < B_NO_ERROR)
		{
			mUseSound = false;
			if (debug) printf("Can't enter audio ADC stream.\n");
		}
	}

	printf("\nMODE SOUND : %s\n\n", (mUseSound ? "USE SOUND YES" : "DON'T USE SOUND"));


} // end of initSound for CMoteur


//***************************************************************************
void CMoteur::closeSound(void)
//***************************************************************************
{
	if (mSubscriber)
	{
		mSubscriber->ExitStream(TRUE);
		mSubscriber->Unsubscribe();
		delete mSubscriber;
		mSubscriber = NULL;
	}

	if (mAdcStream) delete mAdcStream; mAdcStream = NULL;
	if (mDacStream) delete mDacStream; mDacStream = NULL;

	// free everything remaining in the buffer list
	while (!mSoundBufList.isEmpty())
	{
		SSoundBuffer *p = mSoundBufList.removeFirst();
		if (p) delete p;
	}

	if (mCurrentPlayBack)  delete mCurrentPlayBack; mCurrentPlayBack = NULL;

	if (mMemoryPool) delete mMemoryPool; mMemoryPool = NULL;
	mMemoryPoolSize = 0;

	mUseSound = true;

} // end of closeSound for CMoteur


//***************************************************************************
bool CMoteur::beRecordStream(void *arg, char *bebuf, size_t cnt, void *header)
//***************************************************************************
{
CMoteur *mMoteur = (CMoteur *)arg;

	// the function always receive 44.1kHz/16bit/Stereo data
	// thus one sample=2 bytes, and there are two samples per time frame == 4 bytes.
	// but only keep 11kHz/16bit/Mono data, ie keep the keep one sample
	// for 4 time frames, i.e. mix 8 samples into only one.
	// since cnt is in bytes and not in samples, nb final samples = cnt/16

	//printf("beRecordStream -- %s %p -- cnt %d", class_name(mMoteur), mMoteur, cnt);

	si32 nb = cnt>>4;
	SSoundBuffer *buf = new SSoundBuffer(nb);
	si16	*data = buf->mBuffer;

	//printf(" -- nb %d -- data %p -- bebuf %p\n", nb, data, bebuf);

	if (!data)
	{
		// there is some kind of allocation problem. Give up
		// HOOK TBLD should send an error message to the CMoteur
		if (debug) printf(" beRecordStream -- no data !\n");
		return false;
	}

	// convert 44.1kHz/16bit/Stereo into 11kHz/16bit/Mono
	si16 *source = (si16 *)bebuf;
	for(; nb>0; nb--)
	{
		//printf(" -- processing %d data\n", nb);
		si32 mean = (source[0]+source[1]+source[2]+source[3]+source[4]+source[5]+source[6]+source[7])/8;
		source += 8;
		*(data++) = mean;
	}

	//printf("  beRecordStream : add to list\n");

	// add data to the sound list
	mMoteur->mSoundBufSem.acquire();
	mMoteur->mSoundBufList.addLast(buf);
	mMoteur->mSoundBufSem.release();

	//printf("  end beRecordStream -- nb buf %d\n", mMoteur->mSoundBufList.size());

	return true;
	
} // end of beRecordStream for CMoteur


//***************************************************************************
bool CMoteur::bePlayStream(void *arg, char *bebuf, size_t cnt, void *header)
//***************************************************************************
{
CMoteur *mMoteur = (CMoteur *)arg;
SSoundBuffer *buf;

	// play back buffers that are in the list, up to the current cnt size.
	// here, we DON'T asumme that the current media kit buffers have the
	// same size than the recorded buffers. This might depend on a variety
	// of conditions (crossplatform being the first one, but differences
	// between BeOS release or current parameters being a possibility too).

	// data is expexcted to be in the recorded buffers as mono/16 bit/11kHz
	// thus source only is 1/8 of the destination samples.
	// since cnt is in bytes and not in samples, nb source samples = cnt/16

	si16 *dest = (si16 *)bebuf;
	si16 *source=NULL;
	si32 counter = cnt>>4;
	
	// get first buffer available, or use the last unprocessed if any
	buf = mMoteur->mCurrentPlayBack;
	mMoteur->mCurrentPlayBack = NULL;

	while(counter>0)
	{
		// get first buffer available, or use the last unprocessed if any
		if (!buf)
		{
			mMoteur->mSoundBufSem.acquire();
			buf = mMoteur->mSoundBufList.removeFirst();
			mMoteur->mSoundBufSem.release();
		}
		
		// houston, we have a problem
		if (!buf) return true;

		ui32 nbSample = buf->mNbSample;
		ui32 nbCopy = nbSample - buf->mNbCurrent;
		if (nbCopy > counter) nbCopy = counter;

		//printf("bePlayStream -- buffer size %d -- %d\n", nbSample, nbCopy);

		source = buf->mBuffer + buf->mNbCurrent;

		// expand the data by mixing it to the current stream data
		for(ui32 i=nbCopy; i>0; i--)
		{
			si16 data = *(source++);
			for(ui32 j=8; j>0; j--)
			{
				si32 d=(*dest) + data;
				if (d<-32767) d=-32767; else if (d>32767) d=32767;
				*(dest++) = d;
			}
		}

		// delete the buffer or keep it for after
		counter -= nbCopy;
		buf->mNbCurrent += nbCopy;
		if (buf->mNbCurrent >= buf->mNbSample)
		{
			delete buf;
			buf = NULL;
		}
	}

	if (buf) mMoteur->mCurrentPlayBack = buf;

	return true;

} // end of bePlayStream for CMoteur


//***************************************************************************
void CMoteur::recordSoundFrame(FILE *f)
//***************************************************************************
{
ui32 nbBuf;
ui32 frameCount;

	frameCount = mFrameCount;

	// important : while this function executes (at the end of recording the
	// current frame), the audio stream is still actively adding more buffers
	// to the audio list.
	//
	// thus the idea is to read the current number of buffer right now
	// and then only store these buffers. the remaining buffer will be saved
	// at next frame.
	//
	// the size method just returns an int, so it can be safely called without
	// locking the buffer list. we know the count may become obsolete right after
	// that operation.

	nbBuf = mSoundBufList.size();
	//if(debug) printf("recordSoundFrame -- nb buf == %d\n", nbBuf);

	// nothing to save ? give up.
	if (!nbBuf) return;

	// write header
	f_bg_write4('APCM', f);
	f_bg_write4(frameCount, f);

	// process buffers
	for(; nbBuf > 0; nbBuf--)
	{
		SSoundBuffer *buf;

		mSoundBufSem.acquire();
		buf = mSoundBufList.removeFirst();
		mSoundBufSem.release();

		if (!buf)
		{
			// there are less buffers than expected, write a null-size
			// tag in the file and give up.
			f_bg_write4(0, f);
			return;
		}

		// write number of _samples_ (not bytes !!)
		// when we will store the ADPCM data, we know the compression has
		// a factor of 1/4 in bytes-size, so there will be samples/2 bytes
		// in the compressed buffer
		
		ui32 nbSample = buf->mNbSample;
		ui32 nbSample2 = nbSample/2;
		f_bg_write4(nbSample, f);
		
		// use memory pool or create a new memory buffer for compressing ?
		// (the memory pool is ideally the same size than one recorded buffer from
		// the Media kit, thus the compressed data _should_ fit always !!)
		rbool usePool = (nbSample2 <= mMemoryPoolSize);

		// allocate the memory for the compressed data and compress it
		char *abuf;
		if (usePool) abuf = (char *)mMemoryPool;
		else
		{
			abuf = new char[nbSample2];
			M_ASSERT_PTR(abuf); // HOOK TBDL : report failure to CMoteur ?
		}
		adpcm_coder(buf->mBuffer, abuf, nbSample, &mAdpcmState);

		//if(debug) printf("WRITE ADPCM -- %d bytes -- valprev %5d -- index %3d\n", nbSample2, state.valprev, state.index);

		// save data
		fwrite(abuf, 1, nbSample2, f);
		
		// free data and buffer
		delete buf;
		if (!usePool) delete abuf;
	}

	// indicate that the list of buffer is finished by writing a null-size tag
	f_bg_write4(0, f);

} // end of recordSoundFrame for CMoteur


//***************************************************************************
void CMoteur::readSoundFrame(FILE *f)
//***************************************************************************
{
ui32 nbSample;

	// we enter here because the stream contains an 'APCM' tag, that should be
	// followed by ADPCM-encoded sound buffers.

	// the stream contains one or more packets in the format :
	// [0] ui32 - number of samples following (0=end)
	// [4] 4 bits samples (this samples/2 bytes to read)
	// Once decoded the samples will be stored in nbsamples*si16 (thus samples*2 bytes).
	// Compression factor is fixed to 4 with this ADPCM.

	while(f_bg_read4(nbSample, f) && nbSample)
	{
		// there is a non-null size tag
		// thus need to read size/2 bytes and convert it into a size si16 buffer
		ui32 nbSample2 = nbSample/2;

		// use memory pool or create a new memory buffer for compressing ?
		// (the memory pool is ideally the same size than one recorded buffer from
		// the Media kit, thus the compressed data _should_ fit always !!)
		rbool usePool = (nbSample2 <= mMemoryPoolSize);

		// allocate the memory for the compressed data 
		char *abuf;
		if (usePool) abuf = (char *)mMemoryPool;
		else
		{
			abuf = new char[nbSample2];
			M_ASSERT_PTR(abuf); // HOOK TBDL : report failure to CMoteur ?
		}

		// create the final unpacked sound buffer
		SSoundBuffer *buf = new SSoundBuffer(nbSample);
		M_ASSERT_PTR(buf && buf->mBuffer);

		// read data
		fread(abuf, 1, nbSample2, f);

		// and decompress it
		adpcm_decoder(abuf, buf->mBuffer, nbSample, &mAdpcmState);

		//if(debug) printf("READ ADPCM -- %d bytes -- valprev %5d -- index %3d\n", nbSample2, state.valprev, state.index);

		// insert buffer : add data to the sound list
		mSoundBufSem.acquire();
		mSoundBufList.addLast(buf);
		mSoundBufSem.release();

		// free data
		if (!usePool) delete abuf;
	}

} // end of readSoundFrame for CMoteur



//---------------------------------------------------------------------------

// eoc
#endif
