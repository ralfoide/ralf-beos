/**************************************************************************

	Projet	: RLib

	Fichier	: TTrippleBuffer.h
	Partie	: Template based dynamic array of item

	Auteur	: RM
	Date		: 020899
	Format	: tabs==2

	Note		:	The tripple buffer is a special kind of double buffer
						with two accessing parts, one for reading and one for
						writing.
						The writer sees a classical double buffer.
						The reader sees also a classical double buffer.
						One a buffer has been written, it becomes pending. At the next
						read, the pending buffer will be selected.

	Semantic:

	[Writer]	-(write)->	[ buffer_write ]
								(swap) X
												[buffer_pending]
																				X (swap)
												[ buffer_read  ] -(read)-> [Reader]

	Usage :
	- Class declaration:
		B is a buffer class that implements the RLockInterface
		and the TDataInterface, thus B must also be able to coerce
		to type T* via the cast operator.
		Exemple : TTrippleBuffer< TData<si16>, si16 > tri_buf(1024);

	- Writer side :
		si16 * ptr = tri_buf.dataWrite();
		... write into ptr, size 1024 items ...
		tri_buf.swapWrite();

	- Reader side :
		si16 * ptr = tri_buf.dataRead();
		... read from ptr, size 1024 items ...
		tri_buf.swapRead();

	Notes		:
	- buffers are allocated at construction time & locked.
		An exception will be thrown on failure.
	- buffers are deallocated at desctruction time or when free()
		is called. No more access should occur after a free().
	- buffers are locked after being allocated and unlocked before
		being freed.
	- NO SYNCHRONIZATION is performed in this class.
		You must derive and implement synchronization in swapRead/swapWrite.
		The synchronization state may be held by the derived class or
		the buffer class.
	- dataRead/dataWrite never return a NULL pointer. When implementing
		synchronization if the state cannot be changed, the same buffer pointers
		will be returned.

**************************************************************************/


#ifndef _TTRIPPLEBUFFER_H_
#define _TTRIPPLEBUFFER_H_

#include "gMacros.h"
#include "gErrors.h"

//-------------------------------------------------------------------------
// A generic template for tripple buffering.
// The lock mechanism is implementation specific.
//
// B is a buffer class that implements the RLockInterface
// and the TDataAllocInterface, thus B must also be able to
// coerce to type T* via the cast operator.
//
// Exemple : TTrippleBuffer< TData<si16>, si16 >

//***********************************************
template< class B, class T > class TTrippleBuffer
//***********************************************
{
public:
								TTrippleBuffer(si32 buffer_size = 0);
	virtual			 ~TTrippleBuffer(void);

					void	alloc(si32 buffer_size);
					void	free(void);

	enum ETripBufIndex
	{
		kTripBufRead    = 0,
		kTripBufPending = 1,
		kTripBufWrite   = 2
	};

	inline	B &		bufferRead(void)		{ return       mBuffer[mIndex[kTripBufRead]];   }
	inline	B &		bufferWrite(void)		{ return       mBuffer[mIndex[kTripBufWrite]];  }
	inline	T *		dataRead(void)			{ return (T *)(mBuffer[mIndex[kTripBufRead]]);  }
	inline	T *		dataWrite(void)			{ return (T *)(mBuffer[mIndex[kTripBufWrite]]); }
	virtual	void	swapRead(void);
	virtual	void	swapWrite(void);

	inline	B &  	buffer(ETripBufIndex index)
		{ M_ASSERT(index>=0 && index<3); return mBuffer[mIndex[index]]; }
	inline	T *  	dataPtr(ETripBufIndex index)
		{ M_ASSERT(index>=0 && index<3); return (T *)(mBuffer[mIndex[index]]); }
protected:
	B							mBuffer[3];
	ETripBufIndex	mIndex[3];
	rbool					mAlloc;

}; // end of TTrippleBuffer





//-------------------------------------------------------------------------
// template-based implementation
//-------------------------------------------------------------------------


//*******************************************************
template< class B, class T >
TTrippleBuffer< B, T >::TTrippleBuffer(si32 buffer_size)
											:	mAlloc(false)
//*******************************************************
{
	mIndex[kTripBufRead   ] = kTripBufRead;
	mIndex[kTripBufPending] = kTripBufPending;
	mIndex[kTripBufWrite  ] = kTripBufWrite;

	if (buffer_size) alloc(buffer_size);	

} // end of constructor for TTrippleBuffer< B, T >


//*******************************************
template< class B, class T >
TTrippleBuffer< B, T >::~TTrippleBuffer(void)
//*******************************************
{
	free();

} // end of destructor for TTrippleBuffer< B, T >


//**************************************************
template< class B, class T >
void TTrippleBuffer< B, T >::alloc(si32 buffer_size)
//**************************************************
{
	M_CHECK(buffer_size > 0);
	M_ASSERT(!mAlloc);

	mIndex[kTripBufRead   ] = kTripBufRead;
	mIndex[kTripBufPending] = kTripBufPending;
	mIndex[kTripBufWrite  ] = kTripBufWrite;

	mBuffer[kTripBufRead   ].alloc(buffer_size);
	mBuffer[kTripBufPending].alloc(buffer_size);
	mBuffer[kTripBufWrite  ].alloc(buffer_size);

	mBuffer[kTripBufRead   ].lockThrow();	M_ASSERT_PTR(dataPtr(kTripBufRead   ));
	mBuffer[kTripBufPending].lockThrow();	M_ASSERT_PTR(dataPtr(kTripBufPending));
	mBuffer[kTripBufWrite  ].lockThrow();	M_ASSERT_PTR(dataPtr(kTripBufWrite  ));

	mAlloc = true;
	
} // end of constructor for TTrippleBuffer< B, T >


//*************************************
template< class B, class T >
void TTrippleBuffer< B, T >::free(void)
//*************************************
{
	if (!mAlloc) return;

	mBuffer[kTripBufWrite  ].unlock();
	mBuffer[kTripBufPending].unlock();
	mBuffer[kTripBufRead   ].unlock();
	
	mBuffer[kTripBufWrite  ].free();
	mBuffer[kTripBufPending].free();
	mBuffer[kTripBufRead   ].free();

	mAlloc = false;

} // end of free for TTrippleBuffer< B, T >


//*****************************************
template< class B, class T >
void TTrippleBuffer< B, T >::swapRead(void)
//*****************************************
{
	ETripBufIndex a = mIndex[kTripBufRead];
	mIndex[kTripBufRead] = mIndex[kTripBufPending];
	mIndex[kTripBufPending] = a;
} // end of swapRead for TTrippleBuffer< B, T >


//******************************************
template< class B, class T >
void TTrippleBuffer< B, T >::swapWrite(void)
//******************************************
{
	ETripBufIndex a = mIndex[kTripBufWrite];
	mIndex[kTripBufWrite] = mIndex[kTripBufPending];
	mIndex[kTripBufPending] = a;
} // end of swapWrite for TTrippleBuffer< B, T >



#endif // of _TTRIPPLEBUFFER_H_

//--------------------------------- eoh ----------------------------------
