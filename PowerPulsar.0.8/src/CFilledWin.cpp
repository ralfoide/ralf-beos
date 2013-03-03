/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CFilledWin.cpp
	Partie	: Loader

	Auteur	: RM
	Date	: 140297
	Format	: tabs==4

*****************************************************************************/

#include "CFilledWin.h"
#include "CButton.h"	// for CCdButton

//---------------------------------------------------------------------------

#define K_MSG_ZOOM_SMALL	'smZo'
#define K_MSG_ZOOM_BIG		'biZo'

#undef K_EXT
#define K_EXT ".tga"	// HOOK RM 161198 -- ".png"
#define K_EXT2 K_EXT

#define K_IMG_ZOOM_NAME "zoombox" K_EXT2


//---------------------------------------------------------------------------

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif


//*******************************************************
CFilledWin::CFilledWin(BRect frame,
					   const char *title, 
					   window_type type,
					   ulong flags,
					   ulong workspace)
		   :BWindow(frame, title, type, flags, workspace)
#define inherited BWindow
//*******************************************************
{
	mView = NULL;
	mZoomSmall = NULL;
	mZoomBig = NULL;
	mZoomDivider = 1;

} // end of constructor for CFilledWin


//***************************
CFilledWin::~CFilledWin(void)
//***************************
{
	if (debug) printf("CFilledWin::~CFilledWin -- destructor\n");
} // end of destructor for CFilledWin


//*************************
bool CFilledWin::init(void)
//*************************
{
	if (debug) printf("CFilledWin::init\n");

	BRect bounds=Bounds();
	mView = new CFilledView(bounds, "filler", B_FOLLOW_LEFT | B_FOLLOW_TOP);
	if (!mView || !mView->init()) return false;
	AddChild(mView);
	mView->MakeFocus();

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

	Minimize(true);
	Show();
	//Run();
	return true;

} // end of init for CFilledWin


//---------------------------------------------------------------------------

//**************************************************************
void CFilledWin::FrameResized(float new_width, float new_height)
//**************************************************************
{
	//if (mView) mView->FrameResized(new_width, new_height);
	inherited::FrameResized(new_width, new_height);
} // end of FrameResized for CFilledWin




//********************************************************************
void CFilledWin::DispatchMessage(BMessage *message, BHandler *handler)
//********************************************************************
{
	if (!gApplication->handleMessageAddons(message))
		inherited::DispatchMessage(message, handler);
}


//*************************************************
void CFilledWin::MessageReceived(BMessage *message)
//*************************************************
{
	switch(message->what)
	{
		case K_MSG_STREAM:
			if (mView) mView->Pulse();
			break;
		case K_MSG_ZOOM_SMALL:
			setZoomDivider(1);
			break;
		case K_MSG_ZOOM_BIG:
			setZoomDivider(-1);
			break;
		default:
			inherited::MessageReceived(message);
			break;
	}

} // end of MessageReceived for CFilledWin


//**********************************
bool CFilledWin::QuitRequested(void)
//**********************************
{
	if (debug) printf("CFilledWin::QuitRequested\n");

	mView = NULL;

	return false;

} // end of QuitRequested for CFilledWin


//******************************************
void CFilledWin::WindowActivated(bool state)
//******************************************
{
	//if (debug) printf("CFilledWin::WindowActivated\n");

	inherited::WindowActivated(state);

} // end of WindowActivated for CFilledWin


//---------------------------------------------------------------------------


//******************************************
void CFilledWin::setZoomDivider(int32 delta)
//******************************************
{
	int32 div = mZoomDivider;
	if (delta < 0) div = max(1, div+delta);
	else		   div = min(8, div+delta);
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



//---------------------------------------------------------------------------


//*******************************
bool CFilledWin::hasViewPtr(void)
//*******************************
{
	return (mView != NULL);
} // end of hasViewPtr for CFilledWin


//********************************************
void CFilledWin::setViewPtr(CFilledView *view)
//********************************************
{
	mView = view;
} // end of hasViewPtr for CFilledWin





//---------------------------------------------------------------------------

// eoc
