/**************************************************************************

	Projet	: RLib

	Fichier	: TRArray.h
	Partie	: Template based dynamic array of item

	Auteur	: RM
	Date		: 010899
	Format	: tabs==4

	Note	:	there is a huge *semantic* difference between TList
					and TRArray thoug the implementation and the functionnality
					are very close. TList is supposedly a list of simple items
					(like intergers, chars, and more frequently pointers), while
					TRArray is an array of complex typed items (structs, classes).

	Note	: TRArray and TData also provide a close functionnality.
					TRArray is supposedly a re-allocatable, dynamic, unlockable
					array of structured data. At the contrary, TData is
					semantically a fixed bunch of bytes, which does not support
					re-allocation but supports a lock/swap mechanism.

	Note	: class name TArray is already used by PowerPlant source code.
					TRArray is used instead.

**************************************************************************/


#ifndef _TRARRAY_H_
#define _TRARRAY_H_

#include "RMacros.h"
#include "RErrors.h"

//-------------------------------------------------------------------------


//*******************************
template< class T > class TRArray
//*******************************
{
public:
					TRArray(si32 size = 0,
									si32 granularity = 1,
									rbool allow_realloc = true);
	virtual ~TRArray(void);

	// TBD : copy-constructor, operator=<list>, operator+=<list>

	inline	si32	size(void) { return mSize;}
					void	resize(si32 size);
					void 	free(void);

	inline	void	append(T &item);
	inline	T *		append(void);
					void	remove(si32 index);

	inline	T &  	itemAt(si32 index);
	inline	T &  	operator[](si32 index) { return itemAt(index) ;}

private:
	rbool	mAllowRealloc;
	si32	mGranularity;
	si32	mSize;
	si32	mRealSize;
	T *		mArray;

}; // end of TRArray



//-------------------------------------------------------------------------
// template-based implementation
//-------------------------------------------------------------------------


//************************************************************
template< class T > TRArray< T >::TRArray(si32 size,
																					si32 granularity,
																					rbool allow_realloc)
//************************************************************
{
	mAllowRealloc = allow_realloc;
	mGranularity = granularity;
	mArray = NULL;
	mSize = 0;
	mRealSize = 0;
	resize(size);
} // end of constructor for TRArray< PV >


//**********************************************
template< class T > TRArray< T >::~TRArray(void)
//**********************************************
{
	free();

} // end of destructor for TRArray< T >


//*************************************************************
template< class T > inline T & TRArray< T >::itemAt(si32 index)
//*************************************************************
{
	if (!mArray || index < 0 || index >= mSize) M_THROW(kErrBadIndex);
	return mArray[index];
	 
} // end of itemAt for TRArray


//***********************************************
template< class T > void TRArray< T >::free(void)
//***********************************************
{
	if (mArray) delete [] mArray;
	mArray = NULL;
	mSize = mRealSize = 0;

} // end of free for TRArray< T >


//************************************************************
template< class T > inline void TRArray< T >::append(T & elem)
//************************************************************
{
	si32 index = mSize;
	resize(index+1);
	mArray[index] = elem;

} // end of append for TRArray< T >


//*******************************************************
template< class T > inline T * TRArray< T >::append(void)
//*******************************************************
{
	si32 index = mSize;
	resize(index+1);
	return &(mArray[index]);

} // end of append for TRArray< T >


//******************************************************
template< class T > void TRArray< T >::resize(si32 size)
//******************************************************
{
	M_ASSERT(size >= 0);

	if (!size)
	{
		free();
		return;
	}

	si32 previous = mSize;
	mSize = size;

	if (mSize > previous && mSize > mRealSize)
	{
		si32 i;
		T *nvxElem;

		M_VERIFY(!mArray || mAllowRealloc);

		if (mGranularity)
			while (mSize > mRealSize)
				mRealSize += mGranularity;
		else
			mRealSize = mSize;

		nvxElem = new T[mRealSize];
		if (!nvxElem) M_THROW(kErrAlloc);

		for(i = 0; i < previous; i++) nvxElem[i] = mArray[i];
		delete [] mArray;
		mArray = nvxElem;
	}

	M_ASSERT_PTR(mArray);

} // end of resize for TRArray< T >


//*******************************************************
template< class T > void TRArray< T >::remove(si32 index)
//*******************************************************
{
	M_ASSERT(mArray && index >=0 && index < mSize);

	for(index++; index < mSize; index++)
		mArray[index-1] = mArray[index];

	if (mAllowRealloc)
		resize(mSize-1);
	else
	{
		mArray[mSize-1] = T();
		mSize--;
	}

} // end of remove for TRArray< T >



#endif // of _TRARRAY_H_

//--------------------------------- eoh ----------------------------------
