/*****************************************************************************

	Projet	: RLib

	Fichier	:	RFFT.cpp
	Partie	: Sound Basis

	Auteur	: RM
	Date		: 150899
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"
#include "RFFT.h"
#include "gDebugPrintf.h"

#if 0
	#define CPRINTF(x) DPRINTF(x)
#else
	#define CPRINTF(x)
#endif

//---------------------------------------------------------------------------


//*******************
RFFT::RFFT(si32 bits)
		 :mBits(bits),
		  mSize(1<<bits)
//*******************
{
	M_CHECK(bits >= 3 && bits <= 30);
  mInterlace.alloc(mSize);
  mHalfInterlace.alloc(mSize>>1);
  mCosTable.alloc(mSize>>1);
  mSinTable.alloc(mSize>>1);
	precalcInterlace(mHalfInterlace, mBits-1);
	precalcInterlace(mInterlace, mBits);
	precalcCosSin();
} // end of constructor for RFFT


//***************
RFFT::~RFFT(void)
//***************
{
} // end of destructor for RFFT


//-------------------------------------------------------
#pragma mark -


//*******************************************************
void RFFT::complexDFT(TDataInterface<RComplex> &in_data,
											TDataInterface<RComplex> &out_data)
//*******************************************************
// This routine is "canonic" because it manages the data
// under the form of a Complex array of data, with no
// optimisation or specific trick. The deal is to serve
// for debugging & reference purposes.
{
	si32 i,k;
	RStAutoLock locker1(in_data);
	RStAutoLock locker2(out_data);

	M_ASSERT(in_data.size() >= mSize);
	M_ASSERT(out_data.size() >= mSize);
	const si32 n = mSize;

	for(i=0; i<n; i++) out_data[i].set(0.,0.);
	for(k=0; k<n; k++)
		for(i=0; i<n; i++)
		{
			fp64 ang = 2.*K_PI*(fp64)k*(fp64)i/(fp64)n;
			RComplex s(cos(ang), -sin(ang));
			out_data[k] += in_data[i]*s;
		}
} // end of complexDFT for RFFT


//***********************************************************
void RFFT::complexIDFT(TDataInterface<RComplex> &in_data,
											 TDataInterface<RComplex> &out_data)
//***********************************************************
// Inverse DFT, canonic implementation.
{
	si32 i;

	RStAutoLock locker1(in_data);
	RStAutoLock locker2(out_data);
	M_ASSERT(in_data.size() >= mSize);

	// first, change the sign of the imaginary part
	for(i=0; i<mSize; i++) in_data[i].im = -in_data[i].im;

	// second, do the canonical fft
	complexDFT(in_data, out_data);

	// third, compensate by dividing the time domain by mSize
	// and change back the sign of the imaginary part
	const fp32 ratio = 1./(fp32)mSize;
	for(i=0; i<mSize; i++)
	{
		out_data[i].re *= ratio;
		out_data[i].im = -(out_data[i].im * ratio);
	}
}


//-------------------------------------------------------
#pragma mark -


//**********************************************************
void RFFT::complexFFT(TDataInterface<RComplex> &in_out_data,
										  rbool half)
//**********************************************************
// This routine is "canonic" because it manages the data
// under the form of a Complex array of data, with no
// optimisation or specific trick. The deal is to serve
// for debugging & reference purposes.
{
	si32 i,j,l,a;
	RStAutoLock locker1(in_out_data);
	const si32 m = (half ? mBits-1  : mBits);
	const si32 n = (half ? mSize>>1 : mSize);

	M_ASSERT(in_out_data.size() >= n);

	// --- first swap input data ---

	// use the fact that the interleaving is only
	// a symetrical interleaved cross-swap
	{
		TData<si32> &interlace = (half ? mHalfInterlace : mInterlace);
		RStAutoLock locker2(interlace);
		j=n>>1;
		// the first half of interlace data is even,
		// swap only data as necessary
		for(i=0; i<j; i++)
		{
			a = interlace[i];
			if (a > i)
			{
				RComplex c = in_out_data[i];
				in_out_data[i] = in_out_data[a];
				in_out_data[a] = c;
			}
		}
		// the second half of interlace data is odd,
		// swap only odd components that are not equal to
		// themselve
		for(i=j+1; i<n; i+=2)
		{
			a = interlace[i];
			if (a > i)
			{
				RComplex c = in_out_data[i];
				in_out_data[i] = in_out_data[a];
				in_out_data[a] = c;
			}
		}
	}

	// --- second, do the fft ---
	// Source :
	// The Scientist and Engineer's Guide to Digital Signal Processing
	// Second Edition by Steven W. Smith, http://www.dspguide.com
	// Chapter 12, page 235.

	RStAutoLock locker3(mCosTable);
	RStAutoLock locker4(mSinTable);

	si32 le  = 2;
	si32 le2 = 1;
	si32 ie2 = mSize>>1;

	for(l=0; l<m; l++, le2=le, le<<=1, ie2>>=1)	// for each stage...
	{
		si32 ie = 0;
		for(j=0; j<le2; j++)							// for each sub DFT
		{
			//fp64 ang = K_PI*(fp64)j/(fp64)le2;
			//RComplex s(cos(ang), -sin(ang));
			RComplex s(mCosTable[ie], mSinTable[ie]);
			ie += ie2;

			for(i=j; i<n; i+=le)					// for each butterfly
			{
				// do butterfly between indexes [i] and [i+le2]
				RComplex &c1 = in_out_data[i    ];
				RComplex &c2 = in_out_data[i+le2];
				RComplex t = c2*s;
				c2  = c1-t;
				c1 += t;
			}
		}
	}
} // end of complexFFT for RFFT


//***********************************************************
void RFFT::complexIFFT(TDataInterface<RComplex> &in_out_data)
//***********************************************************
// Inverse FFT, canonic implementation.
// Source :
// The Scientist and Engineer's Guide to Digital Signal Processing
// Second Edition by Steven W. Smith, http://www.dspguide.com
// Chapter 12, page 236.
{
	si32 i;

	RStAutoLock locker1(in_out_data);
	M_ASSERT(in_out_data.size() >= mSize);

	// first, change the sign of the imaginary part
	for(i=0; i<mSize; i++) in_out_data[i].im = -in_out_data[i].im;

	// second, do the canonical fft
	complexFFT(in_out_data);

	// third, compensate by dividing the time domain by mSize
	// and change back the sign of the imaginary part
	const fp32 ratio = 1./(fp32)mSize;
	for(i=0; i<mSize; i++)
	{
		in_out_data[i].re *= ratio;
		in_out_data[i].im = -(in_out_data[i].im * ratio);
	}
}


//-------------------------------------------------------
#pragma mark -


//*******************************************************
void RFFT::complexFFT(TDataInterface<RComplex> &in_data,
											TDataInterface<RComplex> &out_data)
//*******************************************************
{
	si32 i,j,l;
	RStAutoLock locker1(in_data);
	RStAutoLock locker2(out_data);

	M_ASSERT(in_data.size() >= mSize);
	M_ASSERT(out_data.size() >= mSize);

	{
		RStAutoLock locker3(mInterlace);
		
		for(i=0; i<mSize; i++)
			out_data[mInterlace[i]] = in_data[i];
	}

	const si32 m = mBits;
	const si32 n = mSize;
	si32 le  = 2;
	si32 le2 = 1;

	for(l=0; l<m; l++, le2=le, le<<=1)	// for each stage...
	{
		for(j=0; j<le2; j++)						// for each sub DFT
		{
			fp64 ang = K_PI*(fp64)j/(fp64)le2;
			RComplex s(cos(ang), -sin(ang));

			for(i=j; i<n; i+=le)					// for each butterfly
			{
				// do butterfly between indexes [i] and [i+le2]
				RComplex &c1 = out_data[i    ];
				RComplex &c2 = out_data[i+le2];
				RComplex t = c2*s;
				c2 = c1-t;
				c1 = c1+t;
			}
		}
	}
} // end of complexFFT for RFFT


//********************************************************
void RFFT::complexIFFT(TDataInterface<RComplex> &in_data,
											 TDataInterface<RComplex> &out_data)
//********************************************************
{
	si32 i;

	RStAutoLock locker1(in_data);
	RStAutoLock locker2(out_data);
	M_ASSERT(in_data.size() >= mSize);
	M_ASSERT(out_data.size() >= mSize);

	// first, change the sign of the imaginary part
	for(i=0; i<mSize; i++) in_data[i].im = -in_data[i].im;

	// second, do the canonical fft
	complexFFT(in_data, out_data);

	// third, compensate by dividing the time domain by mSize
	// and change back the sign of the imaginary part
	const fp32 ratio = 1./(fp32)mSize;
	for(i=0; i<mSize; i++)
	{
		out_data[i].re *= ratio;
		out_data[i].im = -(out_data[i].im * ratio);
	}
}

//-------------------------------------------------------
#pragma mark -


//*******************************************************
void RFFT::realFFT(TDataInterface<RComplex> &in_out_data)
//*******************************************************
// Input : the imaginary part is ignored
// Output: real & imaginary data, upper half is not usable.
{
	si32 i,j;

	RStAutoLock locker(in_out_data);
	M_ASSERT(in_out_data.size() >= mSize);
	M_ASSERT(mBits>2);

	const si32 n  = mSize;
	const si32 n2 = mSize>>1;
	const si32 n4 = mSize>>2;

	// separate even and odd points
	for(i=0; i<n2; i++)
		in_out_data[i].set(in_out_data[2*i].re, in_out_data[2*i+1].re);

	// calculate complex FFT on lower half
	complexFFT(in_out_data, true);

	// even/odd frequency domain decomposition
	for(i=1; i<n4; i++)
	{
		const si32 im = n2-i;
		const si32 ip2= n2+i;
		const si32 ipm= n -i;

		RComplex c( (in_out_data[i].im + in_out_data[im].im)/2,
							 -(in_out_data[i].re + in_out_data[im].re)/2);

		in_out_data[ip2] = c;
		in_out_data[ipm].set(c.re, -c.im);

		c.set((in_out_data[i].re + in_out_data[im].re)/2,
				  (in_out_data[i].im - in_out_data[im].im)/2);

		in_out_data[i ] = c;
		in_out_data[im].set(c.re, -c.im);
	}
	
	in_out_data[3*n4].set(in_out_data[n4].re, 0.);
	in_out_data[  n2].set(in_out_data[ 0].re, 0.);
	in_out_data[  n4].im = 0.;
	in_out_data[   0].im = 0.;

	// last FFT stage
	const si32 le  = mSize;
	const si32 le2 = le>>1;

	for(j=0; j<le2; j++)							// for each sub DFT
	{
		fp64 ang = K_PI*(fp64)j/(fp64)le2;
		RComplex s(cos(ang), -sin(ang));

		for(i=j; i<n; i+=le)					// for each butterfly
		{
			// do butterfly between indexes [i] and [i+le2]
			RComplex &c1 = in_out_data[i    ];
			RComplex &c2 = in_out_data[i+le2];
			RComplex t = c2*s;
			c2  = c1-t;
			c1 += t;
		}
	}
}



//********************************************************
void RFFT::realIFFT(TDataInterface<RComplex> &in_out_data)
//********************************************************
// Input : the real & imaginary part from 0 to size()/2-1
//         are used only. Upper half is ignored.
// Output: real data only, imaginary set to 0.
{
	si32 i;

	RStAutoLock locker(in_out_data);
	M_ASSERT(in_out_data.size() >= mSize);

	// force frequency domain to be symmetrical
	for(i=mSize/2+1; i<mSize; i++)
		in_out_data[i].set(in_out_data[mSize-i].re, -in_out_data[mSize-i].im);

	// add real & imaginary parts together
	for(i=0; i<mSize; i++)
		in_out_data[i].re += in_out_data[i].im;

	// do real fft on data
	realFFT(in_out_data);

	// transform back to real data
	// set imaginary data to zero
	const fp32 coef = 1./(fp32)mSize;
	for(i=0; i<mSize; i++)
		in_out_data[i].set((in_out_data[i].re + in_out_data[i].im)*mSize, 0.);

}


//---------------------------------------------------------------------------



//********************************************************
void RFFT::fastRealFFT1(TDataInterface<fp32> &in_out_real,
												TDataInterface<fp32> &dummy_im)
//********************************************************
// Compute a real FFT using the canonical complex FFT.
// Input :
//	- real data in array in_out_real.
//	- dummy imaginary array for temporary use,
// Output :
//	- module of complex fft result in the real array
//
// Level of optimization : very mild :-p
{
	si32 i,j,l,a;
	RStAutoLock locker1(in_out_real);
	RStAutoLock locker2(dummy_im);
	RStAutoLock locker3(mCosTable);
	RStAutoLock locker4(mSinTable);
	RStAutoLock locker5(mInterlace);

	const si32 m = mBits;
	const si32 n = mSize;

	M_ASSERT(in_out_real.size() >= n);

	fp32	*ptr_re  = in_out_real.data();
	fp32	*ptr_im  = dummy_im.data();
	fp32	*ptr_cos = mCosTable.data();
	fp32	*ptr_sin = mSinTable.data();
	si32	*interlace = mInterlace.data();

	// --- first swap input data ---

	// use the fact that the interleaving is only
	// a symetrical interleaved cross-swap
	{
		j=n>>1;
		// the first half of interlace data is even,
		// swap only data as necessary
		for(i=0; i<j; i++)
		{
			a = interlace[i];
			ptr_im[i] = 0.;
			if (a > i)
			{
				fp32 sw = ptr_re[i];
				ptr_re[i] = ptr_re[a];
				ptr_re[a] = sw;
			}
		}
		// the second half of interlace data is odd,
		// swap only odd components that are not equal to
		// themselve
		for(i=j+1; i<n; i+=2)
		{
			a = interlace[i];
			ptr_im[i-1] = 0.;
			ptr_im[i  ] = 0.;
			if (a > i)
			{
				fp32 sw = ptr_re[i];
				ptr_re[i] = ptr_re[a];
				ptr_re[a] = sw;
			}
		}
	}

	// --- second, do the fft ---
	// Source :
	// The Scientist and Engineer's Guide to Digital Signal Processing
	// Second Edition by Steven W. Smith, http://www.dspguide.com
	// Chapter 12, page 235.


	si32 le  = 2;
	si32 le2 = 1;
	si32 ie2 = mSize>>1;

	for(l=0; l<m; l++, le2=le, le<<=1, ie2>>=1)	// for each stage...
	{
		si32 ie = 0;
		for(j=0; j<le2; j++)							// for each sub DFT
		{
			fp32 scos = ptr_cos[ie];
			fp32 ssin = ptr_sin[ie];
			ie += ie2;

			for(i=j; i<n; i+=le)					// for each butterfly
			{
				// do butterfly between indexes [i] and [i+le2]
				si32 i2 = i+le2;
				fp32 cr = ptr_re[i];				// c1 = in_out_data[i    ];
				fp32 ci = ptr_im[i];
				fp32 tt = ptr_re[i2];				// c2 = in_out_data[i+le2];
				fp32 tr, ti = ptr_im[i2];
				tr = tt * scos - ti * ssin;	// t = c2*s;
				ti = tt * ssin + ti * scos;
				ptr_re[i2] = cr-tr;					// c2  = c1-t;
				ptr_im[i2] = ci-ti;
				ptr_re[i]  = cr+tr;					// c1 += t;
				ptr_im[i]  = ci+ti;
			}
		}
	}

	// now compute module of resulting values and put it into real data
	for(i=0; i<n; i++)
	{
		fp32 re = *ptr_re;
		fp32 im = *(ptr_im++);
		*(ptr_re++) = sqrt(re*re+im*im);
	}

} // end of fastRealFFT1 for RFFT


//---------------------------------------------------------------------------


//***************************************************
void RFFT::bitSort(TDataInterface<si32> &in_out_data)
//***************************************************
// Test routine
{
	si32 i,j,a,c;
	RStAutoLock locker1(in_out_data);
	RStAutoLock locker2(mInterlace);
	M_ASSERT(in_out_data.size() >= mSize);

	// use the fact that the interleaving is only
	// a symetrical interleaved cross-swap
	j=mSize>>1;
	// the first half of interlace data is even,
	// swap only data as necessary
	for(i=0; i<j; i++)
	{
		a = mInterlace[i];
		if (a>i)
		{
			c = in_out_data[i];
			in_out_data[i] = in_out_data[a];
			in_out_data[a] = c;
		}
	}
	// the second half of interlace data is odd,
	// swap only odd components that are not equal to
	// themselve
	for(i=j+1; i<mSize; i+=2)
	{
		a = mInterlace[i];
		if (a>i)
		{
			c = in_out_data[i];
			in_out_data[i] = in_out_data[a];
			in_out_data[a] = c;
		}
	}
}



//******************************************************************
void RFFT::precalcInterlace(TDataInterface<si32> &buffer, si32 bits)
//******************************************************************
{
	RStAutoLock locker(buffer);
	si32 a,i,j,f;

	// Le filtrage numerique / Beaufils & Rami / Sybex / p.131

	#if 0	// -- fast version
		si32 *p = buffer.data();
		*p = 0;
		for(i=0, f=1; i<bits; i++, f<<=1)
		{
			si32 *p1 = p;
			si32 *p2 = p+f;
			for(j=0; j<f; j++)
			{
				a = (*p1) << 1;
				*(p1++) = a;
				*(p2++) = a+1;
			}
		}
	#else	// -- secure version
		si32 size = 1<<bits;
		for(i=0; i<size; i++) buffer[i]=0;
		for(i=0, f=1; i<bits; i++, f<<=1)
			for(j=0; j<f; j++)
			{
				a = buffer[j] << 1;
				buffer[j] = a;
				buffer[j+f] = a+1;
			}
	#endif

} // end of precalcInterlace for RFFT


//****************************
void RFFT::precalcCosSin(void)
//****************************
{
	RStAutoLock locker1(mCosTable);
	RStAutoLock locker2(mSinTable);

	si32 le2 = mSize>>1;
	fp32 coef = K_PI/(fp32)le2;
	for(si32 i=0; i<le2; i++)
	{
		fp32 ang = coef*(fp32)i;
		mCosTable[i] = cos(ang);
		mSinTable[i] = sin(ang);
	}

} // end of precalcInterlace for RFFT


//---------------------------------------------------------------------------

// eoc
