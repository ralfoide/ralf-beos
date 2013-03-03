/*************************************************************************

New Header format :

	Projet	: Portable Lib

	Fichier	: bRBlitWindow.cpp
	Partie	: base class for a wrapper around BWindow

	Auteur	: RM
	Date		: 210698
	Format	: tabs==2

*************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RBlitWindow.h"
//#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"

//------------------------------------------------------------------------
// internal messages

#define K_MSG_BLITWIN_CHANGE_FPS	'BwCf'

//------------------------------------------------------------------------


//************************************************************************
RBlitWindow::RBlitWindow(RRect frame, sptr title, rbool show)
        		:RThread(),
        		 RWindow(frame, title)
//************************************************************************
{
	mFpsTempo = 0.0;
	mBitmapBuffer = NULL;
	mView = NULL;
	mRunning = false;

	// init
	setFrameRate(K_BLITWIN_FRAME_RATE);

	mView = new RBlitView(Bounds());
	M_ASSERT_PTR(mView);
	AddChild(mView);

	RWindow::init(show);

} // end of constructor for RBlitWindow


//************************************************************************
void RBlitWindow::deleteWindow(void)
//************************************************************************
{
	// ask the feeder RThread to exit
	askToKill();
	waitForEnd();

	// destroy the window and it's associated BeOS thread
	RWindow::deleteWindow();
} // end of deleteWindow for RBlitWindow


//***************************************************************************
RBlitWindow::~RBlitWindow(void)
//***************************************************************************
{
	// never to be used directly under BeOS

} // end of destructor for RBlitWindow


//************************************************************************
void RBlitWindow::msgQuitRequested(void)
//************************************************************************
{
	if (mControlThread)
		mControlThread->postMessage(K_MSG_WINDOW_QUIT_REQUESTED, 0, (vptr)this);
} // end of msgQuitRequested for RBlitWindow




//------------------------------------------------------------------------


//************************************************************************
void RBlitWindow::setBitmapBuffer(RBitmapBuffer *buffer)
//************************************************************************
{
	mBitmapBuffer = buffer;

	if (!mRunning)
	{
		launch();
		mRunning = true;
	}
} // end of setBitmapBuffer for RBlitWindow


//************************************************************************
void RBlitWindow::setFrameRate(fp64 hertz)
//************************************************************************
{
	if (hertz < K_EPSILON) hertz = K_BLITWIN_FRAME_RATE;
	mFpsTempo = 1e6/hertz;
	if (mRunning) selfPostMessage(K_MSG_BLITWIN_CHANGE_FPS);
} // end of setFrameRate for RBlitWindow


//************************************************************************
void RBlitWindow::run(void)
//************************************************************************
{
ui32 msg;
ui32 msgParam;
vptr msgData;
fp64 temps, deltatemps;
fp64 fpstempo = mFpsTempo;
RSem frameRateSem;
rbool loop = true;

	if (debug) printf("RBlitWindow::run -- start\n");

	frameRateSem.acquire();	// rend le semaphore bloquant

	try
	{
		while(loop)
		{
			temps = systemTime();
	
			//lecture message
			if (receiveMessage(msg, msgParam, msgData))
			{
				// process message
				switch(msg)
				{
					case K_MSG_RTHREAD_EXIT:
						loop = false;
						break;
					case K_MSG_BLITWIN_CHANGE_FPS:
						fpstempo = mFpsTempo;
						break;
				}
			}
	
			// process frame
			if(mBitmapBuffer)
			{
				RBitmap *offscreen;
			
				offscreen = mBitmapBuffer->acquireRead();
				M_ASSERT_PTR(offscreen);
			
				BBitmap *bmap = offscreen->bBitmap();
				if (bmap) mView->update(bmap);
			
				mBitmapBuffer->releaseRead();
			}
	
			// synchronize to frame rate
			deltatemps = temps+fpstempo-systemTime();
			if (deltatemps > 0) frameRateSem.acquireTimeout(deltatemps);
		}
	}
	M_CATCH("RBlitWindow Feeder Loop");

	if (debug) printf("RBlitWindow::run -- end of loop\n");

} // end of run for RBlitWindow




//************************************************************************
void RBlitWindow::getScreenMouseLoc(ui32 &x, ui32 &y)
//************************************************************************
{
	if (mView && Lock())
	{
		BPoint location;
		uint32 buttons;
		mView->GetMouse(&location, &buttons);
		location = mView->ConvertToScreen(location);
		Unlock();
		x = (ui32)location.x;
		y = (ui32)location.y;
	}
	else
	{
		x = 0;
		y = 0;
	}
} // end of getScreenMouseLoc for RBlitWindow


#endif // of RLIB_BEOS
//------------------------------- eoc ------------------------------------

