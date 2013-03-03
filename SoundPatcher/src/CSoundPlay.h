/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CSoundPlay.h
	Partie	: Sound Processing

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CSOUNDPLAY_
#define _H_CSOUNDPLAY_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RSoundPlay.h"

//---------------------------------------------------------------------------


//**********************************
class CSoundPlay : public RSoundPlay
//**********************************
{
public:
	CSoundPlay(SSharedSoundBuffer &shared) : RSoundPlay(shared) { }

//----
protected:

	virtual rbool processBufferFloat(float *buf, ui32 cnt);
	virtual rbool processBufferShort(si16 *source, ui32 cnt);
	virtual rbool processBuffer(si16 *buf, ui32 cnt) { return processBufferShort(buf, cnt); }
}; // end of class defs for CSoundPlay


//---------------------------------------------------------------------------

#endif // RLIB_BEOS
#endif // of _H_CSOUNDPLAY_

// eoh
