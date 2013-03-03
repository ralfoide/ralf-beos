/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CFilledWin.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_CFILLEDWIN_
#define _H_CFILLEDWIN_

#include "globals.h"
#include "CFilledView.h"

//---------------------------------------------------------------------------

class CCdButton;			// in CInterfaceWin.h


//*******************************
class CFilledWin : public BWindow
//*******************************
{
public:
			CFilledWin(	BRect frame,
						const char *title, 
						window_type type,
						ulong flags,
						ulong workspace = B_CURRENT_WORKSPACE);
virtual		~CFilledWin(void);

		bool init(void);

virtual	void DispatchMessage(BMessage *message, BHandler *handler);
virtual	void MessageReceived(BMessage *message);
virtual	bool QuitRequested(void);
virtual	void WindowActivated(bool state);
virtual	void FrameResized(float new_width, float new_height);

inline	void forceRedraw(void) { if (mView) mView->Pulse(); }

		bool hasViewPtr(void);
		void setViewPtr(CFilledView *view);
inline	bool setColorSpace(EColorSpace mode)
			 { if (mView) return mView->setColorSpace(mode); else return false; }
inline	void getInfos(int32 &sx, int32 &sy,
					  EColorSpace &mode,
					  uint8	* &buffer, int32 &bpr)
			 { if (mView) mView->getInfos(sx, sy, mode, buffer, bpr);}

private:

		void setZoomDivider(int32 delta);

	int32		mZoomDivider;
	CFilledView *mView;
	CCdButton	*mZoomSmall, *mZoomBig;

}; // end of class defs for CFilledWin


//---------------------------------------------------------------------------

#endif // of _H_CFILLEDWIN_

// eoh
