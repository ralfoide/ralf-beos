/*************************************************************************

New Header format :

	Projet	: Portable Lib

	Fichier	: bRBlitWindow.h
	Partie	: base class for a wrapper around BWindow

	Auteur	: RM
	Date		: 210698
	Format	: tabs==2

*************************************************************************/

#ifndef _H_BRBLITWINDOW_
#define _H_BRBLITWINDOW_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RWindow.h"
#include "TRotateBuffer.h"
#include "RBitmap.h"
#include "RThread.h"

#include <View.h>

//---------------------------------------------------------------------------
// bitmap buffer shared with the display window

typedef TRotateBuffer<RBitmap *>	RBitmapBuffer;

// default "frame rate" is 25 fps (= 40000 microseconds)
#define K_BLITWIN_FRAME_RATE		25.0


//------------------------------------------------------------------------
// RBlitView class

//****************************
class RBlitView : public BView
//****************************
{
public:
	RBlitView(BRect frame, const char *name="RBlitView", uint32 resizeMask=B_FOLLOW_ALL)
		: BView(frame, name, resizeMask, B_WILL_DRAW)
		{ mBounds = Bounds(); SetViewColor(B_TRANSPARENT_32_BIT); mMap = NULL; }
	virtual void Draw(BRect r) { if (mMap) DrawBitmap(mMap, BPoint(0,0)); else FillRect(r); }
	void	update(BBitmap *map) { mMap = map; if (Window()->Lock()) { Draw(mBounds); Window()->Unlock(); }}
private:
	BRect		 mBounds;
	BBitmap *mMap;
}; // end of RBlitView


//------------------------------------------------------------------------
// RBlitWindow class

//*********************************************************
class RBlitWindow : public RThread, virtual public RWindow
//*********************************************************
{
public:
	RBlitWindow(RRect frame, sptr title, rbool show=true);
	virtual void deleteWindow(void);				// use this instead of ~RBlitWindow
	virtual ~RBlitWindow(void);							// never call this directly
	virtual void msgQuitRequested(void);		// from RWindow
	
	// services

	void	setFrameRate(fp64 hertz);
	void	setBitmapBuffer(RBitmapBuffer *buffer);

	// void getMouse(x, y, buttons)
	void	getScreenMouseLoc(ui32 &x, ui32 &y);	// hack

	// --- variables protegees ---
protected:

	virtual void run(void);								// from RThread

	fp64	mFpsTempo;				// 1/frame_rate_hz in microseconds
	rbool	mRunning;
	RBitmapBuffer	*mBitmapBuffer;

	// -- pure internal beos-specific stuff --
private:

	RBlitView	*mView;

}; // end of class RBlitWindow


//------------------------------------------------------------------------

#endif // of RLIB_BEOS
#endif // of _H_BRBLITWINDOW_

//------------------------------- eoh ------------------------------------

