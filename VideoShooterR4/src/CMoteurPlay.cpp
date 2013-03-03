/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CMoteurPlay.cpp
	Partie	: Moteur

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#include "CMoteurPlay.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"

//---------------------------------------------------------------------------

ui32 _gDiffPlay=0;
ui32 _gConstPlay=0;


//***************************************************************************
CMoteurPlay::CMoteurPlay(RThread *parent, RPath &path)
						:RThread(),
						 CMoteurDocument(parent),
						 mOffscreen(K_BITMAP_BUFFER),
						 mDocumentPath(path),
						 mSoundPlay(mSharedSoundBuffer)
//***************************************************************************
{
	mDiffBuffer = NULL;
	mDiffLen = 0;
	mDisplay = NULL;
	mDisplayType = kDisplayWindow;
	mDisplaySx = K_SX;
	mDisplaySy = K_SY;
	mCurrentImg  = NULL;
	mFrameDiffTotal = 0;
	mFrameDiffService = 0;
	mFrameCount = 0;
	mUseSound = true;
	mMemoryPool = NULL;
	mMemoryPoolSize = 0;

	// init
	si32 n = K_BITMAP_BUFFER;

	printf("CMoteurPlay::init -- moteur %p, name %s\n", this, class_name(this));

	mDiffBuffer = new ui8[K_SX*K_SY*2];
	M_ASSERT_PTR(mDiffBuffer);

	mCurrentImg  = new RBitmap(K_SX, K_SY, kPixel256);
	M_ASSERT_PTR(mCurrentImg );

  for(n--; n>=0; n--)
	{
		RBitmap *map = new RBitmap(K_SX, K_SY, kPixel256);
		M_ASSERT_PTR(map);
		map->erase();
		mOffscreen.initItemAt(n, map);
	}

	setThread(this);

} // end of constructor for CMoteurPlay


//***************************************************************************
CMoteurPlay::~CMoteurPlay(void)
//***************************************************************************
{
	if (debug) printf("CMoteurPlay::~CMoteurPlay\n");
	if (mCurrentImg ) delete mCurrentImg ; mCurrentImg  = NULL;
	if (mDiffBuffer ) delete mDiffBuffer ; mDiffBuffer  = NULL;

	if (debug) printf("CMoteurPlay::~CMoteurPlay -- DONE\n");
} // end of destructor for CMoteurPlay


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteurPlay::initFile(FILE * &f)
//***************************************************************************
{
	f = fopen(mDocumentPath.path(), "rb");

	if (debug) printf("File %s open in read mode\n", mDocumentPath.path());
	M_ASSERT_PTR(f);

	// read file header
	ui32	signature;
	ui32	version;
	
	M_ASSERT_PTR(f_bg_read4(signature, f) && f_bg_read4(version, f));
	M_ASSERT_PTR(signature == K_FILE_HEADER_SIGNATURE);
	M_ASSERT_PTR(version == K_FILE_HEADER_VERSION);

} // end of initFile for CMoteurPlay


//***************************************************************************
void CMoteurPlay::initSound(void)
//***************************************************************************
{
	mUseSound = mSoundPlay.soundDeviceOpen();
	if (debug && !mUseSound) printf("CMoteurPlay : no sound device open\n");

	if (mUseSound)
	{
		// adjust the memory pool used for compression/decompression
		// the memory pool contains enough space for one buffer of the Media Kit
		
		mMemoryPoolSize = mSoundPlay.getIdealBufferSize();
		mMemoryPool = new ui8[mMemoryPoolSize];
		M_ASSERT_PTR(mMemoryPool);
		if (debug) printf("Size of Memory Pool : %ld bytes, ptr %p\n", mMemoryPoolSize, mMemoryPool);
	
		// start sound right now
		mSoundPlay.start();
	}

} // end of initSound for CMoteurPlay


//***************************************************************************
void CMoteurPlay::closeSound(void)
//***************************************************************************
{
	if (mUseSound) mSoundPlay.stop();

	if (mMemoryPool) delete mMemoryPool; mMemoryPool = NULL;
	mMemoryPoolSize = 0;

	mUseSound = true;

} // end of closeSound for CMoteurPlay


//---------------------------------------------------------------------------



//***************************************************************************
void CMoteurPlay::run(void)
//***************************************************************************
{
ui32 msg;
ui32 msgParam;
ui32 frametime;
vptr msgData;
fp64 temps, deltatemps, tempsInitial;
RSem frameRateSem;
rbool loop = true;
FILE *file = NULL;

	frameRateSem.acquire();	// rend le semaphore bloquant

	try
	{
		// open file
		initFile(file);
	
		// the first time, the moteur must create a display and use it
		createDisplay();
	
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
						if (debug) printf("CMoteurPlay::run --> request to stop\n");
						loop = false;
						break;
					case K_MSG_CHANGE_DISPLAY:
						createDisplay();
						break;
				}
			}
	
			// process frame
			frametime = playFrame(file);
			if (!frametime) selfPostMessage(K_MSG_RTHREAD_EXIT);

			// synchronize to frame rate
/*			if(debug)
			{
				deltatemps = temps+K_FRAME_RATE-systemTime();
				printf("frame rate : %d -- actual %7.0f\n", frametime, deltatemps);
			}
*/
			deltatemps = temps+frametime-systemTime();
			//printf("deltatemps %f\n", deltatemps);
			if (deltatemps > 0) frameRateSem.acquireTimeout(deltatemps);
		}
	
		tempsInitial = (systemTime() - tempsInitial)/1e6;
		if (debug) printf("\n\nNb diff frames = %ld , nb const frames = %ld\n", _gDiffPlay, _gConstPlay);
		if (debug) printf("\n\nStats : %ld frames, %.2f seconds, fps = %.2f\n",
			mFrameCount, tempsInitial, (tempsInitial > K_EPSILON ? mFrameCount/tempsInitial : 0.0));
	}
	M_CATCH("Moteur Play");

	closeSound();
	if (file) fclose(file);
	closeDisplay();

} // end of run for CMoteurPlay


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteurPlay::createDisplay(void)
//***************************************************************************
{
	if (debug) printf("CMoteurPlay::createDisplay (type %d)\n", displayType());
	if (mDisplay) closeDisplay();

	// reject unsupported display option
	if (displayType() != kDisplayWindow) M_THROW(kErrUnsupported);

	RRect rect = RWindow::centerWindow(mDisplaySx, mDisplaySy);
	mDisplay = new RBlitWindow(rect, K_APP_NAME " " K_APP_VERS " Playback");
	M_ASSERT_PTR(mDisplay);
	// init and show window
	mDisplay->setControlThread(getParent());
printf("moteur play <%p> -- parent <%p> -- window %p\n", this, getParent(), mDisplay);
	setWindow(mDisplay);
	mDisplay->setQuitAppWhenWindowClosed(true);
	mDisplay->setBitmapBuffer(&mOffscreen);

} // end of createDisplay for CMoteurPlay


//***************************************************************************
void CMoteurPlay::closeDisplay(void)
//***************************************************************************
{
	if (debug) printf("CMoteurPlay::closeDisplay\n");
	if (!mDisplay) return;
	mDisplay->deleteWindow();
	mDisplay = NULL;
} // end of closeDisplay for CMoteurPlay


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


//***************************************************************************
void CMoteurPlay::displayFrame(void)
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

} // end of displayFrame for CMoteurPlay


//---------------------------------------------------------------------------


//***************************************************************************
rbool CMoteurPlay::f_bg_read4(ui32 &s, FILE *f)
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
ui32 CMoteurPlay::playFrame(FILE *f)
//***************************************************************************
{
ui32 type;
ui32 frameCount;
ui32 len;
ui32 frametime = (ui32)K_FRAME_RATE;
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
					//if (debug) printf("Play Frame <%5d> -- len %8d\n", frameCount, len);
					// decode delta pack into current frame
					fread(mDiffBuffer, 1, len, f);
					mDiffLen = len;
					unpackFrame();
					displayFrame();
				}
				f_bg_read4(frametime, f);
				if (frametime < K_FRAME_RATE) frametime = (ui32)K_FRAME_RATE;
			
				_gDiffPlay++;
				process = false;
				break;

			// video null frame
			case 'RALF':
				f_bg_read4(frameCount, f);	// read & ignore

				_gConstPlay++;
				process = false;
				break;

			// sound buffer in ADPCM format
			case 'APCM':
				f_bg_read4(frameCount, f);	// read & ignore

				//printf("APCM : Frame %5d -- %5d  -- nb bufs %d\n", frameCount, mFrameCount, mSharedSoundBuffer.mList.size());

				readSoundFrame(f);
				break;

			default:
				if (debug) printf("PLAY FRAME unknow type code %.4s\n", (char *)&type);
				return 0;
		}
	}

	mFrameCount++;
	return frametime;

} // end of playFrame for CMoteurPlay


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteurPlay::unpackFrame(void)
//***************************************************************************
// simple RLE decoding
{
ui8 *pnew = mCurrentImg->data();
ui8 *diff = mDiffBuffer;
//ui32 diffBytes = 0;
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

} // end of unpackFrame for CMoteurPlay


//---------------------------------------------------------------------------


//***************************************************************************
void CMoteurPlay::readSoundFrame(FILE *f)
//***************************************************************************
{
ui32 nbSample;

	// we enter here because the stream contains an 'APCM' tag, that should be
	// followed by ADPCM-encoded sound buffers.
	//
	// Important : if "mUseSound" is false, sound must not be decoded.
	// In which case, the file pointer is just advanced...

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

		if (!mUseSound)
		{
			// Important : if "mUseSound" is false, sound must not be decoded.
			// In which case, the file pointer is just advanced...
			fseek(f, nbSample2, SEEK_CUR);
			continue;
		}

		// use memory pool or create a new memory buffer for compressing ?
		// (the memory pool is ideally the same size than one recorded buffer from
		// the Media kit, thus the compressed data _should_ fit always !!)
		rbool usePool = (nbSample2 <= mMemoryPoolSize);

		// allocate the memory for the compressed data 
		ui8 *abuf;
		if (usePool) abuf = mMemoryPool;
		else
		{
			abuf = new ui8[nbSample2];
			M_ASSERT_PTR(abuf); // HOOK TBDL : report failure to CMoteurPlay ?
		}

		// create the final unpacked sound buffer
		SSoundBuffer *buf = new SSoundBuffer(nbSample);
		M_ASSERT_PTR(buf && buf->mBuffer);

		// read data
		fread(abuf, 1, nbSample2, f);

		// and decompress it
		mAdpcmCodec.decode(abuf, buf->mBuffer, nbSample2);

		// insert buffer : add data to the sound list
		mSharedSoundBuffer.mSem.acquire();
		mSharedSoundBuffer.mList.addLast(buf);
		mSharedSoundBuffer.mSem.release();

		// free data
		if (!usePool) delete abuf;
	}

} // end of readSoundFrame for CMoteurPlay



//---------------------------------------------------------------------------

// eoc
