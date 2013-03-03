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

extern "C" void * filterInit(uint32 index);

#define ADDON_NAME "Schmur3d & Scope3d"

rgb_color make_color(uint8 r, uint8 g, uint8 b);

static const bool debug=false;

#ifndef M_PI
#define M_PI 3.141592654
#endif

#define K_S1 64
#define K_S2 64

#define K_NB_CUBE 6

//--------------------------------------------------------------------


//*************************************
class CSchmur3dFilter : public CFilter
//*************************************
{
public:

	CSchmur3dFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CSchmur3dFilter(void) 		{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void);
	virtual void processFrame8(SFrameInfo &frame);

private:
	bool	mUseGk8;
	T3d	 	*t3d;
	TShape	*shape;
	long	outside_point;

	float 	meanalpha,meanbeta,meandelta;
	float	pathangle;

	bool	init3d(void);

}; // end of class defs for CSchmur3dFilter



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
	bool	mUseGk8;
	T3d	 	*t3d;
	TShape	*shape;

	float 	meanalpha,meanbeta,meandelta;
	float	pathangle;

	long	cube_point[K_NB_CUBE];	// first point of top of cube

	bool	init3d(void);

}; // end of class defs for CScope3dFilter


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
	// this add-on declares two filters
	switch(index)
	{
		case 0:	return new CSchmur3dFilter;
		case 1: return new CScope3dFilter;
	}	
	return NULL;
} // end of filterInit



//-----------------------------------------------------------------------


//******************************
bool CSchmur3dFilter::load(void)
//******************************
{
	sFilter.name = "Schmur 3d";
	sFilter.author = "R'alf";
	sFilter.info = "You know what it is ? It's a phoque.";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionLast;
	sFilter.supportedMode = kColor8Bit;

	sFilter.icon = loadBitmap("./add-ons.icons/fft.tga");
	return init3d();
}  // end of load for CSchmur3dFilter



//*********************************************
rgb_color make_color(uint8 r, uint8 g, uint8 b)
//*********************************************
{
	rgb_color	c;
	
	c.red = r;
	c.green = g;
	c.blue = b;
	return(c);
}


//********************************
bool CSchmur3dFilter::init3d(void)
//********************************
{
	t3d = new T3d(640,480);
	if (!t3d) return false;
	shape = new TShape(t3d);
	if (!shape) return false;

	// complex shape

	rgb_color col;
	const float s1 = K_S1;
	const float s2 = K_S2;
	
	// points of inside cube : 0->3 & 4->7
	// x--y screen , z profondeur

	shape->add_point(-s1, -s1, -s1);		// base of cube
	shape->add_point( s1, -s1, -s1);
	shape->add_point( s1, -s1,  s1);
	shape->add_point(-s1, -s1,  s1);

	shape->add_point(-s1, s1, -s1);		// top of cube
	shape->add_point( s1, s1, -s1);
	shape->add_point( s1, s1,  s1);
	shape->add_point(-s1, s1,  s1);

	// points outisde	 for each face : 8(front)->11(left)+12 bottom+13 top
	outside_point =
	shape->add_point(  0,  0, -s2);
	shape->add_point( s2,  0,   0);
	shape->add_point(  0,  0,  s2);
	shape->add_point(-s2,  0,   0);
	shape->add_point(  0,-s2,   0);
	shape->add_point(  0, s2,   0);

	// make outside faces
	#define M_MK_FACE(p1,p2,p3,p4,p5)		\
		shape->add_triangle(p1,p4,p5 ,col);	\
		shape->add_triangle(p2,p1,p5 ,col);	\
		shape->add_triangle(p3,p2,p5 ,col);	\
		shape->add_triangle(p4,p3,p5 ,col);

	// make tetrahedres
								col = make_color(240, 10, 20);
	M_MK_FACE(1,2,6,5,9);		col = make_color(240,200, 20);
	M_MK_FACE(2,3,7,6,10);		col = make_color( 40,210,220);
	M_MK_FACE(3,0,4,7,11);		col = make_color(240, 10,220);
	M_MK_FACE(0,1,5,4,8);		col = make_color( 10,230, 20);
	M_MK_FACE(3,2,1,0,12);		col = make_color( 20, 10,240);
	M_MK_FACE(4,5,6,7,13);

	return true;
}


//*********************************
bool CSchmur3dFilter::prepare(void)
//*********************************
{
	mUseGk8 = sPrepare.isGameKit
			&& sPrepare.mode == kColor8Bit
			&& sPrepare.gameKitScreen != NULL
			&& line_8 != NULL;

	meanalpha=0;
	meanbeta=0;
	meandelta=0;
	pathangle = 0;
	t3d->useGk(mUseGk8, this);

	return true;
} // end of prepare for CSchmur3dFilter


//****************************************************
void CSchmur3dFilter::processFrame8(SFrameInfo &frame)
//****************************************************
{
char	*base;
long	bpr;
long	i;
long	val;
float 	alpha,beta,delta;
float 	xc,yc;

	base	= (char *)frame.screen;
	bpr		= sPrepare.bpr;

	const float kCoef = M_PI/360;
	alpha = frame.meanFftHigh[0]*kCoef/2048;
	delta = frame.meanFftBass[0]*kCoef/2048;
	beta  = (alpha+delta)/2;

	meanalpha = (meanalpha+alpha)/2;
	meanbeta  = (meanbeta +beta)/2;
	meandelta = (meandelta+delta)/2;
	alpha -= meanalpha;
	beta  -= meanbeta;
	delta -= meandelta;
	alpha += frame.frame*M_PI/300.0;
	beta += frame.frame*M_PI/60.0;
	delta += frame.frame*M_PI/200.0;

	xc = 320-128*cos(pathangle)+64*sin(pathangle*3);
	yc = 240+64*cos(pathangle/2)+16*sin(pathangle*4*3);
	pathangle += M_PI/128;
	if (pathangle > 12*M_PI) pathangle -= 12*M_PI;

	// set up the cubes outside points
	#define K_COEF3 (2*4096)
	val = (frame.meanFftBass[0]+frame.meanFftBass[1])/K_COEF3;
	i=outside_point;
	shape->pt_z[i++] = -64*(K_S2+val);	val = (frame.meanFftBass[0]+frame.meanFftBass[2])/K_COEF3;
	shape->pt_x[i++] =  64*(K_S2+val);	val = (frame.meanFftBass[0]+frame.meanFftBass[3])/K_COEF3;
	shape->pt_z[i++] =  64*(K_S2+val);	val = (frame.meanFftBass[0]+frame.meanFftBass[4])/K_COEF3;
	shape->pt_x[i++] = -64*(K_S2+val);	val = (frame.meanFftBass[0]+frame.meanFftBass[5])/K_COEF3;
	shape->pt_y[i++] = -64*(K_S2+val);	val = (frame.meanFftBass[0]+frame.meanFftBass[6])/K_COEF3;
	shape->pt_y[i  ] =  64*(K_S2+val);

	t3d->setFrame(base, bpr);
	shape->draw(alpha,beta,delta, xc, yc);

	// frame.meanFftBass[0];

	//if (modifiers() & B_LEFT_SHIFT_KEY) a -= FOV/10.0;
	//if (modifiers() & B_CONTROL_KEY && speed < 16) speed++;

}  // end of processFrame8 for CSchmur3dFilter


//-----------------------------------------------------------------------



//*****************************
bool CScope3dFilter::load(void)
//*****************************
{
	sFilter.name = "Scope 3d";
	sFilter.author = "R'alf";
	sFilter.info = "Sample 3d code by B. S., Be Inc.";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionLast;
	sFilter.supportedMode = kColor8Bit;

	sFilter.icon = loadBitmap("./add-ons.icons/fft.tga");
	return init3d();
}  // end of load for CScope3dFilter


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


//********************************
bool CScope3dFilter::prepare(void)
//********************************
{
	mUseGk8 = sPrepare.isGameKit
			&& sPrepare.mode == kColor8Bit
			&& sPrepare.gameKitScreen != NULL
			&& line_8 != NULL;

	meanalpha=0;
	meanbeta=0;
	meandelta=0;
	pathangle = 0;
	t3d->useGk(mUseGk8, this);

	return true;
} // end of prepare for CScope3dFilter


//***************************************************
void CScope3dFilter::processFrame8(SFrameInfo &frame)
//***************************************************
{
char	*base;
long	bpr;
long	i;
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
		long val = (10+frame.meanFftBass[i]/8192)*64;
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
