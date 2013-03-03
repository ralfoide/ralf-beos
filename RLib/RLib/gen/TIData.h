/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	TData.h
	Partie	: RLib basics

	Auteur	: RM
	Date		: 150899
	Format	: tabs==2

	An interface to a lockable bunch of data.	
	See TData.h for implementations.
	The interfaces (this file) was separated from TData.h afterwards
	to explicit the difference between the interface and the proposed
	implementation.

	Note		: TRArray and TData may seem to have close functionnality.
						TRArray is supposedly a re-allocatable, dynamic, unlockable
						array of structured data. At the contrary, TData is
						semantically a fixed bunch of bytes, which does not support
						re-allocation but supports a lock/swap mechanism.

*****************************************************************************/

#ifndef _H_TDATAINTERFACE_
#define  _H_TDATAINTERFACE_

#include "gMachine.h"
#include "RHandleLock.h"

//---------------------------------------------------------------------------


//*****************************************************
template< class T > class TDataInterface : public RLock
//*****************************************************
// a generic interface for an array of data with a lock
// mechanism (for Mac Handles)
{
public:
	virtual				~TDataInterface(void) = 0;
	virtual	si32	size(void)								const = 0;
	virtual	T *		data(void)								const = 0;
	virtual				operator T* ()						const	{ return (T *)data(); }
	virtual	T &		operator [] (si32 index)	const = 0;
};


//****************************************************
template< class T >
class TDataAllocInterface : public TDataInterface< T >
//****************************************************
// a generic interface for an array of data
// that can alloc or free itself
{
public:
	virtual				~TDataAllocInterface(void) = 0;
	virtual	void	alloc(si32 nb) = 0;
	virtual	void	free(void) = 0;
};



//---------------------------------------------------------------------------
// inline implementation



//************************************************************
template< class T > TDataInterface< T >::~TDataInterface(void)
//************************************************************
{
}


//**************************************************
template< class T >
TDataAllocInterface< T >::~TDataAllocInterface(void)
//**************************************************
{
}



//---------------------------------------------------------------------------


#endif // of  _H_TDATAINTERFACE_

// eoh
