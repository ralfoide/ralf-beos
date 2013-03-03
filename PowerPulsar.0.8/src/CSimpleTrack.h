/*****************************************************************************

	Projet	: Pulsar

	Fichier	:	CInterfaceWin.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CSIMPLETRACK_
#define _H_CSIMPLETRACK_

#include "Benaphore.h"
#include "globals.h"
#include <Path.h>

//---------------------------------------------------------------------------

class CFilter;

//---------------------------------------------------------------------------



//************
enum ELineMode
//************
{
	kLineModeOff,
	kLineModeOn,
	kLineModeRepeat,
	kLineModeHand,
	kLineModeMax		// number of modes in this enum
};


//***************
struct SLineParam
//***************
{
	SLineParam(void){ filter = NULL; start = 0; len = 10000+(rand()%20000);
									 mode=kLineModeOn; repeat = len+500+(rand()%5000);
									 inBlock=false; blockIndex=0;
								}

	CFilter *filter;
	ELineMode mode;
	int32		start, len, repeat;	// 0 minimum. in milliseconds

	BMessage	param;
	bool			inBlock;
	uint32		blockIndex;
	double		current_start, current_stop, current_repeat;
};


// HOOK RM : B.'s Mutex doesn't work for x86. No time to find out why.
// Wrapping a new one around a BLocker

class CTrackMutex
{
public:
	CTrackMutex(void) : mLocker("CTrackMutex") {}
	~CTrackMutex(void) {}
	status_t Reset(bool locked, char *name = "") { return B_NO_ERROR; }
	status_t Reset(void) { return B_NO_ERROR; }
	status_t Lock() { return mLocker.Lock(); }
	status_t LockIfAvailable(void) { return B_ERROR; } // not implemented bcoz not used below
	status_t Unlock(void) { mLocker.Unlock(); return B_NO_ERROR; }
private:
	BLocker mLocker;
};




//****************
class CSimpleTrack
//****************
{
public:
	CSimpleTrack(void)		{ mLocker.Reset(false, "track_mutex"); }
	~CSimpleTrack(void)	{ }

	void init(void)	{ }

	status_t lock(void)							{ return (mLocker.Lock() >= B_NO_ERROR);					}
	void			unlock(void)						{ mLocker.Unlock(); 														}

	uint32			countLine(void)				{ return mLineList.CountItems();									}
	SLineParam *getLine(uint32 index)	{ return (SLineParam *)mLineList.ItemAt(index);		}
	void				addLine(SLineParam *p)	{ mLineList.AddItem(p); 												}
	SLineParam	*delLine(uint32 index)	{ return (SLineParam *)mLineList.RemoveItem(index);	}

private:

	CTrackMutex			mLocker;			// locker for the list array
	BList 			mLineList;		// list of ptrs on SLineParam

};


//*******************************
class CTrackHeader : public BView
//*******************************
{
public:
	CTrackHeader(BRect frame,
								const char *name,
								uint32 resizeMask = B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, 
								uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);

	~CTrackHeader(void) {	}

	virtual void FrameResized(float new_width, float new_height);
	virtual void Draw(BRect upd);

private:

	float	mTimeScale;	// x = time_in_millisecond*timeScale, ts=x/time_ms
	// exemple : x=100 pour 1 mn -> timeScale = 100/(60e3)

	BRect bounds;
	BFont	mFont;
	font_height mFontHeight;
	float mFontOffset;
};


//*****************************
class CTrackView : public BView
//*****************************
{
public:
	CTrackView(BRect frame,
								const char *name,
								//BMessage *message,
								uint32 resizeMask = B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM, 
								uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);

	~CTrackView(void) {	}

	void makeDefaultList(BList &filterList);
	void makeEmptyList(void);
	void updateList(void);

	virtual void FrameResized(float new_width, float new_height);
	virtual void MouseDown		(BPoint where);
	virtual void MouseMoved		(BPoint where, uint32 code, const BMessage *a_message);
	virtual void Draw				(BRect upd);
	virtual void MessageReceived(BMessage *message);

	void drawLine(BRect itemrect, int32 index, bool erase=false, BRect *upd=NULL);

private:

	uint32	mNbLine;
	uint32	mLineSize;
	uint32	mColumn;
	float	mTimeScale;	// x = time_in_millisecond*timeScale, ts=x/time_ms
	// exemple : x=100 pour 1 mn -> timeScale = 100/(60e3)

	BBitmap *mPositionBitmap;
	BRect		mPositionSize;
	long			mPositionX, mPositionY;

	BBitmap *mModeBitmap;
	BRect		mModeSize;
	long			mModeX, mModeY;

	bool mChangedLine;
	BRect bounds;
	bool	mMouseInside;
	bool mIsDropTarget;
	BPoint mMouseCoord;
	int32	mOverIndex;
	BFont	mFont;
	font_height mFontHeight;
	float mFontOffset;

	enum EOverInsertPoint
	{
		kOverInsertMiddle,
		kOverInsertBefore,
		kOverInsertAfter
	};

	EOverInsertPoint	mOverInsertPoint;


	//BList	mLine;	// ptr of SLineParam
};


//---------------------------------------------------------------------------

#endif // of _H_CSIMPLETRACK_

// eoh

