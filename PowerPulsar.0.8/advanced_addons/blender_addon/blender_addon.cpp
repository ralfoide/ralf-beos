/*****************************************************************************

	Projet	: Pulsar Sample AddOn

	Fichier	: slider_addon.cpp
	Partie	: End

	Auteur	: RM
	Date	: 050300
	Format	: tabs==4

*****************************************************************************/

#include "PP_Be.h"
#include "CFilter.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" void * filterInit(uint32 index);

#define ADDON_NAME "Blender"

//--------------------------------------------------------------------


//***********************************
class CBlenderFilter : public CFilter
//***********************************
{
public:

	CBlenderFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CBlenderFilter(void) 		{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void);
	virtual void processFrame32(SFrameInfo &frame);
	virtual void terminate(void);
	virtual void unload();

}; // end of class defs for CBlenderFilter



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

	CBlenderFilter *info = new CBlenderFilter;
	if (!info) return NULL;		// memory error, give up

	// returns the instance -- the caller will check that this instance
	// is derived from CFilter but is _not_ a CFilter...
	return info;
} // end of filterInit


//*****************************
bool CBlenderFilter::load(void)
//*****************************
{
	sFilter.name = "Blender Addon";
	sFilter.author = "R'alf";
	sFilter.info = "Sample code";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionFirst;
	sFilter.supportedMode = kRgb32Bit;

	return true;
}  // end of load for CBlenderFilter





//*******************************
void CBlenderFilter::unload(void)
//*******************************
{
}


//********************************
bool CBlenderFilter::prepare(void)
//********************************
{
	return true;
}



//**********************************
void CBlenderFilter::terminate(void)
//**********************************
{
}

// --


//****************************************************
void CBlenderFilter::processFrame32(SFrameInfo &frame)
//****************************************************
{
	int32 sx = sPrepare.sx;
	int32 sy = sPrepare.sy;
	int32 bpr = sPrepare.bpr/sizeof(ulong);

	uint32 *base = (uint32 *)frame.screen;

	for(; sy>0; sy--, base += bpr)
		for(int32 x=sx; x>0; x--)
		{
			uint32 a = base[x];
			if (a) base[x] = (a>>1) & 0x7F7F7F7F;
		}

}  // end of processFrame




#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

// eoc
