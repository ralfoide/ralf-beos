/**************************************************************************

	Projet	: Portable Lib

	Fichier	: TQList.h
	Partie	: Template based queue list

	Auteur	: RM
	Date		: 170798
	Format	: tabs==2

**************************************************************************/


#ifndef _TQLIST_H_
#define _TQLIST_H_

//-------------------------------------------------------------------------
// "PV" acronym stands for "Pointer on Void"
// and is used to remind that hopefully only small structs or pointers
// should be used for the template herein
//
// WARNING ! In fact this implementation ASSUMES PV to be a pointer.
// Items are initialized to NULL and NULL is returned by a failing functions.
//
// QList items should all derive from TQItem.


//***************
struct SQListItem
//***************
{
	SQListItem(void)	{mNext = NULL;}
	SQListItem	*mNext;
};


//*******************************
template< class PV > class TQList
//*******************************
{
public:
  TQList(void);
  virtual ~TQList(void);

	// TBD : copy-constructor, operator=<list>, operator+=<list>

	void	makeEmpty(void);

  si32	size(void) 		{ return mTaille;				}
	rbool	isEmpty(void)	{ return mHead == NULL; }

  void	addLast(PV item);
  PV		removeFirst(void);	// returns NULL if empty

private:
   si32 mTaille;	// this is just an 'estimation'
   PV   mHead;
   PV   mQueue;

}; // end of TQList



//-------------------------------------------------------------------------
// template-based implementation
//-------------------------------------------------------------------------



//*************************************************************************
template< class PV > inline TQList< PV >::TQList(void)
//*************************************************************************
{
   mHead = 0;
   mQueue = 0;
   mTaille = 0;

} // end of constructor for TQList< PV >


//*************************************************************************
template< class PV > inline TQList< PV >::~TQList(void)
//*************************************************************************
{
   makeEmpty();

} // end of destructor for TQList< PV >


//*************************************************************************
template< class PV > inline void TQList< PV >::makeEmpty(void)
//*************************************************************************
{
	// the TQList _must_ not be aware of either the items are malloc
	// or not by the client. Memory leak may occur and be the responsibility
	// of the caller.
	while(!isEmpty()) removeFirst();

} // end of makeEmpty for TQList< PV >


//*************************************************************************
template< class PV > inline void TQList< PV >::addLast(PV elem)
//*************************************************************************
{

	elem->mNext = NULL;
	if (mHead)
	{
		mQueue->mNext = elem;
		mQueue = elem;
	}
	else
	{
		mQueue = mHead = elem;
	}
	mTaille++;

} // end of addLast for TQList< PV >


//*************************************************************************
template< class PV > inline PV TQList< PV >::removeFirst(void)
//*************************************************************************
{
PV p;

	if (!mHead) return NULL;
	p = mHead;
	mHead = (PV)mHead->mNext;
	if (!mHead) mQueue = NULL;
	mTaille--;
	return p;

} // end of removeFirst for TQList< PV >


#endif // of _TQLIST_H_

//--------------------------------- eoh ----------------------------------
