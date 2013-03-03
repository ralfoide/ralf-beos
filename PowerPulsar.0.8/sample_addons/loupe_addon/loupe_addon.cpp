/*****************************************************************************

	Projet	: Pulsar Sample Loupe AddOn

	Fichier	: sample_addon.cpp
	Partie	: End

	Auteur	: RM
	Date	: 040597 -- version 0.2 (C funcs)
	Date	: 010797 -- version 0.3 (CFilter)
	Format	: tabs==4

*****************************************************************************/

#include "PP_Be.h"
#include "CFilter.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" void * filterInit(uint32 index);

#define ADDON_NAME "Loupe"

//--------------------------------------------------------------------

//***************************************
class CSampleLoupeFilter : public CFilter
//***************************************
{
public:

	CSampleLoupeFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CSampleLoupeFilter(void) 		{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void);
	virtual void processFrame8(SFrameInfo &frame);
	virtual void processFrame32(SFrameInfo &frame);

}; // end of class defs for CSampleLoupeFilter


//--------------------------------------------------------------------

//********************
int main(int, char **)
//********************
// Code to help a little the fellow user double-clicking the
// add-on icon directly.
{
	new BApplication("application/x-vnd.ralf-vanilla");
	BAlert *box = new BAlert(ADDON_NAME,
							 "Thanks for using " ADDON_NAME " !\n\n"
							 "Please move the " ADDON_NAME " executable into\n"
							 "the PowerPulsar add-on directory\n"
							 "and try again !",
							 "Great !");
	box->Go();
	delete be_app;
	return 0;
}

//--------------------------------------------------------------------


//*****************************
void * filterInit(uint32 index)
//*****************************
{
	// this add-on only declares ONE filter
	if (index > 0) return NULL;

	CSampleLoupeFilter *info = new CSampleLoupeFilter;
	if (!info) return NULL;		// memory error, give up

	// returns the instance -- the caller will check that this instance
	// is derived from CFilter but is _not_ a CFilter...
	return info;
} // end of filterInit


//*********************************
bool CSampleLoupeFilter::load(void)
//*********************************
{
	sFilter.name = "Loupe";
	sFilter.author = "R'alf";
	sFilter.info = "Sample code";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionLast-0.19;
	sFilter.supportedMode = (EColorSpace) (kColor8Bit | kRgb32Bit);

	return true;
}  // end of load for CSampleLoupeFilter


//************************************
bool CSampleLoupeFilter::prepare(void)
//************************************
{
	return true;
} // end of prepare for CSampleLoupeFilter


//*******************************************************
void CSampleLoupeFilter::processFrame8(SFrameInfo &frame)
//*******************************************************
{
int i,j;
uchar *base;
long bpr;
int y1;
int y2;
int w = sPrepare.sx;

	base = frame.screen;
	bpr =  sPrepare.bpr;
	int32 *sound = frame.rawHigh;

	// loupe -- 320 pixels de 150 a 470, zoom *8
	
	// cree un double avec des '250' partout
	uint32 val=0xFAFAFAFA;		// couleur jaune
	double ref;
	uint32 *iref = (uint32 *)&ref;
	*(iref++) = val;
	*iref = val;

	double *m = (double *)(base+160*bpr);
	j=w/8;
	long mean=0;
	for(i=0; i<j/2;)
	{
		mean+=sound[i++];
		mean+=sound[i++];
	}
	mean /= j;
	mean = (50*mean)/32768;

	int b=bpr/8;
	int a2,a3,a4,a5,a6,a7,a8;
	a2=b+b; a3=a2+b; a4=a3+b; a4=a3+b; a5=a4+b; a6=a5+b; a7=a6+b; a8=a7+b;

	for(i=0; i<j; i++, m++)
	{
		y1 = 50+50*sound[i]/32768-mean;
		y2 = 50+50*sound[i+1]/32768-mean;
		if (y1 > y2) { int a=y1; y1=y2; y2=a; }
		if (y1 < 50-20) y1 = 50-20;
		if (y2 < 50-20) y2 = 50-20;
		if (y1 > 50+20-1) y1 = 50+20-1;
		if (y2 > 50+20-1) y2 = 50+20-1;
		y1 -= 50-20;
		y2 -= 50-20;

		double ref2=ref;	// makes CW happy : generates stdfx
		double *p = m+y1*a8;
		int y;

		for(y=y1; y<=y2; y++, p+=a8)
		{
			*(p   ) = ref2;
			*(p+b ) = ref2;
			*(p+a2) = ref2;
			*(p+a3) = ref2;
			*(p+a4) = ref2;
			*(p+a5) = ref2;
			*(p+a6) = ref2;
			*(p+a7) = ref2;
		}
	}
}  // end of filterProcessFrame


//********************************************************
void CSampleLoupeFilter::processFrame32(SFrameInfo &frame)
//********************************************************
/*
*/
{
int i,j;
int y1;
int y2;
int w = sPrepare.sx;
int x1,x2;

	int32 *sound = frame.rawHigh;

	// loupe -- 320 pixels de 150 a 470, zoom *8
	j=w/8;
	long mean=0;
	for(i=0; i<j; i++) mean+=sound[i];
	mean /= j;
	mean = 50*mean/32768;
	x1=0;
	x2=7;
	for(i=0; i<j; i++, x1+=8, x2+=8)
	{
		y1 = 50+50*sound[i]/32768-mean;
		y2 = 50+50*sound[i+1]/32768-mean;
		if (y1 > y2) { int a=y1; y1=y2; y2=a; }
		if (y1 < 50-20) y1 = 50-20;
		if (y2 < 50-20) y2 = 50-20;
		if (y1 > 50+20-1) y1 = 50+20-1;
		if (y2 > 50+20-1) y2 = 50+20-1;
		y1 -= 50-20;
		y2 -= 50-20;

		const ulong c1 = (250<<16)|(250<<8);
		const ulong c2 =           (250<<8);
		blitRect32(x1,150+y1*8,x2,150+8+y2*8,c2);
		blitRect32(x1,150+y1*8,x2,150+8+y1*8,c1);
	}
}  // end of filterProcessFrame


// eoc
