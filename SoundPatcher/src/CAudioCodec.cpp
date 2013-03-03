/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CAudioCodec.cpp
	Partie	: Audio Codec (basic class + ADPCM implementation)

	Auteur	: RM
	Date		: 190798
	Format	: tabs==2

*****************************************************************************/

#include "CAudioCodec.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"


//---------------------------------------------------------------------------


//***************************************************************************
CAdpcmCodec::CAdpcmCodec(void)
//***************************************************************************
{
	mAdpcmState.valprev = 0;
	mAdpcmState.index = 0;
} // end of constructor for CAdpcmCodec


//***************************************************************************
CAdpcmCodec::~CAdpcmCodec(void)
//***************************************************************************
{

} // end of destructor for CAdpcmCodec


//---------------------------------------------------------------------------


//***************************************************************************
ui32 CAdpcmCodec::encode(si16 *indata, ui8 *outdata, ui32 inputSampleLen)
//***************************************************************************
{
	// when we will store the ADPCM data, we know the compression has
	// a factor of 1/4 in bytes-size, so there will be samples/2 bytes
	// in the compressed buffer

	ui32 nbSample2 = inputSampleLen/2;
	if (!outdata)
	{
		outdata = new ui8[nbSample2];
		M_ASSERT_PTR(outdata);
	}

	adpcm_coder(indata, outdata, inputSampleLen, &mAdpcmState);

	return nbSample2;

} // end of encode for CAdpcmCodec


//***************************************************************************
ui32 CAdpcmCodec::decode(ui8 *indata, si16 *outdata, ui32 inputByteLen)
//***************************************************************************
{
	ui32 nbSample = inputByteLen*2;
	if (!outdata)
	{
		outdata = new si16[nbSample];
		M_ASSERT_PTR(outdata);
	}

	adpcm_decoder(indata, outdata, nbSample, &mAdpcmState);

	return nbSample;

} // end of decode for CAdpcmCodec



//---------------------------------------------------------------------------

// eoc
