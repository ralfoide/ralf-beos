/*************************************************************************

New Header format :

	Projet	: Portable Lib

	Fichier	: bRWindow.h
	Partie	: base class for a wrapper around BWindow

	Auteur	: RM
	Date		: 060698
	Format	: tabs==2

--------------------

Old Header format :

	Projet : wrotozoom
	Fichier: window.h

	Class : CWindow
	Header for window.cpp

	Pour creer une fenetre, creer une instance avec new puis
	appeler registerWndClass puis appeler createWindow.

*************************************************************************/

#ifndef _H_BRWINDOW_
#define _H_BRWINDOW_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RRect.h"
#include "RThread.h"

#include <Window.h>

//------------------------------------------------------------------------
// message send by the window to the control thread, if any
// msg has a "data" value pointing on this RWindow

#define	K_MSG_WINDOW_QUIT_REQUESTED	'WiQr'

//------------------------------------------------------------------------
// la classe RWindow

//*******************************************************
class RWindow : public BWindow, virtual public RInterface
//*******************************************************
{
public:
	RWindow(RRect frame, sptr title, rbool show=true);
	virtual void deleteWindow(void);		// use this instead of ~RWindow
	virtual ~RWindow(void);							// must not be called explicitely

	// -- control thread --

	RThread	*	controlThread(void)						{ return mControlThread; }
	void			setControlThread(RThread *t)	{ mControlThread = t; }

	// -- window flags --

	void setQuitAppWhenWindowClosed(rbool state) { mQuitAppWhenWindowClosed = state; }
	rbool quitAppWhenWindowClosed(void)					 { return mQuitAppWhenWindowClosed;  }

	// --- main callbacks and services ---
	virtual void msgQuitRequested(void);
	virtual void msgActivated(rbool activated);

	void show(rbool state);
	void update(void);
	void activate(rbool state);
	void moveTo(ui32 x, ui32 y);
	void resizeTo(ui32 sx, ui32 sy);
	void size(ui32 &sx, ui32 &sy);
	void position(ui32 &x, ui32 &y);

	#ifdef RLIB_BEOS
		virtual bool QuitRequested(void);
		virtual	void WindowActivated(bool state);
		// bool	IsHidden() const; <-- TBDL
		// bool	IsActive() const;
		// Get/SetSizeLimits
	#endif

	#ifdef RLIB_WIN32
		// the following Win32-specific is here only for later reference (TBDL ?)
		virtual void msgCreate		(WPARAM wparam, LPARAM lparam);
		virtual void msgShowWindow(WPARAM wparam, LPARAM lparam);
		virtual void msgActivate	(WPARAM wparam, LPARAM lparam);
		virtual void msgCommand		(WPARAM wparam, LPARAM lparam);
		virtual BOOL msgClose			(WPARAM wparam, LPARAM lparam);
		virtual void msgDestroy		(WPARAM wparam, LPARAM lparam);
		virtual void msgPaint			(WPARAM wparam, LPARAM lparam);
		virtual void msgKeyDown		(int nVirtKey, LPARAM lKeyData);
		virtual void msgKeyUp			(int nVirtKey, LPARAM lKeyData);
	#endif

	// -- special services --
	// (may not be implemented correctly)

	// return the ideal screen-centered frame for a window of size sx/sy
	// [fake implementations must return RRect(0,0,sx-1,sy-1)]
	static RRect centerWindow(ui32 sx, ui32 sy);

	// --- variables protegees ---
protected:

	// methode d'init (register la classe de window sous Win32, cree la fenetre
	// et l'affiche si necessaire). Les classes derivee _doivent_ appeler
	// cette methode de base depuis leur constructeur.
	void init(rbool show);

	rbool		 mQuitAppWhenWindowClosed;	// default to false
	RThread	*mControlThread;						// defaults to NULL (none)

	// -- pure internal beos-specific stuff --
private:

	rbool	mDeleteCorrect;

}; // end of class RWindow


//------------------------------------------------------------------------

#endif // of RLIB_BEOS
#endif // of _H_BRWINDOW_

//------------------------------- eoh ------------------------------------

