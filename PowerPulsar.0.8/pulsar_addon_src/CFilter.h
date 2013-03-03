/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CFilter.h
	Partie	: Add-ons for filters

	Auteur	: RM
	Date	: 040900 -- version 0.4 (just make comments accurate)
	Date	: 010797 -- version 0.3 (CFilter)
	Format	: tabs==2

	Explanation of add-ons :
	Filters in Pulsar are add-ons that draw stuff on the screen based on the
	sound buffers or the real time ffts of the sound.
	Pulsar manages two kind of buffers : 
	- 8-bit screen buffers,
	- 32-bit screen buffers.

	8-bit screen buffers use the BeOS system palette.


	Some sample add-ons are delivered with PowerPulsar. They may not be completly
	document (just bug me to do so, in that case !), yet they are good skelettons
	to start with.

	The simple way to do an add-on is as follow :
	- implement & export the C function "CFilter * filterInit(uint32 index)",
	  where you simply create your CFilter derived class.
	- derive a class from CFilter, you should at least implement two methods,
	  namely load() and either processFrame8() or processFrame32().
	- CFilter::load() must be implemented, this is where you will fill info about
	  your add-on, name, auhtor, etc. You DON'T need to call the inherited function,
	  it does nothing. The most important in load() is that you set a flag value,
	  which tells if your addon can use 8 or 32 bpp screens, or both. If your add-on
	  DOES NOT SET this flag to something valid, it will never be called !
	- CFilter::processFrame8/32() is where you draw the output, by using the info about
	  the sound and the stream. You receive in input a structure with all the info
	  you should need (and probably more).

	You can enhance this usage by implementing the other methods :
	- unload() is called just before the add-on image is unloaded, the next call will be
	  the destructor of each CFilter instance.
	- prepare() is called just before the stream goes on, thus after load() but before
	  processFrame8/32(). You can cache some info, as you may need. You receive information
	  on what's going to happen and you return a bool value indicating you accept playing
	  or not. If the addon returns false, it won't be called again until the user stops
	  and starts again.
	- terminate() is called when the stream is stopped, thus after any processFrame8/32()
	  but before unload().

	Important: during the lifetime of a CFilter-derived instance, load() will be called
	once. Then when the user starts the play, prepare() will be called once, followd
	by a bunch of calls to processFrame8/32() and finally one call to terminate(). When the
	app quits or the user removes the add-on from the track list, unload() will be called
	once and immediately afterwards the CFilter instance will be destroyed.
	It is important to notice that the tupple prepare-processFrames*N-terminate will be
	called several times during the lifetime of the instance, namely for each play.

	Important-2: the CFilter instance *owns the SLoadInfo and SPrepareInfo. It is
	considered a bad idea to modify these values (they are filled by PowerPulsar for the
	add-on, never read back). SPrepareInfo is pretty important because it contains, among
	other things, the size of the output screen. Your add-on MUST NOT suppose that the
	screen will be at a fixed size ! In the current PowerPulsar version, the screen
	size is limited to 640x480, but I plan to change this is later versions. Of course it
	won't be modifiable on the fly, i.e. not when a play is occuring.
	Also your add-on MUST support the BPR (bytes per row) of the screen (info also
	available in SPrepareInfo), as video memory need not be exactly aligned on the
	screen width.

	Important-3: think cross-platform ! BeOS supposedly runs under PowerPC too
	which is a Big-Endian architecture, whilst Intel is Little-Endian. PowerPulsar was
	original written on a BeBox (PPC, Big-Endian) and ported later to BeOS Intel with
	few minor modifications. You can easily make your code endian-free too !
	Of course when you want to deal with bytes, you might want to just implement two
	cases with a #ifdef (use #ifdef __INTEL__ in this case, it works OK).
	Note that the ARGB order for 32-bit buffers is reversed for BeOS on PowerMacs.
	(I no longer have my BeBox with me do test this, but anyway 32-bit buffer on my
	BeBox 133 with a Millenium 2 were just practically too slow to be usable !)

	[new to 030797]
	EFilterPosition preferredPosition :
	the filter defines this in order to let the program know that it want to be
	one of first or last called filters. The default value is kFilterPositionAny.
	This is mainly used in the interface rules for guiding the user in the choice of
	its filter order. But, this is not a strict guideline, which means that several
	filters of kind kFilterPositionFirst can be found at the start of the stream; but
	typcally you won't find them after a kFilterPositionAny or kFilterPositionLast.
	Two main rules here :
	- the filters in the list are mandatory in the order kFilterPositionFirst, then
	  kFilterPositionAny then kFilterPositionLast.
	- if the first filter is a kFilterPositionFirst, then this filter HAS the obligation
	  of filling the whole screen or at least erasing it. Which means that in that case
	  the program won't erase the screen, the filter is assumed to do it.
	- if the first filter is NOT a kFilterPositionFirst, the screen will be filled with
	  black.

	[new to 030797 !! the whole mess has been cleanified]
	The structure of the CFilter changes even before I release it !
	There are now sections in it :
	- struct SFilterInfo : info filled by the add-on to let the PowerPulsar have
	  information about the CFilter instance.
	- struct SLoadInfo : info that Pulsar gives to the add-on, this struct
	  is filled between the call to the constructor and the call to the load()
	  method. The add-on can read these, they won't change during its whole life.
	- struct SPrepareInfo : info filled by Pulsar before preare() gets called.
	  These info won't change between the prepare() and the terminate() call, but they
	  might changed between two prepare() call.
	- struct SFrameInfo : info filled by Pulsar just before calling processFrame().
	  This will hopefully change at every frame, which includes the address of the
	  frame buffer or the input sound or fft buffers -- yes, these might change too, since
	  I'm implementing double buffering there...

	The methods always get called like this :
	- filterInit(), called once
	- constructor(), called once
	- [SLoadInfo is filled here]
	- load(), called once
	- loop :
	   - [SPrepareInfo is filled here}
			- prepare()
					- loop : up to 60 or 90 Hz... (machine dependant)
					   - [SFrameInfo is filled here]
					   - several processFrame(), 
			- terminate()
	- unload()
	- destructor()
	- image unloaded.

*****************************************************************************/

#ifndef _H_CFILTER_
#define _H_CFILTER_

#ifndef _PP_CF_INC
#include <OS.h>
#include <image.h>
#include <GameKit.h>
#include <Path.h>
#include <Directory.h>
#endif

#ifdef _PP_CF_INC
	#pragma export on
	#define _CFILTER_IMPEXP
#endif

//---------------------------------------------------------------------------
// this is the only function that the add-on must declare
// the function received the index from 0 to n, at it must returns
// a new CFilter derived class if it wants to define a new filter or
// NULL when it wishes to stop.

class CFilter;

#ifdef __INTEL__
	extern "C"
	{
#endif
typedef void *(* filterInitProto) (uint32 index);
#ifdef __INTEL__
	}
#endif


// -- dummy protos for the graphic card hooks (inspired from DR8.2, huh?!!)

typedef long (*line_8_proto)(long x1, long x2, long y1, long y2, uchar color,
													   bool useClip, short clipLeft, short clipTop, short clipRight, short clipBottom);
typedef long (*line_32_proto)(long x1, long x2, long y1, long y2, ulong color,
															bool useClip, short clipLeft, short clipTop, short clipRight, short clipBottom);
typedef long (*rect_8_proto)(long x1, long y1, long x2, long y2, uchar color);
typedef long (*rect_32_proto)(long x1, long y1, long x2, long y2, ulong color);
typedef long (*blit_proto)(long x1, long y1, long x2, long y2, long width, long height);
typedef long (*invert_rect_32_proto)(long x1, long y1, long x2, long y2);

/*
Index hook 8
long draw_adraw_array_with_8_bit_depth(indexed_color_line *array, long numItems, 
         bool clipToRect, short clipLeft, short clipTop, 
         short clipRight, short clipBottom)

Index hook 9
long draw_adraw_array_with_32_bit_depth(rgb_color_line *array, long numItems, 
         bool clipToRect, short clipLeft, short clipTop, 
         short clipRight, short clipBottom)
*/

#ifdef __POWERPC__			// PPC
	#define K_ALPHA_MASK 		0x000000FF
	#define K_ALPHA_OFFSET	0
	#define K_RED_OFFSET		8
	#define K_GREEN_OFFSET	16
	#define K_BLUE_OFFSET		24
#else										// x86
	#define K_ALPHA_MASK 		0xFF000000
	#define K_ALPHA_OFFSET	24
	#define K_RED_OFFSET		16
	#define K_GREEN_OFFSET	8
	#define K_BLUE_OFFSET		0
#endif

#define K_ALPHA_OPAQUE		0xFF
#define K_ALPHA_TRANSP		0x00


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -- SFilterInfo --

//******************
// EFilterPosition is no longer an enum but a float.
// Position 0.0 means at the beginning of the stream,
// Position 1.0 means at the end of the stream. 
// Filter that don't care should use the intermediate 0.5 position.
// Filter that WANT to be at first position, i.e. 0.0 exactly (:) < epsilon...)
// will be asked to clear the display too.
// Currently very few support of this in the loader, really.
//******************
#define EFilterPosition 			float
#define kFilterPositionFirst	0.0
#define kFilterPositionAny		0.5
#define kFilterPositionLast		1.0

enum EColorSpace
{
	kColor8Bit	= 1,
	kRgb32Bit		= 2
};

//****************
struct SFilterInfo
//****************
{
	// -- info filled by the add-on, i.e. BY YOU !!
	// (NULL at load time)

	char 		*name;									// name of filter
	char 		*author;								// author (name, email... one line)
	char 		*info;									// one line of extra info
	uint32	majorVersion;
	uint32	minorVersion;
	BBitmap	*icon;									// BRect(0,0,31,31) please...

	bool						hasSettings;		// set it to true if you can create a private view for settings
	bool						eraseScreen;		// set it to true if you fill the whole screen by yourself
	EFilterPosition position;				// defaults to kFilterPositionAny
	EColorSpace 		supportedMode;	// kColor8Bit or/xor kRgb32Bit
	BMessage				modelParam;			// a model of params for this filter
																	// add your "parameters" template to this one

	// -- for the private usage of add-on
	// (NULL at load time - Pulsar won't even look at this,
	//  this is a reminence from C extensions on the Macintosh...)
	// (class derivation makes this useless.)
	void 		*user;

}; // end of SFilterInfo


//---------------------------------------------------------------------------
#pragma mark -- SLoadInfo --


//**************
struct SLoadInfo
//**************
{
	// index received by filterInit() when it created this instance
	uint32 		index;

	// minor/major version of Pulsar that the add-on can support...
	// The way I handle this for the moment just sucks. Give me a hint !
	uint32 		majorPulsarVersion;
	uint32 		minorPulsarVersion;
	uint32 		quanticPulsarVersion;

	// -- info set by Pulsar
	// (you should not modify, consider them as being private)
	// (you can read iid & addonEntry if you like, and use them to get interesting
	//  things like getting back resource or attributes from the file... but never try
	//  to modify them.)
	bool			enabled;
	bool			running;	
	image_id	iid;							// the image ID of this filter
	entry_ref	addonEntry;				// the entry ref of this add-on file

	// more usefull stuff -- please read only !
	BPath			dirApplication;		// a short hand for "./"
	BPath			dirAddon;					// and a one for "./add-ons"
	BPath			dirImg;						// and another one for "./img"
	// (you are supposed to put _your_ images in ./img and I put mines in ./img_pulsar,
	//  and please don't mix...)

	// Use this BMessage if you want to manage your own settings.
	// This message is saved in the app's prefs if any and will be restored
	// next time the addon is loaded by the app
	// [RM 060100 --> TBDL, this is just a placeholder, the msg is not yet saved/reloaded] 
	BMessage 	mAddonSetting;

}; // end of SLoadInfo


//---------------------------------------------------------------------------
#pragma mark -- SPrepareInfo --


//*****************
struct SPrepareInfo
//*****************
{
	// -- fixed info about play
	// (these are fixed before prepare() gets called)
	int32	sx;
	int32	sy;
	int32	sxy;						// equals sx*sy for fast memsets...
	bool	isGameKit;

	EColorSpace mode;			// kColor8Bit or kRgb32Bit
	int32				bpr;			// can be != sx

	BMessage	*param;			// the params that will be transmitted at the first frame
												// allocated before prepare() is called, there is one param
												// per track-line containing the same add-on

	// stuff valid only when isGameKit is true ;
	// In non game kit mode, these variables are truly undefined.

	BWindowScreen *gameKitScreen;	// the current instance of the Game kit. You can grab
																// here lots of interesting information on the screen.
	int32 maxVideoLine;						// number of 640-pixel wide line that can be allocated
																// in the video memory using the game kit.
	int32 pulsarVideoLine;				// number of lines reserved by Pulsar for double-buffering.

	// note : any video memory left between pulsarVideoLine and maxVideoLine can be
	// used by the add-ons. Pulsar won't touch that or consider it being a bug.
	// Currently, pulsarVideoLine is fixed to 960 (480*2 for double buferring) and
	// maxVideoLine is incorrectly set to 960 too. Ask the graphic driver by yourself
	// to check the real amount of memory.

	// access to the Be palette
	rgb_color	colorList[256];
	uint32		colorMap[256];		// ARGB mode (cf. K_A/R/G/B_OFFSET defines above)

}; // end of SPrepareInfo


//---------------------------------------------------------------------------
#pragma mark -- SFrameInfo --


//***************
struct SFrameInfo
//***************
{
	SFrameInfo(void)
	{	screen = NULL; rawHigh = NULL; rawBass = NULL; fftHigh = NULL; fftBass = NULL;
		averageFps = 0.0; lastFps = 0.0; frame = 0; startingLine = 0;
		averageFftHigh = 0.0; averageFftBass = 0.0; param=NULL;
		paramChanged = false; blockStart=false; blockIndex=0;
		for(long i=0;i<8;i++) {meanFftHigh[i]=0.0; meanFftBass[i]=0.0;}
	}

	// -- informations that change at every frame
	// this structure is received in processFrame8/32()

	double		currentTime;		// current time from start in milliseconds
	bool			paramChanged;		// the BMessage *param has been modified since last call
	bool			blockStart;			// a new "repeat" or "handfree" block has begun
	uint32		blockIndex;			// index of block in the track
	BMessage	*param;					// params that change at each frame -- same than sPrepare.param ptr

	uint8		*screen;					// base pointer of screen
	uint16 		startingLine;		// valid for Game Kit only, the top line of the current draw buffer

	// -- dynamic info about play
	// -- raw samples are mono, 16 bits each, stored on int32 for best usage
	// -- raw buffers are 512 * int32 wide.
	// -- fft are computed on 512 points. Only the 256 first points are interesting.
	int32		*rawHigh;					// 512 samples from the mono input 44100 Hz channel
	int32		*rawBass;					// 512 samples with average of each 16 samples from the above one
	float		*fftHigh;					// a 512-points FFT from the rawHigh buffer
	float		*fftBass;					// a 512-points FFT from the rawBass buffer

	// -- average values (computed from the FFT at each frame)

	double		averageFftHigh;	// the average of every first 256-points of the high fft
	double		averageFftBass;	// the average of every first 256-points of the bass fft
	double		meanFftHigh[8];	// the average of 8 first 32-points of the high fft
	double		meanFftBass[8];	// the average of 8 first 32-points of the bass fft

	double		averageFps;			// currently not computed
	double		lastFps;				// the current instantaneous fps
	int32			frame;					// frame counter, reset before prepare() gets called

}; // end of SFrameInfo


//---------------------------------------------------------------------------
#pragma mark -- class CFilter --


//***********
class CFilter
//***********
{
public:
	inline CFilter(void);
	virtual inline ~CFilter(void);

	// info filled by the add-on in load() :
	SFilterInfo		sFilter;

	// help routines for filling sFilter -- see comments under SFilterInfo
	void setName  (char *_text)							{ sFilter.name	= _text; 				}
	void setAuthor(char *_text)							{ sFilter.author= _text;				}
	void setInfo  (char *_text)							{ sFilter.info	= _text;				}
	void setVersion(uint32 major, uint32 minor)
																					{ sFilter.majorVersion=major;
																						sFilter.minorVersion=minor; 	}
	void setIcon	(BBitmap *ic)							{ sFilter.icon 	= ic; 					}
	void setPosition(EFilterPosition pos)		{ sFilter.position = pos;				}
	void setSupportedMode(EColorSpace mode)	{ sFilter.supportedMode = mode;	}
	void setHasSettings	(bool b)						{ sFilter.hasSettings = b;			}
	void willEraseScreen(bool b)						{ sFilter.eraseScreen = b; 			}


	// info filled by Pulsar for the add-on :
	SLoadInfo			sLoad;
	SPrepareInfo	sPrepare;
	SFrameInfo		*sFrame;			// same that processFrame8/32() argument. Valid only after a prepare() call.

	// -- callback functions

	virtual bool 	load		  			(void)										{return false;}
	virtual bool 	prepare		  		(void)										{return false;}
	virtual void 	processFrame8 	(SFrameInfo & /*frame*/)	{ }
	virtual void 	processFrame32	(SFrameInfo & /*frame*/)	{ }
	virtual void 	terminate	  		(void)										{ }
	virtual void 	unload		  		(void)										{ }
	virtual void 	paramChanged 		(void)										{ }	// reserved for future use -- trash this line
	virtual BView *	settingsOpened(void)										{ return NULL; }
	virtual void	settingsClosed	(void)										{ }

	// [RM 060100] handleMessage receives messages forwared by the rendering window's MessageReceived
	// The callback must return true if it takes care of the msg, and false if the msg has to be
	// passed to the next filter or back to the app.
	// Note that you must be careful to *at least* not handle one of the keys used by pulsar to exit
	// the fullscreen mode (typically ESC).

	virtual	bool	handleMessage		(BMessage *msg)					{ return false; }

	// -- some service calls

	// draws stuff in the current 8 bpp buffer (using bresenham or others)
	// call the game kit hooks when enabled
	void blitLine8 (long x1, long y1, long x2, long y2, uchar couleur);
	void blitLine32(long x1, long y1, long x2, long y2, ulong couleur);
	void blitRect8 (long x1, long y1, long x2, long y2, uchar couleur);
	void blitRect32(long x1, long y1, long x2, long y2, ulong couleur);
	//void blitInvert8(long x1, long y1, long x2, long y2, uchar couleur); - RSN
	//void blitInvert16(long x1, long y1, long x2, long y2, uchar couleur); - RSN
	// line array -- RSN

	// find a directory by name under the Application directory
	bool findDir(BDirectory *dir, char *name);

	// use datatypes to open the 'name' image in the img directory
	BBitmap *loadBitmap(char *name);

	// transforms a 32 RGB bitmap into 8 bit
	BBitmap *makeBitmap8Bit(BBitmap *source);

	// use mkimghrd from bedevsupport (aka W. Adams, cf BeNewsLetter 77)
	// and use this to transform the icon from the C header to a BBitmap
	BBitmap * makeIconFromHeader(int width, int height,
															color_space cspace,
															int bytesperpixel,
															unsigned char *bits);

	// hooks -- also set up from prepare().
	// hook from the graphic card driver, ONLY DEFINED in GAME KIT MODE !!!
	// these are for conveniance only, not all drivers defined them.
	// Undefined hooks are set to NULL, enjoy !!
	// Also note that the game kit allow hooks to change when ScreenConnected()
	// is called due to a workspace change. I don't support that, hooks are only
	// checked once before prepare() is called.
	line_8_proto					line_8;					// Graphic Driver Hook 3
	line_32_proto					line_32;				// Graphic Driver Hook 4
	rect_8_proto					rect_8;					// Graphic Driver Hook 5
	rect_32_proto					rect_32;				// Graphic Driver Hook 6
	blit_proto						blit;						// Graphic Driver Hook 7
	invert_rect_32_proto	invert_rect_32;	// Graphic Driver Hook 11

	// -- fake BBitmap-like methods
	//    (Bits() use SFrameInfo stuff and thus must only be called from
	//    inside a valid processFrame8/32() call)
	
	inline	void				*Bits() 				{ return sFrame->screen;	}
	inline	int32				BitsLength()		{ return sPrepare.sxy; 		}
	inline	int32				BytesPerRow()		{ return sPrepare.bpr;		}
	inline	EColorSpace	ColorSpace()		{ return sPrepare.mode;		}
	inline	BRect				Bounds()				{ return BRect(0,0,sPrepare.sx-1,sPrepare.sy-1); }


private:

	virtual	void	_Reserved1(){}
	virtual	void	_Reserved2(){}
	virtual	void	_Reserved3(){}
	virtual	void	_Reserved4(){}

	uint32				_reserved[4];

}; // end of class defs for CFilter


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -

// inline definitions

//***********************************
inline CFilter::CFilter(void)
//***********************************
{
	// -- SFilterInfo stuff

	sFilter.name = NULL;
	sFilter.author = NULL;
	sFilter.info = NULL;
	sFilter.icon = NULL;
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 0;
	sFilter.icon = NULL;

	sFilter.hasSettings = false;
	sFilter.eraseScreen = false;
	sFilter.position = kFilterPositionAny;
	sFilter.supportedMode = (EColorSpace) (kColor8Bit | kRgb32Bit);
	// BList	params; -- empty at first do params.AddItem(new SFilterParam)...

	sFilter.user = NULL;	// here is exception : the only time Pulsar touch this is here.

	// -- SLoadInfo stuff

	sLoad.index = 0;
	sLoad.majorPulsarVersion=0;
	sLoad.minorPulsarVersion=0;

	sLoad.enabled = false;
	sLoad.running = false;

	// -- SPrepareInfo stuff

	sPrepare.sx = 640;
	sPrepare.sy = 480;
	sPrepare.bpr = 640;
	sPrepare.sxy = sPrepare.bpr*sPrepare.sy;
	sPrepare.isGameKit = false;
	sPrepare.mode = kColor8Bit;
	sPrepare.gameKitScreen = NULL;
	sPrepare.maxVideoLine = 0;
	sPrepare.pulsarVideoLine = 0;
	sPrepare.param = 0;

	// frame stuff
	sFrame = NULL;

	// -- define default graphic card hooks ;-)
	line_8 = NULL;
	line_32 = NULL;
	rect_8 = NULL;
	rect_32 = NULL;
	blit = NULL;
	invert_rect_32 = NULL;
} // end of inline constructor for SFilterInfo


//************************************
inline CFilter::~CFilter(void)
//************************************
/*
	Warning : doesn't automagically deletes non-null ptrs.
	I usually do that in my code since I reset deleted ptrs to NULL but I can't
	rely on third-parties to do that in a clean way.
*/
{
	// -- SFilterInfo stuff

	sFilter.name = NULL;
	sFilter.author = NULL;
	sFilter.info = NULL;
	sFilter.icon = NULL;
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 0;
	sFilter.icon = NULL;

	// sFilter.user = NULL; <-- never do that, it's the add-on specific stuff

	// -- SLoadInfo stuff
	
	// no NULL ptrs here...

	// -- SPrepareInfo stuff

	sPrepare.gameKitScreen = NULL;
	sPrepare.maxVideoLine = 0;
	sPrepare.pulsarVideoLine = 0;

	// frame stuff
	sFrame = NULL;

} // end of inline destructor for SFilterInfo

//---------------------------------------------------------------------------

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

#endif // of _H_CFILTER_

// eoh
