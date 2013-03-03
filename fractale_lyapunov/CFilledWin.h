/*****************************************************************************

	Projet	: Pulsar

	Fichier	:	CFilledWin.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CFILLEDWIN_
#define _H_CFILLEDWIN_


//---------------------------------------------------------------------------

#undef K_USE_ZOOM

//---------------------------------------------------------------------------

#include "main.h"
#include "Window.h"
#include "CFilledView.h"

//---------------------------------------------------------------------------
// You can post a message like this one to the window to force it to redraw
// just call :
// CFilledWin *win;
// win->PostMessage(K_MSG_STREAM);
//
// This is stricly equivalent to call : win->forceRedraw();

#define K_MSG_STREAM	'filW'


//---------------------------------------------------------------------------

#ifdef K_USE_ZOOM
	class CCdButton;			// in CInterfaceWin.h
#endif

class CFilledView;

//**************************************
class CFilledWin : public BWindow
//**************************************
{
public:
	CFilledWin(	BRect frame,
							const char *title, 
							window_type type,
							ulong flags,
							ulong workspace = B_CURRENT_WORKSPACE);
	virtual ~CFilledWin(void);

	bool init(void);

	virtual	void MessageReceived(BMessage *message);
	virtual bool QuitRequested(void);
	virtual	void WindowActivated(bool state);
	virtual	void FrameResized(float new_width, float new_height);

	inline void forceRedraw(void) { if (mView) mView->Pulse(); }

	bool hasViewPtr(void);
	void setViewPtr(CFilledView *view);
	inline void getInfos(int32 &sx, int32 &sy, color_space &mode,
												uint8	* &buffer, int32 &bpr)
								{ if (mView) mView->getInfos(sx, sy, mode, buffer, bpr);}

private:


	CFilledView *mView;

#ifdef K_USE_ZOOM
	void setZoomDivider(int32 delta);
	int32			mZoomDivider;
	CCdButton		*mZoomSmall, *mZoomBig;
#endif

}; // end of class defs for CFilledWin


//---------------------------------------------------------------------------

#endif // of _H_CFILLEDWIN_

// eoh
