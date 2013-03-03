/*****************************************************************************

	Projet	: Pulsar Sample FFT AddOn

	Fichier	: line_addon.cpp
	Partie	: End

	Auteur	: RM
	Date	: 040597 -- version 0.2 (C funcs)
	Date	: 010797 -- version 0.3 (CFilter)
	Date	: 130499 -- Mixed FFT, FFT2, Loupe, DemiCercle
	Format	: tabs==4

*****************************************************************************/

#include "PP_Be.h"
#include "CFilter.h"
#include "fastmath.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" void * filterInit(uint32 index);

uint32 color_line(int32 y, float inv_max);
uint32 color_line2(int32 y, int32 y_max);

#define ADDON_NAME "Sample Lines"

#ifndef M_PI
	#define M_PI	3.1415926538
#endif

//--------------------------------------------------------------------


//*********************************
class  CFft2Filter : public CFilter
//*********************************
{
public:

	CFft2Filter(void) : CFilter()	{ /* nothing */ }
	virtual ~CFft2Filter(void) 		{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void);
	virtual void processFrame8(SFrameInfo &frame);
	virtual void processFrame32(SFrameInfo &frame);

// -- this part of the sample code yet to be finished -- RM 040900
//	virtual BView *settingsOpened(void);

private:
	BView * myview;

}; // end of class defs for CSampleFftFilter


//**************************************
class  CSampleFftFilter : public CFilter
//**************************************
{
public:

	CSampleFftFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CSampleFftFilter(void) 	{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void)			{ return true; }
	virtual void processFrame8(SFrameInfo &frame);
	virtual void processFrame32(SFrameInfo &frame);

}; // end of class defs for CSampleFftFilter


//*************************************
class CCentralFftFilter : public CFilter
//*************************************
{
public:

	CCentralFftFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CCentralFftFilter(void) 	{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void)			{ return true; }
	virtual void processFrame8(SFrameInfo &frame);
	virtual void processFrame32(SFrameInfo &frame);

}; // end of class defs for CCentralFftFilter


//****************************************
class CSampleCercleFilter : public CFilter
//****************************************
{
public:

	CSampleCercleFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CSampleCercleFilter(void) 		{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void) 				{ return true; }
	virtual void processFrame8(SFrameInfo &frame);
	virtual void processFrame32(SFrameInfo &frame);

}; // end of class defs for CSampleCercleFilter

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
#pragma mark -


//*****************************
void * filterInit(uint32 index)
//*****************************
{
	// this add-on declares four filters
	switch(index)
	{
		case 0: 	return new CSampleFftFilter;
		case 1: 	return new CCentralFftFilter;
		case 2: 	return new CFft2Filter;
		case 3: 	return new CSampleCercleFilter;
		default:	return NULL;		// no more add-ons to declare
	}
} // end of filterInit


//--------------------------------------------------------------------
#pragma mark -


//***************************************
uint32 color_line(int32 y, float inv_max)
//***************************************
{
	double ang = (double)y;
	ang *= inv_max * M_PI * 2;
	double red   = cos(ang);
	double green = cos(ang + M_PI/3);
	double blue  = cos(ang + 2*M_PI/3);
	int32 ir = 128+(int32)(red  *127.);
	int32 ig = 128+(int32)(green*127.);
	int32 ib = 128+(int32)(blue *127.);
	return (ir<<K_RED_OFFSET)|(ir<<K_GREEN_OFFSET)|(ib<<K_BLUE_OFFSET);
}


//**************************************
uint32 color_line2(int32 y, int32 y_max)
//**************************************
{
	y_max /= 4;
	int32 coef = (int32)(512.*(double)y / (double)y_max);
	int32 ir=0, ig=0, ib=0;
	if (coef > 0 && coef < 256)
	{
		ir = coef;
		ig = coef;
		ib = 0;
	}
	else if (coef >= 256)
	{
		coef -= 256;
		if (coef > 255) coef = 255;
		ir = coef;
		ig = 255-coef;
		ib = 0;
	}
	
	return (ir<<K_RED_OFFSET)|(ig<<K_GREEN_OFFSET)|(ib<<K_BLUE_OFFSET);
}



//--------------------------------------------------------------------
#pragma mark -


//*******************************
bool CSampleFftFilter::load(void)
//*******************************
{
	sFilter.name	= "FFT Scope";
	sFilter.author	= "R'alf";
	sFilter.info	= "Sample code";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionLast-0.21;		// just random stuff
	sFilter.supportedMode = (EColorSpace)(kColor8Bit | kRgb32Bit);

	sFilter.modelParam.AddRect("Bass Frame", BRect(10,10,10+255,10+99));
	sFilter.modelParam.AddRect("High Frame", BRect(640-256-10,10,640-256-10+255,10+99));
	sFilter.modelParam.AddBool("Average", true);
	sFilter.modelParam.AddFloat("Divider", 8192.0);

	return true;
}  // end of load for CSampleFftFilter


//*****************************************************
void CSampleFftFilter::processFrame8(SFrameInfo &frame)
//*****************************************************
{
	float *highFft;
	float *bassFft;
	int i;
	int y1;

	highFft	= frame.fftHigh;
	bassFft	= frame.fftBass;

	// fft droite (right)

	i = 10+100-(int)(frame.averageFftBass/8192.);
	if (i<0) i=0;
	if (i>479) i=479;
	long z=i;

	for(i=0; i<255; i++)
	{
		y1 = 100-(int)((bassFft[i])/8192.);
		if (y1<0) y1=0;
		if (y1 > 99) y1=99;

		// 52 == vert pomme (apple green)
		blitLine8(10+i, 10+y1, 10+i, 10+100/*+y2*/, 52);
	}
	blitLine8(10, z, 10+256, z, 250);

	// fft gauche (left)

	i = 10+100-(int)(frame.averageFftHigh/8192.);
	if (i<0) i=0;
	if (i>479) i=479;
	z=i;
	for(i=0; i<255; i++)
	{
		y1 = 100-(int)((highFft[i])/8192.);
		if (y1<0) y1=0;
		if (y1 > 99) y1=99;

		// 250 == jaune pomme
		blitLine8(640-256-10+i, 10+y1, 640-256-10+i, 10+100, 250);
	}
	blitLine8(640-256-10, z, 640-10, z, 52);

}  // end of processFrame8 for CSampleFftFilter


//******************************************************
void CSampleFftFilter::processFrame32(SFrameInfo &frame)
//******************************************************
{
	float *highFft;
	float *bassFft;
	int i;
	int y1;

	highFft	= frame.fftHigh;
	bassFft	= frame.fftBass;

	const ulong vert  = (0x00 << K_RED_OFFSET) | (0xDF << K_GREEN_OFFSET);
	const ulong jaune = (0xDF << K_RED_OFFSET) | (0xDF << K_GREEN_OFFSET);

	// fft droite (right)

	i = 10+100-(int)(frame.averageFftBass/8192.);
	if (i<0) i=0;
	if (i>479) i=479;
	long z=i;

	for(i=0; i<255; i++)
	{
		y1 = 100-(int)((bassFft[i])/8192.);
		if (y1<0) y1=0;
		if (y1 > 99) y1=99;

		// 52 == vert pomme (apple green)
		blitLine32(10+i, 10+y1, 10+i, 10+100/*+y2*/, vert);
	}
	blitLine32(10, z, 10+256, z, jaune);

	// fft gauche (left)

	i = 10+100-(int)(frame.averageFftHigh/8192.);
	if (i<0) i=0;
	if (i>479) i=479;
	z=i;
	for(i=0; i<255; i++)
	{
		y1 = 100-(int)((highFft[i])/8192.);
		if (y1<0) y1=0;
		if (y1 > 99) y1=99;

		// 250 == jaune pomme
		blitLine32(640-256-10+i, 10+y1, 640-256-10+i, 10+100, jaune);
	}
	blitLine32(640-256-10, z, 640-10, z, vert);

}  // end of processFrame32 for CSampleFftFilter


//--------------------------------------------------------------------
#pragma mark -


//********************************
bool CCentralFftFilter::load(void)
//********************************
{
	sFilter.name	= "FFT Central";
	sFilter.author	= "R'alf";
	sFilter.info	= "Sample code";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionLast-0.22;
	sFilter.supportedMode = (EColorSpace)(kColor8Bit | kRgb32Bit);

	return true;
}  // end of load for CCentralFftFilter


//******************************************************
void CCentralFftFilter::processFrame8(SFrameInfo &frame)
//******************************************************
{
	float *bassFft;
	int i;
	long nframe;
	int y1;
	int h = sPrepare.sy;
	int w = sPrepare.sx;
	int w2 = w/2;

	bassFft	= frame.fftBass;
	nframe	= frame.frame;

	for(i=0; i<h; i++)
	{
		long a = (i+nframe)&0x1ff;
		if (a > 255) continue;
		y1 = (int)((bassFft[a]) / 32768.);
		if (y1 < 0) y1 = -y1;
		if (y1 < 3) continue;
		if (y1 > w2) y1 = w2;
		blitLine8(w2-y1, i, w2+y1, i, i+y1);
	}
}  // end of processFrame8 for CCentralFftFilter


//*******************************************************
void CCentralFftFilter::processFrame32(SFrameInfo &frame)
//*******************************************************
{
float *bassFft;
int i;
long nframe;
int y1;
int h = sPrepare.sy;
int w = sPrepare.sx;
int w2 = w/2;
//double inv_max_h = 1./h;

	bassFft	= frame.fftBass;
	nframe	= frame.frame;

	for(i=0; i<h; i++)
	{
		long a = (i+nframe)&0x1ff;
		if (a > 255) continue;
		y1 = (int)((bassFft[a]) / 32768.);
		if (y1 < 0) y1 = -y1;
		if (y1 < 1) continue;
		if (y1 > w2) y1 = w2;
		// printf("0x%08lx\n", color_line(h, inv_max_h));
		blitLine32(w2-y1, i, w2+y1, i, color_line2(y1, w2));
	}
}  // end of processFrame32 for CCentralFftFilter


//--------------------------------------------------------------------
#pragma mark -


//**************************
bool CFft2Filter::load(void)
//**************************
{
	sFilter.name	= "FFT 2";
	sFilter.author	= "R'alf";
	sFilter.info	= "Sample code";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionLast-0.21;
	sFilter.supportedMode = (EColorSpace)(kColor8Bit | kRgb32Bit);
	sFilter.hasSettings = true;

	sFilter.modelParam.AddRect("Bass Frame", BRect(10,10,10+255,10+99));
	sFilter.modelParam.AddRect("High Frame", BRect(640-256-10,10,640-256-10+255,10+99));
	sFilter.modelParam.AddBool("Average", true);
	sFilter.modelParam.AddFloat("Divider", 8192.0);

	myview = NULL;

	return true;
}  // end of load for CFft2Filter


//*****************************
bool CFft2Filter::prepare(void)
//*****************************
{
	return true;
} // end of prepare for CFft2Filter


/*
-- this part of the sample code yet to be finished -- RM 040900
//***************************************
BView * CFft2Filter::settingsOpened(void)
//***************************************
{
	//printf("CFft2Filter::settingsOpened -- (%p)\n",myview);
	if (myview) return myview;
	myview = new BView(BRect(0,0,200,50), "myview", B_FOLLOW_ALL, B_WILL_DRAW);
	//printf("CFft2Filter::settingsOpened -- new view %p\n", myview);
	if (!myview) return NULL;
	myview->SetViewColor(150,40,40);
	return myview;
	// DO NOT call inherited here !!	
} // end of settingsOpened for CFft2Filter
*/


//************************************************
void CFft2Filter::processFrame8(SFrameInfo &frame)
//************************************************
{
	#define VERT 52		// vert pomme (apple green)
	#define JAUNE 250	// jaune pomme

	long xb=10;
	long xa=640-256-10;
	long y=470;
	for(long i=0; i<8; i++)
	{
		int a = (int)(frame.meanFftHigh[i]/8192.);
		int b = (int)(frame.meanFftBass[i]/8192.);
		if (a < 0) a = 0;
		if (b < 0) b = 0;
		if (a > 300) a = 300;
		if (b > 300) b = 300;
		
		long y2=y-b;
		long x2=xb+31;
		blitLine8(xb,y,xb,y2,VERT);
		blitLine8(xb,y2,x2,y2,VERT);
		xb+=32;

		y2=y-a;
		x2=xa+31;
		blitLine8(xa,y,xa,y2,JAUNE);
		blitLine8(xa,y2,x2,y2,JAUNE);
		xa += 32;
	}
	
	if(1)
	{
			int a = (int)(frame.averageFftHigh/8192.);
			int b = (int)(frame.averageFftBass/8192.);
			if (a < 0) a = 0;
			if (b < 0) b = 0;
			if (a > 300) a = 300;
			if (b > 300) b = 300;
			
			long y2=y-b;
			blitLine8(10,y2,10+256,y2,JAUNE);

			y2=y-a;
			blitLine8(640-256-10,y2,640-10,y2,VERT);
	}		

}  // end of processFrame8 for CFft2Filter


//*************************************************
void CFft2Filter::processFrame32(SFrameInfo &frame)
//*************************************************
{
	const ulong vert  = (0x00 << K_RED_OFFSET) | (0xDF << K_GREEN_OFFSET);
	const ulong jaune = (0xDF << K_RED_OFFSET) | (0xDF << K_GREEN_OFFSET);
	#undef VERT
	#undef JAUNE
	#define VERT vert		// 52 vert pomme (apple green)
	#define JAUNE jaune		// 250 jaune pomme

	long xb=10;
	long xa=640-256-10;
	long y=470;
	for(long i=0; i<8; i++)
	{
		int a = (int)(frame.meanFftHigh[i]/8192.);
		int b = (int)(frame.meanFftBass[i]/8192.);
		if (a < 0) a = 0;
		if (b < 0) b = 0;
		if (a > 300) a = 300;
		if (b > 300) b = 300;
		
		long y2=y-b;
		long x2=xb+31;
		blitLine32(xb,y,xb,y2,VERT);
		blitLine32(xb,y2,x2,y2,VERT);
		xb+=32;

		y2=y-a;
		x2=xa+31;
		blitLine32(xa,y,xa,y2,JAUNE);
		blitLine32(xa,y2,x2,y2,JAUNE);
		xa += 32;
	}
	
	if(1)
	{
			int a = (int)(frame.averageFftHigh/8192.);
			int b = (int)(frame.averageFftBass/8192.);
			if (a < 0) a = 0;
			if (b < 0) b = 0;
			if (a > 300) a = 300;
			if (b > 300) b = 300;
			
			long y2=y-b;
			blitLine32(10,y2,10+256,y2,JAUNE);

			y2=y-a;
			blitLine32(640-256-10,y2,640-10,y2,VERT);
	}		

}  // end of processFrame32 for CFft2Filter


//--------------------------------------------------------------------
#pragma mark -


//**********************************
bool CSampleCercleFilter::load(void)
//**********************************
{
	sFilter.name	= "DemiCercle";
	sFilter.author	= "R'alf";
	sFilter.info	= "Sample code";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionLast-0.2;

	return true;
}  // end of load for CSampleCercleFilter


//********************************************************
void CSampleCercleFilter::processFrame8(SFrameInfo &frame)
//********************************************************
{
	float *chanL;
	float *chanR;
	int i;
	uchar *base;
	long bpr;
	long nframe;
	int y1;
	int y2;

	base 	= frame.screen;
	bpr		= sPrepare.bpr;
	nframe	= frame.frame;
	chanL = frame.fftHigh;
	chanR = frame.fftBass;

	// demi cercles

	const float inc_pi = M_PI/2/64;
	float angle=0;
	for(i=0; i<64; i++, angle+=inc_pi)
	{
		float rx = inl_b_cos_90(angle);
		float ry = inl_b_cos_90(M_PI/2-angle);
		long x1=150+(int)(128*rx);
		y1=     200-(int)(128*ry);
		y2=(int)(chanL[i]/16384.);
		long x2=150+(int)((128-y2)*rx);
		y2=     200-(int)((128-y2)*ry);

		blitLine8(x1,y1,x2,y2,i+nframe);
	}

	for(angle=0, i=0; i<64; i++, angle+=inc_pi)
	{
		float rx = inl_b_cos_90(angle);
		float ry = inl_b_cos_90(M_PI/2-angle);
		long x1=450+(int)(50*rx);
		y1=     300-(int)(50*ry);
		y2=(int)(chanR[i]/16384.);
		long x2=450+(int)((60+y2)*rx);
		y2=     300-(int)((60+y2)*ry);

		blitLine8(x1,y1,x2,y2,i+nframe);
	}

}  // end of processFrame8 for CSampleCercleFilter

//*********************************************************
void CSampleCercleFilter::processFrame32(SFrameInfo &frame)
//*********************************************************
{
	float *chanL;
	float *chanR;
	int i;
	uchar *base;
	long bpr;
	long nframe;
	int y1;
	int y2;

	base 	= frame.screen;
	bpr		= sPrepare.bpr;
	nframe	= frame.frame;
	chanL = frame.fftHigh;
	chanR = frame.fftBass;

	// demi cercles

	const float inc_pi = M_PI/2/64;
	float angle=0;
	double inv_max = 1./128.;
	for(i=0; i<64; i++, angle+=inc_pi)
	{
		float rx = inl_b_cos_90(angle);
		float ry = inl_b_cos_90(M_PI/2-angle);
		long x1=150+(int)(128*rx);
		y1=     200-(int)(128*ry);
		y2=(int)(chanL[i]/16384.);
		long x2=150+(int)((128-y2)*rx);
		y2=     200-(int)((128-y2)*ry);

		blitLine32(x1,y1,x2,y2,color_line((i+nframe)&0x7F, inv_max));
	}

	for(angle=0, i=0; i<64; i++, angle+=inc_pi)
	{
		float rx = inl_b_cos_90(angle);
		float ry = inl_b_cos_90(M_PI/2-angle);
		long x1=450+(int)(50*rx);
		y1=     300-(int)(50*ry);
		y2=(int)(chanR[i]/16384.);
		long x2=450+(int)((60+y2)*rx);
		y2=     300-(int)((60+y2)*ry);

		blitLine32(x1,y1,x2,y2,color_line((i+nframe)&0x7F, inv_max));
	}

}  // end of processFrame8 for CSampleCercleFilter


#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif


// eoc
