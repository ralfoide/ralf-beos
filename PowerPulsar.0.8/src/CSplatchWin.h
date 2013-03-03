/*****************************************************************************

	Projet		: Pulsar

	Fichier	:	CSplatchWin.h
	Partie		: Loader

	Auteur		: RM
	Format		: tabs==2

*****************************************************************************/

#ifndef _H_CSPLATCHWIN_
#define _H_CSPLATCHWIN_

#include "globals.h"
#include "CWindow.h"


//---------------------------------------------------------------------------

class CSplatchView;
class CVertBarView;

//**************************************
class CSplatchWin : public CWindow
//**************************************
{
public:
	CSplatchWin(SWinPref &pref,
						const char *title, 
						window_type type,
						ulong flags,
						ulong workspace = B_CURRENT_WORKSPACE);
	virtual ~CSplatchWin(void);

	virtual bool init(bool aboutbox=true);
	void forceText(void);
	void startLoading(void);
	void endLoading(void);
	virtual bool QuitRequested(void);
	virtual void DispatchMessage(BMessage *message, BHandler *handler);
	virtual void WorkspacesChanged(uint32 old_ws, uint32 new_ws);
	virtual void ScreenChanged(BRect screen_size, color_space depth);
private:

	CSplatchView *mImage;
	CVertBarView *mBar;
	bool					mIsLoading;

}; // end of class defs for CSplatchWin


//---------------------------------------------------------------------------

#endif // of _H_CSPLATCHWIN_

// eoh
