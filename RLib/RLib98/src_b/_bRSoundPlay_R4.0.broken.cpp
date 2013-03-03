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

#include <ByteOrder.h>

//---------------------------------------------------------------------------
// select native endianess for the stream

#if B_HOST_IS_LENDIAN == 1
	#define K_NATIVE_MEDIA_ENDIAN B_MEDIA_LITTLE_ENDIAN
#elif B_HOST_IS_BENDIAN == 1
	#define K_NATIVE_MEDIA_ENDIAN B_MEDIA_BIG_ENDIAN
#else
	#error Neither B_HOST_IS_LENDIAN nor B_HOST_IS_BENDIAN are defined !
#endif

#define K_PREFERED_BUF_SIZE	sizeof(fp32)*1024

//---------------------------------------------------------------------------


//***************************************************************************
RSoundPlay::RSoundPlay(SSharedSoundBuffer &shared)
					 :mSharedSoundBuffer(&shared)
//***************************************************************************
{
	
	mUseSound = false;
	mStarted = false;
	mCurrentPlayBack = NULL;

	// be specific
	mRoster = NULL;
#ifdef K_USE_BBUFFERDPRODUCER
	mProducer = NULL;
#elif defined K_USE_BSOUNDPLAYER
	mSoundPlayer = NULL;
#endif 

	initSound();

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
	if (mUseSound) return;

	status_t err;
	try
	{
		if (!mRoster) mRoster = BMediaRoster::Roster(&err);
		if (err < B_OK) throw err;
		err = mRoster->GetAudioMixer(&mAudioMixer);
		if (err < B_OK) throw err;

#ifdef K_USE_BBUFFERDPRODUCER
		mProducer = new RSoundProducer();
		if (!mProducer) throw kErrNoSoundResource;
		err = mRoster->RegisterNode(mProducer);
		if (err < B_OK) throw err;
		mUseSound = true;
#elif defined K_USE_BSOUNDPLAYER
		err = mRoster->GetAudioOutput(&mAudioOutput);
		if (err < B_OK) throw err;
		media_raw_audio_format format = {44100.0, 2, media_raw_audio_format::B_AUDIO_FLOAT,
																		 K_NATIVE_MEDIA_ENDIAN, K_PREFERED_BUF_SIZE};
		mSoundPlayer = new BSoundPlayer(&format, "VideoShooter", bePlayStream, NULL, (void *)this);
		if (mSoundPlayer) mUseSound = true;
#endif 
	}
	catch(...)
	{
		if (debug) printf("Exception catch while trying to init to the Media Kit Mixer\n");
		if (mRoster) mRoster = NULL;
#ifdef K_USE_BBUFFERDPRODUCER
		delete mProducer;
		mProducer = NULL;
#elif defined K_USE_BSOUNDPLAYER
		delete mSoundPlayer;
		mSoundPlayer = NULL;
#endif 
	}

} // end of initSound for RSoundPlay


//***************************************************************************
void RSoundPlay::closeSound(void)
//***************************************************************************
{

#ifdef K_USE_BBUFFERDPRODUCER
	if (mUseSound)
	{
		if (mPlayOutput.source != media_source::null && mAudioMixerInput.destination != media_destination::null)
		{
			status_t err;
			printf("mRoster->StopNode...\n");
			err = mRoster->StopNode(mProducer->Node(), mProducer->TimeSource()->Now(), true);
			printf("... err %08x\n", err);
			printf("mRoster->Disconnect...\n");
			err = mRoster->Disconnect(mPlayOutput.node.node, mPlayOutput.source, mAudioMixerInput.node.node, mAudioMixerInput.destination);
			printf("... err %08x\n", err);
			mPlayOutput.source = media_source::null;
			mAudioMixerInput.destination = media_destination::null;
		}
	}
#elif defined K_USE_BSOUNDPLAYER
	if (mSoundPlayer)
	{
		mSoundPlayer->Stop();
		delete mSoundPlayer;
		mSoundPlayer = NULL;
	}
#endif

	mStarted = false;

	// free everything remaining in the buffer list
	mSharedSoundBuffer->mSem.acquire();
	while (!mSharedSoundBuffer->mList.isEmpty())
	{
		SSoundBuffer *p = mSharedSoundBuffer->mList.removeFirst();
		if (p) delete p;
	}
	mSharedSoundBuffer->mSem.release();

	delete mCurrentPlayBack;
	mCurrentPlayBack = NULL;

	mUseSound = false;

} // end of closeSound for RSoundPlay


//---------------------------------------------------------------------------


//***************************************************************************
void RSoundPlay::start(void)
//***************************************************************************
{
	if (!mUseSound) M_THROW(kErrNoSoundResource);

#ifdef K_USE_BBUFFERDPRODUCER
	media_format fmt;
	media_node tsnode;
	int32 count = 0;
	
	try
	{
		status_t err = mRoster->GetFreeInputsFor(mAudioMixer, &mAudioMixerInput, 1, &count, B_MEDIA_RAW_AUDIO);
		if (err < B_OK) throw "mRoster->GetFreeInputsFor failed";
		if (count <  1) throw "Mixer busy";

		err = mRoster->GetFreeOutputsFor(mProducer->Node(), &mPlayOutput, 1, &count, B_MEDIA_RAW_AUDIO);
		if (err < B_OK) throw "mRoster->GetFreeOutputsFor failed";
		if (count <  1) throw "Producer busy";

		err = mRoster->GetTimeSource(&tsnode);
		if (err < B_OK)
		{
			if (debug) printf("Switching back to systen default time source\n");
			err = mRoster->GetSystemTimeSource(&tsnode);
			if (err < B_OK) throw "mRoster->GetSystemTimeSource failed";
		}

		err = mRoster->SetTimeSourceFor(mProducer->ID(), tsnode.node);
		if (err < B_OK) throw "mRoster->SetTimeSourceFor failed";

		// add setup mProducer callbacks here

		media_raw_audio_format format = {44100.0, 2, media_raw_audio_format::B_AUDIO_FLOAT, //B_AUDIO_SHORT,
																		 K_NATIVE_MEDIA_ENDIAN, K_PREFERED_BUF_SIZE};
		fmt.u.raw_audio = format;
		fmt.type = B_MEDIA_RAW_AUDIO;

		err = mRoster->Connect(mPlayOutput.source, mAudioMixerInput.destination, &fmt, &mPlayOutput, &mAudioMixerInput);
		if (err < B_OK) throw "mRoster->Connect failed";

		bigtime_t then = mProducer->TimeSource()->Now()+50000LL;
		mRoster->StartNode(mProducer->Node(), then);
		
	}
	catch(const char *s)
	{
		if (debug) printf("RSoundPlay::start Exception : '%s'\n", s);
		M_THROW(kErrNoSoundResource);
	}
	catch(...}
	{
		if (debug) printf("RSoundPlay::start -- Unknown Exception\n");
		M_THROW(kErrNoSoundResource);
	}
#elif defined K_USE_BSOUNDPLAYER

	if (!mSoundPlayer) M_THROW(kErrNoSoundResource);
	mSoundPlayer->Start();
	mSoundPlayer->SetHasData(true);

#endif

	mStarted = true;

} // end of start for RSoundPlay


//***************************************************************************
void RSoundPlay::stop(void)
//***************************************************************************
{
	closeSound();

} // end of stop for RSoundPlay


//---------------------------------------------------------------------------


//***************************************************************************
void RSoundPlay::bePlayStream(void *user, void *buf, size_t cnt,
														  const media_raw_audio_format &format)
//***************************************************************************
{
RSoundInterface *object = (RSoundInterface*) user;
RSoundPlay *elvis = M_CAST_AS(object, RSoundPlay);

	if (format.format == media_raw_audio_format::B_AUDIO_FLOAT)
		elvis->processBufferFloat((fp32 *)buf, (ui32)cnt);
	else if (format.format == media_raw_audio_format::B_AUDIO_SHORT)
		elvis->processBufferShort((si16 *)buf, (ui32)cnt);

} // end of bePlayStream for RSoundPlay


//---------------------------------------------------------------------------


//***************************************************************************
ui32 RSoundPlay::getIdealBufferSize(void)
//***************************************************************************
{
	if (!mUseSound) M_THROW(kErrNoSoundResource);
	return K_PREFERED_BUF_SIZE;
}

//---------------------------------------------------------------------------


//*******************************************************
rbool RSoundPlay::processBufferShort(si16 *buf, ui32 cnt)
//*******************************************************
{
	printf("RSoundPlay::processBufferShort -- call is not supported\n");
	M_THROW(kErrUnsupported);
	return false;
}


//*********************************************************
rbool RSoundPlay::processBufferFloat(fp32 *dest, ui32 cnt)
//*********************************************************
// default implementation that processes the buffers
// as containing 44kHz/16/ST samples (ST means 2 samples per time frame)
{
	SSoundBuffer *buf;
	const fp32 coef = 1/32768;

	// play back buffers that are in the list, up to the current cnt size.
	// here, we DON'T asumme that the current media kit buffers have the
	// same size than the recorded buffers. This might depend on a variety
	// of conditions (crossplatform being the first one, but differences
	// between BeOS release or current parameters being a possibility too).

	// data is expexcted to be in the recorded buffers as ST/16 bit/44kHz
	// since cnt is in bytes and not in samples, nb source samples = cnt/(sizeof fp32)

	si16 *source=NULL;
	si32 counter = cnt/sizeof(fp32);

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

		si32 nbSample = (si32)buf->mNbSample;
		si32 nbCopy = nbSample - buf->mNbCurrent;
		if (nbCopy > counter) nbCopy = counter;

		source = buf->mBuffer + buf->mNbCurrent;

		// put data into the current stream data
		for(ui32 i=nbCopy; i>0; i--) *(dest++) = (*(source)++) * coef;

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


//---------------------------------------------------------------------------


#ifdef K_USE_BBUFFERDPRODUCER


//*************************************************
RSoundProducer::RSoundProducer()
							 :BMediaNode("RSoundProducerNode"),
								BBufferProducer(B_MEDIA_RAW_AUDIO),
								mPlayer(NULL)
//*************************************************
{
	if (debug) printf("RSoundProducer::FormatProposal\n");
}


//*******************************
RSoundProducer::~RSoundProducer()
//*******************************
{
	if (debug) printf("RSoundProducer::FormatProposal\n");
}


//---------------------------------------------------------------------------
// BBufferProducer + BMediaNode specifics


//***********************************************************************
status_t RSoundProducer::FormatSuggestionRequested(media_type type,
																									 int32 quality,
																									 media_format * format)
//***********************************************************************
{
	if (debug) printf("RSoundProducer::FormatSuggestionRequested\n");
	if (type <= 0) type = B_MEDIA_RAW_AUDIO;
	if (type != B_MEDIA_RAW_AUDIO) return B_MEDIA_BAD_FORMAT;
	format->type = type;
	format->u.raw_audio = media_raw_audio_format::wildcard;
	format->u.raw_audio.frame_rate = 44100.0;
	format->u.raw_audio.channel_count = 2;
	format->u.raw_audio.byte_order = K_NATIVE_MEDIA_ENDIAN;
	format->u.raw_audio.format = media_raw_audio_format::B_AUDIO_FLOAT;	// B_AUDIO_SHORT
	format->u.raw_audio.buffer_size = K_PREFERED_BUF_SIZE;
}


//******************************************************************
status_t RSoundProducer::FormatProposal(const media_source & output,
																				media_format * format)
//******************************************************************
{
	if (debug) printf("RSoundProducer::FormatProposal\n");
	if (format->type <= 0)
	{
		FormatSuggestionRequested(B_MEDIA_RAW_AUDIO, 0, format);
		return B_OK;
	}
	if (format->type != B_MEDIA_RAW_AUDIO) return B_MEDIA_BAD_FORMAT;

	#define M_CHECK(n,v)																									\
		if (format->u.raw_audio. n <= media_raw_audio_format::wildcard. n )	\
			format->u.raw_audio. n = v;																				\
		else if (format->u.raw_audio. n != m_raw_format. n)									\
			return B_MEDIA_BAD_FORMAT;

	M_CHECK(frame_rate, 44100.0);
	M_CHECK(channel_count, 2);
	M_CHECK(format, media_raw_audio_format::B_AUDIO_FLOAT);
	M_CHECK(byte_order, K_NATIVE_MEDIA_ENDIAN);
	if (format->u.raw_audio.buffer_size <= media_raw_audio_format::wildcard.buffer_size)
		format->u.raw_audio.buffer_size = K_PREFERED_BUF_SIZE;				

	#undef M_CHECK;
	return B_OK;
}


//***********************************************************************************
status_t RSoundProducer::FormatChangeRequested(const media_source & source,
																							 const media_destination & destination,
																							 media_format * io_format,
																							 int32 * out_change_count)
//***********************************************************************************
{
	if (debug) printf("RSoundProducer::FormatChangeRequested\n");
	status_t err = FormatProposal(source, io_format);
	if (err >= B_OK) allocBuffer();
	return err;
}


//***************************************************************
status_t RSoundProducer::GetNextOutput(int32 * cookie,
																			 media_output * out_output)
//***************************************************************
{
	if (debug) printf("RSoundProducer::GetNextOutput\n");
	if (*cookie == 0)
	{
		*out_output = m_output;
		*cookie = 1;
		return B_OK;
	}

	return B_BAD_INDEX;		// There's only one output.
}


//********************************************************
status_t RSoundProducer::DisposeOutputCookie(int32 cookie)
//********************************************************
{
	if (debug) printf("RSoundProducer::DisposeOutputCookie\n");
	return B_OK;
}


//**********************************************************************
status_t RSoundProducer::SetBufferGroup(const media_source & for_source,
																				BBufferGroup * group)
//**********************************************************************
{
	if (debug) printf("RSoundProducer::SetBufferGroup\n");
	if (for_source != mPlayOutput.source)
	{
		if (debug) printf("SoundProducer::SetBufferGroup(): bad source\n");
		return B_MEDIA_BAD_SOURCE;
	}
	if (group != mBufferGroup)
	{
		delete mBufferGroup;
		mBufferGroup = group;
	}
	return B_OK;
}


//************************************************************************
status_t RSoundProducer::PrepareToConnect(const media_source & what,
																					const media_destination & where,
																					media_format * format,
																					media_source * out_source,
																					char * out_name)
//************************************************************************
{
	if (debug) printf("RSoundProducer::PrepareToConnect\n");
	if (what != m_output.source)
	{
		if (debug) printf("SoundProducer::PrepareToConnect(): bad source\n");
		return B_MEDIA_BAD_SOURCE;
	}
	if (mPlayOutput.destination != media_destination::null)
	{
		if (debug) printf("SoundProducer::PrepareToConnect(): already connected\n");
		return B_MEDIA_BAD_DESTINATION;
	}

	if (!mPlayOutput.format.u.raw_audio.buffer_size)
		if (format->u.raw_audio.buffer_size > 0)
			mPlayOutput.format.u.raw_audio.buffer_size = format->u.raw_audio.buffer_size;
		else
			format->u.raw_audio.buffer_size = K_PREFERED_BUF_SIZE;

#if 1
		char fmt[100];
		string_for_format(m_output.format, fmt, 100);
		if (debug) printf("we're suggesting %s\n", fmt);
		string_for_format(*format, fmt, 100);
		if (debug) printf("he's expecting %s\n", fmt);
#endif

	// if (!format_is_compatible(*format, m_output.format)) ... ?

	mPlayOutput.destination = where;
	mPlayOutput.format = *format;
	*out_source = mPlayOutput.source;
	strncpy(out_name, Name(), B_MEDIA_NAME_LENGTH);
	return B_OK;
}


//*****************************************************************
void RSoundProducer::Connect(status_t error,
														 const media_source & source,
														 const media_destination & destination,
														 const media_format & format,
														 char * io_name)
//*****************************************************************
{
	if (debug) printf("RSoundProducer::Connect\n");
	M_ASSERT(source == m_output.source);

	if (error < B_OK)
	{
		if (debug) printf("SoundProducer::Connect(): we were told about an error\n");
		mPlayOutput.destination = media_destination::null;
		return;
	}

#if 1
	char fmt[100];
	string_for_format(format, fmt, 100);
	if (debug) printf("Connect(): format %s\n", fmt);
#endif

	mPlayOutput.destination = destination;
	mPlayOutput.format = format;

	allocBuffer();

	BBufferProducer::GetLatency(&mDownstreamLatency);
	strncpy(io_name, Name(), B_MEDIA_NAME_LENGTH);

}


//**************************************************************
void RSoundProducer::Disconnect(const media_source & what,
																const media_destination & where)
//**************************************************************
{
	if (debug) printf("RSoundProducer::Disconnect\n");
	if (what != mPlayOutput.source)
	{
		if (debug) printf("SoundProducer::Disconnect(): source is incorrect\t");
		return;
	}
	// We can't disconnect from someone who isn't connected to us.
	if (consumer != mPlayOutput.destination)
	{
		if (debug) printf("SoundProducer::Disconnect(): destination is incorrect\n");
		return;
	}

	mPlayOutput.destination = media_destination::null;
	mPlayOutput.format.u.raw_audio.buffer_size = 0;
	delete mBufferGroup;
	mBufferGroup = 0;
}


//*****************************************************************
void RSoundProducer::LateNoticeReceived(const media_source & what,
																				bigtime_t how_much,
																				bigtime_t performance_time)
//*****************************************************************
{
	if (debug) printf("RSoundProducer::LateNoticeReceived\n");
	
	// TBDL...
}


//**********************************************************
void RSoundProducer::EnableOutput(const media_source & what,
																	bool enabled,
																	int32 * change_tag)
//**********************************************************
{
	if (debug) printf("RSoundProducer::EnableOutput\n");

	if (what != mPlayOutput.source)
	{
		if (debug) printf("SoundProducer::EnableOutput(): bad source\n");
		return;
	}

	// TBDL
	// m_muted = !enabled;
	// *change_tag = IncrementChangeTag();

	if (m_output.destination != media_destination::null)
		SendDataStatus(!enabled ? B_DATA_NOT_AVAILABLE : B_DATA_AVAILABLE, 
			mPlayOutput.destination, TimeSource()->Now());
}


//*************************************************************************************
status_t RSoundProducer::VideoClippingChanged(const media_source & for_source,
																							int16 num_shorts,
																							int16 * clip_data,
																							const media_video_display_info & display,
																							int32 * out_from_change_count)
//*************************************************************************************
{
	if (debug) printf("RSoundProducer::VideoClippingChanged -- ROTFL !!\n");
	return B_ERROR;
}


//***********************************************************
status_t RSoundProducer::GetLatency(bigtime_t * out_lantency)
//***********************************************************
{
	status_t err = BBufferProducer::GetLatency(out_latency);
	// if (err >= B_OK) *out_latency = TotalLatency(); --- HOOK TBDL
	*out_latency += 500LL;

	return err;
}

//---------------------------------------------------------------------------
// BMediaNode only

virtual	port_id ControlPort() const = 0;

virtual	BMediaAddOn* AddOn(
				int32 * internal_id) const = 0;	/* Who instantiated you -- or NULL for app class */



#endif // of K_USE_BBUFFERDPRODUCER


#endif // RLIB_BEOS


// eoc
