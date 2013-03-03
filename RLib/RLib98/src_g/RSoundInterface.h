/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	RSoundInterface.h
	Partie	: Sound Processing

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_GRSOUNDINTERFACE_
#define _H_GRSOUNDINTERFACE_

#include "gMachine.h"
#include "TQList.h"
#include "RSem.h"

#include "R3MediaDefs.h"

//---------------------------------------------------------------------------

//*************************************
struct SSoundBuffer : public SQListItem
//*************************************
{
	SSoundBuffer(void) : SQListItem() { mBuffer=NULL; mNbSample=0; mNbCurrent=0; }
	SSoundBuffer(ui32 samples) : SQListItem() { mBuffer=new si16[samples]; mNbSample=samples; mNbCurrent=0; }
	~SSoundBuffer(void)	{ if (mBuffer) delete mBuffer; }
	si16	*mBuffer;
	ui32	mNbSample;
	ui32	mNbCurrent;
};

//---------------------------------------------------------------------------

//***********************
struct SSharedSoundBuffer
//***********************
{
	TQList<SSoundBuffer *> mList;
	RSem	mSem;
};

//---------------------------------------------------------------------------


//***************************************
class RSoundInterface : public RInterface
//***************************************
{
	virtual void start(void) = 0;
	virtual void stop(void)  = 0;
	virtual ui32 getIdealBufferSize(void) = 0;
	virtual rbool soundDeviceOpen(void) = 0;
};


//---------------------------------------------------------------------------

#endif // of _H_GRSOUNDINTERFACE_

// eoh
