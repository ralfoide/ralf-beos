/*****************************************************************************

	Projet	: Nerdkill 3d

	Fichier	: CApp.h
	Partie	: the Nerdkill 3d App

	Auteur	: RM
	Date		: 010698
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CNERD3DAPP_
#define _H_CNERD3DAPP_

#include "gMachine.h"
#include "RApp.h"
#include "externs.h"
#include "RPath.h"
#include <Directory.h>

//---------------------------------------------------------------------------
// app-wide defines

#define K_APP_MIME "application/x-vnd.ralf.SoundPatcher"
#define K_APP_NAME "SoundPatcher"
#define K_APP_VERS "0.0.1"

#define K_APP_DOC_MIME	"application/x-vnd.ralf.SoundPatcher-sound"
#define K_APP_PREF_MIME	"application/x-vnd.ralf.SoundPatcher-pref"

#define K_SETTINGS_NAME			"SoundPatcher.ini"
#define K_AUTOPLAY_NAME			"test.sp"

#define K_TRACKER_DIR_NAME	"Tracker"
#define K_ADDON_DIR_NAME		"add-ons"
#define K_IMG_DIR_NAME			"img"

//---------------------------------------------------------------------------

class CApp;
class CMoteur;

//---------------------------------------------------------------------------
// common public messages

#define K_MSG_CLOSE_DOCUMENT	'cDoc'		// lparam=nop, vparam=this object

//---------------------------------------------------------------------------


//**********************
class CApp : public RApp
//**********************
{
public:
	CApp(sptr sig = K_APP_MIME);
	virtual ~CApp(void);

	void init(void);
	void terminate(void);

	void aboutRequested(void);
	virtual void MessageReceived(BMessage *message);
	virtual void processMessage(ui32 msg, ui32 lparam=0, vptr data=NULL, ui32 *channel=NULL);
//	virtual void openDocument(RPath &path, rbool last=true);

	// --- methodes interface

	void moteurMsg(ui32 msg, ui32 lparam=0, vptr data=NULL, ui32 datasize=0);
//	void getAppPath(RPath &path);
//	void setFileType(RPath &path, sptr type);
	
	// --- membres publiques

protected:

	// create app-specific param window if any
	void		createWindow(void);

	BDirectory mAppDir;
	
	// moteur (thread)
	CMoteur	*mMoteur;

}; // end of class defs for CApp


//---------------------------------------------------------------------------

#endif // of _H_CNERD3DAPP_

// eoh
