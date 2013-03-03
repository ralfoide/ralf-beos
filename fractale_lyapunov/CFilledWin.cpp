/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CFilledWin.cpp
	Partie	: Loader

	Auteur	: RM
	Date		: 140297
	Format	: tabs==2

*****************************************************************************/

#include <stdio.h>
#include "CFilledWin.h"

#ifdef K_USE_ZOOM
	#include "CButton.h"	// for CCdButton
#endif

//---------------------------------------------------------------------------


#ifdef K_USE_ZOOM
	#define K_MSG_ZOOM_SMALL	'smZo'
	#define K_MSG_ZOOM_BIG		'biZo'
	#define K_EXT ".png"
	#define K_IMG_ZOOM_NAME "zoombox" K_EXT
#endif

//---------------------------------------------------------------------------

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

//***************************************************************************
CFilledWin::CFilledWin(BRect frame,
											 const char *title, 
											 window_type type,
											 ulong flags,
											 ulong workspace)
						 :BWindow(frame, title, type, flags, workspace)
//***************************************************************************
{
	mView = NULL;
#ifdef K_USE_ZOOM
	mZoomSmall = NULL;
	mZoomBig = NULL;
	mZoomDivider = 1;
#endif

} // end of constructor for CFilledWin


//***************************************************************************
CFilledWin::~CFilledWin(void)
//***************************************************************************
{
	if (debug) printf("CFilledWin::~CFilledWin -- destructor\n");
} // end of destructor for CFilledWin


//***************************************************************************
bool CFilledWin::init(void)
//***************************************************************************
{
	if (debug) printf("CFilledWin::init\n");

	BRect bounds=Bounds();
	mView = new CFilledView(bounds, "filler", B_FOLLOW_LEFT | B_FOLLOW_TOP);
	if (!mView || !mView->init()) return false;
	AddChild(mView);
	mView->MakeFocus();

#ifdef K_USE_ZOOM
	#define K 12
	BRect r(0,bounds.bottom-K-1,K-1,bounds.bottom-2);
	mZoomSmall = new CCdButton(r, "", 0, 2, K_IMG_ZOOM_NAME,
													new BMessage(K_MSG_ZOOM_SMALL),
													B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	if (mZoomSmall) mView->AddChild(mZoomSmall);

	r.OffsetBy(K,0);
	mZoomBig = new CCdButton(r, "", 1, 2, K_IMG_ZOOM_NAME,
												new BMessage(K_MSG_ZOOM_BIG),
												B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	if (mZoomBig) mView->AddChild(mZoomBig);
#endif

	//Minimize(true);	<-- choose if window starts hidden or visible
	Show();
	//Run();
	return true;

} // end of init for CFilledWin


//---------------------------------------------------------------------------

//***************************************************************************
void CFilledWin::FrameResized(float new_width, float new_height)
//***************************************************************************
{
	//if (mView) mView->FrameResized(new_width, new_height);
	BWindow::FrameResized(new_width, new_height);
} // end of FrameResized for CFilledWin


//***************************************************************************
void CFilledWin::MessageReceived(BMessage *message)
//***************************************************************************
{
	//if (debug) printf("CFilledWin::MessageReceived\n");

	switch(message->what)
	{
		case K_MSG_STREAM:

			// remove all duplicates message of redraw that may slow down the process
			{
				BMessageQueue *queue = MessageQueue();
				if (!queue) throw "CFilledWin: No Msg Queue !";
				BMessage *msg;
				while ((msg = queue->FindMessage(K_MSG_STREAM, 0)) != NULL)
					queue->RemoveMessage(msg);
			}

			if (mView) mView->Pulse();
			break;

#ifdef K_USE_ZOOM
		case K_MSG_ZOOM_SMALL:
			setZoomDivider(1);
			break;

		case K_MSG_ZOOM_BIG:
			setZoomDivider(-1);
			break;
#endif

		default:
			BWindow::MessageReceived(message);
			break;
	}

} // end of MessageReceived for CFilledWin


//***************************************************************************
bool CFilledWin::QuitRequested(void)
//***************************************************************************
{
	if (debug) printf("CFilledWin::QuitRequested\n");

	mView = NULL;

	if(0)
	{
		// if you enable this, closing the window WON'T kill the window nor the app
	}
	else
	{
		// this is the default behavoir : both the app are closed.
		// (the app will close the window)

		gApplication->PostMessage(B_QUIT_REQUESTED);
	}

	return false;	

} // end of QuitRequested for CFilledWin


//***************************************************************************
void CFilledWin::WindowActivated(bool state)
//***************************************************************************
{
	//if (debug) printf("CFilledWin::WindowActivated\n");

	BWindow::WindowActivated(state);

} // end of WindowActivated for CFilledWin


//---------------------------------------------------------------------------

#ifdef K_USE_ZOOM

//***************************************************************************
void CFilledWin::setZoomDivider(int32 delta)
//***************************************************************************
{
	int32 div = mZoomDivider;
	if (delta < 0) div = max(1, div+delta);
	else					 div = min(8, div+delta);
	if (div != mZoomDivider)
	{
		int32 sx,sy;
		sx = 640/div;
		sy = 480/div;
		mZoomDivider = div;
		if (Lock())
		{
			ResizeTo(sx,sy);
			mView->setZoom(sx,sy);
			Unlock();
		}
	}
} // end of setZoomDivider for CFilledWin

#endif


//---------------------------------------------------------------------------


//***************************************************************************
bool CFilledWin::hasViewPtr(void)
//***************************************************************************
{
	return (mView != NULL);
} // end of hasViewPtr for CFilledWin


//***************************************************************************
void CFilledWin::setViewPtr(CFilledView *view)
//***************************************************************************
{
	mView = view;
} // end of hasViewPtr for CFilledWin





//---------------------------------------------------------------------------

// eoc
