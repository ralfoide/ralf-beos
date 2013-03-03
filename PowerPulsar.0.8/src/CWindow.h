/*****************************************************************************

	Projet		: Pulsar

	Fichier	: CWindow.h
	Partie		: Window

	Auteur		: RM
	Date			: 251097
	Format		: tabs==2

*****************************************************************************/

#ifndef _H_CWINDOW_
#define _H_CWINDOW_

#include "CPreference.h"

//---------------------------------------------------------------------------

class CWindow;


//*************
struct SWinPref
//*************
{
	char		*mBaseName;		// never freed
	BRect		mFrame;
	bool		mIsVisible;
	
	void loadPref(CPreference &pref);
	void savePref(CPreference &pref, CWindow *win);
	SWinPref(void) { mBaseName=NULL; mIsVisible = true; }
};


//---------------------------------------------------------------------------


//****************************
class CWindow : public BWindow
//****************************
{
public:
	CWindow(SWinPref &pref,
				 const char *title, 
				 window_type type,
				 ulong flags,
				 ulong workspace = B_CURRENT_WORKSPACE);
	virtual ~CWindow(void);

	virtual bool init(void);

	virtual void FrameResized(float new_width, float new_height);
	virtual void Zoom(BPoint rec_position, float rec_width, float rec_height);
	virtual bool QuitRequested(void);

protected:

	BRect	mBounds;
	BRect	mFrame;
	bool		mInitShow;							// default is true (used at end of init)
	bool		mQuitRequestSavePref;		// default is true (used in QuittRequested)
	bool		mPostQuitApp;						// default is true (used in QuiRequested)
	bool		mQuitRequestReturnVal;		// default is true (value returned by QuitRequested)
	bool	mAutoSaveFramePref;			// default is true (used in QuitRequested)

	SWinPref mPref;


}; // end of class defs for CWindow


//---------------------------------------------------------------------------

#endif // of _H_CWINDOW_

// eoh

