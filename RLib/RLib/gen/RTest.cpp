/*****************************************************************************

	Projet	: RLib

	Fichier	: RTest.cpp
	Partie	: lib basics

	Auteur	: RM
	Date		: 010899
	Format	: tabs==2

	Set of specific test routines for basis service classes

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_TEST_ROUTINES

#include "gDebugPrintf.h"

#include "RTest.h"
#include "TList.h"
#include "TData.h"
#include "TRArray.h"
#include "TTrippleBuffer.h"
#include "RAtomic.h"
#include "RFFT.h"

#include <stdlib.h>	// for srand(), rand()
#include <time.h>		// for time

//------------------------------------------------------------
// prototypes

static void test_TData(void);
static void test_TList(void);
static void test_TRArray(void);
static void test_TripBuf(void);
static void test_RAtomic(void);
static void test_complex_RFFT(void);
static void test_real_RFFT(void);
static void test_RFFT_timing(void);

//------------------------------------------------------------


//******************
void rlib_test(void)
//******************
{
	DPRINTF(("BEGIN rlib_test\n"));

	// basic data handling

	if (0)
	{
		test_TData();
		test_TList();
		test_TRArray();
		test_TripBuf();
		test_RAtomic();
	}

	// more complex stuff

	if(0)
	{
		if (0) test_complex_RFFT();
		if (0) test_real_RFFT();
		if (1) test_RFFT_timing();
	}

	DPRINTF(("\nEND rlib_test\n"));
}

//------------------------------------------------------------


//**************
class CTestClass
//**************
{
public:
	CTestClass(void)
		: mAtom(mAtomBase++)
		{ DPRINTF(("  CTestClass\n\tconstructor %p\n\tatom %08x\n", this, mAtom));
			mAtomBase *= 0x01010101;
		}
	~CTestClass(void)
		{ DPRINTF(("  CTestClass\n\tdestructor %p\n\tatom %08x\n", this, mAtom)); }
	void dump(void)
		{ DPRINTF(("  CTestClass - dump %p - atom %08x\n", this, mAtom)); }

protected:
	si32 mAtom;
	static si32 mAtomBase;
};

si32 CTestClass::mAtomBase = 0;


//------------------------------------------------------------


//*******************
void test_TData(void)
//*******************
{
	DPRINTF(("\ntest TData - BEGIN\n\n"));

	TData<char> data0;
	data0.alloc(16);
	data0.lockThrow();
		DPRINTF(("TData data0/1a : data %p - size %d\n", data0.data(), data0.size()));
		data0.unlock();
	data0.free();

	data0.alloc(16, kDataAllocNew);
	data0.lockThrow();
		DPRINTF(("TData data0/2b : data %p - size %d\n", data0.data(), data0.size()));
		data0.unlock();
	data0.free();
	#ifdef RLIB_MACOS
		data0.alloc(16, kDataAllocMacSys);
		data0.lockThrow();
			DPRINTF(("TData data0/2c : data %p - size %d\n", data0.data(), data0.size()));
			data0.unlock();
		data0.free();

		data0.alloc(16, kDataAllocMacTemp);
		data0.lockThrow();
			DPRINTF(("TData data0/2d : data %p - size %d\n", data0.data(), data0.size()));
			data0.unlock();
		data0.free();

		data0.alloc(16*1024*1024);
		data0.lockThrow();
			DPRINTF(("TData data0/3e : data %p - size %d\n", data0.data(), data0.size()));
			data0.unlock();
		data0.free();
	#endif

	if(1)
	{
		TData<CTestClass> data1;
		data1.alloc(3);

		data1.lockThrow();
			DPRINTF(("TData data1/3f : data %p - size %d\n", data1.data(), data1.size()));
			data1[0].dump();
			data1[1].dump();
			data1[2].dump();
		data1.unlock();
	}

	DPRINTF(("\ntest TData - END\n\n"));
}


//*********************
void test_TRArray(void)
//*********************
{
	DPRINTF(("\ntest TRArray - BEGIN\n\n"));

	CTestClass reference;
	DPRINTF(("reference:\n"));
	reference.dump();
	TRArray<CTestClass> array1(0, 2, true);
	DPRINTF(("TRArray/1a : size %d\n", array1.size()));
	array1.append(reference);
	DPRINTF(("TRArray/1a1: size %d\n", array1.size()));
	array1.append(reference);
	DPRINTF(("TRArray/1a2: size %d\n", array1.size()));
	array1.append(reference);
	DPRINTF(("TRArray/1a3: size %d\n", array1.size()));
	array1[0].dump();
	array1[1].dump();
	array1[2].dump();
	DPRINTF(("TRArray/1b : size %d -- ptr[0] %p\n", array1.size(), &(array1[0])));
	array1.remove(0);
	array1.remove(0);
	array1.remove(0);
	array1.free();
	DPRINTF(("TRArray/1c : size %d\n", array1.size()));

	TRArray<CTestClass> array2(3, 0, false);
	DPRINTF(("TRArray/2d : size %d -- ptr[0] %p\n", array2.size(), &(array2[0])));
	array2[0] = reference;
	array2[1] = reference;
	array2[2] = reference;
	array2[0].dump();
	array2[1].dump();
	array2[2].dump();
	DPRINTF(("TRArray/2e : size %d -- ptr[0] %p\n", array2.size(), &(array2[0])));
	array2.remove(1);
	DPRINTF(("TRArray/2f : size %d -- ptr[0] %p\n", array2.size(), &(array2[0])));
	array2[0].dump();
	array2[1].dump();

	DPRINTF(("\ntest TRArray - END\n\n"));
}


//*******************
void test_TList(void)
//*******************
{
	DPRINTF(("\ntest TList\n"));
}


//*********************
void test_TripBuf(void)
//*********************
{
	DPRINTF(("\ntest test_TripBuf - BEGIN\n\n"));

	TTrippleBuffer<TData<si16>, si16> trip_buf(4096);
	//TTrippleBuffer<TData<si16>, si16>::ETripBufIndex index;
	si16 *i;
	i = trip_buf.dataRead();
	i = trip_buf.dataWrite();
	trip_buf.swapRead();
	trip_buf.swapWrite();
	i = trip_buf.dataPtr(TTrippleBuffer<TData<si16>, si16>::kTripBufRead);
	i = trip_buf.dataPtr(TTrippleBuffer<TData<si16>, si16>::kTripBufPending);
	i = trip_buf.dataPtr(TTrippleBuffer<TData<si16>, si16>::kTripBufWrite);
	const TData<si16> & buf = trip_buf.buffer(TTrippleBuffer<TData<si16>, si16>::kTripBufRead);
	bool status = buf.isLocked();
	
	trip_buf.free();

	TTrippleBuffer<TData<si16>, si16> tribuf2;
	tribuf2.alloc(16);

	DPRINTF(("\ntest test_TripBuf - END\n\n"));
}


//*********************
void test_RAtomic(void)
//*********************
{
	// this test doesn't check the "atomicity" of the operation,
	// rather that the operation gives the expected numerical result.
	// We have to trust the implementation of atomic(), otherwise the
	// test is too much specific...
	DPRINTF(("\ntest test_RAtomic - BEGIN\n\n"));

	RAtomic value(5);
	DPRINTF(("Atomic - init value : %d\n", (si32)value));
	value.inc();
	DPRINTF(("Atomic - value.inc : %d\n", (si32)value));
	value.dec();
	value.dec();
	DPRINTF(("Atomic - value.dec*2 : %d\n", (si32)value));

	DPRINTF(("\ntest test_RAtomic - END\n\n"));
}


//**************************
void test_complex_RFFT(void)
//**************************
{
	DPRINTF(("\ntest test_complex_RFFT - BEGIN\n\n"));

	const fp32 kEpsilon = 1e-5;
	bool result;
	si32 i,j;

	// do FFT from 3 to 10 bits (8 to 1024 values)
	for(si32 N=3; N<=6; N++)
	{
		const si32 M = 1<<N;

		DPRINTF(("\n\n######## FFT %d BITS --- %d VALUES #########\n\n", N, M));

		TData<si32>			bits(M);
		TData<RComplex> data(M);
		TData<RComplex> compare(M);
		TData<RComplex> dft(M);
		TData<RComplex> idft(M);
		TData<fp32> 		real(M);
		TData<fp32> 		dummy(M);
		RFFT fft(N);

		// first check the bit sorting
		// reverse twice the array -- the result should be the original array
		RStAutoLock locker(bits);
		for(i=0; i<M; i++) bits[i] = i;

		fft.bitSort(bits);

		DPRINTF(("Bit Reversed Array :\n"));
		for(i=0; i<M && i<80; i++)
			printf("%4d  %s", bits[i], ((i%8==7 || i==M-1) ? "\n" : ""));

		fft.bitSort(bits);

		result = true;
		for(i=0; i<M && result; i++) result = (bits[i] == i);
		if (result)
			DPRINTF(("Bit Sorting is OK\n\n"));
		else
			DPRINTF(("Bit Sorting INVALID at index %d\n\n", i));
		if (!result) goto abort;

		// three times the same test (FFT then IFFT and compare)
		// on three kind of data : zeroes, random, imaginary=0.
		// For random, always use same set for testing.
		// Data is explicilty unlocked during FFT calls, to certify
		// that the FFT locks as needed.

		RStAutoLock locker0(compare);
		srand(0);
		for(j=0; j<4; j++)
		{
			DPRINTF(("**** Test Data Set #%d *****\n\n", j));
			{
				RStAutoLock locker1(data);
				if(j==0)
					for(i=0; i<M; i++)
						data[i].set(0., 0.);
				else if(j==1)
					for(i=0; i<M; i++)
						data[i].set((fp32)i/(fp32)M, 0.);
				else if(j==2)
					for(i=0; i<M; i++)
						data[i].set(((fp32)(rand()%65535))/32768.-1.,
											  ((fp32)(rand()%65535))/32768.-1.);
				else if(j==3)
					for(i=0; i<M; i++)
						data[i].set(1.0, (i>=M/2 ? 1.0 : 0.0));
						//data[i].set((fp32)(rand()%i), (fp32)(rand()%(M-i)));

				DPRINTF(("Data Before the FFT :\n"));
				for(i=0; i<M; i++)
				{
					compare[i] = data[i];	// keep comparison data
					printf("<% 1.4f, % 1.4f> ", data[i].re, data[i].im);
					if (i%4 == 3 || i == M-1) printf("\n");
				}
			}

			fft.complexDFT(data, dft);

			{
				DPRINTF(("\nData After the Complex DFT :\n"));
				RStAutoLock locker5(dft);
				for(i=0; i<M; i++)
				{
					printf("<% 1.4f, % 1.4f> ", dft[i].re, dft[i].im);
					if (i%4 == 3 || i == M-1) printf("\n");
				}
			}

			fft.complexIDFT(dft, idft);

			{
				DPRINTF(("Data After the Complex IDFT :\n"));
				RStAutoLock locker6(idft);
				for(i=0; i<M; i++)
				{
					printf("<% 1.4f, % 1.4f> ", idft[i].re, idft[i].im);
					if (i%4 == 3 || i == M-1) printf("\n");
				}
			}

			fft.complexFFT(data, dft);
			
			{
				DPRINTF(("\nData After the FFT-2 :\n"));
				RStAutoLock locker2(dft);
				for(i=0; i<M; i++)
				{
					printf("<% 1.4f, % 1.4f> ", dft[i].re, dft[i].im);
					if (i%4 == 3 || i == M-1) printf("\n");
				}
			}

			fft.complexIFFT(dft, idft);
			
			{
				DPRINTF(("Data After the IFFT-2 :\n"));
				RStAutoLock locker3(idft);
				for(i=0; i<M; i++)
				{
					printf("<% 1.4f, % 1.4f> ", idft[i].re, idft[i].im);
					if (i%4 == 3 || i == M-1) printf("\n");
				}
			}

			fft.complexFFT(data);
			
			{
				DPRINTF(("\nData After the FFT :\n"));
				RStAutoLock locker2(data);
				for(i=0; i<M; i++)
				{
					printf("<% 1.4f, % 1.4f> ", data[i].re, data[i].im);
					if (i%4 == 3 || i == M-1) printf("\n");
				}
			}

			fft.complexIFFT(data);
			
			{
				DPRINTF(("Data After the IFFT :\n"));
				RStAutoLock locker3(data);
				for(i=0; i<M; i++)
				{
					printf("<% 1.4f, % 1.4f> ", data[i].re, data[i].im);
					if (i%4 == 3 || i == M-1) printf("\n");
				}
			}

			if (j < 2)	// need imaginary==0 for a "real fft"
			{
				{
					RStAutoLock locker7a(real);
					RStAutoLock locker7b(compare);
					for(i=0; i<M; i++) real[i] = compare[i].re;
				}

				DPRINTF(("\n\n"));

				fft.fastRealFFT1(real, dummy);

				{
					DPRINTF(("Data After the Fast Real FFT 1 [3] :\n"));
					RStAutoLock locker8a(real);
					for(i=0; i<M; i++)
						printf("% 1.4f %s", real[i], (i%8 == 7 || i == M-1) ? "\n" : "");
				}
			}

			DPRINTF(("\n\n"));

			// compare result data
			{
				RStAutoLock locker4a(data);
				result = true;
				for(i=0; i<M && result; i++)
					result = (fabs(data[i].re-compare[i].re) <= kEpsilon)
								&& (fabs(data[i].im-compare[i].im) <= kEpsilon);
				if (result)
					DPRINTF(("Comparison : CORRECT\n"));
				else
					DPRINTF(("Comparison : INVALID at index %d\n%g  <--> %g =>  %g\n%g  <--> %g =>  %g\n",
						i, data[i].re, compare[i].re, fabs(data[i].re-compare[i].re),
						data[i].im, compare[i].im, fabs(data[i].im-compare[i].im)));
				if (!result) goto abort;
			}

			{
				RStAutoLock locker4b(idft);
				result = true;
				for(i=0; i<M && result; i++)
					result = (fabs(idft[i].re-compare[i].re) <= kEpsilon)
								&& (fabs(idft[i].im-compare[i].im) <= kEpsilon);
				if (result)
					DPRINTF(("Comparison2: CORRECT\n"));
				else
					DPRINTF(("Comparison2 : INVALID at index %d\n%g  <--> %g =>  %g\n%g  <--> %g =>  %g\n",
						i, idft[i].re, compare[i].re, fabs(idft[i].re-compare[i].re),
						idft[i].im, compare[i].im, fabs(idft[i].im-compare[i].im)));
				if (!result) goto abort;
			}

			if (j < 2)	// need imaginary==0 for a "real fft"
			{
				RStAutoLock locker8a(real);
				for(i=0; i<M && result; i++)
					result = (fabs(real[i]-(!dft[i])) <= kEpsilon);
				if (result)
					DPRINTF(("Comparison3: CORRECT\n"));
				else
					DPRINTF(("Comparison3 : INVALID at index %d\n%g  <--> %g =>  %g\n",
						i, real[i], !dft[i], fabs(real[i]-(!dft[i]))));
				if (!result) goto abort;
			}

			DPRINTF(("\n\n"));
		}
	}

abort:

	DPRINTF(("\ntest test_complex_RFFT - END\n\n"));
}


//***********************
void test_real_RFFT(void)
//***********************
{
	DPRINTF(("\ntest test_real_RFFT - BEGIN\n\n"));

	si32 i;
	bool result;

	// do FFT from 2 to 10 bits (4 to 1024 values)
	for(si32 N=4; N<=4; N++)
	{
		const si32 M = 1<<N;

		DPRINTF(("\n\n######## FFT %d BITS --- %d VALUES #########\n\n", N, M));

		TData<RComplex> source(M);
		TData<RComplex> data(M);
		TData<RComplex> compare(M);
		RFFT fft(N);

		// create a set of real data (im = 0)
		{
			RStAutoLock locker1(data);
			RStAutoLock locker2(source);
			for(i=0; i<M; i++)
			{
				data[i].set((fp32)i/(fp32)M, 0.);
				source[i] = data[i];
			}
		}

		// do the out-of-place FFT onto this, then do the in-palce fft with data
		// compare will be used later to check result from real in-place fft
		fft.complexFFT(data, compare);
		fft.realFFT(data);

		// dump source vs. complex vs. real fft data
		{
			RStAutoLock locker1(source);
			RStAutoLock locker2(data);
			RStAutoLock locker3(compare);

			DPRINTF(("\nData Before The FFT :\n"));
			for(i=0; i<M; i++)
				printf("<% 1.4f, % 1.4f> %s", source[i].re, source[i].im, (i%4 == 3 || i == M-1) ? "\n" : "");

			DPRINTF(("\nData After The COMPLEX FFT :\n"));
			for(i=0; i<M; i++)
				printf("<% 1.4f, % 1.4f> %s", compare[i].re, compare[i].im, (i%4 == 3 || i == M-1) ? "\n" : "");

			DPRINTF(("\nData After The REAL FFT :\n"));
			for(i=0; i<M; i++)
				printf("<% 1.4f, % 1.4f> %s", data[i].re, data[i].im, (i%4 == 3 || i == M-1) ? "\n" : "");

			DPRINTF(("\n"));
			result = true;
			for(i=0; i<M && result; i++)
				result = (fabs(data[i].re-compare[i].re) <= K_EPSILON)
							&& (fabs(data[i].im-compare[i].im) <= K_EPSILON);
			if (result)
				DPRINTF(("Comparison : CORRECT\n"));
			else
				DPRINTF(("Comparison : INVALID at index %d\n%g  <--> %g =>  %g\n%g  <--> %g =>  %g\n",
					i, data[i].re, compare[i].re, fabs(data[i].re-compare[i].re),
					data[i].im, compare[i].im, fabs(data[i].im-compare[i].im)));
			if (!result) goto abort;
		}

		// now do the in-place real IFFT
		fft.realIFFT(data);

		// dump & check
		{
			RStAutoLock locker1(source);
			RStAutoLock locker2(data);

			DPRINTF(("\nData After The REAL IFFT :\n"));
			for(i=0; i<M; i++)
				printf("<% 1.4f, % 1.4f> %s", data[i].re, data[i].im, (i%4 == 3 || i == M-1) ? "\n" : "");

			DPRINTF(("\n"));
			result = true;
			for(i=0; i<M && result; i++)
				result = (fabs(data[i].re-source[i].re) <= K_EPSILON)
							&& (fabs(data[i].im-source[i].im) <= K_EPSILON);
			if (result)
				DPRINTF(("Comparison : CORRECT\n"));
			else
				DPRINTF(("Comparison : INVALID at index %d\n%g  <--> %g =>  %g\n%g  <--> %g =>  %g\n",
					i, data[i].re, source[i].re, fabs(data[i].re-source[i].re),
					data[i].im, source[i].im, fabs(data[i].im-source[i].im)));
			if (!result) goto abort;
		}
	}

abort:

	DPRINTF(("\ntest test_real_RFFT - END\n\n"));
}



//*************************
void test_RFFT_timing(void)
//*************************
{
	si32 i, tick1, tick2, tick3, tick4, tick5, total, totalf;
	DPRINTF(("\ntest test_RFFT_timing - BEGIN\n\n"));


	if (1)
	{
		const si32 K=60000000;

		tick1 = ::TickCount();		
		srand(0);
		fp32 a = ((fp32)(rand()%65535))/32768.-1.;
		fp32 za = (fp32)0.;
		fp32 a1 = (fp32)1.;
		fp32 ae = (fp32)K_EPSILON;
		for(i=0; i<K; i++)
		{
			a *= a;
			a += a;
			if (a < za) a = za - a;
			if (a < ae) a = a1;
			a /= a;
		}

		tick2 = ::TickCount();
		srand(0);
		fp64 b = ((fp64)(rand()%65535))/32768.-1.;
		fp64 zb = (fp64)0.;
		fp64 b1 = (fp64)1.;
		fp64 be = (fp64)K_EPSILON;
		for(i=0; i<K; i++)
		{
			b *= b;
			b += b;
			if (b < zb) b = zb - b;
			if (b < be) b = b1;
			b /= b;
		}

		tick3 = ::TickCount();

		fp32 coef= 1e6/60./(fp32)K;
		fp32 t32 = (fp32)(tick2-tick1)*coef;
		fp32 t64 = (fp32)(tick3-tick2)*coef;

		srand(a);	// just to actually use the values and fool the compiler
		srand(b);
		
		DPRINTF(("Timing :\n\tfp32 ops : %3.3f us\n\tfp64 ops : %3.3f us\n", t32, t64));
		DPRINTF(("Total ticks : [32] %d -- [64] %d --> %3.3f %%\n", tick2-tick1, tick3-tick2, (t64/t32*100.)));
		return;
	}

	// use 10-bits FFTs (1024 pts)
	const si32 N=10;
	const si32 M = 1<<N;
	const si32 K=400;

	RFFT fft(N);
	TData<RComplex> data(M), result(M);
	TData<fp32> real(M), dummy(M);
	RStAutoLock locker1(data);
	RStAutoLock locker2(result);
	RStAutoLock locker3(real);
	RStAutoLock locker4(dummy);

	// get a random data set
	srand(time(NULL));
	for(i=0; i<M; i++)
	{
		data[i].set(((fp32)(rand()%65535))/32768.-1.,
							  ((fp32)(rand()%65535))/32768.-1.);
		real[i] = data[i].re;
	}


	{
		tick1 = ::TickCount();
		if(1) fft.complexDFT(data, result);								// only once
	}
	{
		tick2 = ::TickCount();
		for(i=0; i<K; i++) fft.complexFFT(data, result);	// K times
	}
	{
		tick3 = ::TickCount();
		for(i=0; i<K; i++) fft.complexFFT(data);					// K times
	}
	{
		tick4 = ::TickCount();
		for(i=0; i<K; i++) fft.fastRealFFT1(real, dummy);	// K times
	}
	tick5 = ::TickCount();

	totalf= tick5-tick2;
	total = tick5-tick1;

	fp32 coef= 1000./60./(fp32)K;
	fp32 ms1 = (fp32)(tick2-tick1)*1000./60.;
	fp32 ms2 = (fp32)(tick3-tick2)*coef;
	fp32 ms3 = (fp32)(tick4-tick3)*coef;
	fp32 ms4 = (fp32)(tick5-tick4)*coef;
	

	// timing is in (very not precise) tick counts.
	// [needs to be changed by a custom millisecond counter class]
	// ticks = 60*seconds, ticks->ms = T/60*1000
	DPRINTF(("Total time elapsed: %3.3f ms\n", (fp32)total*coef));
	DPRINTF(("Total FFT  elapsed: %3.3f ms\n", (fp32)totalf*coef));
	DPRINTF(("Average timings:\n"
					 "DFT  : %3.3f ms\n"
					 "FFT 2: %3.3f ms\n"
					 "FFT 1: %3.3f ms\n"
					 "Fast1: %3.3f ms\n",
					 ms1,ms2,ms3,ms4));

	DPRINTF(("\ntest test_RFFT_timing - END\n\n"));
}


/*-------------------------------------------------------------*/
/*

Expected result:
----------------

Welcome to AnaSon!
Debug Build Aug  2 1999, 00:49:50

BEGIN rlib_test

test TData - BEGIN

TData data0/1a : data 0x03bda4c8 - size 16
TData data0/2b : data 0x03bda4c8 - size 16
TData data0/2c : data 0x00578760 - size 16
TData data0/2d : data 0x05bc9eb0 - size 16
TData data0/3e : data 0x02ba9230 - size 16777216
  CTestClass
    constructor 0x03bda4cc
    atom 00000000
  CTestClass
    constructor 0x03bda4d0
    atom 01010101
  CTestClass
    constructor 0x03bda4d4
    atom 05040302
TData data1/3f : data 0x03bda4cc - size 3
  CTestClass - dump 0x03bda4cc - atom 00000000
  CTestClass - dump 0x03bda4d0 - atom 01010101
  CTestClass - dump 0x03bda4d4 - atom 05040302
  CTestClass
    destructor 0x03bda4d4
    atom 05040302
  CTestClass
    destructor 0x03bda4d0
    atom 01010101
  CTestClass
    destructor 0x03bda4cc
    atom 00000000

test TData - END


test TList

test TRArray - BEGIN

  CTestClass
    constructor 0x047875f8
    atom 0f0a0603
reference:
  CTestClass - dump 0x047875f8 - atom 0f0a0603
TRArray/1a : size 0
  CTestClass
    constructor 0x03bda4d0
    atom 23140a04
  CTestClass
    constructor 0x03bda4d4
    atom 46230f05
TRArray/1a1: size 1
TRArray/1a2: size 2
  CTestClass
    constructor 0x03bda4b4
    atom 7e381506
  CTestClass
    constructor 0x03bda4b8
    atom d2541c07
  CTestClass
    constructor 0x03bda4bc
    atom 4a782408
  CTestClass
    constructor 0x03bda4c0
    atom efa52d09
  CTestClass
    destructor 0x03bda4d4
    atom 0f0a0603
  CTestClass
    destructor 0x03bda4d0
    atom 0f0a0603
TRArray/1a3: size 3
  CTestClass - dump 0x03bda4b4 - atom 0f0a0603
  CTestClass - dump 0x03bda4b8 - atom 0f0a0603
  CTestClass - dump 0x03bda4bc - atom 0f0a0603
TRArray/1b : size 3 -- ptr[0] 0x03bda4b4
  CTestClass
    destructor 0x03bda4c0
    atom efa52d09
  CTestClass
    destructor 0x03bda4bc
    atom 0f0a0603
  CTestClass
    destructor 0x03bda4b8
    atom 0f0a0603
  CTestClass
    destructor 0x03bda4b4
    atom 0f0a0603
TRArray/1c : size 0
  CTestClass
    constructor 0x03bda4cc
    atom cbdc370a
  CTestClass
    constructor 0x03bda4d0
    atom ea1e420b
  CTestClass
    constructor 0x03bda4d4
    atom 566c4e0c
TRArray/2d : size 3 -- ptr[0] 0x03bda4cc
  CTestClass - dump 0x03bda4cc - atom 0f0a0603
  CTestClass - dump 0x03bda4d0 - atom 0f0a0603
  CTestClass - dump 0x03bda4d4 - atom 0f0a0603
TRArray/2e : size 3 -- ptr[0] 0x03bda4cc
  CTestClass
    constructor 0x04787578
    atom 1dc75b0d
  CTestClass
    destructor 0x04787578
    atom 1dc75b0d
TRArray/2f : size 2 -- ptr[0] 0x03bda4cc
  CTestClass - dump 0x03bda4cc - atom 0f0a0603
  CTestClass - dump 0x03bda4d0 - atom 0f0a0603

test TRArray - END

[destruct array2 of 3 items]
  CTestClass
    destructor 0x03bda4d4
    atom 1dc75b0d
  CTestClass
    destructor 0x03bda4d0
    atom 0f0a0603
  CTestClass
    destructor 0x03bda4cc
    atom 0f0a0603
[destruct reference]
  CTestClass
    destructor 0x047875f8
    atom 0f0a0603

END rlib_test



/*
/*-------------------------------------------------------------*/

#endif // of RLIB_TEST_ROUTINES

// eoc


