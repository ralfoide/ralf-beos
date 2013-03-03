/*****************************************************************************

	Projet	: Nerdkill 3d

	Fichier	: CApp.cpp
	Partie	: the Nerdkill 3d App

	Auteur	: RM
	Date		: 010698
	Format	: tabs==2

*****************************************************************************/

#include "CApp.h"
#include "gTypeDefs.h"
#include "gMacros.h"

#include "CMoteur.h"
#include <Alert.h>
#include <NodeInfo.h>
#include <Node.h>

//---------------------------------------------------------------------------

CApp *gApplication = NULL;

//---------------------------------------------------------------------------
// HOOK TBDL HACK HACK !

extern rbool gPlaybackMode;
extern char *gFilename;

//---------------------------------------------------------------------------
// private messages

//---------------------------------------------------------------------------


//***************************************************************************
CApp::CApp(sptr sig) : RApp(sig)
//***************************************************************************
{
} // end of constructor for CApp


//***************************************************************************
CApp::~CApp(void)
//***************************************************************************
{
	if (debug) printf("CApp::~CApp -- destructor\n");
} // end of destructor for CApp


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark -


//***************************************************************************
void CApp::init(void)
//***************************************************************************
{
app_info info;
BEntry fichier,dummy;
status_t result;

	if (debug) printf("CApp::init\n");

	GetAppInfo(&info);
	result=fichier.SetTo(&info.ref);
	if (!result) result=fichier.GetParent(&mAppDir);
	if (result) return;

	createWindow();

	mMoteur = new CMoteur;
	M_ASSERT_PTR(mMoteur);
	mMoteur->launch(K_RUNABLE_PRI_NORMAL);
/*
	if (gPlaybackMode) moteurMsg(K_MSG_PLAY_DEFAULT);
	else moteurMsg(K_MSG_RECORD_DEFAULT);
*/
} // end of init for CApp


//***************************************************************************
void CApp::aboutRequested(void)
//***************************************************************************
{
	// TBDL (later) : post message to moteur in order to
	// graphically display the about box in a neat gfx

	BAlert *box;

	box = new BAlert(	K_APP_NAME " - About box",
										K_APP_NAME " " K_APP_VERS " for BeOS R3"
										"\n" B_UTF8_COPYRIGHT " R'alf"
										"\n\ndev release " K_APP_VERS " - " __DATE__ ", " __TIME__
										"\n\nContact : raphael@powerpulsar.com"
										"\nand http://www.powerpulsar.com/"
										"\n\nGfx by \"SpH\"",
										"    OK    ", NULL, NULL,
										B_WIDTH_FROM_WIDEST, B_INFO_ALERT);

	if (box) box->Go();

} // end of aboutRequested for CApp


//***************************************************************************
void CApp::terminate(void)
//***************************************************************************
{
	if (debug) printf("CApp::terminate\n");

	if (mMoteur) mMoteur->askToKill();

	//Lock();
	//if (mFilledWin && mFilledWin->Lock()) mFilledWin->Quit();
	//Unlock();

	if (mMoteur)
	{
		mMoteur->waitForEnd();
		delete mMoteur;
		mMoteur = NULL;
	}

	if (debug) printf("end of CApp\n");

} // end of QuitRequested for CApp




//***************************************************************************
void CApp::MessageReceived(BMessage *message)
//***************************************************************************
{
	BApplication::MessageReceived(message);

} // end of MessageReceived for CApp




//***************************************************************************
void CApp::createWindow(void)
//***************************************************************************
{
	// create app-specific param window if any

} // end of createWindow


//***********************************************************************
void CApp::moteurMsg(ui32 msg, ui32 lparam, vptr data, ui32 datasize)
//***********************************************************************
{
	//ui32 nul=0;

	if (mMoteur) mMoteur->postMessage(mMoteur, msg, lparam, data, &datasize);

} // end of moteurMsg for CApp


//***********************************************************************
void CApp::getAppPath(RPath &path)
//***********************************************************************
{
	BPath bpath(&mAppDir, "");
	path = bpath.Path();

} // end of getAppPath for CApp


//***********************************************************************
void CApp::processMessage(ui32 msg, ui32 lparam, vptr data, ui32 *channel)
//***********************************************************************
{
	// this function is similar to moteurMsg() except that it filter
	// some very app-specific messages. All ignored messages go to the moteur.

	switch(msg)
	{
		case K_MSG_RTHREAD_EXIT:
			printf("CApp::processMessage -- K_MSG_RTHREAD_EXIT\n");
			// ask the application to quit
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;

		default:
			moteurMsg(msg, lparam, data, 0);
	}

} // end of processMessage for CApp


//**********************************************
void CApp::openDocument(RPath &path, rbool last)
//**********************************************
{
	if (debug) printf("CApp::openDocument -- last %d -- path %s\n", last, path.path());
	if (!last) return;	// only open the last document

	RPath *p = new RPath(path);
	M_ASSERT_PTR(p);
	moteurMsg(K_MSG_PLAY_DOCUMENT, last, (void*)p, sizeof(*p));

} // end of openDocument for CApp


//********************************************
void CApp::setFileType(RPath &path, sptr type)
//********************************************
{
	if (debug) printf("CApp::setFileType -- last %s -- path %s\n", type, path.path());
	try
	{
		BNode node(path.path());
		BNodeInfo info;
		if (node.InitCheck() >= B_NO_ERROR
				&& info.SetTo(&node) >= B_NO_ERROR) info.SetType(type);
	}
	M_CATCH("CApp::setFileType");

} // end of setFileType for CApp


// eoc
