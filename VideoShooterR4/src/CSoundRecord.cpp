/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CSoundRecord.cpp
	Partie	: Record son

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#include "CSoundRecord.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"

#include <math.h>

//---------------------------------------------------------------------------


//***************************************************************************
rbool CSoundRecord::processBufferFloat(fp32 *source, ui32 cnt)
//***************************************************************************
{
	// TBDL : COMMENT ARE OBSOLETE
	// the function always receive 44.1kHz/float/Stereo data
	// thus one sample=4 bytes, and there are two samples per time frame == 8 bytes.
	// but only keep 11kHz/16bit/Mono data, ie keep only one sample
	// for 4 time frames, i.e. mix 8 samples into only one.
	// since cnt is in bytes and not in samples, nb final samples = cnt/4/8 = cnt/32

	si32 nb = cnt/8/sizeof(fp32);
	SSoundBuffer *buf = new SSoundBuffer(nb);
	si16	*data = buf->mBuffer;

	if (debug) printf("CSoundRecord::processBufferFloat source %p -- cnt %ld\n", source, cnt);

	if (!data)
	{
		// there is some kind of allocation problem. Give up
		// HOOK TBLD should send an error message to the CMoteur
		if (debug) printf(" CSoundRecord::processBufferFloat -- no data !\n");
		return false;
	}

	// convert 44.1kHz/float/Stereo into 11kHz/16bit/Mono
	for(; nb>0; nb--)
	{
		fp32 mean = (source[0]+source[1]+source[2]+source[3]+source[4]+source[5]+source[6]+source[7])/8;
		source += 8;
		*(data++) = (si16)(mean * 32767.f);
	}

	// add data to the sound list
	mSharedSoundBuffer->mSem.acquire();
	mSharedSoundBuffer->mList.addLast(buf);
	mSharedSoundBuffer->mSem.release();

	return true;
	
} // end of processBufferFloat for CSoundRecord


//***************************************************************************
rbool CSoundRecord::processBufferShort(si16 *source, ui32 cnt)
//***************************************************************************
{
	// the function always receive 44.1kHz/16bit/Stereo data
	// thus one sample=2 bytes, and there are two samples per time frame == 4 bytes.
	// but only keep 11kHz/16bit/Mono data, ie keep the keep one sample
	// for 4 time frames, i.e. mix 8 samples into only one.
	// since cnt is in bytes and not in samples, nb final samples = cnt/16

	si32 nb = cnt>>4;
	SSoundBuffer *buf = new SSoundBuffer(nb);
	si16	*data = buf->mBuffer;

	if (debug) printf("CSoundRecord::processBufferShort source %p -- cnt %ld\n", source, cnt);

	if (!data)
	{
		// there is some kind of allocation problem. Give up
		// HOOK TBLD should send an error message to the CMoteur
		if (debug) printf(" CSoundRecord::processBufferShort -- no data !\n");
		return false;
	}

	// convert 44.1kHz/16bit/Stereo into 11kHz/16bit/Mono
	for(; nb>0; nb--)
	{
		si32 mean = (source[0]+source[1]+source[2]+source[3]+source[4]+source[5]+source[6]+source[7])/8;
		source += 8;
		*(data++) = mean;
	}

	// add data to the sound list
	mSharedSoundBuffer->mSem.acquire();
	mSharedSoundBuffer->mList.addLast(buf);
	mSharedSoundBuffer->mSem.release();

	return true;
	
} // end of processBufferShort for CSoundRecord



//---------------------------------------------------------------------------

// eoc
