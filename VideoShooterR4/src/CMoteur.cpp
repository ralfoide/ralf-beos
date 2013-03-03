/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CMoteur.cpp
	Partie	: Moteur

	Auteur	: RM
	Date		: 220798
	Format	: tabs==2

*****************************************************************************/

#include "CMoteur.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"

#include "CMoteurPlay.h"
#include "CMoteurRecord.h"
#include "RPaneDialog.h"

//---------------------------------------------------------------------------

// HOOK TBDL HACK HACK !

extern rbool gPlaybackMode;
extern char *gFilename;


//***************************************************************************
CMoteur::CMoteur(void)
				:RThread()
//***************************************************************************
{

} // end of constructor for CMoteur


//***************************************************************************
CMoteur::~CMoteur(void)
//***************************************************************************
{
} // end of destructor for CMoteur



//---------------------------------------------------------------------------



//***************************************************************************
void CMoteur::run(void)
//***************************************************************************
{
ui32 msg;
ui32 msgParam;
vptr msgData;
rbool loop = true;

	if(debug) printf("CMoteur::run\n");

	try
	{

	RPaneDialog *dialog;
	if(1)
	{
		if(debug) printf("### new RPaneDialog\n");
		dialog = new RPaneDialog(RRect(50,50,50+200,50+150),
																					"VideoShooter Run",
																					"Quit", "Record", "Play");
																					// ok, cancel, retry
		if(debug) printf("### M_ASSERT_PTR RPaneDialog\n");
		M_ASSERT_PTR(dialog);
		
		BButton *b = new BButton(BRect(10,10,100,30), "BButton", "BButton", NULL);
		if (dialog->Lock())
		{
			dialog->AddChild(b);
			dialog->Unlock();
		}
		
		if(debug) printf("### go RPaneDialog\n");
		dialog->go(*this, 'TEST', false);
		if(debug) printf("### ok RPaneDialog\n");
	}

		while(loop)
		{
			//lecture message
			waitForMessage(msg, msgParam, msgData);

			// process message
			switch(msg)
			{
				case 'TEST':
					if(debug) printf("Message TEST !\n");
					break;
				case K_OK_BUTTON_PANEKEY:
					if(debug) printf("Message K_OK_BUTTON_PANEKEY ! (Quit)\n");
					if(dialog)
					{
						dialog->deleteWindow();
						dialog = NULL;
						gApplication->processMessage(K_MSG_RTHREAD_EXIT);
					}
					break;
				case K_CANCEL_BUTTON_PANEKEY:
					if(debug) printf("Message K_CANCEL_BUTTON_PANEKEY ! (Record)\n");
					selfPostMessage(K_MSG_RECORD_DEFAULT);
					break;
				case K_APPLY_BUTTON_PANEKEY:
					if(debug) printf("Message K_APPLY_BUTTON_PANEKEY ! (Play)\n");
					selfPostMessage(K_MSG_PLAY_DEFAULT);
					break;
				
				case K_MSG_PLAY_DEFAULT:
					if(debug) printf("CMoteur::msg K_MSG_PLAY_DEFAULT\n");
					{
						RPath appdir;
						gApplication->getAppPath(appdir);
						char s[1024];
						sprintf(s, "%s/%s", appdir.path(), K_AUTOPLAY_NAME);
						RPath path(s);
						openPlayDocument(&path);
					}
					break;

				case K_MSG_RECORD_DEFAULT:
					if(debug) printf("CMoteur::msg K_MSG_RECORD_DEFAULT\n");
					{
						RPath appdir;
						gApplication->getAppPath(appdir);
						char s[1024];
						sprintf(s, "%s/%s", appdir.path(), K_AUTOPLAY_NAME);
						RPath path(s);
						openRecordDocument(&path);
					}
					break;

				case K_MSG_PLAY_DOCUMENT:
					if(debug) printf("CMoteur::msg K_MSG_PLAY_DOCUMENT\n");
					if (msgData)
					{
						RPath *path = (RPath *)msgData;
						openPlayDocument(path);
						delete path;
					}
					break;

				case K_MSG_WINDOW_QUIT_REQUESTED:
					closeDocument((RInterface *)msgData);
					break;

				case K_MSG_RTHREAD_EXIT:
					if(debug) printf("CMoteur::msg K_MSG_RTHREAD_EXIT\n");
					loop = false;
					break;
			}
		}

if(dialog) dialog->deleteWindow();

	}
	M_CATCH("Moteur Play");

	closeAllDocuments();

	// ask application to kill itself (i.e. to quit cleanly)
	gApplication->processMessage(K_MSG_RTHREAD_EXIT);

} // end of run for CMoteur


//***************************************************************************
void CMoteur::closeDocument(RInterface *doc_win)
//***************************************************************************
{
	if (debug) printf("CMoteur::closeDocument -- win <%p>\n", doc_win);

	// scan the document list in search of this thread
	si32 n = mDocList.size();
	for(si32 i=0; i<n; i++)
	{
		CMoteurDocument *doc = mDocList.itemAt(i);
		if (doc && doc->getWindow() == doc_win)
		{
			mDocList.removeItemAt(i);
			RThread *thread = doc->getThread();
			if (debug) printf("found matching window -- thread %p\n", thread);
			M_ASSERT_PTR(thread);
			thread->askToKill();
			thread->waitForEnd();
			return;
		}
	}

	printf("no matching window\n");

} // end of closeDocument for CMoteur


//***************************************************************************
void CMoteur::closeAllDocuments(void)
//***************************************************************************
{
	if (debug) printf("CMoteur::closeAllDocuments\n");

	si32 n = mDocList.size();
	for(si32 i=0; i<n; i++)
	{
		CMoteurDocument *doc = mDocList.removeLastItem();
		if (!doc) continue;
		RThread *thread = doc->getThread();
		if (!thread) continue;
		thread->askToKill();
		thread->waitForEnd();
	}

} // end of closeAllDocuments for CMoteur


//*****************************************
void CMoteur::openPlayDocument(RPath *path)
//*****************************************
{
	if (debug) printf("CMoteur::openPlayDocument -- path %s\n", path->path());

	CMoteurPlay *mMoteurPlay = new CMoteurPlay(this, *path);
	M_ASSERT_PTR(mMoteurPlay);
	mDocList.addItem(mMoteurPlay);
	mMoteurPlay->launch(K_RUNABLE_PRI_NORMAL);

	printf("CApp::openPlayDocument -- mMoteurPlay <%p> moteur <%p> -- done & running\n", mMoteurPlay, this);

} // end of openPlayDocument for CMoteur


//*******************************************
void CMoteur::openRecordDocument(RPath *path)
//*******************************************
{
	if (debug) printf("CMoteur::openRecordDocument -- path %s\n", path->path());

	CMoteurRecord *mMoteurRecord = new CMoteurRecord(this, *path);
	M_ASSERT_PTR(mMoteurRecord);
	mDocList.addItem(mMoteurRecord);
	mMoteurRecord->launch(K_RUNABLE_PRI_NORMAL);

	printf("CApp::openRecordDocument -- done & running\n");

} // end of openRecordDocument for CMoteur



//---------------------------------------------------------------------------

// eoc
