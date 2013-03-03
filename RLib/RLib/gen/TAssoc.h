/**************************************************************************

	Projet	: Portable Lib

	Fichier	: TAssoc.h
	Partie	: Template based association dictionnary

	Auteur	: RM
	Date		: 260698
	Format	: tabs==2

**************************************************************************/


#ifndef _H_TASSOC_
#define _H_TASSOC_

#include "gMachine.h"
#include "gMacros.h"
#include "gErrors.h"
#include "TList.h"

//-------------------------------------------------------------------------

//**************************************************
template< class KEY, class VALUE > struct SAssocItem
//**************************************************
{
	KEY 	mKey;
	VALUE	mValue;
};

//*********************************************
template< class KEY, class VALUE > class TAssoc
//*********************************************
{
public:
  TAssoc(void);
  virtual ~TAssoc(void);

	// services

	void	set(KEY key, VALUE &value);		// add or set existing
	VALUE	get(KEY key);									// throw if key doesn't exist
	void	del(KEY key);									// fail silently if key doesn't exist
	
protected:

	virtual void	makeEmpty(void);

	// list of buffers
	TList<SAssocItem<KEY,VALUE> *>	mList;

	// internal routines
	// find entry and return >=0 if exists, -1 if doesn't
	si32	findIndex(KEY key);
	// find entry or return NULL
	SAssocItem<KEY,VALUE> *	findPtr(KEY key);

}; // end of TAssoc



//-------------------------------------------------------------------------
// template-based implementation
//-------------------------------------------------------------------------


//******************************************************************************
template< class KEY, class VALUE >
inline TAssoc< KEY,VALUE >::TAssoc(void)
//******************************************************************************
{
	mList.makeEmpty();
} // end of itemAt for TAssoc


//*************************************************************************
template< class KEY, class VALUE >
inline TAssoc< KEY,VALUE >::~TAssoc(void)
//*************************************************************************
{
	makeEmpty();

} // end of destructor for TAssoc


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


//*************************************************************************
template< class KEY, class VALUE >
inline void TAssoc< KEY,VALUE >::makeEmpty(void)
//*************************************************************************
{
SAssocItem<KEY,VALUE> *item;

	while((item = mList.removeLastItem()) != NULL) delete item;

} // end of makeEmpty for TAssoc< KEY,VALUE >


//*************************************************************************
template< class KEY, class VALUE >
inline si32 TAssoc< KEY,VALUE >::findIndex(KEY key)
//*************************************************************************
{
SAssocItem<KEY,VALUE> *item;
si32 n = mList.size();

	for(n--; n>=0; n--)
	{
		item = mList.itemAt(n);
		if (item->mKey == key) return n;
	}

	return -1;

} // end of findIndex for TAssoc< KEY,VALUE >


//*************************************************************************
template< class KEY, class VALUE >
inline SAssocItem<KEY,VALUE> * TAssoc< KEY,VALUE >::findPtr(KEY key)
//*************************************************************************
{
SAssocItem<KEY,VALUE> *item;
si32 n = mList.size();

	for(n--; n>=0; n--)
	{
		item = mList.itemAt(n);
		if (item->mKey == key) return item;
	}

	return NULL;

} // end of findPtr for TAssoc< KEY,VALUE >


//*************************************************************************
template< class KEY, class VALUE >
inline void TAssoc< KEY,VALUE >::set(KEY key, VALUE &value)
//*************************************************************************
// add or set existing
{
SAssocItem<KEY,VALUE> *item;

	item = findPtr(key);
	if (item)
	{
		item->mValue = value;
		return;
	}

	// no key, create a new one !

	item = new SAssocItem<KEY,VALUE>;
	M_ASSERT_PTR(item);
	item->mValue = value;
	item->mKey = key;
	if (!mList.addItem(item)) M_THROW(kErrInit);

} // end of set for TAssoc< KEY,VALUE >


//*************************************************************************
template< class KEY, class VALUE >
inline VALUE TAssoc< KEY,VALUE >::get(KEY key)
//*************************************************************************
// throw if key doesn't exist
{
SAssocItem<KEY,VALUE> *item;

	item = findPtr(key);
	if (item) return item->mValue;
	M_THROW(kErrNotFound);

} // end of set for TAssoc< KEY,VALUE >


//*************************************************************************
template< class KEY, class VALUE >
inline void TAssoc< KEY,VALUE >::del(KEY key)
//*************************************************************************
// throw if key doesn't exist
{
SAssocItem<KEY,VALUE> *item;
si32 i = findIndex(key);

	if (i>=0)
	{
		item = mList.removeItemAt(i);
		if (item) delete item;
	}

} // end of del for TAssoc< KEY,VALUE >


#endif // of _H_TASSOC_

//--------------------------------- eoh ----------------------------------
