/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CSoundPlay.cpp
	Partie	: Sound Processing

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "CSoundPlay.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"

#define DEBUG 1

#if DEBUG==1
	#define DPRINTF(s) printf s
#else
	#define DPRINTF(s)
#endif


extern bool global_hack;

//---------------------------------------------------------------------------


//********************************************************
rbool CSoundPlay::processBufferShort(si16 *dest, ui32 cnt)
//********************************************************
{
SSoundBuffer *buf;

	// play back buffers that are in the list, up to the current cnt size.
	// here, we DON'T asumme that the current media kit buffers have the
	// same size than the recorded buffers. This might depend on a variety
	// of conditions (crossplatform being the first one, but differences
	// between BeOS release or current parameters being a possibility too).

	// data is expexcted to be in the recorded buffers as mono/16 bit/11kHz
	// thus source only is 1/8 of the destination samples.
	// since cnt is in bytes and not in samples, nb source samples = cnt/16

	si16 *source=NULL;
	si32 counter = cnt>>4;

	DPRINTF(("CSoundPlay::processBufferShort source %p -- cnt %ld\n", dest, cnt));

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

		ui32 nbSample = buf->mNbSample;
		ui32 nbCopy = nbSample - buf->mNbCurrent;
		if (nbCopy > (ui32)counter) nbCopy = counter;

		source = buf->mBuffer + buf->mNbCurrent;

		// expand the data by mixing it to the current stream data
		for(ui32 i=nbCopy; i>0; i--)
		{
			si16 data = *(source++);
			for(ui32 j=8; j>0; j--)
			{
				si32 d=(*dest) + data;
				if (d<-32767) d=-32767; else if (d>32767) d=32767;
				*(dest++) = d;
			}
		}

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

} // end of processBufferShort for CSoundPlay


//*********************************************************
rbool CSoundPlay::processBufferFloat(float *dest, ui32 cnt)
//*********************************************************
{
	SSoundBuffer *buf;
	const float coef = 1.f/32768.f;

	// play back buffers that are in the list, up to the current cnt size.
	// here, we DON'T asumme that the current media kit buffers have the
	// same size than the recorded buffers. This might depend on a variety
	// of conditions (crossplatform being the first one, but differences
	// between BeOS release or current parameters being a possibility too).

	// data is expexcted to be in the recorded buffers as mono/16 bit/11kHz
	// thus source only is 1/8 of the destination samples.
	// since cnt is in bytes and not in samples, nb source samples = cnt/16

	si16 *source=NULL;
	si32 counter = cnt/sizeof(float)/8;
	float *end = (float *)(((uchar *)dest)+cnt);
	
	DPRINTF(("CSoundPlay::processBufferFloat source %p -- cnt %ld\n", dest, cnt));

	// get first buffer available, or use the last unprocessed if any
	buf = mCurrentPlayBack;
	mCurrentPlayBack = NULL;

	DPRINTF(("writing cnt %ld -- counter %ld\n", cnt, counter));
	DPRINTF(("nb bufs to process %ld\n",  mSharedSoundBuffer->mList.size()));

if(global_hack)
{
	while(counter-->0) *(dest++) = (float)rand()/(float)RAND_MAX; return true;
}

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
		if (!buf) break;

		ui32 nbSample = buf->mNbSample;
		si32 nbCopy = nbSample - buf->mNbCurrent;
		if (nbCopy > counter) nbCopy = counter;

		source = buf->mBuffer + buf->mNbCurrent;
		// expand the data by mixing it to the current stream data
		for(ui32 i=nbCopy; i>0; i--)
		{
			float data = (float)(*(source)++) * coef;
			for(ui32 j=8; j>0; j--) *(dest++) = data;
		}

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

	// fill up buffer with zeros
	for(; dest<end; ) *(dest++) = 0.;

	return true;

} // end of processBuffer for CSoundPlay


//---------------------------------------------------------------------------
#endif // RLIB_BEOS


// eoc
