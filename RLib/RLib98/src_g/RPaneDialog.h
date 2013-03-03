/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RPaneDialog.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 120798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RPANEDIALOG_
#define _H_RPANEDIALOG_

#include "gMachine.h"
#include "RPaneWindow.h"
#include "RLayerPane.h"
#include "RValuePane.h"
#include "RButton.h"
#include "RString.h"
#include "gErrors.h"

//---------------------------------------------------------------------------
// in the reply, the lparam contains the index of the button (0->2, from left to
// right) and the vparam contains the pointer onto the RPaneDialog.

#define K_MSG_PANEDIALOG_REPLY	'PdRy'

//---------------------------------------------------------------------------
// the keys reserved by the First, Second and Third auto-buttons.
// Since theses buttons are placed from left to right, they usually
// means first=OK, second=Cancel, third=Apply.
// By default the first button is the default one.
//
// If the user of the dialog doesn't provide the strings -- and thus
// the dialog won't create the button -- then the user might still
// create buttons that send these messages to the controlling thread.
//
// The thread receives these key as the 'msg', lparam set to 1 (button
// activated), and vparam is a pointer onto the dialog.

#define K_OK_BUTTON_PANEKEY 		(RPaneKey)'BuOK'
#define K_CANCEL_BUTTON_PANEKEY (RPaneKey)'BuCl'
#define K_APPLY_BUTTON_PANEKEY 	(RPaneKey)'BuAp'

#define K_OK_BUTTON							0
#define K_CANCEL_BUTTON					1
#define K_APPLY_BUTTON					2

//---------------------------------------------------------------------------

#define K_DIALOG_DEFAULT_BORDER	K_LAYOUT_DEFAULT_BORDER

//---------------------------------------------------------------------------


//************************************
class RPaneDialog : public RPaneWindow
//************************************
{
public:
	RPaneDialog(RRect frame, sptr title,
							sptr button0=NULL, sptr button1=NULL, sptr button2=NULL);
	virtual void deleteWindow(void);		// use this instead of ~RPaneDialog
	virtual ~RPaneDialog(void);					// must not be called explicitely

	// the two ways to use the dialog
	// the dialog is not finished initializing until one of these is
	// called.

	ui32 goModal(rbool closeAfterReply = true);
	void go(RThread &reply,
					ui32 msg=K_MSG_PANEDIALOG_REPLY,
					rbool closeAfterReply = true);

	// the assoc necessary
	// note that attached RValuablePane's will _automatically_ grab
	// the assoc via this function when attached.
	RPaneAssoc *getAssoc(void) { return &mAssoc; }

//----
protected:

	// startup parameters
	RString		mButtonName[3];
	RThread	 *mReply;
	ui32			mReplyMsg;
	rbool			mOwnThread;
	rbool			mCloseAfterReply;
	ui32			mSelected;

	// the assoc -- this makes a dialog different from a window
	RPaneAssoc	mAssoc;

	//----------------------------------
	class RAsyncDialog : public RThread
	//----------------------------------
	{
	public:
		RAsyncDialog(RPaneDialog *dialog) : RThread() { dialog = mDialog; }
		virtual void run(void);
		RPaneDialog *mDialog;
	}; // end of class RAsyncDialog

	friend class RAsyncDialog;

}; // end of class defs for RPaneDialog


//---------------------------------------------------------------------------

#endif // of _H_RPANEDIALOG_

// eoh
