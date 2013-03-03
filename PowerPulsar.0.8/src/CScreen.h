/*****************************************************************************

	Projet	: Pulsar

	Fichier:	CScreen.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CSCREEN_
#define _H_CSCREEN_

#include "globals.h"
#include "CFilter.h"

//---------------------------------------------------------------------------
class CFilter;


//****************
enum EScreenMemory
//****************
{
	kScreenMemoryOnBoard,
	kScreenMemoryOffscreen
};


//*******************
enum EScreenBuffering
//*******************
{
	kScreenBufferingSingle  = 1,
	kScreenBufferingDouble = 2,
	kScreenBufferingTripple = 3
};


//---------------------------------------------------------------------------


//**************************************
class CScreen : public BWindowScreen
//**************************************
{
public:
	CScreen(const char *title, ulong space = B_8_BIT_640x480);
	virtual ~CScreen(void);

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
	virtual	void DispatchMessage(BMessage *message, BHandler *handler);

private:

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
	void	benchmark_memory(uchar *video, uchar *memory);

}; // end of class defs for CScreen


//---------------------------------------------------------------------------

#endif // of _H_CSCREEN_

// eoh

