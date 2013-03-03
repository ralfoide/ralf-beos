/*****************************************************************************

	Projet	: RLib

	Fichier	: RApp.h
	Partie	: Main app class

	Auteur	: RM
	Date	: 270798 (Win32)
	Date	: 160598
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_RAPP_
#define _H_RAPP_

#if defined(RLIB_WIN32) && defined(RLIB_WIN32_USES_MFC)

#include "RGenApp.h"

//---------------------------------------------------------------------------

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// #include "resource.h"       // main symbols

//---------------------------------------------------------------------------

class RAppMDIWindow;

//---------------------------------------------------------------------------

/// Main Platform-Specific Application Class

//*****************************************
class RApp : public CWinApp, public RGenApp
//*****************************************
{
public:
		RApp(sptr regKeyName = "ralf\\rlibapp")
			: mAppRegKeyName(regKeyName), CWinApp(), RGenApp(), mWin(NULL) { }
virtual ~RApp(void)		{ }

	// --- CWinApp callbacks ---
	// MFC-AFX CWinApp stuff
public:

virtual	BOOL	InitInstance(void);
virtual	int		ExitInstance(); // return app exit code
virtual	int		Run();
virtual	BOOL	OnIdle(LONG lCount);

afx_msg	void	OnRAppAbout(void) { aboutRequested(); }
afx_msg	void	OnRAppFileOpen(void);

	
		DECLARE_MESSAGE_MAP()

		DECLARE_DYNAMIC(RApp)

public:

	//virtual void AboutRequested(void)
	//virtual bool QuitRequested(void)		{ Terminate(); return true; }
	//virtual void MessageReceived(BMessage *message) { }
	//virtual void RefsReceived(BMessage *a_message);

	// --- RGenApp callbacks ---

	virtual void Init(void) = 0;		// called after InitInstance()
	virtual void Terminate(void) = 0;	// called before ExitInstance()
	virtual void AboutRequested(void)	{ /* post message to moteur ? */ };
	virtual void OnIdle(void)			{ }

	// important note : the behavior of RRLibapp::processMessage
	// this function is similar to moteurMsg() except that it filters
	// some very app-specific messages. All ignored messages go to the moteur.
	virtual void ProcessMessage(RMessage *msg) { }

	// should be derived by the caller if it wants to be notified that the application
	// was open by double clicking a document or if a file was dropped on the application
	virtual void OpenDocument(RPath &path, rbool last=true) { }

	// --- methodes interface

	HINSTANCE	GetAppInstance(void)	{ return m_hInstance; }
	LPTSTR		GetCmdLine	  (void)	{ return m_lpCmdLine; }

	CWnd *		GetMainWindow(void)		{ return m_pMainWnd; }
	void		SetMainWindow(CWnd *wnd, rbool override = false, rbool erase = false);

	// --- membres publiques

	// public members from hairy CWinApp
	//
	// Startup args (need not be changed, setup by winmain and can be
	// read by the RApp object later...)
	// HINSTANCE m_hInstance;
	// HINSTANCE m_hPrevInstance;
	// LPTSTR m_lpCmdLine;
	// int m_nCmdShow;

	// public member that should be affected
	//
	// controlling window for the main thread :
	// CWnd * m_pMainWnd;

private:

	RString	mAppRegKeyName;
	RAppMDIWindow *mWin;

	void	createMDI(void);
	void	deleteMDI(void);

}; // end of class defs for RApp


//---------------------------------------------------------------------------

#endif // #if defined(RLIB_WIN32) && defined(RLIB_WIN32_USES_MFC)
#endif // of _H_RAPP_


/****************************************************************

	$Log: RApp.h,v $
	Revision 1.2  2000/04/05 06:25:13  Ralf
	Partially updated to new lib conventions.
	
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoh
