/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	RSoundPlay.cpp
	Partie	: Sound Processing

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RSoundPlay.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"


#define K_USES_SOUND_PLAYER_R4

#include <MediaRoster.h>

//---------------------------------------------------------------------------


//***************************************************************************
RSoundPlay::RSoundPlay(SSharedSoundBuffer &shared)
					 :mSharedSoundBuffer(&shared)
//***************************************************************************
{
	
	mUseSound = true;
	mStarted = false;
	mCurrentPlayBack = NULL;

	// be specific
	mSubscriber = NULL;
	mDacStream = NULL;
	mSoundPlayer = NULL;

	//initSound();

} // end of constructor for RSoundPlay


//***************************************************************************
RSoundPlay::~RSoundPlay(void)
//***************************************************************************
{
	if (mStarted) stop();
	closeSound();

} // end of destructor for RSoundPlay


//---------------------------------------------------------------------------


//***************************************************************************
void RSoundPlay::initSound(void)
//***************************************************************************
{

	mCurrentPlayBack = NULL;

	#ifdef K_USES_SOUND_PLAYER_R4
		if (debug) printf("RSoundPlay::initSound + K_USES_SOUND_PLAYER_R4\n");
		if (!mSoundPlayer)
		{
			status_t err; 
			BMediaRoster *gMediaRoster;
			media_node *outNode;
			outNode=new media_node;
			gMediaRoster = BMediaRoster::Roster(&err);
			if (gMediaRoster && err == B_OK) err = gMediaRoster->GetAudioOutput(outNode);
	        if (!gMediaRoster || err != B_OK)
	        { 
	         	/* the Media Server appears to be dead -- handle that here */ 
	         	mUseSound=false;
	         	if (debug) printf("NO MEDIA ROSTER -- roster %p -- error %08lx (%ld)\n", gMediaRoster, err, err);
				if (!mUseSound) M_THROW(kErrNoSoundResource);
	        }
			
	
			//media_raw_audio_format format = {44100.0, 2, media_raw_audio_format::B_AUDIO_SHORT,
			//																 B_MEDIA_LITTLE_ENDIAN, 4*1024};
	
			mSoundPlayer = new BSoundPlayer(/*&format,*/ "RLibSoundPlayer", sPlayBuffer, NULL, (void *)this);
         	if (debug) printf("sound player %p\n", mSoundPlayer);
		}
	#else
		if (debug) printf("RSoundPlay::initSound\n");
		if (!mDacStream) mDacStream = new BDACStream();
		if (!mDacStream) mUseSound = false;
		//if (!mUseSound) M_THROW(kErrNoSoundResource);
	
		if (!mSubscriber) mSubscriber = new BSubscriber("RSoundPlay");
	
		mDacStream->SetSamplingRate(44100.0);
		if (mSubscriber->Subscribe(mDacStream) < B_NO_ERROR)
		{
			mUseSound = false;
			if (debug) printf("Can't subscribe to DAC stream.\n");
		}
	#endif

	if (!mUseSound) M_THROW(kErrNoSoundResource);

} // end of initSound for RSoundPlay


//***************************************************************************
void RSoundPlay::closeSound(void)
//***************************************************************************
{
	#ifdef K_USES_SOUND_PLAYER_R4
		if (mSoundPlayer)
		{
			mSoundPlayer->Stop();
			delete mSoundPlayer;
			mSoundPlayer = NULL;
		}
	#else
		if (mSubscriber)
		{
			mSubscriber->ExitStream(TRUE);
			mSubscriber->Unsubscribe();
			delete mSubscriber;
			mSubscriber = NULL;
		}
	
		if (mDacStream) delete mDacStream; mDacStream = NULL;
	#endif

	// free everything remaining in the buffer list
	mSharedSoundBuffer->mSem.acquire();
	while (!mSharedSoundBuffer->mList.isEmpty())
	{
		SSoundBuffer *p = mSharedSoundBuffer->mList.removeFirst();
		if (p) delete p;
	}
	mSharedSoundBuffer->mSem.release();

	if (mCurrentPlayBack) delete mCurrentPlayBack; mCurrentPlayBack = NULL;

	mUseSound = true;

} // end of closeSound for RSoundPlay


//---------------------------------------------------------------------------


//***************************************************************************
void RSoundPlay::start(void)
//***************************************************************************
{
	if (debug) printf("RSoundPlay::start -- use sound %d -- sound player %p\n", mUseSound, mSoundPlayer);
	if (!mUseSound) M_THROW(kErrNoSoundResource);
	initSound();

	#ifdef K_USES_SOUND_PLAYER_R4
		if (mSoundPlayer)
		{
			mSoundPlayer->Start();
			mSoundPlayer->SetHasData(true);
			if (debug) printf("mSoundPlayer start with data\n");
		}
		else
			M_THROW(kErrNoSoundResource);
	#else
		if (mSubscriber->EnterStream(NULL, true, (void *)this, RSoundPlay::bePlayStream, NULL, TRUE) < B_NO_ERROR)
		{
			mUseSound = false;
			if (debug) printf("Can't enter audio DAC stream.\n");
			M_THROW(kErrNoSoundResource);
		}
	#endif

} // end of start for RSoundPlay


//***************************************************************************
void RSoundPlay::stop(void)
//***************************************************************************
{
	closeSound();

} // end of stop for RSoundPlay


//---------------------------------------------------------------------------


//*************************************************************
void RSoundPlay::sPlayBuffer(void *user, void *buffer, size_t size,
						 	const media_raw_audio_format &format)
//*************************************************************
{
	if (!user || !buffer || !size) return;

	RSoundPlay *elThis = (RSoundPlay *)user;

	// we asked for SHORT format when creating the BSoundPlayer class.
	// Nevertheless, R4 Media Kit doesn't care and will ask for FLOATs here
	// so we nethertheless need to implement this if we want sound :(
	if (format.format == media_raw_audio_format::B_AUDIO_FLOAT)
		elThis->processBufferFloat((float *)buffer, (ui32)size); 
	if (format.format == media_raw_audio_format::B_AUDIO_SHORT)
		elThis->processBufferShort((si16 *)buffer, (ui32)size);
		
} // end of sPlayBuffer for RSoundPlay


//***************************************************************************
bool RSoundPlay::bePlayStream(void *arg, char *bebuf, size_t cnt, void *header)
//***************************************************************************
{
RSoundInterface *object = (RSoundInterface*) arg;
RSoundPlay *elvis = M_CAST_AS(object, RSoundPlay);

	if (elvis) return elvis->processBuffer((si16 *)bebuf, cnt);
	return false;

} // end of bePlayStream for RSoundPlay


//---------------------------------------------------------------------------


//***************************************************************************
ui32 RSoundPlay::getIdealBufferSize(void)
//***************************************************************************
{
	if (!mUseSound) M_THROW(kErrNoSoundResource);
	#ifdef K_USES_SOUND_PLAYER_R4
		return 4*1024;
	#else
		size_t bufferSize;
		int32 bufferCount;
		bool isRunning;
		int32 subscriberCount;
	
		if (mDacStream)
		{
			if (mDacStream->GetStreamParameters(&bufferSize, &bufferCount, &isRunning, &subscriberCount) >= B_NO_ERROR)
				return bufferSize;
		}
		M_THROW(kErrInit);	// object not initialized !
		return 0;
	#endif
}

//---------------------------------------------------------------------------


//***************************************************
rbool RSoundPlay::processBuffer(si16 *dest, ui32 cnt)
//***************************************************
// default implementation that processes the buffers
// as containing 44kHz/16/ST samples (ST means 2 samples per time frame)
{
SSoundBuffer *buf;

	// play back buffers that are in the list, up to the current cnt size.
	// here, we DON'T asumme that the current media kit buffers have the
	// same size than the recorded buffers. This might depend on a variety
	// of conditions (crossplatform being the first one, but differences
	// between BeOS release or current parameters being a possibility too).

	// data is expexcted to be in the recorded buffers as ST/16 bit/44kHz
	// since cnt is in bytes and not in samples, nb source samples = cnt/2

	si16 *source=NULL;
	si32 counter = cnt>>1;
	
	// get first buffer available, or use the last unprocessed if any
	buf = mCurrentPlayBack;
	mCurrentPlayBack = NULL;

	while(counter>0)
	{
		// get first buffer available, or use the last unprocessed if any
		if (!buf)
		{
			mSharedSoundBuffer->mSem.acquire();
			buf = mSharedSoundBuffer->mList.removeFirst();
			mSharedSoundBuffer->mSem.release();
		}
		
		// houston, we have a problem
		if (!buf) return true;

		ui32 nbSample = buf->mNbSample;
		ui32 nbCopy = nbSample - buf->mNbCurrent;
		if (nbCopy > (ui32)counter) nbCopy = (ui32)counter;

		source = buf->mBuffer + buf->mNbCurrent;

		// mix data into the current stream data
		for(ui32 i=nbCopy; i>0; i--)
		{
			si32 d=(*dest) + *(source++);
			if (d<-32767) d=-32767; else if (d>32767) d=32767;
			*(dest++) = d;
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

	if (buf) mCurrentPlayBack = buf;

	return true;

} // end of processBuffer for RSoundPlay


//**********************************************************
rbool RSoundPlay::processBufferShort(si16 *source, ui32 cnt)
//**********************************************************
{
	return true;
} // end of processBufferShort for RSoundPlay


//***********************************************************
rbool RSoundPlay::processBufferFloat(float *source, ui32 cnt)
//***********************************************************
{
	return true;
} // end of processBufferFloat for RSoundPlay


//---------------------------------------------------------------------------
#endif // RLIB_BEOS


// eoc
