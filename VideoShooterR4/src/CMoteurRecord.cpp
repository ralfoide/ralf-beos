/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CMoteurRecord.cpp
	Partie	: Moteur

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#include "CMoteurRecord.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"
#include <Alert.h>
#include <StopWatch.h>

//---------------------------------------------------------------------------

ui32 _gDiffRec=0;
ui32 _gConstRec=0;



//***************************************************************************
CMoteurRecord::CMoteurRecord(RThread *parent, RPath &path)
							:RThread(),
							 CMoteurDocument(parent),
							 mOffscreen(K_BITMAP_BUFFER),
							 mDocumentPath(path),
							 mSoundRecord(mSharedSoundBuffer)
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
	mScreenCapture = NULL;
	mMouseX = 0;
	mMouseY = 0;
	mMouseVisible = false;
	mFrameDiffTotal = 0;
	mFrameDiffService = 0;
	mFrameCount = 0;
	mUseSound = true;
	mMemoryPool = NULL;
	mMemoryPoolSize = 0;

	// be specific
	mScreenId = B_MAIN_SCREEN_ID;

	// init

	si32 n = K_BITMAP_BUFFER;

	printf("CMoteurRecord::init -- moteur %p, name %s\n", this, class_name(this));

	mDiffBuffer = new ui8[mDisplaySx*mDisplaySy*2];
	M_ASSERT_PTR(mDiffBuffer);

	mPreviousImg = new RBitmap(mDisplaySx, mDisplaySy, kPixel256);
	mCurrentImg  = new RBitmap(mDisplaySx, mDisplaySy, kPixel256);
	M_ASSERT_PTR(mPreviousImg);
	M_ASSERT_PTR(mCurrentImg );

  for(n--; n>=0; n--)
	{
		RBitmap *map = new RBitmap(mDisplaySx, mDisplaySy, kPixel256);
		M_ASSERT_PTR(map);
		map->erase();
		mOffscreen.initItemAt(n, map);
	}

	setThread(this);

} // end of constructor for CMoteurRecord


//***************************************************************************
CMoteurRecord::~CMoteurRecord(void)
//***************************************************************************
{
	if (mCurrentImg ) delete mCurrentImg ; mCurrentImg  = NULL;
	if (mPreviousImg) delete mPreviousImg; mPreviousImg = NULL;
	if (mDiffBuffer ) delete mDiffBuffer ; mDiffBuffer  = NULL;

} // end of destructor for CMoteurRecord


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteurRecord::initFile(FILE * &f)
//***************************************************************************
{
	f = fopen(mDocumentPath.path(), "wb");

	printf("File %s open in write mode\n", mDocumentPath.path());
	M_ASSERT_PTR(f);

	gApplication->setFileType(mDocumentPath, K_APP_DOC_MIME);

	// write file header
	M_ASSERT_PTR(f_bg_write4(K_FILE_HEADER_SIGNATURE, f) && f_bg_write4(K_FILE_HEADER_VERSION, f));

} // end of initFile for CMoteurRecord


//***************************************************************************
void CMoteurRecord::initSound(void)
//***************************************************************************
{
	mUseSound = mSoundRecord.soundDeviceOpen();
	if (debug && !mUseSound) printf("CMoteurRecord : no sound device open\n");

	if (mUseSound)
	{
		// adjust the memory pool used for compression/decompression
		// the memory pool contains enough space for one buffer of the Media Kit
		
		mMemoryPoolSize = mSoundRecord.getIdealBufferSize();
		mMemoryPool = new ui8[mMemoryPoolSize];
		M_ASSERT_PTR(mMemoryPool);
		if (debug) printf("Size of Memory Pool : %ld bytes, ptr %p\n", mMemoryPoolSize, mMemoryPool);
	
		// start sound right now
		mSoundRecord.start();
	}

} // end of initSound for CMoteurRecord


//***************************************************************************
void CMoteurRecord::closeSound(void)
//***************************************************************************
{
	if (mUseSound) mSoundRecord.stop();

	if (mMemoryPool) delete mMemoryPool; mMemoryPool = NULL;
	mMemoryPoolSize = 0;

	mUseSound = true;

} // end of closeSound for CMoteurRecord


//---------------------------------------------------------------------------



//***************************************************************************
void CMoteurRecord::run(void)
//***************************************************************************
{
ui32 msg;
ui32 msgParam;
vptr msgData;
fp64 temps, deltatemps, tempsInitial;
RSem frameRateSem;
rbool loop = true;
FILE *file = NULL;

	if (debug) printf("CMoteurRecord::run\n");
	if (debug) printf("  doc path %s\n",mDocumentPath.path());

	frameRateSem.acquire();	// rend le semaphore bloquant

	try
	{
		// check bitdepth
		color_space cs = checkColorSpace();

		// the first time, the moteur must create a display and use it
		createDisplay();

		// create a bitmap for storing the screen, at the given color space
		mScreenCapture = new BBitmap(BRect(0,0,mDisplaySx-1,mDisplaySy-1), cs);
		M_ASSERT_PTR(mScreenCapture);
	
		initFile(file);
	
		// open sound play
		initSound();

		tempsInitial = systemTime();

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
			recordFrame(file, temps);

			// synchronize to frame rate
			deltatemps = temps+K_FRAME_RATE-systemTime();
			//printf("deltatemps %f\n", deltatemps);
			if (deltatemps > 0) frameRateSem.acquireTimeout(deltatemps);
		}

		tempsInitial = (systemTime() - tempsInitial)/1e6;
		if (debug) printf("\n\nNb diff frames = %ld , nb const frames = %ld\n", _gDiffRec, _gConstRec);
		if (debug) printf("\n\nStats : %ld frames, %.2f seconds, fps = %.2f\n",
			mFrameCount, tempsInitial, (tempsInitial > K_EPSILON ? mFrameCount/tempsInitial : 0.0));
	
		closeSound();
		if (file) fclose(file);
		closeDisplay();

	}
	M_CATCH("Moteur Record");

	delete mScreenCapture;
	mScreenCapture = NULL;

} // end of run for CMoteurRecord


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteurRecord::createDisplay(void)
//***************************************************************************
{
	if (debug) printf("CMoteurRecord::createDisplay (type %d)\n", displayType());
	if (mDisplay) closeDisplay();

	// reject unsupported display option
	if (displayType() != kDisplayWindow) M_THROW(kErrUnsupported);

	RRect rect = RWindow::centerWindow(mDisplaySx, mDisplaySy);
	mDisplay = new RBlitWindow(rect, K_APP_NAME " " K_APP_VERS " Record");
	M_ASSERT_PTR(mDisplay);
	// init and show window
	setWindow(mDisplay);
	mDisplay->setControlThread(getParent());
	mDisplay->setQuitAppWhenWindowClosed(true);
	mDisplay->setBitmapBuffer(&mOffscreen);

	// HOOK TBLD : HACK HACK !!
	// move the window _almost_ outside of the recording zone
	mDisplay->moveTo(630, 20);

} // end of createDisplay for CMoteurRecord


//***************************************************************************
void CMoteurRecord::closeDisplay(void)
//***************************************************************************
{
	if (debug) printf("CMoteurRecord::closeDisplay\n");
	if (!mDisplay) return;
	mDisplay->deleteWindow();
	mDisplay = NULL;
} // end of closeDisplay for CMoteurRecord


//---------------------------------------------------------------------------


//***************************************************************************
color_space CMoteurRecord::checkColorSpace(void)
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

	if (debug) { printf("Screen : color space %04lx, Frame ", (ui32)cs); frame.PrintToStream(); }

	if (cs != B_CMAP8)
	{
		BAlert *box;
	
		box = new BAlert(	K_APP_NAME " - About box",
											K_APP_NAME " " K_APP_VERS " for BeOS R3"
											"\nYour screen is not configured in 8-Bits/pixel "
											"color mode.\nThis parameter is set via the Screen "
											"Preference Panel.\n"
											"ScreenShooter largely better works in 8 bpp.\n",
											"Continue", "Check Again", NULL,
											B_WIDTH_FROM_WIDEST, B_INFO_ALERT);
	
		// check again ?
		if (box && box->Go() == 1)
			return checkColorSpace();
	}

	return cs;	// return current color space

} // end of checkColorSpace for CMoteurRecord


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteurRecord::displayFrame(void)
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

} // end of displayFrame for CMoteurRecord


//---------------------------------------------------------------------------


//***************************************************************************
rbool CMoteurRecord::f_bg_write2(ui32 s, FILE *f)
//***************************************************************************
{
ui8 a,b;
	a=s;
	b=s>>8;
	return fwrite(&b, 1, 1, f) && fwrite(&a, 1, 1, f);
}

//***************************************************************************
rbool CMoteurRecord::f_bg_write4(ui32 s, FILE *f)
//***************************************************************************
{
ui8 a,b,c,d;
	a=s;
	b=s>>8;
	c=s>>16;
	d=s>>24;
	return fwrite(&d, 1, 1, f) && fwrite(&c, 1, 1, f) && fwrite(&b, 1, 1, f) && fwrite(&a, 1, 1, f);
}


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteurRecord::recordFrame(FILE *f, fp64 temps)
//***************************************************************************
{
ui32 diff;
ui32 frameCount;
//BStopWatch stop1("CMoteurRecord::recordFrame");
	
	frameCount = mFrameCount;

	if (mUseSound) recordSoundFrame(f);

	grabFrame();
	drawMouse(); // -- this is no longer needed in R4
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
			fp64 t=systemTime()-temps;
			ui32 t1=(ui32)t;
			printf("  frame time : %ld\n", t1);
			f_bg_write4(t1, f);
			if (debug) fflush(f);

			_gDiffRec++;
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

			_gConstRec++;
		}

		mFrameDiffTotal += 8;					// saved data
		mFrameDiffService += 8;				// services codes (frame extra)
	}

	if (debug && frameCount%50==0)
		printf("Frame : total %8ld <+%7ld> -- RLE %8ld <+%7ld> -- ratio %7.2f\n",
			mFrameDiffTotal, mDiffLen, mFrameDiffService, diff, (mFrameDiffTotal ? 100.0*mFrameDiffService/mFrameDiffTotal : 0.0));

	// swap frames
	RBitmap *p = mPreviousImg;
	mPreviousImg = mCurrentImg;
	mCurrentImg = p;

	mFrameCount++;

} // end of recordFrame for CMoteurRecord


//***************************************************************************
void CMoteurRecord::grabFrame(void)
//***************************************************************************
{
//BStopWatch stop1("CMoteurRecord::grabFrame");

	ui32 sbpr   = (ui32)mScreenCapture->BytesPerRow();
	ui8 *source = (ui8 *)mScreenCapture->Bits();

//printf("save frame size <%ldx%ld>\n", mCurrentImg->dataSx(), mCurrentImg->dataSy());
//mScreenCapture->Bounds().PrintToStream();
//if (!sbpr) { printf("CMoteurRecord::grabFrame -- no sbpr\n"); return; }
//if (!source) { printf("CMoteurRecord::grabFrame -- no source\n"); return; }

	// actually request a copy of the bitmap
	{
		BRect rect(0,0,mDisplaySx-1,mDisplaySy-1);
		BScreen screen(mScreenId);
		screen.ReadBitmap(mScreenCapture, true, &rect);
	}

	ui8 *dest   = mCurrentImg->data();
	ui32 dbpr   = mCurrentImg->dataBpr();

//if (!dbpr) { printf("CMoteurRecord::grabFrame -- no dbpr\n"); return; }
//if (!dest) { printf("CMoteurRecord::grabFrame -- no dest\n"); return; }
//printf("grab -- sbpr %ld -- source %p -- dbpr %ld -- dest %p\n",  sbpr,source,dbpr,dest);

	//the case where sbpr == dbpr will be very rare
	//exemple typical screen = 1024x768, recording 640x480 :-(
	if (sbpr == dbpr)
	{
//printf("bitslength %ld -- datasxy %ld\n", mScreenCapture->BitsLength(), mCurrentImg->dataSxy());
		memcpy(dest, source, mCurrentImg->dataSxy());
	}
	else
	{
		ui32 dsy 		= mDisplaySy;
		ui32 bpr		= min(sbpr, dbpr);
		for(; dsy>0; dsy--)
		{
			memcpy(dest, source, bpr);
			source += sbpr;
			dest += dbpr;
		}
	}

	mDisplay->getScreenMouseLoc(mMouseX, mMouseY);
	mMouseVisible = true;

//printf("end of grabFrame\n");
//printf("   ");

} // end of grabFrame for CMoteurRecord


//***************************************************************************
void CMoteurRecord::drawMouse(void)
//***************************************************************************
{
const ui8 red = 42;
ui8 *dest = mCurrentImg->data();
ui32 bpr  = mCurrentImg->dataBpr();
ui32 x,y;

	x = mMouseX;
	y = mMouseY;
	if (mMouseVisible && x < mDisplaySx && y < mDisplaySy)
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

} // end of drawMouse for CMoteurRecord


//***************************************************************************
ui32 CMoteurRecord::diffFrame(void)
//***************************************************************************
// simple RLE encoding
// return true if images are different
{
//BStopWatch stop1("CMoteurRecord::diffFrame");
ui8 *pold = mPreviousImg->data();
ui8 *pnew = mCurrentImg->data();
ui8 *diff = mDiffBuffer;
ui8 *diffcount;
ui32 diffBytes = 0;
ui32 sxy = mCurrentImg->dataSxy();
ui32 count;
rbool forcediff;

//	if (debug) printf("CMoteurRecord::diffFrame -- pold %p pnew %p diff %p\n", pold, pnew, diff);
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
//printf("   ");
	return diffBytes;

} // end of diffFrame for CMoteurRecord


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteurRecord::recordSoundFrame(FILE *f)
//***************************************************************************
{
ui32 nbBuf;
ui32 frameCount;

//BStopWatch stop1("CMoteurRecord::recordSoundFrame");

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

	nbBuf = mSharedSoundBuffer.mList.size();
	if(debug) printf("recordSoundFrame -- nb buf == %ld\n", nbBuf);

	// nothing to save ? give up.
	if (!nbBuf) return;

	// write header
	f_bg_write4('APCM', f);
	f_bg_write4(frameCount, f);

	// process buffers
	for(; nbBuf > 0; nbBuf--)
	{
		SSoundBuffer *buf;

		mSharedSoundBuffer.mSem.acquire();
		buf = mSharedSoundBuffer.mList.removeFirst();
		mSharedSoundBuffer.mSem.release();

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

		// allocate the memory for the compressed data
		ui8 *abuf;
		if (usePool) abuf = (ui8 *)mMemoryPool;
		else
		{
			abuf = new ui8[nbSample2];
			M_ASSERT_PTR(abuf); // HOOK TBDL : report failure to CMoteurRecord ?
		}

		//  and compress it
		mAdpcmCodec.encode(buf->mBuffer, abuf, nbSample);

		// save data
		fwrite(abuf, 1, nbSample2, f);

		// free data and buffer
		delete buf;
		if (!usePool) delete abuf;
	}

	// indicate that the list of buffer is finished by writing a null-size tag
	f_bg_write4(0, f);

//printf("   ");

} // end of recordSoundFrame for CMoteurRecord


//---------------------------------------------------------------------------

// eoc
