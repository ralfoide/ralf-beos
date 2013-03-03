/*****************************************************************************

	Projet	: Pulsar

	Fichier:	CPrefWin.h
	Date		:	251097

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CPREFWIN_
#define _H_CPREFWIN_

#include "globals.h"
#include "CWindow.h"

//---------------------------------------------------------------------------

#define K_WINDOW_BORDER_WIDTH 5
#define K_MSG_MATROX	'mtrx'
#define K_MSG_UMAX		'umax'
#define K_MSG_PVV_DEF	'pvvD'
#define K_MSG_POV_DEF K_MSG_PVV_DEF

//---------------------------------------------------------------------------


//***********************************
class CPrefVideoView : public BView
//***********************************
{
public:
	CPrefVideoView(BRect frame,
					char *title,
					long resize,
					long flags=B_WILL_DRAW);
	~CPrefVideoView(void) { }

	#define inherited BView
	virtual void MessageReceived(BMessage *msg);
	virtual	void AttachedToWindow(void)		{ setValue(); inherited::AttachedToWindow(); }
	virtual	void DetachedFromWindow(void)	{ getValue(); inherited::DetachedFromWindow(); }
	#undef inherited

private:
	void setValue(void);
	void getValue(void);

	BRadioButton *mOffscreenSingle, *mOffscreenDouble, *mOffscreenTripple;
	BRadioButton *mOnBoardSingle, *mOnBoardDouble, *mOnBoardTripple;
	BRadioButton *mOnBoardMemory, *mOffscreenMemory;

	BCheckBox    *mMatroxSupport;
	BCheckBox		 *mUmaxSupport;
};


//***********************************
class CPrefOtherView : public BView
//***********************************
{
public:
	CPrefOtherView(BRect frame,
					char *title,
					long resize,
					long flags=B_WILL_DRAW);
	~CPrefOtherView(void) { }

#define inherited BView
	virtual void MessageReceived(BMessage *msg);
	virtual	void AttachedToWindow(void)		{ setValue(); inherited::AttachedToWindow(); }
	virtual	void DetachedFromWindow(void)	{ getValue(); inherited::DetachedFromWindow(); }
#undef inherited

private:
	void setValue(void);
	void getValue(void);

	BCheckBox    *mLoadDefaultConfig;
	BCheckBox    *mDisplayFrameRate;
	BCheckBox		 *mDisplayGkSup;
};


//***********************************
class CPrefView : public BView
//***********************************
{
public:
	CPrefView(BRect frame,
					char *title,
					long resize,
					const char *bitmapname,
					long flags=B_WILL_DRAW);
	~CPrefView(void) { if (mImage8) delete mImage8;
									if (mImage32) delete mImage32; }

	virtual void Draw(BRect upd);
	virtual void MouseDown(BPoint where);
	virtual	void MessageReceived(BMessage *msg);

private:

	BRect settingBounds;
	BRect bounds;
	BRect referenceBounds;
	BBitmap *mImage32;
	BBitmap *mImage8;
	uint32	 mSx, mSy;

	uint32	mCurrentSetting;
	uint32	mNbSetting;
	uint32	mSettingZoneSx;
	uint32	mSettingZoneSy;
	
	BView	*mCurrentView;
	BList	mSetting;			// list of Ptr on BView
};


//---------------------------------------------------------------------------


//***********************************
class CPrefWin : public CWindow
//***********************************
{
public:
	CPrefWin(SWinPref &pref,
							const char *title, 
							window_type type,
							ulong flags,
							ulong workspace = B_CURRENT_WORKSPACE);
	virtual ~CPrefWin(void);

	bool init(void);

	virtual bool QuitRequested(void);
	virtual	void MessageReceived(BMessage *msg);

private:

	CPrefView *mView;

}; // end of class defs for CPrefWin


//---------------------------------------------------------------------------

#endif // of _H_CPREFWIN_

// eoh

