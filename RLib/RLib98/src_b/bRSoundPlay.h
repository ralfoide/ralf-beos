/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	RSoundPlay.h
	Partie	: Sound Processing

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RSOUNDPLAY_
#define _H_RSOUNDPLAY_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RSoundInterface.h"

#include <SoundPlayer.h>

//---------------------------------------------------------------------------


//***************************************
class RSoundPlay : public RSoundInterface
//***************************************
{
public:
	// this calls initSound()
	// this will throw kErrNoSoundResource on error.
	RSoundPlay(SSharedSoundBuffer &shared);
	virtual ~RSoundPlay(void);

	rbool	soundDeviceOpen(void) { initSound(); return mUseSound; }

	void start(void);
	void stop(void);

	ui32 getIdealBufferSize(void);


//----
protected:

	void	initSound(void);
	void	closeSound(void);

	// HOOK TBDL : create a default implementation that processes the buffers
	// as containing 44kHz/16/ST samples (ST means 2 samples per time frame)
	virtual rbool processBufferFloat(float *buf, ui32 cnt);
	virtual rbool processBufferShort(si16 *source, ui32 cnt);
	virtual rbool processBuffer(si16 *buf, ui32 cnt);

	rbool	mUseSound;
	rbool	mStarted;
 
	SSharedSoundBuffer *mSharedSoundBuffer;

	// be-specific

	BSubscriber	*mSubscriber;
	BDACStream	*mDacStream;

	BSoundPlayer	*mSoundPlayer;		// R4

	SSoundBuffer *mCurrentPlayBack;

private:
	static bool bePlayStream(void *arg, char *buf, size_t cnt, void *header);
	static void sPlayBuffer(void *user, void *buffer, size_t size, const media_raw_audio_format &format);

}; // end of class defs for RSoundPlay


//---------------------------------------------------------------------------

#endif // RLIB_BEOS
#endif // of _H_RSOUNDPLAY_

// eoh
