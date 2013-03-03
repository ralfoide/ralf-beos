/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CInterfaceWin.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_CINTERFACEWIN_
#define _H_CINTERFACEWIN_

#include "globals.h"
#include "CWindow.h"
#include <Path.h>

//---------------------------------------------------------------------------

#define K_EXT ".tif"	// HOOK RM 161198 -- ".png"
#define K_EXT2 ".tga"	// HOOK RM 161198 -- K_EXT

#define K_IMG_DIR				"img_pulsar"
//#define K_IMG_FOND			"fond2" K_EXT
#define K_IMG_FOND				"matiere" K_EXT
#define K_IMG_BOUTON_DEFAULT	"boutons" K_EXT
#define K_IMG_BOUTON_FULL		"fullscreen" K_EXT
#define K_IMG_BOUTON_START		"start" K_EXT
#define K_IMG_BOUTON_PREVIEW	"preview" K_EXT
#define K_IMG_BOUTON_CD			"cdbtn" K_EXT
#define K_IMG_BOUTON_AIFF		"aiffbtn" K_EXT
#define K_IMG_BOUTON_MIC		"micbtn" K_EXT
#define K_IMG_BOUTON_MP3		"mp3btn" K_EXT
#define K_IMG_BOUTON_MOD		"mopbtn" K_EXT
#define K_IMG_ADDONLISTITEM		"addons" K_EXT
#define K_IMG_BOUTON_QUIT		"quit" K_EXT2
#define K_IMG_BOUTON_INFOS		"infos" K_EXT
#define K_IMG_BOUTON_ABOUT		"about" K_EXT
#define K_IMG_BOUTON_LOAD		"load" K_EXT
#define K_IMG_BOUTON_SAVE		"save" K_EXT2
#define K_IMG_BOUTON_NEW		"new" K_EXT
#define K_IMG_BOUTON_FONDCD		"table" K_EXT		//K_EXT - 483*108
#define K_IMG_BOUTON_BTNCD		"cdplay2a" K_EXT 	//K_EXT - 152*44 / 4*2 / 6*2
#define K_IMG_POSITION			"position2" K_EXT
#define K_IMG_MODE				"modes_frho" K_EXT

#define K_BTN_DEF_OFF	0
#define K_BTN_DEF_ON	1
#define K_BTN_DEF_OVER	2
#define K_BTN_DEF_DISA	3

#define K_ADDONLISTITEM_OFF		0
#define K_ADDONLISTITEM_OVER	1

#define K_ADDON_SX	64
#define K_ADDON_SY 	64

#define K_ICON_PX 	6
#define K_ICON_PY 	6

#define K_INPUT_SX	69
#define K_INPUT_SY	25

#define K_PLAY_SX	105
#define K_PLAY_SY	25

#define K_CD_SX		38
#define K_CD_SY 	22
#define K_CD_PLAY	0
#define K_CD_STOP	1
#define K_CD_PREV	2
#define K_CD_NEXT	3
#define K_CD_PAUSE	4
#define K_CD_EJECT	5

#define K_MSG_DEL_LINE_PARAM 'dlLP'

//---------------------------------------------------------------------------

#define K_WINDOW_BORDER_WIDTH 5

#define K_MENU_ITEM_8_BPP	0
#define K_MENU_ITEM_16_BPP	1
#define K_MENU_ITEM_32_BPP	2


//---------------------------------------------------------------------------

class CCdTable;
class CFondPatternView;
class CButton;
class CCdButton;
class CAddonListView;
class CTrackHeader;
class CTrackView;
class CFilter;
class CInterfaceWin;

//---------------------------------------------------------------------------


//****************************************
class CMyFileRefFilter : public BRefFilter
//****************************************
{
public:
virtual bool Filter(const entry_ref	*ref,
					BNode			*node,
					struct stat		*st,
	   				const char		*mimetype);
};




//---------------------------------------------------------------------------


//**********************************
class CInterfaceWin : public CWindow
//**********************************
{
public:
				CInterfaceWin(SWinPref &pref,
							  const char *title, 
							  window_type type,
							  ulong flags,
							  ulong workspace = B_CURRENT_WORKSPACE);
virtual 		~CInterfaceWin(void);

virtual	bool	init(void);

virtual void	DispatchMessage(BMessage *message, BHandler *handler);
virtual void	MessageReceived(BMessage *message);
virtual bool	QuitRequested(void);

		void	listFilters(BList &filterList);
		void	makeDefaultList(BList &filterList);
		void	createCdInterface(BView *back);

		void	askLoad(void);
		void	askSave(void);
		void	askSaveAs(void);
		void	doLoad(BMessage *msg);
		void	doSaveAs(BMessage *msg);
		void	loadFromFile(BPath &path);

private:

		void	setInputMethod(int32 code);
		void	displayTrack(CCdButton *play);
		int		createMenu(void);
		void	saveToFile(BPath &path);

		uint32	mBpp;

		bool	mMustSaveAs;
		BFilePanel *mLoadPanel, *mSavePanel;
		CMyFileRefFilter *mMyFileRefFilter;
		BPath	mLastSavePath;

		CAddonListView	 *mAddonList;
		CFondPatternView *mFond;
		CButton *mQuitBtn, *mPlayBtn, *mStopBtn;

		CButton	*mFullBtn, *mFullStartBtn, *mPreviewBtn;
		CButton	*mCdBtn, *mMicBtn, *mAiffBtn, *mMp3Btn, *mModBtn;
		BPopUpMenu *mMenuBpp;

		CTrackView *mTrack;

		BStringView *mCurrFps;
		CCdTable *mCdTable;
		CCdButton *mCdPlay, *mCdStop, *mCdEject, *mCdNext, *mCdPrev;
		BStringView *mCdIndex;

		BMenuBar	*mMenuBar;
		BMenu		*mMenuFile, *mMenuEdit, *mMenuInput, *mMenuStream,
					*mMenuCd, *mMenuView, *mMenuNetwork, *mMenuAddons;

}; // end of class defs for CInterfaceWin


//---------------------------------------------------------------------------

#endif // of _H_CINTERFACEWIN_

// eoh

