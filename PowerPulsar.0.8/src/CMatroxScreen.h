/*****************************************************************************

	Projet	: Pulsar

	Fichier:	CMatroxScreen.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CMATROXSCREEN_
#define _H_CMATROXSCREEN_

#include "globals.h"
#include "CFilter.h"
#include "CScreen.h"

// for pci_info and get_nth_pci_info()
#include <drivers/PCI.h>
// for load_add_on()
#include <image.h>
#include <GraphicsCard.h>

//---------------------------------------------------------------------------

// typedef long (*control_graphics_card)(ulong message,void *buf);
typedef long (*__control_graphics_card)(ulong, void*);	


//---------------------------------------------------------------------------


//**************************************
class CMatroxScreen : public BWindow
//**************************************
{
public:
	CMatroxScreen(const char *title, ulong space = B_8_BIT_640x480);
	virtual ~CMatroxScreen(void);

	bool init(void);

	void swap(uint8 * &screen, uint16 &line);
	void lock_draw();
	void unlock_draw();

	sem_id getInitCompleteSem(void) { return initCompleteSem; }
	void getHooks(CFilter &ref);
	void getInfos(int32 &sx, int32 &sy, EColorSpace &_mode,
								uint8	* &buffer, int32 &bpr,
								int32 &maxLine, int32 &pulsarLine);

	virtual void ScreenConnected(bool active);
	virtual void Quit(void);
	virtual bool QuitRequested(void);
	virtual void DispatchMessage(BMessage *message, BHandler *handler);

private:

	void MoveDisplayArea(short x, short y);
	graphics_card_info *CardInfo(void) { return &gci; }
	graphics_card_hook CardHookAt(ulong index) { return hooks[index]; }
	frame_buffer_info *FrameBufferInfo(void) { return &fbi; }

	BView *mFakeView;

	status_t   error;
	sem_id initCompleteSem;

	bool	first;
	bool	drawing_enable;

	EScreenMemory		mMemory;
	EScreenBuffering	mOffscreenBuffering;
	EScreenBuffering	mOnBoardBuffering;

	// utility booleans
	bool	 	mIsOffscreen;
	bool		mUmaxHack;
	bool		mUseMatrox;

	ulong space;
	EColorSpace mode;
	int	mOnBoardPageNum;			// number of visible page
	int	mOffscreenPageNum;
	short row;
	short height;
	uchar *base, *draw_base, *buffer;

	int32 draw_lock;		// for the draw benaphore
	sem_id draw_sem;

	uchar *mScreenBuffer;
	uint32	mScreenBufSize1, mScreenBufSize2;

	void	test_draw(void);

	// --- driver stuff

	pci_info pcii;
	image_id iid;
	__control_graphics_card cgc;
	graphics_card_spec gcs;
	graphics_card_config gcc;
	graphics_card_info gci;
	graphics_card_hook hooks[B_HOOK_COUNT];
	frame_buffer_info fbi;


}; // end of class defs for CMatroxScreen


//---------------------------------------------------------------------------

#endif // of _H_CMATROXSCREEN_

// eoh

