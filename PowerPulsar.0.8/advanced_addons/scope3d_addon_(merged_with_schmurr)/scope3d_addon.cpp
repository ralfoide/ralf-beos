/*****************************************************************************

	Projet	: Pulsar Sample FFT AddOn

	Fichier	: sample_addon.cpp
	Partie	: End

	Auteur	: RM
	Date	: 040597 -- version 0.2 (C funcs)
	Date	: 010797 -- version 0.3 (CFilter)
	Format	: tabs==4

*****************************************************************************/

#include "PP_Be.h"
#include "CFilter.h"
#include <math.h>
#include "t3d.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" _ADDON_IMPEXP void * filterInit(uint32 index);

#define ADDON_NAME "Scope3d"

rgb_color make_color(uint8 r, uint8 g, uint8 b);

static const bool debug=false;

#ifndef M_PI
#define M_PI 3.141592654
#endif

#define K_NB_CUBE 6

//--------------------------------------------------------------------


//*************************************
class CScope3dFilter : public CFilter
//*************************************
{
public:

	CScope3dFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CScope3dFilter(void) 		{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void);
	virtual void processFrame8(SFrameInfo &frame);

private:
	bool mUseGk8;
	T3d	 		*t3d;
	TShape	*shape;

	float 	meanalpha,meanbeta,meandelta;
	float		pathangle;

	long		cube_point[K_NB_CUBE];	// first point of top of cube

	bool init3d(void);

}; // end of class defs for CScope3dFilter


//--------------------------------------------------------------------

//**********************************************
int main(int, char **)
//**********************************************
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


//**********************************************
void * filterInit(uint32 index)
//**********************************************
{
	// this add-on only declares ONE filter
	if (index > 0) return NULL;

	CScope3dFilter *info = new CScope3dFilter;
	if (!info) return NULL;		// memory error, give up

	// returns the instance -- the caller will check that this instance
	// is derived from CFilter but is _not_ a CFilter...
	return info;
} // end of filterInit


//*******************************
bool CScope3dFilter::load(void)
//*******************************
{
	sFilter.name = "Scope 3d";
	sFilter.author = "R'alf";
	sFilter.info = "Sample 3d code by B. S., Be Inc.";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionLast;
	sFilter.supportedMode = B_COLOR_8_BIT;

	sFilter.icon = loadBitmap("./add-ons.icons/fft.tga");
	return init3d();
}  // end of load for CScope3dFilter



//*********************************************
rgb_color	make_color(uint8 r, uint8 g, uint8 b)
//*********************************************
{
	rgb_color	c;
	
	c.red = r;
	c.green = g;
	c.blue = b;
	return(c);
}


//*******************************
bool CScope3dFilter::init3d(void)
//*******************************
{
	t3d = new T3d(640,480);
	if (!t3d) return false;
	shape = new TShape(t3d);
	if (!shape) return false;

	// complex shape

	if(1)
	{
		float size=256;
		rgb_color col;
		float w = size;
		const long N = K_NB_CUBE;
		float sw = w/N;
		float sh = sw;
		
		float xx=-size/2;
		float height = 50;
	
		// points of base
		// x--y screen , z profondeur
		shape->add_point(xx+ 0, 0,0);
		shape->add_point(xx+ 0,sh,0);
		shape->add_point(xx+ w,sh,0);
		shape->add_point(xx+ w, 0,0);
	
		// base of cubes : N+1 points
		for(long i=0; i<=N; i++)
		{
			shape->add_point(xx+i*sw, 0,0);
			shape->add_point(xx+i*sw,sh,0);
		}

		// top of N cubes
		for(long i=0; i<N; i++)
		{
			cube_point[i] = shape->add_point(xx+i*sw   , 0,height);
			shape->add_point(xx+i*sw   ,sh,height);
			shape->add_point(xx+i*sw+sw, 0,height);
			shape->add_point(xx+i*sw+sw,sh,height);
		}
	
		// make N cubes
		for(long i=0; i<N; i++)
		{
			//col = make_color(32+5+10*i,32+5+10*i,32+5+10*i);
			col = make_color(20+i*20, 220-i*30, 140);
			long b=4+2*i;
			long t=4+2+2*N+4*i;
			shape->add_poly(b  ,b+1,b+3,b+2,col);
			shape->add_poly(b  ,t  ,t+2,b+2,col);
			shape->add_poly(b+2,t+2,t+3,b+3,col);
			shape->add_poly(b+3,t+3,t+1,b+1,col);
			shape->add_poly(b+1,t+1,t  ,b  ,col);
			shape->add_poly(t  ,t+1,t+3,t+2,col);
		}	
	}
	return true;
}


//**********************************
bool CScope3dFilter::prepare(void)
//**********************************
{
	mUseGk8 = sPrepare.isGameKit
						&& sPrepare.mode == B_COLOR_8_BIT
						&& sPrepare.gameKitScreen != NULL
						&& line_8 != NULL;

	meanalpha=0;
	meanbeta=0;
	meandelta=0;
	pathangle = 0;
	t3d->useGk(mUseGk8, this);

	return true;
} // end of prepare for CScope3dFilter


//***************************************
void CScope3dFilter::processFrame8(SFrameInfo &frame)
//***************************************
{
char	*base;
long		bpr;
long		i;
float 	alpha,beta,delta;
float 	xc,yc;
		
	base	= (char *)frame.screen;
	bpr		= sPrepare.bpr;

	const float kCoef = M_PI/360;
	alpha = frame.meanFftHigh[0]*kCoef/4096;
	beta  = frame.averageFftBass*kCoef/512;
	delta = frame.meanFftBass[0]*kCoef/4096;

	meanalpha = (meanalpha+alpha)/2;
	meanbeta  = (meanbeta +beta)/2;
	meandelta = (meandelta+delta)/2;
	alpha -= meanalpha;
	beta  -= meanbeta;
	delta -= meandelta;
	beta += M_PI/4;

	xc = 320+128*cos(pathangle)+64*sin(pathangle*3);
	yc = 240+64*cos(pathangle/2)+32*sin(pathangle*4);
	pathangle += M_PI/256;
	if (pathangle > 12*M_PI) pathangle -= 12*M_PI;

	// set up the left cubes
	for(i=0; i<K_NB_CUBE/2; i++)
	{
		long index = cube_point[K_NB_CUBE-1-i];
		long	val = (10+frame.meanFftBass[i]/8192)*64;
		long *ptz = &(shape->pt_z[index]);
		*(ptz) = val;
		*(++ptz) = val;
		*(++ptz) = val;
		*(++ptz) = val;

		index = cube_point[i];
		val = (10+frame.meanFftHigh[i]/8192)*64;
		ptz = &(shape->pt_z[index]);
		*(ptz) = val;
		*(++ptz) = val;
		*(++ptz) = val;
		*(++ptz) = val;
	}

	t3d->setFrame(base, bpr);
	shape->draw(alpha,beta,delta, xc, yc);

	// frame.meanFftBass[0];

	//if (modifiers() & B_LEFT_SHIFT_KEY) a -= FOV/10.0;
	//if (modifiers() & B_CONTROL_KEY && speed < 16) speed++;

}  // end of processFrame8 for CScope3dFilter


// eoc
