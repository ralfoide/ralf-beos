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

#define K_USE_BSOUNDPLAYER
#undef  K_USE_BBUFFERDPRODUCER

#include "RSoundInterface.h"
#include <MediaRoster.h>
#include <BufferProducer.h>
#include <SoundPlayer.h>

//---------------------------------------------------------------------------

class RSoundPlay;

//---------------------------------------------------------------------------


#ifdef K_USE_BBUFFERDPRODUCER

//*******************************************
class RSoundProducer : public BBufferProducer
//*******************************************
{
public:
									RSoundProducer();
virtual						~RSoundProducer();

									// BBufferProducer idiosyncrasis...
virtual	status_t	FormatSuggestionRequested(media_type type, int32 quality, media_format * format);
virtual	status_t	FormatProposal(const media_source & output, media_format * format);
virtual	status_t	FormatChangeRequested(const media_source & source, const media_destination & destination, media_format * io_format, int32 * out_change_count);
virtual	status_t	GetNextOutput(int32 * cookie, media_output * out_output);
virtual	status_t	DisposeOutputCookie(int32 cookie);
virtual	status_t	SetBufferGroup(const media_source & for_source, BBufferGroup * group);
virtual	status_t	PrepareToConnect(const media_source & what, const media_destination & where, media_format * format, media_source * out_source, char * out_name);
virtual	void			Connect(status_t error, const media_source & source, const media_destination & destination, const media_format & format, char * io_name);
virtual	void			Disconnect(const media_source & what, const media_destination & where);
virtual	void			LateNoticeReceived(const media_source & what, bigtime_t how_much, bigtime_t performance_time);
virtual	void			EnableOutput(const media_source & what, bool enabled, int32 * change_tag);
virtual	status_t	GetLatency(bigtime_t * out_lantency);
virtual	status_t	VideoClippingChanged(const media_source & for_source, int16 num_shorts, int16 * clip_data, const media_video_display_info & display, int32 * out_from_change_count);

									// our internal hacks
				void			setPlayer(RSoundPlayer *_player) { mPlayer = _player; }

protected:

				RSoundPlay		*mPlayer;
				BBufferGroup	*mBufferGroup;
				media_input		mAudioMixerInput;
				media_output	mPlayOutput;

				bigtime_t			mDownstreamLatency;
				bigtime_t			mPrivateLatency;

}; // end of RSoundProducer

#endif // of K_USE_BBUFFERDPRODUCER

//***************************************
class RSoundPlay : public RSoundInterface
//***************************************
{
public:
	// this calls initSound()
	// this will throw kErrNoSoundResource on error.
	RSoundPlay(SSharedSoundBuffer &shared);
	virtual ~RSoundPlay(void);

	rbool	soundDeviceOpen(void) { return mUseSound; }

	void start(void);
	void stop(void);

	ui32 getIdealBufferSize(void);


//----
protected:

	void	initSound(void);
	void	closeSound(void);

	// HOOK TBDL : create a default implementation that processes the buffers
	// as containing 44kHz/16/ST samples (ST means 2 samples per time frame)
	virtual rbool processBufferFloat(fp32 *buf, ui32 cnt);
	virtual rbool processBufferShort(si16 *buf, ui32 cnt);

	rbool	mUseSound;
	rbool	mStarted;
 
	SSharedSoundBuffer *mSharedSoundBuffer;

	// be-specific

	BMediaRoster		*mRoster;
	media_node			mAudioMixer;

#ifdef K_USE_BBUFFERDPRODUCER
	RSoundProducer	*mProducer;
#elif defined K_USE_BSOUNDPLAYER
	media_node			mAudioOutput;
	BSoundPlayer		*mSoundPlayer;
#endif 
	SSoundBuffer *mCurrentPlayBack;

private:
	static void bePlayStream(void *user, void *buf, size_t cnt,
													 const media_raw_audio_format &format);

}; // end of class defs for RSoundPlay


//---------------------------------------------------------------------------

#endif // RLIB_BEOS
#endif // of _H_RSOUNDPLAY_

// eoh
