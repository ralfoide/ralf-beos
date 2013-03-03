/************************************************************
	Author	: Jonathan Perret
	1997	: JP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Jonathan Perret <jperret@cybercable.fr>

************************************************************/

#include "PP_Be.h"
#include <math.h>

//#define DEBUG 1
#include <support/Debug.h>

#include "bigfft_addon.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" void *filterInit(uint32 index);

//---------------------------------------------------------


// filter-specific info that can be in a constant
struct MyFilterInfo
{
	char *name;			// name of filter
	char *author;		// author (name, email... one line)
	char *info;			// one line of extra info
	uint32	majorVersion;
	uint32	minorVersion;

	bool			eraseScreen;
	EFilterPosition position;				// defaults to kFilterPositionAny
	EColorSpace 	supportedMode;			// kColor8Bit
	
	void FillSFilterInfo(SFilterInfo& sFilter) 
	{
		sFilter.name = name;
		sFilter.author = author;
		sFilter.info = info;
		sFilter.majorVersion = majorVersion;
		sFilter.minorVersion = minorVersion;
		sFilter.eraseScreen = eraseScreen;
		sFilter.position = position;
		sFilter.supportedMode = supportedMode;
	};
	
};

static MyFilterInfo bigfft_info =
{
	"Big FFT", "Jonathan Perret <jperret@cybercable.fr>",
	"Background FFT bars", // extra info
	0,1, // version
	false, // eraseScreen
	kFilterPositionAny,   // position
	kColor8Bit // supportedMode
};


//--------------------------------------------------------------------


//********************
int main(int, char **)
//********************
// Code to help a little the fellow user double-clicking the
// add-on icon directly.
{
	#define ADDON_NAME "BigFFT"
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


void *filterInit(uint32 index)
{
	switch(index)
	{
		case 0:	return new bigfft_addon;
	}
	return 0;
}

//--------------------------------------------------------------------


bigfft_addon::bigfft_addon()
: CFilter()
{
}

bool bigfft_addon::load()
{
//	(new BAlert(0,"load",
//		"clik"))->Go();

	bigfft_info.FillSFilterInfo(sFilter);
	
	// let's load
	BFile *rfile = new BFile(&sLoad.addonEntry,B_READ_ONLY);
	BResources *res = new BResources(rfile);
	
	size_t datalength;
	
	void *icon_data = 
		res->FindResource('ICON',"BEOS:L:STD_ICON",&datalength);
	ASSERT_WITH_MESSAGE(datalength==1024,"failed to load the icon");
	
	if(!icon_data || datalength!=1024)
		sFilter.icon = 0;
	else {
		sFilter.icon = new BBitmap(BRect(0,0,31,31),B_COLOR_8_BIT);
		sFilter.icon->SetBits(icon_data,datalength,0,B_COLOR_8_BIT);
		free(icon_data);
		}	

	return true;
}

bool bigfft_addon::prepare()
{
	fBounds.Set(0,0,sPrepare.sx-1,sPrepare.sy-1);
		
	return true;
}

// The ugliest sorting routine ever written by man ;-)
// I can't even manage a bubble sort, shame on me :(((
template<class T>
void uglysort(T *src,T *dest,int n)
{
	bool *used=new bool[n];
	memset(used,0,n*sizeof(bool));

	for(int i=0;i<n;i++) {
		dest[i] = 1e999;
		int minpos = 0;
		// find unused minimum 
		for(int j=0;j<n;j++)
			if(!used[j] && src[j]<=dest[i]) {
				minpos = j;
				dest[i] = src[j];
				}
		used[minpos] = true;
		}
	
	delete used;
}

const double fudge = 0.5e-3;

void bigfft_addon::processFrame8(SFrameInfo &frame)
{
	uint32 prevx=-1,x,y;
	
//	double sortedFftBass[8];
//	
//	uglysort(frame.meanFftBass,sortedFftBass,8);
//	
//	for(int i=0;i<8;i++) {
//		y = fBounds.bottom-(sortedFftBass[i]*fudge);
//		Box(0,y,fBounds.right,y,59-i);
//		}
		
	for(int i=0;i<8;i++) {
		y = fBounds.bottom-(frame.meanFftBass[i]*fudge);
		x = (i+1)*sPrepare.sx/8-1;
		Box(prevx+1,y,x,fBounds.bottom,39-i);
		prevx = x;
		}
	
}

void bigfft_addon::terminate()
{
}

void bigfft_addon::unload()
{
}

// graphics support routines
#pragma mark -- graphics support --

void bigfft_addon::Box(BRect rect,uchar col)
{
	if (rect_8 && sPrepare.isGameKit) {
		int32 a=sFrame->startingLine;
		rect_8(int(rect.left),a+int(rect.top),
			int(rect.right),a+int(rect.bottom),col);
		}
	else {
		int y;
		uchar *offset;
		
		rect = rect & fBounds;
		
		offset=((uchar *)Bits())+int(rect.top)*BytesPerRow()+int(rect.left);
		for (y=0;y<=rect.IntegerHeight();y++) {
			memset(offset,col,rect.IntegerWidth()+1);
			offset+=BytesPerRow();
			}
		}

}


#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

// eoc
