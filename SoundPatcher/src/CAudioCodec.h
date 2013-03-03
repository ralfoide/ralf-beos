/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CAudioCodec.h
	Partie	: Audio Codec (basic class + ADPCM implementation)

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CAUDIOCODEC_
#define _H_CAUDIOCODEC_

#include "gMachine.h"

#include "adpcm.h"



//---------------------------------------------------------------------------

//********************************************
class CAudioCodecInterface : public RInterface
//********************************************
{
public:

	// encode :
	// indata : sample stream (2 bytes per sample, 2 samples per stereo slot)
	// inputSampleLen : number of samples (number bytes in indata divided by 2)
	// outdata : encoded binary stream (pointer on existing buffer, or use NULL
	//					 and the buffer will be allocated (using new ui8|n]))
	// return value : number of bytes in the encoded stream

	virtual ui32 encode(si16 *indata, ui8 *outdata, ui32 inputSampleLen) = 0;

	// decode :
	// reverse arguments from encode.
	// indata : input encoded binary stream
	// inputByteLen : number of bytes in indata
	// outdata : out sample stream (2 bytes per sample, 2 samples per stereo slot)
	//					(should be a pointer on existing buffer, or use NULL
	//					 and the buffer will be allocated (using new si16|n]))
	// return value : number of samples present in outdata.
	virtual ui32 decode(ui8 *indata, si16 *outdata, ui32 inputByteLen) = 0;

	// returns a signature identifying the codec
	virtual ui32 getSignature(void) = 0;

}; // end of class defs for CAudioCodecInterface


//---------------------------------------------------------------------------


//*********************************************
class CAdpcmCodec : public CAudioCodecInterface
//*********************************************
{
public:
	CAdpcmCodec(void);
	virtual ~CAdpcmCodec(void);

	ui32	encode(si16 *indata, ui8 *outdata, ui32 inputSampleLen);
	ui32	decode(ui8 *indata, si16 *outdata, ui32 inputByteLen);

	ui32	getSignature(void) { return 'APCM'; }

//----
protected:

	struct adpcm_state	mAdpcmState;
 
}; // end of class defs for CAdpcmCodec


//---------------------------------------------------------------------------

#endif // of _H_CAUDIOCODEC_

// eoh
