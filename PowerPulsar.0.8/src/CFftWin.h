/*****************************************************************************

	Projet	: Pulsar

	Fichier	:	CFftWin.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CFFTWIN_
#define _H_CFFTWIN_

#include "globals.h"
#include "CPulsarApp.h"

//---------------------------------------------------------------------------

class CFftView;
class CSoundView;


//*******************************
class CFondFftView : public BView
//*******************************
{
public:
	CFondFftView(BRect frame, char *title, long resize, long flags=B_WILL_DRAW, bool chan=false);
	~CFondFftView(void) {}

	virtual	void KeyDown(const char *bytes, int32 numBytes)
		{ gApplication->touches(bytes, numBytes); }
	virtual void MouseDown(BPoint where) { MakeFocus(true); }

	void Draw(BRect upd);
private:
	BBitmap *mFond;
};


//*****************************
class CFftWin : public BWindow
//*****************************
{
public:
	CFftWin(	BRect frame,
							const char *title, 
							window_type type,
							ulong flags,
							ulong workspace = B_CURRENT_WORKSPACE);
	virtual ~CFftWin(void);

	bool init(void);

	virtual	void MessageReceived(BMessage *message);
	virtual bool QuitRequested(void);

	void forceRedraw(void);

private:

	CFondFftView	*mFond;
	BView				*mView[4];
	CFftView		*mFftView[2];
	CSoundView	*mSoundView[2];

}; // end of class defs for CFftWin


//---------------------------------------------------------------------------

#endif // of _H_CFFTWIN_

// eoh
