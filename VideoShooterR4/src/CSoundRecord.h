/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CSoundRecord.h
	Partie	: Record son

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CSOUNDRECORD_
#define _H_CSOUNDRECORD_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RSoundRecord.h"


//---------------------------------------------------------------------------


//**************************************
class CSoundRecord : public RSoundRecord
//**************************************
{
public:
	CSoundRecord(SSharedSoundBuffer &shared, ESoundDevice device = kSoundDeviceMic)
		: RSoundRecord(shared, device) { }

//----
protected:

	virtual rbool processBufferFloat(fp32 *source, ui32 cnt);
	virtual rbool processBufferShort(si16 *source, ui32 cnt);
 
}; // end of class defs for CSoundRecord


//---------------------------------------------------------------------------

#endif // RLIB_BEOS
#endif // of _H_CSOUNDRECORD_

// eoh
