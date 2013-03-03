/*************************************************************************

New Header format :

	Projet	: Portable Lib

	Fichier	: bRWindow.cpp
	Partie	: base class for a wrapper around BWindow

	Auteur	: RM
	Date		: 060698
	Format	: tabs==2

--------------------

Old Header format :
	Projet : wrotozoom
	Fichier: window.cpp

	Classe RWindow,
	une classe de base de fenetre

*************************************************************************/

#include "gMachine.h"
#include "RWindow.h"
#include "RThread.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"
#include <Screen.h>

//------------------------------------------------------------------------


//************************************************************************
RWindow::RWindow(RRect frame, sptr title, rbool show)
        :BWindow(frame, (const char *)title, B_TITLED_WINDOW,
        				 B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
//************************************************************************
{
	mQuitAppWhenWindowClosed = false;
	mDeleteCorrect = false;
	mControlThread = NULL;
} // end of constructor for RWindow


//************************************************************************
void RWindow::deleteWindow(void)
//************************************************************************
{
	mDeleteCorrect = true;
	if (Lock()) Quit();
} // end of deleteWindow for RWindow


//***************************************************************************
RWindow::~RWindow(void)
//***************************************************************************
{
	// never to be used directly under BeOS
	if (!mDeleteCorrect) throw RErrEx("RWindow::~RWindow called explicitely !!");

} // end of destructor for RWindow


//------------------------------------------------------------------------


//************************************************************************
void RWindow::init(rbool show)
//************************************************************************
{
	if (show) Show();
	else Unlock();
} // end of init for RWindow


//------------------------------------------------------------------------


//************************************************************************
void RWindow::msgQuitRequested(void)
//************************************************************************
{
	if (mControlThread)
		mControlThread->postMessage(K_MSG_WINDOW_QUIT_REQUESTED, 0, (vptr)this);
} // end of msgQuitRequested for RWindow


//************************************************************************
void RWindow::msgActivated(rbool activated)
//************************************************************************
{
} // end of msgActivated for RWindow


//------------------------------------------------------------------------


//************************************************************************
void RWindow::show(rbool state)
//************************************************************************
{
	if (Lock())
	{
		if (state && IsHidden()) Show();
		else if (!state && !IsHidden()) Hide();
		Unlock();
	}
} // end of show for RWindow


//************************************************************************
void RWindow::update(void)
//************************************************************************
{
	if (Lock())
	{
		UpdateIfNeeded();
		Unlock();
	}
} // end of update for RWindow


//************************************************************************
void RWindow::activate(rbool state)
//************************************************************************
{
	if (Lock())
	{
		Activate(state);
		Unlock();
	}
} // end of activate for RWindow


//************************************************************************
void RWindow::moveTo(ui32 x, ui32 y)
//************************************************************************
{
	if (Lock())
	{
		MoveTo(x, y);
		Unlock();
	}
} // end of moveTo for RWindow


//************************************************************************
void RWindow::resizeTo(ui32 sx, ui32 sy)
//************************************************************************
{
	if (Lock())
	{
		ResizeTo(sx, sy);
		Unlock();
	}
} // end of resizeTo for RWindow


//************************************************************************
void RWindow::size(ui32 &sx, ui32 &sy)
//************************************************************************
{
	BRect frame = Frame();
	sx = frame.IntegerWidth()+1;
	sy = frame.IntegerHeight()+1;
} // end of size for RWindow


//************************************************************************
void RWindow::position(ui32 &x, ui32 &y)
//************************************************************************
{
	BRect frame = Frame();
	x = (ui32)frame.left;
	y = (ui32)frame.top;
} // end of position for RWindow


//------------------------------------------------------------------------


//************************************************************************
bool RWindow::QuitRequested(void)
//************************************************************************
{
	msgQuitRequested();
	if (mQuitAppWhenWindowClosed) gApplication->moteurMsg(K_MSG_RTHREAD_EXIT, 0, this);
	return false;
} // end of QuitRequested for RWindow


//************************************************************************
void RWindow::WindowActivated(bool state)
//************************************************************************
{
	msgActivated(state);
} // end of WindowActivated for RWindow


//------------------------------------------------------------------------


//************************************************************************
RRect RWindow::centerWindow(ui32 sx, ui32 sy)
//************************************************************************
{
	RRect rect(0,0,sx-1, sy-1);

	if(1)
	{
		BScreen screen;
		ui32 ssx=screen.Frame().IntegerWidth()+1;
		ui32 ssy=screen.Frame().IntegerHeight()+1;
		rect.offsetBy(ssx>sx ? (ssx-sx)/2 : 0, ssy>sy ? (ssy-sy)/2 : 0);
	}
	
	return rect;
} // end of centerWindow for RWindow

//------------------------------- eoc ------------------------------------

