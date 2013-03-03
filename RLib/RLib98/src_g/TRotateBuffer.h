/**************************************************************************

	Projet	: Portable Lib

	Fichier	: TRotateBuffer.h
	Partie	: Template based rotating buffer

	Auteur	: RM
	Date		: 060698
	Format	: tabs==2

	WARNING ! In fact this implementation ASSUMES PV to be a pointer.
	Items are initialized to NULL and NULL is returned by a failing functions.

**************************************************************************/


#ifndef _H_TROTATEBUFFER_
#define _H_TROTATEBUFFER_

#include "gMachine.h"
#include "gMacros.h"
#include "gErrors.h"
#include "TList.h"
#include "RSem.h"

#define BUF_DEBUG

//-------------------------------------------------------------------------
// "PV" acronym stands for "Pointer on Void"
// and is used to remind that hopefully only small structs or pointers
// should be used for the template herein.
//
// WARNING ! In fact this implementation ASSUMES PV to be a pointer.
// Items are initialized to NULL and NULL is returned by a failing functions.

//***************
enum ERotateState
//***************
{
	kRotateFree,
	kRotateActiveWrite,
	kRotateActiveRead,
	kRotateReady
};

//*************************************
template< class PV > struct SRotateItem
//*************************************
{
	PV						mItem;
	ERotateState 	mState;
};

//**********************************************************
template< class PV > class TRotateBuffer : public RInterface
//**********************************************************
{
public:
  TRotateBuffer(si32 nbitems = 3);
  virtual ~TRotateBuffer(void);

	// services

	void	initItemAt(si32 index, PV item);
	PV		itemAt(si32 index);

	PV		acquireWrite(void);
	void	releaseWrite(void);
	PV		acquireRead(void);
	void	releaseRead(void);

protected:

	// perform allocations and deletions (need not be called,
	// since already called by the cons/destructor).
	void	init(si32 nbitems);
	void	makeEmpty(void);

	// list of buffers
	si32											mNbBuffer;
	TList<SRotateItem<PV> *>	mBufferList;

	// current indexes
	si32	mCurrentRead;
	si32	mCurrentWrite;

	// housekeeping
	RSem	mLock;

}; // end of TRotateBuffer



//-------------------------------------------------------------------------
// template-based implementation
//-------------------------------------------------------------------------


//******************************************************************************
template< class PV > inline TRotateBuffer< PV >::TRotateBuffer(si32 nbitems)
//******************************************************************************
/*
   Renvoie l'element "index" s'il existe ou NULL.
   Le premier element est a l'index 0.
*/
{
	mNbBuffer = 0;
	mCurrentRead = mCurrentWrite = 0;
	init(nbitems);
} // end of itemAt for TRotateBuffer


//*************************************************************************
template< class PV > inline TRotateBuffer< PV >::~TRotateBuffer(void)
//*************************************************************************
{
	makeEmpty();
	mNbBuffer = 0;

} // end of destructor for TRotateBuffer< PV >


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


//*************************************************************************
template< class PV > inline void TRotateBuffer< PV >::init(si32 nbitems)
//*************************************************************************
{
si32 n;

	mBufferList.makeEmpty();
	mNbBuffer = nbitems;
	mCurrentRead = mCurrentWrite = 0;
	if (!mNbBuffer) return;
	for(n=mNbBuffer; n>0; n--)
	{
		SRotateItem<PV> *item = new SRotateItem<PV>;
		M_ASSERT_PTR(item);
		item->mItem = NULL;
		item->mState = kRotateFree;
		if (!mBufferList.addItem(item)) M_THROW(kErrInit);
	}

} // end of makeEmpty for TRotateBuffer< PV >


//*************************************************************************
template< class PV > inline void TRotateBuffer< PV >::makeEmpty(void)
//*************************************************************************
{
SRotateItem<PV> *item;

	if (!mNbBuffer) return;
	mNbBuffer = 0;
	mCurrentRead = mCurrentWrite = 0;
	while((item = mBufferList.removeLastItem()) != NULL) delete item;

} // end of makeEmpty for TRotateBuffer< PV >


//*************************************************************************
template< class PV > inline void TRotateBuffer< PV >::initItemAt(si32 index, PV _item)
//*************************************************************************
{
SRotateItem<PV> *item;

	item = mBufferList.itemAt(index);
	if (!item) M_THROW(kErrInit);
	item->mItem = _item;

} // end of initItemAt for TRotateBuffer< PV >


//*************************************************************************
template< class PV > inline PV TRotateBuffer< PV >::itemAt(si32 index)
//*************************************************************************
{
SRotateItem<PV> *item;

	item = mBufferList.itemAt(index);
	return item->mItem;

} // end of removeLastItem for TRotateBuffer< PV >


//*************************************************************************
template< class PV > inline PV TRotateBuffer< PV >::acquireWrite(void)
//*************************************************************************
{
SRotateItem<PV> *item;
si32 i,j;
si32 n = mNbBuffer;

	mLock.acquire();

	#ifdef BUF_DEBUG
		i = j = mCurrentWrite;
		item = mBufferList.itemAt(i);
		mLock.release();
		return item->mItem;
	#endif

	i = j = mCurrentWrite;
	i++;
	if (i == n) i = 0;

	item = mBufferList.itemAt(i);
	if (item->mState == kRotateActiveRead)
	{
		// assert : mCurrentRead == i here
		SRotateItem<PV> *item2 = mBufferList.itemAt(j);
		PV						_mItem 	= item2->mItem;
		ERotateState 	_mState = item2->mState;
		item2->mItem 	= item->mItem;
		item2->mState = item->mState;
		item->mItem 	= _mItem;
		item->mState 	= _mState;
		mCurrentRead = j;
	}
	item->mState = kRotateActiveWrite;

	mCurrentWrite = i;
	mLock.release();
	return item->mItem;

} // end of acquireWrite for TRotateBuffer< PV >


//*************************************************************************
template< class PV > inline void TRotateBuffer< PV >::releaseWrite(void)
//*************************************************************************
{
SRotateItem<PV> *item;

	mLock.acquire();
	item = mBufferList.itemAt(mCurrentRead);
	item->mState = kRotateReady;
	mLock.release();

} // end of releaseWrite for TRotateBuffer< PV >


//*************************************************************************
template< class PV > inline PV TRotateBuffer< PV >::acquireRead(void)
//*************************************************************************
{
SRotateItem<PV> *item;
si32 i,j,ok=-1;
si32 n = mNbBuffer;

	// HOOK TBDL -- not clear & suspicious

	mLock.acquire();

	#ifdef BUF_DEBUG
		i = j = mCurrentRead;
		item = mBufferList.itemAt(i);
		mLock.release();
		return item->mItem;
	#endif


	i = j = mCurrentRead;
	do
	{
		i++;
		if (i == n) i = 0;
		item = mBufferList.itemAt(i);
		if (item->mState == kRotateReady) ok = i;
		else break;
	}
	while(i != j);

	if (ok < 0) ok = j;
	item = mBufferList.itemAt(i);
	item->mState = kRotateActiveRead;
	mCurrentRead = i;

	mLock.release();
	return item->mItem;

} // end of acquireRead for TRotateBuffer< PV >


//*************************************************************************
template< class PV > inline void TRotateBuffer< PV >::releaseRead(void)
//*************************************************************************
{
SRotateItem<PV> *item;

	mLock.acquire();
	item = mBufferList.itemAt(mCurrentRead);
	item->mState = kRotateFree;
	mLock.release();

} // end of releaseRead for TRotateBuffer< PV >


#endif // of _H_TROTATEBUFFER_

//--------------------------------- eoh ----------------------------------
