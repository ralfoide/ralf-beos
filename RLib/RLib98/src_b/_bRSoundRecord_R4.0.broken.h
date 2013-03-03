/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	RSoundRecord.h
	Partie	: Sound Processing

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RSOUNDRECORD_
#define _H_RSOUNDRECORD_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RSoundInterface.h"

#include <MediaRoster.h>
#include <BufferProducer.h>
#include "SoundConsumer.h"

//---------------------------------------------------------------------------
class SoundConsumer;

//***************
enum ESoundDevice
//***************
{
	kSoundDeviceCd			,// = B_CD_IN,
	kSoundDeviceLineIn	,//= B_LINE_IN,
	kSoundDeviceMic			,//= B_MIC_IN
};

//---------------------------------------------------------------------------


//*****************************************
class RSoundRecord : public RSoundInterface
//*****************************************
{
public:
	// this calls initSound()
	// this will throw kErrNoSoundResource on error.
	RSoundRecord(SSharedSoundBuffer &shared, ESoundDevice device);
	virtual ~RSoundRecord(void);

	rbool	soundDeviceOpen(void) { return mUseSound; }

	void start(void);
	void stop(void);

	ui32 getIdealBufferSize(void);

	void selectDevice(ESoundDevice device);
	ESoundDevice currentDevice(void)	{ return mSoundDevice; }

//----
protected:

	void	initSound(void);
	void	closeSound(void);

	// HOOK TBDL : create a default implementation that processes the buffers
	// as containing 44kHz/16/ST samples (ST means 2 samples per time frame)
	virtual rbool processBufferFloat(fp32 *source, ui32 cnt);
	virtual rbool processBufferShort(si16 *source, ui32 cnt);

	rbool	mUseSound;
	rbool	mStarted;
 
	SSharedSoundBuffer *mSharedSoundBuffer;

	ESoundDevice mSoundDevice;

private:

	// be-specific
	BMediaRoster	*mRoster;
	SoundConsumer	*mConsumer;
	media_node		mAudioInputNode;
	media_output	mAudioOutput;
	media_input		mRecInput;
	int32					mBufSize;

	static void beSoundRecordFunc(void * cookie, bigtime_t timestamp,
																void * data, size_t datasize, const media_raw_audio_format & format);
	//static bool beRecordStream(void *arg, char *buf, size_t cnt, void *header);

}; // end of class defs for RSoundRecord


//---------------------------------------------------------------------------

#endif // RLIB_BEOS
#endif // of _H_RSOUNDRECORD_

// eoh
