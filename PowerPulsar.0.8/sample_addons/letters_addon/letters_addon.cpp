/*****************************************************************************

	Projet	: Pulsar Sample Letters AddOn

	Fichier	: letters_addon.cpp
	Partie	: End

	Auteur	: RM
	Date		: 100797
	Format	: tabs==2

*****************************************************************************/

#include "PP_Be.h"
#include "CFilter.h"
#include "alphabet.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" void * filterInit(uint32 index);

#define ADDON_NAME "Letters"

//--------------------------------------------------------------------

bool debug=false;

#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif

//--------------------------------------------------------------------

//**********************************
class CLetterFilter : public CFilter
//**********************************
{
public:

	CLetterFilter(void) : CFilter()	{ mLetter = 0; mGray = 0; }
	virtual ~CLetterFilter(void) 		{ /* nothing */ }

	virtual bool	load(void);
	virtual bool	prepare(void);
	virtual void	processFrame8(SFrameInfo &frame);
	virtual void	processFrame32(SFrameInfo &frame);
	virtual	bool	handleMessage(BMessage *msg);
	virtual void	terminate(void);
	virtual void	unload();

private:

	uchar		mLetter;		// 0 if none
	int32		mGray;			// 31->0
	int32		mPosX, mPosY;

}; // end of class defs for CLetterFilter


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

	CLetterFilter *info = new CLetterFilter;
	if (!info) return NULL;		// memory error, give up

	// returns the instance -- the caller will check that this instance
	// is derived from CFilter but is _not_ a CFilter...
	return info;
} // end of filterInit


//****************************
bool CLetterFilter::load(void)
//****************************
{
	sFilter.name 		= "Img Letter";
	sFilter.author	= "R'alf";
	sFilter.info 		= "Sample code";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionAny+0.8;
	sFilter.supportedMode = (EColorSpace)(kColor8Bit | kRgb32Bit);

	return true;
}  // end of load for CLetterFilter


//******************************
void CLetterFilter::unload(void)
//******************************
{
	// wait_lock has already been deleted in terminate()
}


//*******************************
bool CLetterFilter::prepare(void)
//*******************************
{
	return true;
}


//*********************************
void CLetterFilter::terminate(void)
//*********************************
{
}


// --


//**********************************************
bool CLetterFilter::handleMessage(BMessage *msg)
//**********************************************
// Use the key down event if it's a letter and no modifier is pressed
// Return false when not handling an event, returns true otherwise.
{
	if (msg && msg->what == B_KEY_DOWN)
	{
		int32 mods = 0;
		int8  key  = 0;
		msg->FindInt8("byte", &key);
		msg->FindInt32("modifiers", &mods);

		if ((mods & (B_SHIFT_KEY | B_COMMAND_KEY | B_CONTROL_KEY | B_MENU_KEY | B_OPTION_KEY)) == 0)
		{
			if (key >= 'a' && key <= 'z')
			{
				mGray = 31;
				mLetter = key-'a'+'A';
				return true;
			}
			else if (key >= 'A' && key <= 'Z')
			{
				mGray = 31;
				mLetter = key;
				return true;
			}
		}
	}
	return false;
}


// --



//**************************************************
void CLetterFilter::processFrame8(SFrameInfo &frame)
//**************************************************
{
	if (!mGray || mLetter<'A' || mLetter>'Z') return;
	const char * lettre = alphabet + (mLetter-'A') * (LX*LY);
 
	#define KX 16		// width of squares
	#define KY 16		// height of squares

	if (mGray >= 31)
	{
		mPosX = (int32)(((double)rand() / (double)RAND_MAX)*(sPrepare.sx - KX*LX));
		mPosY = (int32)(((double)rand() / (double)RAND_MAX)*(sPrepare.sy - KY*LY));
	}

	for(int32 j=0, y=mPosY; j<LY; j++, y+=KY)
		for(int32 i=0, x=mPosX; i<LX; i++, x+=KX)
			if (*(lettre++) != '.')
				blitRect8(x, y, x+KX-1, y+KY-1, mGray);
				
	mGray--;

}  // end of processFrame8


//***************************************************
void CLetterFilter::processFrame32(SFrameInfo &frame)
//***************************************************
{
	if (!mGray || mLetter<'A' || mLetter>'Z') return;
	const char *lettre = alphabet + (mLetter-'A') * (LX*LY);

	#define KX 16		// width of squares
	#define KY 16		// height of squares

	if (mGray >= 31)
	{
		mPosX = (int32)(((double)rand() / (double)RAND_MAX)*(sPrepare.sx - KX*LX));
		mPosY = (int32)(((double)rand() / (double)RAND_MAX)*(sPrepare.sy - KY*LY));
	}

	uint32 color = sPrepare.colorMap[mGray];

	for(int32 j=0, y=mPosY; j<LY; j++, y+=KY)
		for(int32 i=0, x=mPosX; i<LX; i++, x+=KX)
			if (*(lettre++) != '.')
				blitRect32(x, y, x+KX-1, y+KY-1, color);

	mGray--;

}  // end of processFrame32


// eoc
