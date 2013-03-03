/***********************************************************
	globals.h
***********************************************************/

#ifndef _H_GLOBALS_
#define _H_GLOBALS_

#include "PP_Be.h"
//#include "fftizer.h"

//--------------------------------------------------------

#define K_BUILD_POWERPULSAR
#undef  K_USE_DATATYPE

#ifdef __POWERPC__
	#define K_USE_PRAGMA_EXPORT
#else  // __INTEL__
	//#define K_USE_PRAGMA_EXPORT -- HOOK RM 151198 -- removed for R4b4 x86
	#undef K_USE_PRAGMA_EXPORT
#endif

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

//--------------------------------------------------------

class CPulsarApp;
extern CPulsarApp *gApplication;
extern "C" bool debug;	// in main.cpp

//--------------------------------------------------------

#define K_MSG_STREAM	'Ralf'

#define N 512

enum EErr
{
//	kNoErr = 0,
	kAllocErr,
	kCreateSemErr
};


enum // fft windows
{
	kFftWindowNone = 0,
	kFftWindowTriangle,
	kFftWindowHann,
	kFftWindowHamming,
	kFftWindowBlackman
};

//--------------------------------------------------------

extern long   gComplexFftMptr;
extern long	  gRealFftMptr;

#define K_NB_FFT_WINDOW	4

extern char *gFftWindowName[K_NB_FFT_WINDOW+1];

extern float *gComplexUtbl;	//[N/4];
extern float *gRealUtbl;	//[N/4];
extern float *gFftHigh; 	//[N*2];
extern float *gFftBass; 	//[N*2];
extern int32 *gSourceHigh; //[N];
extern int32 *gSourceBass; //[N];
extern int32 *gSrcCpyHigh; //[N];
extern int32 *gSrcCpyBass; //[N];
extern float *gFftWindow;	// NOT ALLOCATED
extern float *gFftPrecomputedWindow; //[N*2]*K_NB_FFT_WINDOW
extern bool gStreamCanContinue; // set to false to ask to stream to stop

//--------------------------------------------------------

class CInit
{
public:
	CInit(void);
	~CInit(void);
};


//--------------------------------------------------------
// prototypes

void initGlobals(void);
void freeGlobals(void);

void lock_stream(void);
void unlock_stream(void);

bool streamer_fftlib(void *arg, char *buf, size_t cnt, void *header);
void streamer_fftlib_float(float *buf, int32 count);


//---------------------------------------------------------------------------

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

//--------------------------------------------------------
#endif // of _H_GLOBALS_
// eoh
