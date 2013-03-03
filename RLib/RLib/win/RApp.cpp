/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	wRApp.cpp
	Partie	: Portable App Wrapper

	Auteur	: RM
	Date		: 300798 (Win32)
	Date		: 010698
	Format	: tabs==2

*****************************************************************************/

#include "RLib.h"

#if defined(RLIB_WIN32) && defined(RLIB_WIN32_USES_MFC)

//---------------------------------------------------------------------------
// MFC-AFX typical code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------

#include "RMacros.h"
#include "RApp.h"
#include "RWindow.h"

//---------------------------------------------------------------------------
// MFC-AFX typical code

IMPLEMENT_DYNAMIC(RApp, CWinApp)

BEGIN_MESSAGE_MAP(RApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnRAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnRAppFileOpen)
	// Standard file based document commands
	// ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	// Standard print setup command
	// ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------


//***************************************************************************
BOOL RApp::InitInstance(void)
//***************************************************************************
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	// RString	key(_T("HKEY_CURRENT_USER\\Software\\"));
	// key += mAppRegKeyName;
	SetRegistryKey((sptr)mAppRegKeyName);

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Parse command line for standard shell commands, DDE, file open
	// CCommandLineInfo cmdInfo;
	// ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	// if (!ProcessShellCommand(cmdInfo))
	//	return FALSE;

	// The one and only window has been initialized, so show and update it.
	// m_pMainWnd->ShowWindow(SW_SHOW);
	// m_pMainWnd->UpdateWindow();

	return true;
} // end of InitInstance for RApp


//***************************************************************************
int RApp::ExitInstance(void)
//***************************************************************************
{
	// Call the CWinApp counterpart
	return CWinApp::ExitInstance();
} // end of ExitInstance for RApp


//***************************************************************************
int RApp::Run(void)
//***************************************************************************
{
	// Call the init() for the RApp
	int value = 0;

	try
	{
		createMDI();
		Init();

		if (m_pMainWnd)
			// Call the CWinApp counterpart
			value = CWinApp::Run();
		else
			// Windows needs at least one window per process ;-)
			// (supposedly for the message pump, as they say !)
			throw RErrEx("\nYou should create a window referenced\n"
										"via RApp::m_pMainWnd under Win32 !");

		// Call the terminate() for the RApp
		Terminate();
		deleteMDI();
	}
	M_CATCH("RApp::InitInstance");

	return value;

} // end of Run for RApp


//***************************************************************************
BOOL RApp::OnIdle(LONG lCount)
//***************************************************************************
{
	try
	{
		OnIdle();
	}
	M_CATCH("RApp::OnIdle");

	return false;

} // end of Run for RApp


//***************************************************************************
void RApp::setMainWindow(CWnd *wnd, rbool override, rbool erase)
//***************************************************************************
//
// Win32-specific service used by RWindow / RAppMDIWindow
{
	if (erase)
	{
		if (wnd == m_pMainWnd || override) m_pMainWnd = NULL;
	}
	else
	{
		if (!m_pMainWnd || override) m_pMainWnd = wnd;
	}
} // end of setMainWindow for RApp


//***************************************************************************
afx_msg void RApp::OnRAppFileOpen(void)
//***************************************************************************
{
	if(debug) DPRINTF("RApp::OnRAppFileOpen\n");
	CWinApp::OnFileOpen();
}


#if 0

//***************************************************************************
void RApp::RefsReceived(BMessage *message)
//***************************************************************************
{

	try
	{
		BEntry entry;
		BPath localpath;
		entry_ref ref;
		uint32 type;
		int32 count;
		int32 i;
		
		message->GetInfo("refs", &type, &count);
		if (type != B_REF_TYPE) return;
		
		for(i = count-1; i>=0; i--)
		{
			if (   message->FindRef("refs", i, &ref) >= B_NO_ERROR
					&& entry.SetTo(&ref) >= B_NO_ERROR
					&& entry.GetPath(&localpath) >= B_NO_ERROR)
			{
				RPath rpath(localpath.Path());
				openDocument(rpath, i==0);
			}
		}
	}
	M_CATCH("RApp::RefsReceived");

} // end of RefsReceived for RApp

#endif

//---------------------------------------------------------------------------



//***************************************************************************
void RApp::createMDI(void)
//***************************************************************************
{
	if (mWin) return;
	mWin = new RAppMDIWindow(RRect(50,50,300,200), "RLib" /* K_APP_NAME " " K_APP_VERS */);
	M_ASSERT_PTR(mWin);
	mWin->setQuitAppWhenWindowClosed(true);
	mWin->maximize();
} // end of createMDI for RApp


//***************************************************************************
void RApp::deleteMDI(void)
//***************************************************************************
{
	if (mWin) mWin->deleteWindow(); mWin = NULL;

} // end of deleteMDI for RApp



//---------------------------------------------------------------------------

#endif // #if defined(RLIB_WIN32) && defined(RLIB_WIN32_USES_MFC)


/****************************************************************

	$Log: RApp.cpp,v $
	Revision 1.2  2000/04/05 06:25:13  Ralf
	Partially updated to new lib conventions.
	
	Revision 1.1  2000/03/13 08:55:14  Ralf
	First revision
	
****************************************************************/

// eoc
