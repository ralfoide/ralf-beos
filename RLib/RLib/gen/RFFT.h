/*****************************************************************************

	Projet	: RLib

	Fichier	:	RFFT.h
	Partie	: Sound Basis

	Auteur	: RM
	Date		: 150899
	Format	: tabs==2

	First version [15-18/8/99] :
	- no optimisation, only theoretical computations
	- use complex fp32 data
	- provides a complex DFT, a complex in-place FFT and
	  a complex not-in-place FFT.
	Many optimisations & things are to be done :
	- use a table for cos/sin
	- unroll the complex computations
	- use fp32 or si32 data arrays instead of complex
	- 4-step butterfly
	- provide a real fft (i.e input = real fp32 data, output
	  is also real fp32 data)
	- mmx2/3dnow/altivec optimisation
	  with a specific case of 2 or 3 ffts at the same time for PP.

*****************************************************************************/

#ifndef _H_RFFT_
#define _H_RFFT_

#include "machine.h"
#include "TData.h"
#include "RComplex.h"

//---------------------------------------------------------------------------


//********
class RFFT
//********
{
public:

	 						RFFT(si32 bits);
							~RFFT(void);			// class shall not be derived

	// theoric, no-optim, implementations

	void				complexDFT (TDataInterface<RComplex> &in_data,
													TDataInterface<RComplex> &out_data);
	void				complexIDFT(TDataInterface<RComplex> &in_data,
													TDataInterface<RComplex> &out_data);

	void				complexFFT (TDataInterface<RComplex> &in_out_data,
													rbool half = false);
	void				complexIFFT(TDataInterface<RComplex> &in_out_data);

	void				complexFFT (TDataInterface<RComplex> &in_data,
													TDataInterface<RComplex> &out_data);
	void				complexIFFT(TDataInterface<RComplex> &in_data,
													TDataInterface<RComplex> &out_data);

	void				realFFT    (TDataInterface<RComplex> &in_out_data);
	void				realIFFT   (TDataInterface<RComplex> &in_out_data);

	// specific optimizations
	// see source for implicit limitations & requirements

	void				fastRealFFT1(TDataInterface<fp32> &in_out_real,
													 TDataInterface<fp32> &dummy_im);

	// utility for checking

	void				bitSort(TDataInterface<si32> &in_out_data);

protected:
	void				precalcCosSin(void);
	void				precalcInterlace(TDataInterface<si32> &buffer, si32 bits);

	const si32	mBits;			// [2..30]
	const si32	mSize;			// 1<<bits
	TData<si32>	mInterlace;
	TData<si32>	mHalfInterlace;
	TData<fp32>	mCosTable;
	TData<fp32>	mSinTable;

}; // end of class defs for RFFT


//---------------------------------------------------------------------------

#endif // of _H_RFFT_

// eoh
