/*****************************************************************************

	Projet	: FlippingWave

	Fichier	: flippingwave.cpp
	Partie	: AddOn

	Auteur	: B. T.

	1997	: BT
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, B. T. <B.@T..org>

*****************************************************************************/

/*****************************************************************************

	Projet	: Pulsar Sample AddOn

	Fichier: sample_addon.cpp
	Partie	: Add-ons

	Auteur	: RM
	Date	: 040597 -- version 0.2 (C funcs)
	Date	: 010797 -- version 0.3 (CFilter)
	Date	: 180997 -- version 0.3.x, updated the comments and the code to keep in sync
	Format	: tabs==4 for better viewing

	This Sample Add-on does nothing. It is just a skeleton that you may use
	for building add-on.

	The simple way to do this :
	- implement & export the C function "CFilter * filterInit(uint32 index)",
	  where  you simply create your CFilter derived class.
	- derive a class from CFilter, you should at least implement two methods
	- CFilter::load() must be implemented, this is where you will fill info about
	  your add-on, name, auhtor, etc. You DON'T need to call the inherited function,
	  it does nothing.
	- CFilter::processFrame8() is where you draw the output, by using the info about
	  the sound and the stream.

	You can enhance this usage by implementing the other methods :
	- unload() called just before the add-on image is unloaded, the next call will be
	  the destructor.
	- prepare() is called just before the stream goes on, thus after load() but before
	  processFrame(). You can cache some info, as you may need.
	- terminate() is called when the stream is stopped, thus after any processFrame()
	  but before unload().

	The function always get called like this :
	- filterInit(), called once
	- constructor(), called once
	- load(), called once
	- loop :
			- prepare()
					- loop : several processFrame8(), up to 60 or 90 Hz... (more if you're lucky)
			- terminate()
	- unload()
	- destructor()
	- image unloaded.

	
	!!!!!!!!!!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!
	>> In order to compile this, you need to get the Datatypes 1.6.3 package
	>> from BeWare to have the sources.
	>> Please go and surf on ftp.be.com/pub/contrib/lib or search into the
	>> BeWare page at http://www.be.com/beware and locate Datatypes163.zip,
	>> then unpack the archive by using the command line tool "unzip" or the
	>> Tracker add-on ExpandMe and go to BeIDE/Settings/Path and insert the
	>> correct path for the Datatype library sources.


*****************************************************************************/

#include "PP_Be.h"
#include "CFilter.h"
#include <math.h>

#undef MAX
#undef MIN
#define MAX( a, b ) ((a)<(b) ? (b) : (a))
#define MIN( a, b ) ((a)<(b) ? (a) : (b))
template <class T> inline T highest( T a, T b )
	{ return a<b ? b : a; }
template <class T> inline T lowest( T a, T b )
	{ return a<b ? a : b; }
#define clip( val, inf, sup ) ((val)>(sup)?(sup):( (val)<(inf)?(inf):(val) ))
#define clip16( val ) clip( val, -32768, 32767 )
#define clip18( val ) clip( val, -262144, 262143 )
#define clip20( val ) clip( val, -1048576, 1048575 )

#define PI M_PI

extern "C" void * filterInit(uint32 index);

#define ADDON_NAME "FlippingWave"

//--------------------------------------------------------------------

//***************************************
class FlippingWaveFilter : public CFilter
//***************************************
{
public:

	FlippingWaveFilter();
	virtual ~FlippingWaveFilter();

	virtual bool load();
	virtual bool prepare();
	virtual void processFrame8(SFrameInfo &info);
	virtual void terminate();
	virtual void unload();

private:
}; // end of class defs for FlippingWaveFilter


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


FlippingWaveFilter::FlippingWaveFilter()
	: CFilter()
{
}
FlippingWaveFilter::~FlippingWaveFilter()
{
}

//*****************************
void * filterInit(uint32 index)
//*****************************
/*
	This function is called when the add-on is loaded.
	This occurs when Pulsar is starting.
	This function is called several times with index=0 then 1 etc
	until it returns NULL. This give you the opportunity to code
	several filters in one add-on.

	Here you must create a new instance of your derived CFilter class.
	You return the pointer on the instance.
	You get called several times until you return NULL.
	You must return a valid object pointer, since the caller will verify
	using ClassInfo that the class is derived from CFilter but is not a
	CFilter instance.
*/
{

	// this add-on only declares ONE filter
	if (index > 0) return NULL;

	FlippingWaveFilter *info = new FlippingWaveFilter;
	if (!info) return NULL;		// memory error, give up

	// returns the instance -- the caller will check that this instance
	// is derived from CFilter but is _not_ a CFilter...
	//printf("new info %p, class %s\n", info, class_name(info));
	return (void *)info;
} // end of filterInit


//*********************************
bool FlippingWaveFilter::load(void)
//*********************************
/*
	Called just after the add-on has succesfully been loaded.
	If you return false, <the add-on will be unloaded immediatly
	and its unload() hook will be called>.
	Use this to set up the vital info about the filter, i.e. the
	strings (optional) and the supported color mode (currently fixed to 8 bits).
	This can also be done in you class constructor.
	The main difference with the constructor is that when you get here,
	the main program will have set up your main members, like the directories,
	the pointers onto buffers, etc. Also you can get back the index of the
	instance indicated by filterInit here. Check the CFilter.h for more info ;-)

	VALID STRUCTS : 'struct SLoadInfo sLoad'
	YOU MUST FIIL : 'struct SFilterInfo sFilter'
*/
{
	sFilter.name = "Flipping wave";		// <-- name that appears in the interface, keep unique and short
	sFilter.author = "BenStorm";
	sFilter.info = "Displays spectrum over waveform";
	sFilter.majorVersion = 0;				// useless for the moment
	sFilter.minorVersion = 1;
	sFilter.position = kFilterPositionAny;
	sFilter.supportedMode = kColor8Bit;

	// return if you accept to be load, if you return false you will be unloaded
	// for example an addon managing some kind of hardware (camera) can check that
	// the device is present here...
	return true;
}  // end of load for FlippingWaveFilter


//***********************************
void FlippingWaveFilter::unload(void)
//***********************************
/*
	Called just before the add-on is unloaded, when the
	program quits.

	VALID STRUCTS : 'struct SLoadInfo sLoad'
					'struct SFilterInfo sFilter'
*/
{
}  // end of unload for FlippingWaveFilter


//************************************
bool FlippingWaveFilter::prepare(void)
//************************************
/*
	Called when the user wants to make the view pulse.
	Gives a chance to the filter to alloc memory or compute
	some tables.

	Important : in the structure "sPrepare", you will find here information
	concerning the output that will *never* change : resolution, depth, BScreen,
	BWindowScreen (if you're in full screen mode), bytes per row, etc.
	Currently, you don't know the exact address of the frame buffer since this
	can change at each frame when using double or tripple buffering.
	
	Your add-on MUST NOT assume the size is fixed (currently, it's always 640x480,
	but this WON'T be always the case), and here you know this size so HERE YOU MUST
	ALLOCATE ARRAYS USING sPrepare.sx and sPrepare.sy.
	Size is given in pixels. If the 'sx' size is 640, it really means you can access
	pixels ranging from 0 to 639.

	VALID STRUCTS : 'struct SLoadInfo sLoad'
					'struct SFilterInfo sFilter'
					'struct SPrepareInfo sPrepare'
*/
{

	return true;
}  // end of prepare for FlippingWaveFilter


//**************************************
void FlippingWaveFilter::terminate(void)
//**************************************
/*
	Called when the user wants to stop pulse the view.
	Give a chance to the filter to free memory.

	VALID STRUCTS : 'struct SLoadInfo sLoad'
					'struct SFilterInfo sFilter'
					'struct SPrepareInfo sPrepare'
*/
{

}  // end of terminate for FlippingWaveFilter


//*******************************************************
void FlippingWaveFilter::processFrame8(SFrameInfo &frame)
//*******************************************************
/*
	Real blit into the screen.
	Look at other add-ons to have an idea of what to do here, but below are the main cases.

	VALID STRUCTS : 'struct SLoadInfo sLoad'
					'struct SFilterInfo sFilter'
					'struct SPrepareInfo sPrepare'
					'struct SFrameInfo frame'
*/
{
//	printf("FlippingWaveFilter::processFrame\n");
//	bigtime_t time_begin = system_time( ), time_end;

#define N 256
#define C( x ) cos( (x)*PI/180.0 )
#define S( x ) sin( (x)*PI/180.0 )
	static uint8 colors[ ] = {
		// black to pure blue
		/*41, 40,*/ 39, 38, 37, 36, 35, 34, 33, 32,
		// blue to violet
		242, 236, 230, 224,
		// violet to red
		184, 185, 186, 187, 188, 42,
		// red to orange
		157, 125, 93,
		// orange to yellow
		101, 87, 81, 75, 69,
		// yellow to white
		250, 251, 252, 253, 254, 63
	};
#define N_COLORS sizeof( colors )/sizeof( uint8 )

	static float alpha = -140.0, speed = 360e-6/4.0, speed2 = 360e-6/1.0;
	static bool been_there = false;
	static bigtime_t last_date;
	if( !been_there ) {
		last_date = system_time( );
		been_there = true;
	}
	bigtime_t date = system_time( );

	float ix = 0.8*C( alpha ), iy = 0.5*S( alpha );
	float jx = 0.8*C( alpha+90.0 ), jy = 0.5*S( alpha+90.0 );

	alpha = -100.0+40.0*S( speed*( float )( date-last_date ) );

	//last_date = date;

	int bpr = sPrepare.bpr, w = sPrepare.sx, h = sPrepare.sy;
	uint8 *video = frame.screen;
	int ox = w/2, oy = h/2+48;
	int32 *wave = frame.rawHigh;
	float *spectrum = frame.fftBass+1;
	int yspans[ 256 ], *yspan = yspans;

	float offset = frame.meanFftBass[ 0 ]*( 1.0f/4000.0f )-20.0;
	double arg = speed2*( float )( date-last_date );
	ox += offset*C( arg );
	oy -= offset*S( arg );

	int max_yspan = 0;
	for( int i = 1; i<N; i ++ ) {
		int y = ( *spectrum ++ )*( 200.0f*1.0f/3000000.0f );
		y = MIN( y, 200 );
		*yspan ++ = y;
		if( y>max_yspan )
			max_yspan = y;
	}

	yspan = yspans;
	for( int i = 1; i<N; i ++ ) {
		float fi = 2*i-N, wa;
		int32 x, x2, y, y2;
		int ysp = *yspan ++, avg_span = max_yspan+ysp;
		uint8 *b, *b2;

		// first time domain sample
		wa = ( float )( *wave ++ )*( 64.0f*1.0f/32768.0f );
		x = ox+( int32 )( wa*ix+fi*jx );
		y = oy-( int32 )( wa*iy+fi*jy );
		b = video+x;
		// same spectrum component, second time domain sample
		fi += 1.0f;
		wa = ( float )( *wave ++ )*( 64.0f*1.0f/32768.0f );
		x2 = ox+( int32 )( wa*ix+fi*jx );
		y2 = oy-( int32 )( wa*iy+fi*jy );
		b2 = video+x2;

		// left & right overspan
		if( x<0 || x>=w || x2<0 || x2>=w )
			continue;
		// of course as long as the wave is not left-right reversed we
		// could break when x>=w...

		int over;
		// bottom of screen overspan
		over = MAX( y, y2 )-( h-1 );
		if( over>=0 ) {
			y -= over;
			y2 -= over;
			ysp -= over;
		}
		// top of screen overspan
		over = MIN( y, y2 )-ysp;
		if( over<0 )
			ysp += over;

		b += bpr*y;
		b2 += bpr*y2;
		for( int j = 0; j<=ysp; j ++ ) {
			int c = j*( N_COLORS-1 )*2/avg_span;
			uint8 color = colors[ c ];
			*b = color;
			*b2 = color;
			b -= bpr;
			b2 -= bpr;
		}
	}

//	time_end = system_time( );
//	printf("FlippingWaveFilter::processFrame %Ld µs\n",time_end-time_begin);
}  // end of processFrame for FlippingWaveFilter


// eoc
