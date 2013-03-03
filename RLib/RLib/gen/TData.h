/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	TData.h
	Partie	: RLib basics

	Auteur	: RM
	Date		: 073099
	Format	: tabs==2

	Two general purpose implementations of the TDataInteraface.
	TData is a bunch of data that allocates or free itself.
	TDataWrapper is a bunch of data that wraps an exisiting memory buffer.
	The difference is that TData owns its own data (thus allocate and free
	it as necessary) whilst TDataWrapper does not own anything (and thus
	won't free anything when destructed).
	Both support the RLockInterface, meaning that you should lock/unlock
	the buffers before accessing them.

	Note		: TRArray and TData may seem to have close functionnality.
						TRArray is supposedly a re-allocatable, dynamic, unlockable
						array of structured data. At the contrary, TData is
						semantically a fixed bunch of bytes, which does not support
						re-allocation but supports a lock/swap mechanism.

*****************************************************************************/

#ifndef _H_TDATA_
#define _H_TDATA_

#include "gMachine.h"
#include "TDataInterface.h"

//---------------------------------------------------------------------------

enum EDataAllocFlag
{
	kDataAllocDefault,
	kDataAllocNew

	#ifdef RLIB_MACOS
	,kDataAllocMacSys
	,kDataAllocMacTemp
	#endif
};


//*********************************************
template< class T >
class TDataWrapper : public TDataInterface< T >
//*********************************************
// a typed array of data that doest not support allocation,
// it merely wraps custom data allocated by someone else.
// (A data wrapper does not own the data so it may not
// allocate nor release any memory.)
// Supports a lock mechanism (useful for Mac Handles).
{
public:
	inline				TDataWrapper(void);
	virtual 			~TDataWrapper(void);

	inline				TDataWrapper(T *pointer, si32 nb_elems);
	virtual void	setMemory   (T *pointer, si32 nb_elems);

	#ifdef RLIB_MACOS
		inline				TDataWrapper(Handle handle);
		virtual void	setMemory   (Handle handle);
	#endif

	inline	si32	size(void)					const	{ return mSize; }
	virtual	T *		data(void)					const	{ M_ASSERT(mData); return mData; }
	virtual				operator T* ()			const	{ return data(); }
	virtual	T &		operator [] (si32 index) const;

protected:

	virtual	rbool	protect(void);
	virtual void	unprotect(void);

	T *						mData;
	si32					mSize;

	#ifdef RLIB_MACOS
		Handle				mHData;
		RHandleLock		mLock;
	#endif

}; // end of class defs for TDataWrapper



//*******************************************
template< class T >
class TData : public TDataAllocInterface< T >
//*******************************************
// a typed array of data that support allocation and a
// lock mechanism.
{
public:
	inline				TData(void);
	inline				TData(si32 nb, EDataAllocFlag flag = kDataAllocDefault);
	virtual 			~TData(void);

	virtual void	alloc(si32 nb) { alloc(nb, kDataAllocDefault); }
	virtual	void	alloc(si32 nb, EDataAllocFlag flag);
	virtual	void	free(void);

	inline	si32	size(void)					const	{ return mSize; }
	virtual	T *		data(void)					const	{ M_ASSERT(mData); return mData; }
	virtual				operator T* ()			const	{ return data(); }
	virtual	T &		operator [] (si32 index) const;

protected:

	virtual	rbool	protect(void);
	virtual void	unprotect(void);

	EDataAllocFlag	mFlag;
	T *							mData;
	si32						mSize;

	#ifdef RLIB_MACOS
		Handle				mHData;
		RHandleLock		mLock;
	#endif

}; // end of class defs for TData



//---------------------------------------------------------------------------
// inline implementation
//
// TDataWrapper class



//******************************************
template< class T >
inline TDataWrapper< T >::TDataWrapper(void)
//******************************************
{
	setMemory(NULL, 0);
}


//***************************************************************
template< class T >
inline TDataWrapper< T >::TDataWrapper(T *pointer, si32 nb_elems)
//***************************************************************
{
	setMemory(pointer, nb_elems);
}

#ifdef RLIB_MACOS

	//***************************************************
	template< class T >
	inline TDataWrapper< T >::TDataWrapper(Handle handle)
	//***************************************************
	{
		setMemory(handle);
	}

#endif


//********************************************************
template< class T > TDataWrapper< T >::~TDataWrapper(void)
//********************************************************
// A data wrapper does not own the data so it may not
// release any memory
{
	M_ASSERT(!isLocked());
	mData = NULL;
	mSize = 0;
	#ifdef RLIB_MACOS
		mHData = NULL;
	#endif
}


//*****************************************************************
template< class T >
inline void TDataWrapper< T >::setMemory(T *pointer, si32 nb_elems)
//*****************************************************************
{
	// Reset values.
	// We own nothing so we release nothing.
	// The caller can deliver a NULL ptr and a null size when he
	// wants to invalidate accesses to the object.
	// But if pointer is NULL, size must be null too, both must be
	// consistent.
	// Sise must always be positive or null.

	M_ASSERT(nb_elems >= 0);
	M_ASSERT((pointer && nb_elems) || (!pointer && !nb_elems));

	mData = pointer;
	mSize = nb_elems;
	#ifdef RLIB_MACOS
		mHData = NULL;
	#endif
}


#ifdef RLIB_MACOS

	//*****************************************************************
	template< class T >
	inline void TDataWrapper< T >::setMemory(Handle handle)
	//*****************************************************************
	{
		// Reset values.
		// We own nothing so we release nothing.
		// The caller can deliver a NULL handle to
		// invalidate accesses to the object.

		mData = NULL;
		mSize = 0;
		mHData = handle;
		
		if (handle)
		{
			Size size = GetHandleSize(handle);
			M_CHECK_NOERR(::MemError());
			mSize = size / sizeof(T);
			M_ASSERT(mSize);
		}
	}

#endif


//**********************************************************
template< class T >
inline T & TDataWrapper< T >::operator [] (si32 index) const
//**********************************************************
{
	M_ASSERT(index >= 0 && index < mSize);
	return data()[index];
}


//*******************************************
template< class T >
inline rbool TDataWrapper< T >::protect(void)
//*******************************************
{
	#ifdef RLIB_MACOS
		if (mHData)
		{
			if (!mLock.protect()) return false;
			mData = (T *)(*mHData);
		}
	#endif

	M_CHECK_PTR(mData);
	return true;
}


//********************************************
template< class T >
inline void TDataWrapper< T >::unprotect(void)
//********************************************
{
	M_ASSERT_PTR(mData);

	#ifdef RLIB_MACOS
		if (mHData)
		{
			mLock.unprotect();
			mData = NULL;
		}
	#endif
}



//---------------------------------------------------------------------------
// TData class



//************************************************
template< class T > inline TData< T >::TData(void)
//************************************************
{
	mFlag = kDataAllocDefault;
	mData = NULL;
	mSize = 0;
	#ifdef RLIB_MACOS
		mHData = NULL;
	#endif
}


//************************************************************************
template< class T > inline TData< T >::TData(si32 nb, EDataAllocFlag flag)
//************************************************************************
{
	mFlag = kDataAllocDefault;
	mData = NULL;
	mSize = 0;
	#ifdef RLIB_MACOS
		mHData = NULL;
	#endif
	alloc(nb, flag);
}


//******************************************
template< class T > TData< T >::~TData(void)
//******************************************
{
	free();
}


//*****************************************************************************
template< class T > inline void TData< T >::alloc(si32 nb, EDataAllocFlag flag)
//*****************************************************************************
{
	free();

	mFlag = flag;
	if (flag == kDataAllocDefault)
	{
		mFlag = kDataAllocNew;
		#ifdef RLIB_MACOS
			if (nb * sizeof(T) > 1024) mFlag = kDataAllocMacTemp;
		#endif
	}

	#ifdef RLIB_MACOS

		OSErr err;
		si32 size = nb * sizeof(T);

		if (mFlag == kDataAllocMacTemp)
		{
			mHData = ::TempNewHandle(size, &err);
			if (err != noErr)
			{
				mHData = NULL;
				if (flag == kDataAllocDefault)
					mFlag = kDataAllocMacSys;
			}
		}

		if (mFlag == kDataAllocMacSys)
		{
			mHData = ::NewHandleSys(size);
			if (!mHData && flag == kDataAllocDefault)
				mFlag = kDataAllocNew;
		}
		
		if (mFlag != kDataAllocNew)
		{
			M_CHECK_PTR(mHData);
			mSize = nb;
			mLock.setHandle(mHData);
			return;
		}

	#endif

	M_ASSERT(mFlag == kDataAllocNew);
	mData = new T [nb];
	mSize = nb;
	M_CHECK_PTR(mData); 
}


//****************************************************
template< class T > inline void TData< T >::free(void)
//****************************************************
{
	M_ASSERT(!isLocked());

	#ifdef RLIB_MACOS
		if (mHData)
		{
			mLock.setHandle(NULL);
			::DisposeHandle(mHData);
			mHData = NULL;
		}
	#endif

	delete [] mData;
	mData = NULL;
	mSize = 0;
}


//***********************************************************************
template< class T > inline T & TData< T >::operator [] (si32 index) const
//***********************************************************************
{
	M_ASSERT(index >= 0 && index < mSize);
	return data()[index];
}


//********************************************************
template< class T > inline rbool TData< T >::protect(void)
//********************************************************
{
	#ifdef RLIB_MACOS
		if (mHData)
		{
			if (!mLock.protect()) return false;
			mData = (T *)(*mHData);
		}
	#endif

	M_CHECK_PTR(mData);
	return true;
}


//*********************************************************
template< class T > inline void TData< T >::unprotect(void)
//*********************************************************
{
	M_ASSERT_PTR(mData);

	#ifdef RLIB_MACOS
		if (mHData)
		{
			mLock.unprotect();
			mData = NULL;
		}
	#endif
}


//---------------------------------------------------------------------------


#endif // of _H_TDATA_

// eoh
