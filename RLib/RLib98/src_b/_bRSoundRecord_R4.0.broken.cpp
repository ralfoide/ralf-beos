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

#include <TimeSource.h>

//---------------------------------------------------------------------------

#define K_PREFERED_BUF_SIZE	sizeof(fp32)*1024

//---------------------------------------------------------------------------


//***************************************************************************
RSoundRecord::RSoundRecord(SSharedSoundBuffer &shared, ESoundDevice device)
						 :mSharedSoundBuffer(&shared)
//***************************************************************************
{
	mSoundDevice = device;
	mUseSound = true;
	mStarted = false;
	mBufSize = K_PREFERED_BUF_SIZE;

	// be specific
	mConsumer = NULL;
	mRoster = NULL;

	initSound();

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
	printf("RSoundRecord::initSound\n");
	status_t err = B_NO_ERROR-1;;
	mRoster = BMediaRoster::Roster(&err);
	if (mRoster)
	{
printf("roster %p\n",mRoster);
		err = mRoster->GetAudioInput(&mAudioInputNode);
printf("GetAudioInput err %ld\n", err);
		if (err >= B_NO_ERROR)
		{
			mConsumer = new SoundConsumer("RSoundRecord", beSoundRecordFunc);
printf("mConsumer %p\n",mConsumer);
			err = mRoster->RegisterNode(mConsumer);
printf("RegisterNode err %ld\n", err);
		}
	}
	mUseSound = (err >= B_NO_ERROR);

} // end of initSound for RSoundRecord


//***************************************************************************
void RSoundRecord::closeSound(void)
//***************************************************************************
{
	printf("RSoundRecord::closeSound\n");
	if (mConsumer)
	{
		mConsumer->SetHooks(0, 0, 0);
		mConsumer->Release();
	}

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
#if 0
	if (mAdcStream) mAdcStream->SetADCInput(mSoundDevice);
#endif
} // end of selectDevice for RSoundRecord


//---------------------------------------------------------------------------


//***************************************************************************
void RSoundRecord::start(void)
//***************************************************************************
{
	bool success=false;
	status_t err;
	media_format fmt;
	media_node use_time_source;
	BTimeSource * tsobj = NULL;

	printf("RSoundRecord::start\n");
	if (!mUseSound) M_THROW(kErrNoSoundResource);
	initSound();

	int32 count = 0;
	err = mRoster->GetFreeOutputsFor(mAudioInputNode, &mAudioOutput, 1, &count, B_MEDIA_RAW_AUDIO);
	if (err < B_OK) printf("RSoundRecord::start: couldn't get free outputs from audio input node\n");
	else if (count < 1) printf("RSoundRecord::start: no free outputs from audio input node\n");
	else
	{
		mBufSize = mAudioOutput.format.u.raw_audio.buffer_size;
		printf("RSoundRecord::start --> buffer size set to %ld\n", mBufSize);

		err = mRoster->GetFreeInputsFor(mConsumer->Node(), &mRecInput, 1, &count, B_MEDIA_RAW_AUDIO);
		if (err < B_OK) printf("RSoundRecord::start: couldn't get free inputs for sound recorder\n");
		else if (count < 1) printf("RSoundRecord::start: no free inputs for sound recorder\n");
		else
		{
			tsobj = mRoster->MakeTimeSourceFor(mAudioInputNode);
			if (!tsobj) printf("RSoundRecord::start: couldn't clone time source from audio input node\n");
			else
			{
				err = mRoster->SetTimeSourceFor(mConsumer->Node().node, tsobj->Node().node);
				if (err < B_OK) printf("RSoundRecord::start: couldn't set the sound recorder's time source\n");
				else
				{
					fmt.u.raw_audio = mAudioOutput.format.u.raw_audio;
					fmt.type = B_MEDIA_RAW_AUDIO;
					err = mConsumer->SetHooks(beSoundRecordFunc, NULL, this);
					if (err < B_OK) printf("RSoundRecord::start: couldn't set the sound recorder's hook functions\n");
					else
					{
						err = mRoster->Connect(mAudioOutput.source, mRecInput.destination, &fmt, &mAudioOutput, &mRecInput);
						if (err < B_OK) printf("RSoundRecord::start: failed to connect sound recorder to audio input node.\n");
						else
						{
							if ((tsobj->Node() != mAudioInputNode) && !tsobj->IsRunning())
							{
								mRoster->StartNode(tsobj->Node(), BTimeSource::RealTime());
								success = true;
							}
						}
					}
				}
			}
		}
	}

	if (tsobj) tsobj->Release();

	if (!success)
	{
		mUseSound = false;
		if (debug) printf("Can't start RSoundRecord.\n");
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
ui32 RSoundRecord::getIdealBufferSize(void)
//***************************************************************************
{
	if (!mUseSound) M_THROW(kErrNoSoundResource);
	return (mBufSize ? mBufSize : K_PREFERED_BUF_SIZE);
#if 0
	if (mAdcStream)
	{
		size_t bufferSize;
		int32 bufferCount;
		bool isRunning;
		int32 subscriberCount;
		if (mAdcStream->GetStreamParameters(&bufferSize, &bufferCount, &isRunning, &subscriberCount) >= B_NO_ERROR)
			return bufferSize;
	}
	else M_THROW(kErrInit);	// object not initialized !
#endif
}

//---------------------------------------------------------------------------

/*
//***************************************************************************
bool RSoundRecord::beRecordStream(void *arg, char *bebuf, size_t cnt, void *header)
//***************************************************************************
{
RSoundInterface *object = (RSoundInterface*) arg;
RSoundRecord *elvis = M_CAST_AS(object, RSoundRecord);

	if (elvis) return elvis->processBuffer((si16 *)bebuf, cnt);
	return false;

} // end of beRecordStream for RSoundRecord
*/

//*************************************************************************
void RSoundRecord::beSoundRecordFunc(void * cookie, bigtime_t timestamp,
																		 void * data, size_t datasize,
																		 const media_raw_audio_format & format)
//*************************************************************************
{
RSoundInterface *object = (RSoundInterface*) cookie;
RSoundRecord *elvis = M_CAST_AS(object, RSoundRecord);

	printf("RSoundRecord::beSoundRecordFunc -- cookie %p -- data %p -- size %ld -- format %ld\n", cookie, data, datasize, format.format);

	if (format.format == media_raw_audio_format::B_AUDIO_FLOAT)
		elvis->processBufferFloat((fp32 *)data, (ui32)datasize);
	else if (format.format == media_raw_audio_format::B_AUDIO_SHORT)
		elvis->processBufferShort((si16 *)data, (ui32)datasize);

} // end of beSoundRecordFunc for RSoundRecord


//---------------------------------------------------------------------------


//************************************************************
rbool RSoundRecord::processBufferFloat(fp32 *source, ui32 cnt)
//************************************************************
// default implementation that processes the buffers
// as containing 44kHz/16/ST samples (ST means 2 samples per time frame)
{
	// the function always EXPECT to receive 44.1kHz/float/Stereo data
	// thus one sample=4 bytes, and there are two samples per time frame == 8 bytes.
	// since cnt is in bytes and not in samples, nb final samples = cnt/4

	si32 cnt2 = cnt/sizeof(fp32);
	SSoundBuffer *buf = new SSoundBuffer(cnt2);	// record stereo data
	si16	*data = buf->mBuffer;

	printf("RSoundRecord::processBufferFloat -- source %p -- cnt %ld\n", source, cnt);

	if (!data)
	{
		// there is some kind of allocation problem. Give up
		// HOOK TBLD should send an error message to the CMoteur
		if (debug) printf(" processBufferFloat -- no data !\n");
		return false;
	}

	// get the data
	while(cnt2 > 0) *(data++) = (si16)(*(source++) * 32767.f);

	// add data to the sound list
	mSharedSoundBuffer->mSem.acquire();
	mSharedSoundBuffer->mList.addLast(buf);
	mSharedSoundBuffer->mSem.release();

	return true;
} // end of processBuffer for RSoundRecord


//************************************************************
rbool RSoundRecord::processBufferShort(si16 *source, ui32 cnt)
//************************************************************
// default implementation that processes the buffers
// as containing 44kHz/16/ST samples (ST means 2 samples per time frame)
{
	// the function always receive 44.1kHz/16bit/Stereo data
	// thus one sample=2 bytes, and there are two samples per time frame == 4 bytes.
	// since cnt is in bytes and not in samples, nb final samples = cnt/2

	SSoundBuffer *buf = new SSoundBuffer(cnt>>1);	// record stereo data
	si16	*data = buf->mBuffer;

	printf("RSoundRecord::processBufferShort -- source %p -- cnt %ld\n", source, cnt);

	if (!data)
	{
		// there is some kind of allocation problem. Give up
		// HOOK TBLD should send an error message to the CMoteur
		if (debug) printf(" processBufferShort -- no data !\n");
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
