/*****************************************************************************

	Projet		: Pulsar

	Fichier	: CWindow.cpp
	Partie		: Window

	Auteur		: RM
	Date			: 251097
	Format		: tabs==2

*****************************************************************************/

#include "CPulsarApp.h"
#include "CWindow.h"

//---------------------------------------------------------------------------

extern CPulsarApp *gApplication;

//---------------------------------------------------------------------------



//***************************************************************************
CWindow::CWindow(SWinPref &pref,
							const char *title, 
							window_type type,
							ulong flags,
							ulong workspace)
			 :BWindow(pref.mFrame, title, type, flags, workspace)
#define inherited BWindow
//***************************************************************************
{
	mPref			= pref;
	mBounds		= Bounds();
	mFrame			= Frame();
	mInitShow							= true;
	mQuitRequestSavePref		= true;
	mPostQuitApp					= true;
	mQuitRequestReturnVal	= true;
	mAutoSaveFramePref			= true;

} // end of constructor for CWindow


//***************************************************************************
CWindow::~CWindow(void)
//***************************************************************************
{
} // end of destructor for CWindow


//---------------------------------------------------------------------------
#pragma mark -


//***************************************************************************
bool CWindow::init(void)
//***************************************************************************
{
	mBounds	= Bounds();
	mFrame		= Frame();

	if (mInitShow)
		Show();
	else
	{
		Minimize(true);
		Run();
	}
	return true;
} // end of init for CWindow


//---------------------------------------------------------------------------
#pragma mark -


//***************************************************************************
bool CWindow::QuitRequested(void)
//***************************************************************************
{
	if(mQuitRequestSavePref)
	{
		if (mAutoSaveFramePref) mPref.mFrame = Frame();
		mPref.savePref(gApplication->mPreference, this);
	}

	if (mPostQuitApp)
		be_app->PostMessage(B_QUIT_REQUESTED);

	return mQuitRequestReturnVal;

} // end of QuitRequested for CWindow


//***************************************************************************
void CWindow::FrameResized(float new_width, float new_height)
//***************************************************************************
{
	inherited::FrameResized(new_width, new_height);
	mBounds = Bounds();
	mFrame = Frame();
} // end of FrameResized for CWindow


//***************************************************************************
void CWindow::Zoom(BPoint rec_position, float rec_width, float rec_height)
//***************************************************************************
{
	inherited::Zoom(rec_position, rec_width, rec_height);
	mBounds = Bounds();
	mFrame = Frame();
} // end of Zoom for CWindow


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -



//****************************************
void SWinPref::loadPref(CPreference &pref)
//****************************************
{
char name[128];
BRect frame;

	if(debug) printf("SWinPref::loadPref\n");

	strcpy(name, "1234p_");
	if (mBaseName)
		strcat(name, mBaseName);
	else
		strcat(name, "default_");

	BMessage message;
	if (!pref.getPref(name, message)) return;

	if (   message.FindRect("frame", &frame) == B_OK
			&& message.FindBool("visible", &mIsVisible) == B_OK
			&& frame.IsValid()
			&& frame.Intersects(BScreen().Frame()))
		mFrame = frame;

} // end of loadPref for SWinPref


//***************************************************************************
void SWinPref::savePref(CPreference &pref, CWindow *win)
//***************************************************************************
{
char name[128];

	if(debug) printf("SWinPref::savePref\n");

	if (!win) return;

	strcpy(name, "1234p_");
	if (mBaseName)
		strcat(name, mBaseName);
	else
		strcat(name, "default_");

	BMessage message;
	if (   message.AddRect("frame", win->Frame()) == B_OK
			&& message.AddBool("visible", mIsVisible) == B_OK)
		pref.setPref(name, message);

} // end of savePref for SWinPref


//---------------------------------------------------------------------------

// eoc
