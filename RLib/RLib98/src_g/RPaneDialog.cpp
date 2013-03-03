/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RPaneDialog.cpp
	Partie	: GUI kit

	Auteur	: RM
	Date		: 110798
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#include "RPaneDialog.h"
#include "RLayoutPane.h"
#include "RErrEx.h"
#include "gErrors.h"


//---------------------------------------------------------------------------


//***************************************************************************
RPaneDialog::RPaneDialog(RRect frame, sptr title,
												 sptr button0, sptr button1, sptr button2)
						:RPaneWindow(frame, title)
//***************************************************************************
{
	mButtonName[0] = button0;
	mButtonName[1] = button1;
	mButtonName[2] = button2;
	mReply = NULL;
	mReplyMsg = K_MSG_PANEDIALOG_REPLY;
	mCloseAfterReply = false;
	mSelected = 0;
	mOwnThread = false;

} // end of constructor for RPaneDialog


//************************************************************************
void RPaneDialog::deleteWindow(void)
//************************************************************************
{
	// destroy the window and it's associated BeOS thread

	if (mReply && mOwnThread)
	{
		mReply->askToKill();
		mReply->waitForEnd();
		delete mReply;
		mReply = NULL;
	}


	RPaneWindow::deleteWindow();
} // end of deleteWindow for RPaneDialog


//***************************************************************************
RPaneDialog::~RPaneDialog(void)
//***************************************************************************
{
} // end of destructor for RPaneDialog


//---------------------------------------------------------------------------


//***************************************************************************
ui32 RPaneDialog::goModal(rbool closeAfterReply)
//***************************************************************************
{
	mCloseAfterReply = closeAfterReply;

	// create the thread...
	mOwnThread = true;
	mReply = new RAsyncDialog(this);
	M_ASSERT_PTR(mReply);
	mReply->launch(K_RUNABLE_PRI_NORMAL);

	// create the buttons, show the dialog
	go(*mReply);

	// wait for the thread to return
	mReply->waitForEnd();
	delete mReply;
	mReply = NULL;
	mOwnThread = false;

	return mSelected;

} // end of goModal for RPaneDialog


//***************************************************************************
void RPaneDialog::go(RThread &reply, ui32 msg, rbool closeAfterReply)
//***************************************************************************
{
ui32 i;
ui32 count=0;
ui32 indexes[3];
RPaneKey key[3] = {K_OK_BUTTON_PANEKEY, K_CANCEL_BUTTON_PANEKEY, K_APPLY_BUTTON_PANEKEY};

	if(debug) printf("RPaneDialog::go\n");

	mReply = &reply;
	mReplyMsg = msg;
	mCloseAfterReply = closeAfterReply;

	// if there is at least one button to add...
	for(i=0; i<3; i++) if (mButtonName[i].len()) indexes[count++] = i;
	if (count)
	{
		// get boundary of window content
		ui32 sx;
		ui32 sy;
		size(sx, sy);
		RRect r(0,0, sx-1,sy-1);
		si32 y = sy + K_DIALOG_DEFAULT_BORDER;
		si32 x = sx - K_DIALOG_DEFAULT_BORDER;

		// create buttons
		for(i=0; i<count; i++)
		{
			if(debug) printf(" -- RPaneDialog::go -- add button %ld\n", i);
			RButton *button = new RButton(mButtonName[indexes[i]], key[indexes[i]], (i==0), mReply);
			M_ASSERT_PTR(button);
			addPane(button);
			RRect r2 = button->bounds();
			x -= r2.width();
			button->moveTo(x, y);
			r = r | button->frame();
			x -= K_DIALOG_DEFAULT_BORDER;
		}

		resizeTo(r.width(), r.height());
	}

	show(true);

} // end of go for RPaneDialog


//---------------------------------------------------------------------------

//***************************************************************************
void RPaneDialog::RAsyncDialog::run(void)
//***************************************************************************
{
ui32 msg;
ui32 msgParam;
vptr msgData;
rbool loop = true;

	if (debug) printf("RAsyncDialog::run -- start\n");

	M_ASSERT_PTR(mDialog);

	try
	{
		while(loop)
		{
			//lecture message
			waitForMessage(msg, msgParam, msgData);

			// process message
			switch(msg)
			{
				case K_MSG_RTHREAD_EXIT:
					loop = false;
					break;
				case K_OK_BUTTON_PANEKEY:
					mDialog->mSelected = K_OK_BUTTON;
					break;
				case K_CANCEL_BUTTON_PANEKEY:
					mDialog->mSelected = K_CANCEL_BUTTON;
					break;
				case K_APPLY_BUTTON_PANEKEY:
					mDialog->mSelected = K_APPLY_BUTTON;
					break;
			}
		}
	}
	M_CATCH("RAsyncDialog Wait Loop");

	if (debug) printf("RAsyncDialog::run -- end of loop\n");

} // end of run for RAsyncDialog for RPaneDialog

//---------------------------------------------------------------------------

// eoc
