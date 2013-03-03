/*****************************************************************************

	Projet	: Pulsar

	Fichier	:	CAudioCD.h
	Partie	: Loader

	Auteur	: RM
	Date    : 040797
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CAUDIOCD_
#define _H_CAUDIOCD_

#include <scsi.h>

//---------------------------------------------------------------------------


//************
class CAudioCD
//************
{
public:
	CAudioCD(void);
	virtual ~CAudioCD(void);

	bool init(void);

	bool initDeviceList();
	void try_dir(const char *directory);

	bool hasDetectedDevice(void) { return mDeviceList.CountItems() > 0; }
	void selectDevice(long index) { mDeviceName = (char *)mDeviceList.ItemAt(index);}

	bool isPlaying(void) { return mIsPlaying; }
	int32 getTrack(void) { return mCurrentTrack; }

	void	play(int32 track=1);
	void	stop(void);
	void	pause(void);
	void	resume(void);
	void	switchPause(void)	{ if (mIsPlaying) pause(); else resume(); }
	void	eject(void);
	void	setVolume(uint8 vol);
	void	scan(bool forward);

	int	 countTracks(void);
	bool getPosition(bool &playing, int32 &track);
	bool getCachedPosition(bool &playing, int32 &track);

	void	prevTrack(void)	{ if (mCurrentTrack>0) mCurrentTrack--; play(mCurrentTrack); 		}
	void	nextTrack(void)	{ mCurrentTrack++; play(mCurrentTrack); if (!mIsPlaying) play(1); }

	BList *getList(void) { return &mDeviceList; }

private:

	bool	play_track(int id, scsi_play_track *track);

	bool		mCachedValid;
	bool		mIsPlaying;
	uint32	mCurrentTrack;	// 0 by default
	BList		mDeviceList;		// a list of ptr on char * for valid names
	char		*mDeviceName;		// default device -- never free this


	scsi_toc					toc;
	scsi_play_track		track;
	scsi_volume				volume;
	scsi_position			position;
	scsi_read_cd			read_cd;
	scsi_scan					sscan;

}; // end of class defs for CAudioCD


//---------------------------------------------------------------------------

#endif // of _H_CAUDIOCD_

// eoh
