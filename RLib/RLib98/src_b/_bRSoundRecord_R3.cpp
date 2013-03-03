/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	RSoundRecord.cpp
	Partie	: Sound Processing

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RSoundRecord.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"

//---------------------------------------------------------------------------


//***************************************************************************
RSoundRecord::RSoundRecord(SSharedSoundBuffer &shared, ESoundDevice device)
						 :mSharedSoundBuffer(&shared)
//***************************************************************************
{
	mSoundDevice = device;
	mUseSound = true;
	mStarted = false;

	// be specific
	mSubscriber = NULL;
	mAdcStream = NULL;

	//initSound();

} // end of constructor for RSoundRecord


//***************************************************************************
RSoundRecord::~RSoundRecord(void)
//***************************************************************************
{
	if (mStarted) stop();
	closeSound();

} // end of destructor for RSoundRecord


//---------------------------------------------------------------------------


//***************************************************************************
void RSoundRecord::initSound(void)
//***************************************************************************
{

	if (!mAdcStream) mAdcStream = new BADCStream();
	if (!mAdcStream) mUseSound = false;
	if (!mUseSound) M_THROW(kErrNoSoundResource);

	if (!mSubscriber) mSubscriber = new BSubscriber("RSoundRecord");

	mAdcStream->SetSamplingRate(44100.0);
	mAdcStream->SetADCInput(mSoundDevice);
	mAdcStream->BoostMic(false);
	if (mSubscriber->Subscribe(mAdcStream) < B_NO_ERROR)
	{
		mUseSound = false;
		if (debug) printf("Can't subscribe to ADC stream.\n");
	}

	//if (!mUseSound) M_THROW(kErrNoSoundResource);

} // end of initSound for RSoundRecord


//***************************************************************************
void RSoundRecord::closeSound(void)
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

	// free everything remaining in the buffer list
	mSharedSoundBuffer->mSem.acquire();
	while (!mSharedSoundBuffer->mList.isEmpty())
	{
		SSoundBuffer *p = mSharedSoundBuffer->mList.removeFirst();
		if (p) delete p;
	}
	mSharedSoundBuffer->mSem.release();

	mUseSound = true;

} // end of closeSound for RSoundRecord


//***************************************************************************
void RSoundRecord::selectDevice(ESoundDevice device)
//***************************************************************************
{
	mSoundDevice = device;
	if (mAdcStream) mAdcStream->SetADCInput(mSoundDevice);
} // end of selectDevice for RSoundRecord


//---------------------------------------------------------------------------


//***************************************************************************
void RSoundRecord::start(void)
//***************************************************************************
{
	if (!mUseSound) M_THROW(kErrNoSoundResource);
	initSound();

	if (mSubscriber->EnterStream(NULL, true, (void *)this, RSoundRecord::beRecordStream, NULL, TRUE) < B_NO_ERROR)
	{
		mUseSound = false;
		if (debug) printf("Can't enter audio ADC stream.\n");
		M_THROW(kErrNoSoundResource);
	}

} // end of start for RSoundRecord


//***************************************************************************
void RSoundRecord::stop(void)
//***************************************************************************
{
	closeSound();

} // end of stop for RSoundRecord


//---------------------------------------------------------------------------


//***************************************************************************
bool RSoundRecord::beRecordStream(void *arg, char *bebuf, size_t cnt, void *header)
//***************************************************************************
{
RSoundInterface *object = (RSoundInterface*) arg;
RSoundRecord *elvis = M_CAST_AS(object, RSoundRecord);

	if (elvis) return elvis->processBuffer((si16 *)bebuf, cnt);
	return false;

} // end of beRecordStream for RSoundRecord


//---------------------------------------------------------------------------


//***************************************************************************
ui32 RSoundRecord::getIdealBufferSize(void)
//***************************************************************************
{
size_t bufferSize;
int32 bufferCount;
bool isRunning;
int32 subscriberCount;

	if (!mUseSound) M_THROW(kErrNoSoundResource);
	if (mAdcStream)
	{
		if (mAdcStream->GetStreamParameters(&bufferSize, &bufferCount, &isRunning, &subscriberCount) >= B_NO_ERROR)
			return bufferSize;
	}
	else M_THROW(kErrInit);	// object not initialized !
	return 4096;
}

//---------------------------------------------------------------------------


//*****************************************************
rbool RSoundRecord::processBuffer(si16 *source, ui32 cnt)
//*****************************************************
// default implementation that processes the buffers
// as containing 44kHz/16/ST samples (ST means 2 samples per time frame)
{
	// the function always receive 44.1kHz/16bit/Stereo data
	// thus one sample=2 bytes, and there are two samples per time frame == 4 bytes.
	// since cnt is in bytes and not in samples, nb final samples = cnt/2

	SSoundBuffer *buf = new SSoundBuffer(cnt>>1);	// record stereo data
	si16	*data = buf->mBuffer;

	if (!data)
	{
		// there is some kind of allocation problem. Give up
		// HOOK TBLD should send an error message to the CMoteur
		if (debug) printf(" beRecordStream -- no data !\n");
		return false;
	}

	// get the data
	memcpy(data, source, cnt);

	// add data to the sound list
	mSharedSoundBuffer->mSem.acquire();
	mSharedSoundBuffer->mList.addLast(buf);
	mSharedSoundBuffer->mSem.release();

	return true;
} // end of processBuffer for RSoundRecord


//---------------------------------------------------------------------------
#endif // RLIB_BEOS


// eoc
