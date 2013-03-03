/*****************************************************************************

	Projet	: Pulsar Sample Demicercle AddOn

	Fichier	: sample_addon.cpp
	Partie	: End

	Auteur	: RM
	Date		: 040597 -- version 0.2 (C funcs)
	Date		: 010797 -- version 0.3 (CFilter)
	Format	: tabs==2

*****************************************************************************/


#include "PP_Be.h"
#include "CFilter.h"
#include "fastmath.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" CFilter * filterInit(uint32 index);

#define ADDON_NAME "DemiCercle"

//--------------------------------------------------------------------

//****************************************
class CSampleCercleFilter : public CFilter
//****************************************
{
public:

	CSampleCercleFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CSampleCercleFilter(void) 		{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void) { return true; } // don't miss that call !
	virtual void processFrame8(SFrameInfo &frame);

}; // end of class defs for CSampleCercleFilter


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


//********************************
CFilter * filterInit(uint32 index)
//********************************
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

	CSampleCercleFilter *info = new CSampleCercleFilter;
	if (!info) return NULL;		// memory error, give up

	// returns the instance -- the caller will check that this instance
	// is derived from CFilter but is _not_ a CFilter...
	return info;
} // end of filterInit


//**********************************
bool CSampleCercleFilter::load(void)
//**********************************
/*
	Called just after the add-on has succesfully been loaded.
	If you return false, <the add-on will be unloaded immediatly
	and its unload() hook will be called>.
	Use this to set up the vital info about the filter, i.e. the
	strings (optional) and the supported color mode.
	This can also be done in you class constructor.
	The main difference with the constructor is that when you get here,
	the main program will have set up your main members, like the directories,
	the pointers onto buffers, etc. Also you can get back the index of the
	instance indicated by filterInit here. Check the CFilter.h for more info ;-)
*/
{
	sFilter.name = "DemiCercle";
	sFilter.author = "R'alf";
	sFilter.info = "Sample code";
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

	const float inc_pi = 3.1415926538/2/64;
	float angle=0;
	for(i=0; i<64; i++, angle+=inc_pi)
	{
		float rx = inl_b_cos_90(angle);
		float ry = inl_b_cos_90(3.1415926538/2-angle);
		long x1=150+128*rx;
		y1=     200-128*ry;
		y2=chanL[i]/16384;
		long x2=150+(128-y2)*rx;
		y2=     200-(128-y2)*ry;

		blitLine8(x1,y1,x2,y2,i+nframe);
	}

	for(angle=0, i=0; i<64; i++, angle+=inc_pi)
	{
		float rx = inl_b_cos_90(angle);
		float ry = inl_b_cos_90(3.1415926538/2-angle);
		long x1=450+50*rx;
		y1=     300-50*ry;
		y2=chanR[i]/16384;
		long x2=450+(60+y2)*rx;
		y2=     300-(60+y2)*ry;

		blitLine8(x1,y1,x2,y2,i+nframe);
	}

}  // end of processFrame8 for CSampleCercleFilter


// eoc
