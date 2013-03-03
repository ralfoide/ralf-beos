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

#include <MediaKit.h>
#include "SoundConsumer.h"			// for BeOS R4.5+ sound

//---------------------------------------------------------------------------


//***************************************************************************
RSoundRecord::RSoundRecord(SSharedSoundBuffer &shared, ESoundDevice device)
						 :mSharedSoundBuffer(&shared)
//***************************************************************************
{
	mSoundDevice = device;
	mUseSound = true;
	mStarted = false;

	// R3 be specific
	mSubscriber = NULL;
	mAdcStream = NULL;

	// R4.5
	mMediaRoster = NULL;

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
	if (debug) printf("RSoundRecord::initSound\n");
	if (mMediaRoster) return;	// already OK

	status_t result = B_OK;
	// build part of the audio
	// use new audio stuff (BeOS R4.5+)
	mAudioOutput.source = media_source::null;
	mRecInput.destination = media_destination::null;

	mMediaRoster = BMediaRoster::Roster(&result);
	if (debug) printf("mMediaRoster %p -- err %ld\n", mMediaRoster, result);
	if (!mMediaRoster || result < B_OK) goto give_up;

	result = mMediaRoster->GetAudioInput(&mAudioInputNode);
	if (result < B_OK) goto give_up;

	mSoundRecorder = new SoundConsumer("RSoundRecord");
	result = mMediaRoster->RegisterNode(mSoundRecorder);
	if (debug) printf("mSoundRecorder %p -- err %ld\n", mSoundRecorder, result);
	if (result < B_OK) goto give_up;

	// hack for testing -- get the first physical input available
	getFirstPhysicalInput(mAudioInputNode);

	if (mMediaRoster)	return;

give_up:
	mUseSound = false;
	if (debug) printf("Can't init sound: err=%ld (0x%08lx), '%s'\n",
		result, result, strerror(result));
	
	// (old media stuff deactivated)

	if (debug) printf("ALERT ! -> switching to old media kit !\n");

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
	if (mMediaRoster)	// R4.5
	{
		// BeOS R4.5 Media Kit
		// Some code is from the SoundCapture sample code

		//	If we are the last connection, the Node will stop automatically since it
		//	has nowhere to send data to.
		status_t err;
		if (mRecInput.destination != media_destination::null)
		{
			err = mMediaRoster->StopNode(mRecInput.node, 0);
			if (debug) printf("mMediaRoster->StopNode -> err %ld, %s\n", err, strerror(err));
			err = mMediaRoster->Disconnect(mAudioOutput.node.node, mAudioOutput.source, mRecInput.node.node, mRecInput.destination);
			if (debug) printf("mMediaRoster->Disconnect -> err %ld, %s\n", err, strerror(err));
			mAudioOutput.source = media_source::null;
			mRecInput.destination = media_destination::null;
		}
		mSoundRecorder->SetHooks(0, 0, 0);
	}

	if (mSubscriber)	// R3
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
	if (debug) printf("RSoundRecord::start\n");
	if (!mUseSound) M_THROW(kErrNoSoundResource);
	initSound();

	if (mMediaRoster)	// R4.5:
	{
		// BeOS R4.5 Media Kit
		// Some code is from the SoundCapture sample code
		
		// hack for testing -- get the first physical input available
		// then	Hook up input (these functions throw a const char * in case of error)
		makeRecordConnection(mAudioInputNode);

		//	And get it going...
		bigtime_t then = mSoundRecorder->TimeSource()->Now()+50000LL;
		mMediaRoster->StartNode(mSoundRecorder->Node(), then);
		//if (debug) printf("then = %ld\nis audio input node kind time source -> %s\n",
		//									then, (mAudioInputNode.kind & B_TIME_SOURCE) ? "YES" : "NO");
		if (mAudioInputNode.kind & B_TIME_SOURCE)
			mMediaRoster->StartNode(mAudioInputNode, mSoundRecorder->TimeSource()->RealTimeFor(then, 0));
		else
			mMediaRoster->StartNode(mAudioInputNode, then);

		if (debug) printf("RSoundRecord::start->StartNode -- ok\n");
	}
	else
	{
		// BeOS PR2-R3 Old Media Kit

		if (mSubscriber->EnterStream(NULL, true, (void *)this, RSoundRecord::beRecordStream, NULL, TRUE) < B_NO_ERROR)
		{
			mUseSound = false;
			if (debug) printf("Can't enter audio ADC stream.\n");
			M_THROW(kErrNoSoundResource);
		}
	}

} // end of start for RSoundRecord


//***************************************************************************
void RSoundRecord::stop(void)
//***************************************************************************
{
	closeSound();

} // end of stop for RSoundRecord


//---------------------------------------------------------------------------


//*********************************************************
void RSoundRecord::getFirstPhysicalInput(media_node &input)
//*********************************************************
{
	live_node_info live[30];
	int32 live_count = 30;
	status_t err;
	err = mMediaRoster->GetLiveNodes(live, &live_count,
																	 NULL, //const media_format *hasInput
																	 NULL, //const media_format *hasOutput
																	 NULL, //const char *name
																	 0);   //uint64 nodeKinds
	if (err < B_OK)
	{
		printf("getFirstPhysicalInput::err = %ld -- %s\n", err, strerror(err));
		throw strerror(err);
	}

	for(int32 i=0; i<live_count; i++)
	{
		media_node &node = live[i].node;
		if (node.kind & B_PHYSICAL_INPUT)
		{
			if (debug) printf("getFirstPhysicalInput -> live node[%ld] -- live name = '%s'\n"
												"Node:\n\tid = %lx\n\tport = %ld\n\tkind = %lx\n",
												i, live[i].name, node.node, node.port, node.kind);
			input = node;
			return;
		}
	}

	if (debug) printf("getFirstPhysicalInput - B_PHYSICAL_INPUT NOT FOUND !!??!\n");
	throw "B_PHYSICAL_INPUT NOT FOUND";

} // end of getFirstPhysicalInput for CPulsarApp


//**************************************************************
void RSoundRecord::makeRecordConnection(const media_node &input)
//**************************************************************
{
	if (debug) printf("RSoundRecord::makeRecordConnection\n");
	status_t err;
	int32 count = 0;

	//	Find an available output for the given input node.
	err = mMediaRoster->GetFreeOutputsFor(input, &mAudioOutput, 1, &count, B_MEDIA_RAW_AUDIO);
	if (err < B_OK)
	{
		if (debug) printf("makeRecordConnection::GetFreeOutputsFor err = %ld -- %s\n", err, strerror(err));
		throw strerror(err);
	}

	if (count < 1)
	{
		if (debug) printf("makeRecordConnection::GetFreeOutputsFor -> no free outputs\n");
		throw strerror(err);
	}

	//	Find an available input for our own Node. Note that we go through the
	//	MediaRoster; calling Media Kit methods directly on Nodes in our app is
	//	not OK (because synchronization happens in the service thread, not in
	//	the calling thread).
	// TODO: explain this
	err = mMediaRoster->GetFreeInputsFor(mSoundRecorder->Node(), &mRecInput, 1, &count, B_MEDIA_RAW_AUDIO);
	if (err < B_OK)
	{
		if (debug) printf("makeRecordConnection::GetFreeInputsFor err = %ld -- %s\n", err, strerror(err));
		throw strerror(err);
	}

	if (count < 1)
	{
		if (debug) printf("makeRecordConnection::GetFreeInputsFor -) no free inputs\n");
		throw strerror(err);
	}

	//	Find out what the time source of the input is.
	//	For most nodes, we just use the preferred time source (the DAC) for synchronization.
	//	However, nodes that record from an input need to synchronize to the audio input node
	//	instead for best results.
	//	MakeTimeSourceFor gives us a "clone" of the time source node that we can manipulate
	//	to our heart's content. When we're done with it, though, we need to call Release()
	//	on the time source node, so that it keeps an accurate reference count and can delete
	//	itself when it's no longer needed.
	// TODO: what about filters connected to audio input?
	media_node use_time_source;
	BTimeSource * tsobj = mMediaRoster->MakeTimeSourceFor(input);
	if (!tsobj)
	{
		if (debug) printf("makeRecordConnection::MakeTimeSourceFor - NULL\n");
		throw strerror(err);
	}

	//	Apply the time source in effect to our own Node.
	err = mMediaRoster->SetTimeSourceFor(mSoundRecorder->Node().node, tsobj->Node().node);
	if (err < B_OK)
	{
		if (debug) printf("makeRecordConnection::SetTimeSourceFor err = %ld -- %s\n", err, strerror(err));
		tsobj->Release();
		throw strerror(err);
	}

	//	Get a format, any format.
	media_format fmt;
	fmt.u.raw_audio = mAudioOutput.format.u.raw_audio;
	fmt.type = B_MEDIA_RAW_AUDIO;

	//	Tell the consumer where we want data to go.
	err = mSoundRecorder->SetHooks(recordProcessFunc, recordNotifyFunc, this);
	if (err < B_OK)
	{
		if (debug) printf("makeRecordConnection::SetHooks err = %ld -- %s\n", err, strerror(err));
		tsobj->Release();
		throw strerror(err);
	}

	//	Using the same structs for input and output is OK in BMediaRoster::Connect().
	err = mMediaRoster->Connect(mAudioOutput.source, mRecInput.destination, &fmt, &mAudioOutput, &mRecInput);
	if (err < B_OK)
	{
		if (debug) printf("makeRecordConnection::Connect err = %ld -- %s\n", err, strerror(err));
		tsobj->Release();
		mSoundRecorder->SetHooks(0, 0, 0);
		throw strerror(err);
	}

	//	Start the time source if it's not running.
	if (tsobj->Node() != input && !tsobj->IsRunning())
		mMediaRoster->StartNode(tsobj->Node(), BTimeSource::RealTime());

	tsobj->Release();	//	we're done with this time source instance!
	if (debug) printf("RSoundRecord::makeRecordConnection -> OK !\n");

} // end of makeRecordConnection for CPulsarApp


//**********************************************************************
void RSoundRecord::recordProcessFunc(void * cookie, bigtime_t timestamp, 
																	 void * data, size_t size,
																	 const media_raw_audio_format &format)
//**********************************************************************
{
	//	Callback called from the SoundConsumer when receiving buffers.
	//if (debug) printf("recordProcessFunc: cookie %p -- data %p -- size %ld -- frm 0x%04lx -- #ch %ld\n",
	//									cookie, data, size, format.format, format.channel_count);

	if (!cookie || !data || size<4) return;
	RSoundInterface *object = (RSoundInterface*) cookie;
	RSoundRecord *elvis = M_CAST_AS(object, RSoundRecord);

	// print typeinfo of the cookie class
	//if (debug) printf("elvis -> %p\n", elvis);
	//if (!elvis && debug)*/ printf("CLASS MISMATCH ! object %p -> %p -- class %s\n",
	//	cookie, object, M_CLASS_NAME(object));

	if (!elvis) return;

	#define _SI16 media_raw_audio_format::B_AUDIO_SHORT				// 0x02
	#define _FP32 media_raw_audio_format::B_AUDIO_FLOAT				// 0x24 (?!!)

	assert(format.channel_count);
	assert((format.format & _SI16) || (format.format & _FP32));
	
	if ((format.format & _SI16) == _SI16)
		elvis->processBufferShort((si16 *)data, size);
	else if ((format.format & _FP32) == _FP32)
		elvis->processBufferFloat((fp32 *)data, size);

	#undef _FP32
	#undef _SI16
}


//*****************************************************************
void RSoundRecord::recordNotifyFunc(void * cookie, int32 code, ...)
//*****************************************************************
{
	if (debug) printf("recordNotifyFunc -- cookie %p -- code %ld\n", cookie, code);
	// Tell the app we've stopped, if it doesn't already know.
	//if (code == B_WILL_STOP || code == B_NODE_DIES)
	//	be_app->PostMessage(K_MSG_STOP_STREAM);
}


//*********************************************************************************
bool RSoundRecord::beRecordStream(void *arg, char *bebuf, size_t cnt, void *header)
//*********************************************************************************
{
RSoundInterface *object = (RSoundInterface*) arg;
RSoundRecord *elvis = M_CAST_AS(object, RSoundRecord);

	if (elvis) return elvis->processBufferShort((si16 *)bebuf, cnt);
	return false;

} // end of beRecordStream for RSoundRecord


//---------------------------------------------------------------------------


//*****************************************
ui32 RSoundRecord::getIdealBufferSize(void)
//*****************************************
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
	//else M_THROW(kErrInit);	// object not initialized !
	return 4096;
}

//---------------------------------------------------------------------------

/*
//*******************************************************
rbool RSoundRecord::processBuffer(si16 *source, ui32 cnt)
//*******************************************************
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
*/

//---------------------------------------------------------------------------
#endif // RLIB_BEOS


// eoc
