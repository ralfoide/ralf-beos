/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CAudioCD.cpp
	Partie	: Loader

	Auteur	: RM
	Date		: 040797
	Format	: tabs==2

	Based on :
		play.c
		Copyright (C) 1995 Be Incorporated.  All Rights Reserved.
		the format command
		(original source for DR9 AA, May-July 1997)

*****************************************************************************/

#include "globals.h"
#include "CAudioCD.h"
#include <unistd.h>
#include <fcntl.h>
#include <scsiprobe_driver.h>
#include "ide_calls.h"

//---------------------------------------------------------------------------


//***************************************************************************
CAudioCD::CAudioCD(void)
//***************************************************************************
{
	mIsPlaying = false;
	mCurrentTrack = 1;
	mDeviceName = NULL;
	mCachedValid = false;

} // end of constructor for CAudioCD


//***************************************************************************
CAudioCD::~CAudioCD(void)
//***************************************************************************
{
int32 n = mDeviceList.CountItems();

	for(int i=n-1; i>0; i--)
	{
		char *p = (char *)mDeviceList.RemoveItem(i);
		if (p) delete p;
	}

} // end of destructor for CAudioCD


//---------------------------------------------------------------------------


//***************************************************************************
bool CAudioCD::init(void)
//***************************************************************************
{
	initDeviceList();
	return true;
} // end of init for CAudioCD



//***************************************************************************
void CAudioCD::try_dir(const char *directory)
//***************************************************************************
{ 
	BDirectory dir; 
	dir.SetTo(directory); 
	if(dir.InitCheck() != B_NO_ERROR) return; 
	
	dir.Rewind(); 
	BEntry entry; 
	while(dir.GetNextEntry(&entry) >= 0)
	{ 
		BPath path; 
		const char *name; 
		entry_ref e; 
		
		if(entry.GetPath(&path) != B_NO_ERROR) continue; 
		name = path.Path(); 
		
		if(entry.GetRef(&e) != B_NO_ERROR) continue; 

		if(entry.IsDirectory())
		{ 
			if(strcmp(e.name, "floppy") == 0) continue; // ignore floppy (it is not silent) 
			try_dir(name);
		} 
		else
		{ 
			int devfd; 
			device_geometry g; 

			if(strcmp(e.name, "raw") != 0) continue; // ignore partitions 

			devfd = open(name, O_RDONLY); 
			if(devfd < 0) continue; 

			if(ioctl(devfd, B_GET_GEOMETRY, &g, sizeof(g)) >= 0)
			{
				if(g.device_type == B_CD)
				{
					// device found
					if (debug) printf("Valid device : %s\n",name);
				
					int l=strlen(name);
					char *p=new char[l+1];
					strcpy(p,name);
					mDeviceList.AddItem(p);
				}
			}
			close(devfd);
		} 
	}
}


//***************************************************************************
bool CAudioCD::initDeviceList(void)
//***************************************************************************
{
	try_dir("/dev/disk");

	// ---

	mDeviceName = (char *)mDeviceList.ItemAt(0);

	char *pe = getenv("PULSAR_CD_DEVICE");
	if (!mDeviceName && !pe)
	{
		#ifdef __POWERPC__
			char s[]="/dev/disk/scsi/030/raw";				// UGLY HACK HERE !!
		#else
			char s[]="/dev/disk/ide/atapi/0/slave/0/raw";			// UGLY HACK HERE !!
		#endif
		fprintf(stderr, "Pulsar backdoor :\n"
									"Please \"export PULSAR_CD_DEVICE=\"%s\" from the Terminal.\n", s);
		int l=strlen(s);
		char *p=new char[l+1];
		strcpy(p,s);
		mDeviceList.AddItem(p);
		mDeviceName = (char *)mDeviceList.ItemAt(0);
	}
	if (pe) { mDeviceName=pe; fprintf(stderr, "CD device forced to %s\n", pe); }

	if (debug) printf("selected CD device : %s\n", mDeviceName);
	return (mDeviceName != NULL);

} // end of initDeviceList for CAudioCD



//***************************************************************************
bool CAudioCD::play_track(int id, scsi_play_track *track)
//***************************************************************************
/*
	returns true on success
*/
{
	mCachedValid = false;

	track->start_index = 1;
	track->end_track = 99;
	track->end_index = 1;
	return (!ioctl(id, B_SCSI_PLAY_TRACK, track));
}

//***************************************************************************
void CAudioCD::play(int32 tracknum)
//***************************************************************************
/*
	tracknum ranges from 1 to N
	If track < 1, uses mCurrentTrack
*/
{
int		id;
int		i;
int		length;
//int	start;
long	req_track = 0;
bool	isplaying;

	if (getPosition(isplaying, req_track))
	{
		mCurrentTrack = req_track;
		if (isplaying && tracknum < 1) return;
	}

	if (tracknum > 0) mCurrentTrack = tracknum;
	if (mCurrentTrack < 1) mCurrentTrack = 1;

	if (!mDeviceName) {mIsPlaying = false; return;}
	id = open(mDeviceName, O_RDONLY);
	if (id >= 0)
	{
		if (!ioctl(id, B_SCSI_GET_TOC, &toc))
		{
			track.start_track = 0;
			for (i = toc.toc_data[2]; i <= toc.toc_data[3]; i++)
			{
				length = (toc.toc_data[(((i + 1) - toc.toc_data[2]) * 8) + 4 + 5] * 60) + 
						 (toc.toc_data[(((i + 1) - toc.toc_data[2]) * 8) + 4 + 6]);
				length -= ((toc.toc_data[((i - toc.toc_data[2]) * 8) + 4 + 5] * 60) + 
						  (toc.toc_data[((i - toc.toc_data[2]) * 8) + 4 + 6]));
				//if (debug) printf(" Track %.2d: %.2d:%.2d - ", i, length / 60, length % 60);
				if (toc.toc_data[((i - toc.toc_data[2]) * 8) + 4 + 1] & 4)
				{
					//	if (debug) printf("DATA\n");
				}
				else
				{
					//	if (debug) printf("AUDIO\n");
					if (!track.start_track) track.start_track = i;
				}
			}
			if (track.start_track)
			{
				if (mCurrentTrack>0)
				{
					req_track = mCurrentTrack;
					if ((req_track < toc.toc_data[2]) || (req_track > toc.toc_data[3]))
					{
						if (debug) printf("Requested track is out of range\n");
					}
					else if (toc.toc_data[((req_track - toc.toc_data[2]) * 8) + 4 + 1] & 4)
					{
						if (debug) printf("Requested track is not an audio track\n");
					}
					else
					{
						track.start_track = req_track;
						mIsPlaying = play_track(id, &track);
					}
				}
				else mIsPlaying = play_track(id, &track);
			}
			else if (debug) printf("No audio tracks on CD\n");
		}

		close(id);
	}

	if (mIsPlaying) { if (debug) printf("Playing %d\n", mCurrentTrack); }
	else if (debug) printf("Can't play %d\n", mCurrentTrack);

} // end of play for CAudioCD


//***************************************************************************
void CAudioCD::stop(void)
//***************************************************************************
{
int id;

	if (!mDeviceName) return;
	id = open(mDeviceName, O_RDONLY);
	if (id >= 0)
	{
		if (debug) printf("Stopping audio\n");
		ioctl(id, B_SCSI_STOP_AUDIO);
		close(id);
	}
	mIsPlaying = false;
	mCachedValid = false;

} // end of stop for CAudioCD


//***************************************************************************
void CAudioCD::pause(void)
//***************************************************************************
{
int id;

	if (!mDeviceName) return;
	id = open(mDeviceName, O_RDONLY);
	if (id >= 0)
	{
		if (debug) printf("Pausing audio\n");
		ioctl(id, B_SCSI_PAUSE_AUDIO);
		close(id);
	}
	mIsPlaying = false;
	mCachedValid = false;

} // end of pause for CAudioCD


//***************************************************************************
void CAudioCD::resume(void)
//***************************************************************************
{
int id;

	if (!mDeviceName) return;
	id = open(mDeviceName, O_RDONLY);
	if (id >= 0)
	{
		if (debug) printf("Resuming audio\n");
		ioctl(id, B_SCSI_RESUME_AUDIO);
		close(id);
	}
	mIsPlaying = true;
} // end of resume for CAudioCD


//***************************************************************************
void CAudioCD::eject(void)
//***************************************************************************
{
int id;

	if (!mDeviceName) return;
	id = open(mDeviceName, O_RDONLY);
	if (id >= 0)
	{
		if (debug) printf("Ejecting audio\n");
		ioctl(id, B_SCSI_EJECT);
		close(id);
	}
	mCurrentTrack = 1;
	mIsPlaying = false;
	mCachedValid = false;
} // end of eject for CAudioCD


//***************************************************************************
void CAudioCD::setVolume(uint8 vol)
//***************************************************************************
{
int id;

	if (!mDeviceName) return;
	id = open(mDeviceName, O_RDONLY);
	if (id >= 0)
	{
		if (debug) printf("Setting volume to %d\n", vol);
		volume.port0_volume = vol;
		volume.port1_volume = vol;
		volume.flags = B_SCSI_PORT0_VOLUME | B_SCSI_PORT1_VOLUME;
		ioctl(id, B_SCSI_SET_VOLUME, &volume);
		close(id);
	}
} // end of setVolume for CAudioCD


//***************************************************************************
void CAudioCD::scan(bool forward)
//***************************************************************************
{
int id;

	if (!mDeviceName) return;
	id = open(mDeviceName, O_RDONLY);
	if (id >= 0)
	{
		if (forward) sscan.direction = 1;
		else sscan.direction = -1;
		sscan.speed = 0;
		if (ioctl(id, B_SCSI_SCAN, &sscan) == B_ERROR)
			if (debug) printf("Error trying to scan\n");
		close(id);
	}
} // end of scan for CAudioCD


//***************************************************************************
int CAudioCD::countTracks(void)
//***************************************************************************
{
	int id;
	scsi_toc toc;
	int track;

	if (!mDeviceName) return 0;
	id = open(mDeviceName, O_RDONLY);
	if (id >= 0)
	{
		status_t result = ioctl(id, B_SCSI_GET_TOC, &toc);
		if (result == B_NO_ERROR)
			track = toc.toc_data[3];
		else
			track = -1;
		close(id);
	}
	return track;
}


//***************************************************************************
bool CAudioCD::getPosition(bool &playing, int32 &track)
//***************************************************************************
{
int id;
bool state=false;

	//printf("get position for %s\n", mDeviceName);
	if (!mDeviceName) return false;
	id = open(mDeviceName, O_RDONLY);
	if (id >= 0)
	{
		if (ioctl(id, B_SCSI_GET_POSITION, &position) != B_NO_ERROR)
		{
			if (debug) printf("Could not get current position\n");
		}
		else
		{
			if (debug) printf("getPosition : status %d -- track = %d\n", position.position[1], position.position[6]);
			switch(position.position[1])
			{
				case 0x00:
					if (debug) printf("Position not supported by device\n");
					break;
				case 0x11:
					if (debug) printf("Playing track %d (%.2d:%.2d:%.2d)\n",
								position.position[6],
								position.position[9],
								position.position[10],
								position.position[11]);
					playing=true;
					track = position.position[6];
					state=true;
					break;
				case 0x12:
					if (debug) printf("Paused at track %d (%.2d:%.2d:%.2d)\n",
						position.position[6],
						position.position[9],
						position.position[10],
						position.position[11]);
					playing=false;
					track = position.position[6];
					state=true;
					break;
				case 0x13:
					if (debug) printf("Play has been completed\n");
					break;
				case 0x14:
					if (debug) printf("Play stopped due to error\n");
					break;
				case 0x15:
					if (debug) printf("No status to return\n");
					break;
				default:
					if (debug) printf("Unexpected result: %.2x\n",
						position.position[1]);
			}
		}
		close(id);
	}

	if (state)
	{
		mCurrentTrack = track;
		mIsPlaying = playing;
		mCachedValid = true;
	}

	return state;
} // end of getPosition for CAudioCD


//***********************************************************
bool CAudioCD::getCachedPosition(bool &playing, int32 &track)
//***********************************************************
{
	if (!mCachedValid)
		return getPosition(playing, track);

	playing = mIsPlaying;
	track = mCurrentTrack;
	return true;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// eoc
