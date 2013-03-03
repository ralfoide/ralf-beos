/***********************************************************
	globals.cpp
***********************************************************/

#include "globals.h"
#include "fftlib.h"
#include "fastmath.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

//--------------------------------------------------------

long	gComplexFftMptr = 0;
long	gRealFftMptr = 0;

float	*gComplexUtbl = NULL; //[N/4];
float	*gRealUtbl = NULL; //[N/4];
float *gFftHigh = NULL; //[N*2];
float *gFftBass = NULL; //[N*2];
float *gFftWindow = NULL;	// NOT ALLOCATED
float *gFftPrecomputedWindow = NULL; //[N*2]*K_NB_FFT_WINDOW
int32	*gSourceHigh = NULL; //[N];
int32	*gSourceBass = NULL; //[N];
int32	*gSrcCpyHigh = NULL; //[N];
int32	*gSrcCpyBass = NULL; //[N];

char *gFftWindowName[K_NB_FFT_WINDOW+1] =
{
	"None",
	"Triangular",
	"Hann",
	"Hamming",
	"Blackman"
};

bool gStreamCanContinue = true;
static int32 stream_lock;		// for the stream benaphore
static sem_id stream_sem=-1;

//--------------------------------------------------------

//****************
CInit::CInit(void)
//****************
{
	initGlobals();
}


//****************
CInit::~CInit(void)
//****************
{
	freeGlobals();
}


//--------------------------------------------------------

//********************
void initGlobals(void)
//********************
{
//int i;

	if (debug) printf("Init : stream benaphore\n");
	// create benaphore
	stream_lock = 0L;
	stream_sem = create_sem(0,"stream_sem");
	if (stream_sem < B_NO_ERROR) throw(kCreateSemErr);

	if (debug) printf("Init : buffers for sound & fft\n");
	gComplexUtbl = new float[(N/4)+1];
	gRealUtbl 	= new float[(N/4)+1];
	gFftHigh 		= new float[N*2];
	gFftBass 		= new float[N*2];
	gFftPrecomputedWindow	= new float[N*2*K_NB_FFT_WINDOW];
	gSourceHigh = new int32[N];
	gSourceBass = new int32[N];
	gSrcCpyHigh = new int32[N];
	gSrcCpyBass = new int32[N];

	if (!gComplexUtbl || !gRealUtbl || !gFftHigh || !gFftBass || !gFftPrecomputedWindow
			|| !gSourceHigh || !gSourceBass || !gSrcCpyHigh || !gSrcCpyBass)
	{
		if (debug) printf("no sound & fft buffers -- mem alloc err\n");
		throw(kAllocErr);
	}

	if (FFTInit(&gComplexFftMptr, N, gComplexUtbl))
		if (debug) printf("Error initializing FFTLIB / FFTInit\n");

	if (rFFTInit(&gRealFftMptr, N, gRealUtbl))
		if (debug) printf("Error initializing FFTLIB / rFFTInit\n");

	if (debug) printf("Buffers sound : %p %p %p %p\n", gSourceHigh, gSourceBass, gSrcCpyHigh, gSrcCpyBass);
	if (debug) printf("Buffers fft   : %p %p\n", gFftHigh, gFftBass);
	if (debug) printf("Others        : %p %p\n", gComplexUtbl, gRealUtbl);

	//for(i = 0; i<N; i ++ ) taps[i]= Cmpx(1.0f, 0.0f);
	for(long i=0; i<2*N; i++) { gFftHigh[i]=0.0; gFftBass[i]=0.0;}
	memset(gSourceHigh, 0, sizeof(int32)*N);
	memset(gSourceBass, 0, sizeof(int32)*N);
	memset(gSrcCpyHigh, 0, sizeof(int32)*N);
	memset(gSrcCpyBass, 0, sizeof(int32)*N);

	if(1)
	{
		// initializes the fft windows
		// 0=none (not in the array)
		// kFftWindowTriangle : f(t)=1-|t|/T
		// kFftWindowHann			: f(t)=1/2*(1+cos(2pi*t/T)
		// kFftWindowHamming	: f(t)=0.54+0.46*cos(2*pi*t/T)
		// kFftWindowBlackman	:	f(t)=0.42+0.5*cos(2*pi*t/T)+0.8*cos(4*pi*t/T)
		
		float *p1=gFftPrecomputedWindow+N*2*0;
		float *p2=gFftPrecomputedWindow+N*2*1;
		float *p3=gFftPrecomputedWindow+N*2*2;
		float *p4=gFftPrecomputedWindow+N*2*3;
		float Ninv = 1.0/(float)N;
		for(float i=-1*N/2; i<N/2; i++, p1++, p2++, p3++, p4++)
		{
			float module = 2.0*M_PI*i*Ninv;
			float cos2   = cos(module);
			
			if (i<0) *p1 = (i+(float)(N/2))/(float)(N/2);
			else     *p1 = 1.0-(float)(i)/(float)(N/2);
			
			*p2 = 0.5*(1.0+cos2);
			*p3 = 0.54+0.46*cos2;
			*p4 = 0.42+0.5*cos2+0.08*cos(2.0*module);	// [RM 051400] fixed 0.8->0.08 for blackman window
		}
	}
}


//--------------------------------------------------------



//**********************************************************
template <class T> void doFiltre(T * const sample,
								 const uint32 nbs,
								 const double Te,		// def: 44100
								 const double f,		// def: fc_Hz / Te, ex: 180/44100
								 const double bw,		// def: bw_Hz / Te, ex:  30/44100
								 const T s_max)		// def: 32767 or 1.
//**********************************************************
{
	const T s_min = - s_max;

	#define K_EPSILON 1e-6
	#define K_PI M_PI

	// test pole
	const double racine = cos(2*K_PI*f);
	if (fabs(racine-1) < K_EPSILON)
	{
		printf("Attention ! Racine probable :\nf = %f (%f Hz)\ncos(2.pi.f) = %f\n",
					 f, f*Te, racine);
		assert(0);
		return;
	}

	// compute intermediates
	const double R = 1-3*bw;
	const double K = (1-2*R*cos(2*K_PI*f)+R*R)/(2-2*racine);
	const double a0 = 1-K;
	const double a1 = 2*(K-R)*racine;
	const double a2 = R*R-K;
	const double b1 = 2*R*racine;
	const double b2 = -R*R;

	double yn1 = 0.;
	double yn2 = 0.;
	double yn0 = 0.;
	for(uint32 i=2; i<nbs; i++)
	{
		yn0 = a0*sample[i] + a1*sample[i-1] + a2*sample[i-2] + b1*yn1 + b2*yn2;
		yn2 = yn1;
		yn1 = yn0;

		// we can now override sample[i-2], clip it before
		T a;
		if (yn0 < s_min) a = s_min;
		else if (yn0 > s_max) a = s_max;
		else a = (T)yn0;
		sample[i-2] = a;
	}

	// last two values of y are imcomplete.
	// assume succesive values of sample have been zero...
	sample[nbs-2] = (T)(b1*yn0 + b2*yn1); 			// y(n+1)
	sample[nbs-1] = (T)(b1*sample[nbs-2] + b2*yn0);	// y(n+2)
}



#define FILTER_HI(T,S,N,M)	doFiltre< T >( S , N , 44100., 21550./44100., 1000./44100. , M )
#define FILTER_LOW(T,S,N,M)	doFiltre< T >( S , N , 44100.,  2550./44100.,  400./44100. , M )

//--------------------------------------------------------



//******************************************************************
bool streamer_fftlib(void *arg, char *buf, size_t cnt, void *header)
//******************************************************************
{
	if (debug) printf("streamer_fftlib: arg=%p, buf=%p, cnt=%ld, header=%p\n", arg, buf, cnt, header);

	signed short *src = (signed short *)buf;
	int32 *srcH = gSourceHigh;
	int32 *srcB = gSourceBass;

	//FILTER_LOW(buf, 

	#define M 16
	const long len = (N-N/M);			// 512-512/16 == 480
	const long offset = N/M;			// 512/16 == 32

	lock_stream();
	memmove(srcB, srcB+offset, len * sizeof(int32));
	srcB += len;
	src--;
	srcH--;
	srcB--;
	for(int j=0, b=15; j<N; j++, b--)
	{
		signed long int a;
		a = ((*(++src)) + (*(++src)))/2;
		*(++srcH) = a;

		if (b==0)
		{
			*(++srcB) = a;
			b=16;
		}
	}

//	FILTER_HI(int32, gSourceHigh, N, 32767);
//	FILTER_LOW(int32, gSourceHigh, N, 32767);

	unlock_stream();

	return gStreamCanContinue;
}




//*************************************************
void streamer_fftlib_float(float *src, int32 count)
//*************************************************
{
	if (debug) printf("streamer_fftlib_float: src=%p, count=%ld\n", src, count);

	int32 *srcH = gSourceHigh;
	int32 *srcB = gSourceBass;

	#define M 16
	const long len = (N-N/M);			// 512-512/16 == 480
	const long offset = N/M;			// 512/16 == 32
	const float coef = 32767.0 / 2.0;

	lock_stream();
	memmove(srcB, srcB+offset, len * sizeof(int32));
	srcB += len;
	for(int j=0, b=15; j<N; j++, b--)
	{
		signed long int a;
//		if (debug) printf("src: %f %f ->", src[0], src[1]);
		a = (int32)(coef * (*(src++) + *(src++)));
//		if (debug) printf("a = %ld\n", a);
		*(srcH++) = a;

		if (b==0)
		{
			*(srcB++) = a;
			b=16;
		}
	}

//	FILTER_HI(int32, gSourceHigh, N, 32767);
//	FILTER_LOW(int32, gSourceHigh, N, 32767);

	unlock_stream();
}


//********************
void freeGlobals(void)
//********************
{
	if (debug) printf("Free Globals\n");

	delete_sem(stream_sem);

	if (gComplexUtbl) delete gComplexUtbl;
	if (gRealUtbl) delete gRealUtbl;
	if (gFftHigh) delete gFftHigh;
	if (gFftBass) delete gFftBass;
	if (gFftPrecomputedWindow) delete gFftPrecomputedWindow;
	if (gSourceHigh) delete gSourceHigh;
	if (gSourceBass) delete gSourceBass;
	if (gSrcCpyHigh) delete gSrcCpyHigh;
	if (gSrcCpyBass) delete gSrcCpyBass;
}


// Release the protection on the general io-registers
// (from S3 driver sample code, DR8.2, by Be Inc)
void unlock_stream(void)
{
	int32	old;

	old = atomic_add (&stream_lock, -1);
	if (old > 1) {
		release_sem(stream_sem);
	}
}	

// Protect the access to the memory or the graphic engine registers by
// locking a benaphore.
// (from S3 driver sample code, DR8.2, by Be Inc)
void lock_stream(void)
{
	int32	old;

	old = atomic_add (&stream_lock, 1);
	if (old >= 1) {
		acquire_sem(stream_sem);	
	}	
}

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

// eoc

