/**************************************************************************

New Header format :

	Projet	: RLib

	Fichier	: TList.h
	Partie	: Template based array list similar to BList

	Auteur	: RM
	Date		: 110498
	Format	: tabs==2

	Comment : granularity set to 8 following B.'s comments about 8 pointers
						fitting in a cache line.

	Note		:	there is a huge *semantic* difference between TList
						and TRArray thoug the implementation and the functionnality
						are very close. TList is supposedly a list of simple items
						(like intergers, chars, and more frequently pointers), while
						TRArray is an array of complex typed items (structs, classes).

	Modifs	:
	01/08/99:	use M_THROW to check index range

--------------------

Old Header format from original file "LISTE.H" and "LISTE.CPP" for Watcom 9.5 :

   File: liste.h

   Project: carnet

   header for the TList< PV >

   Modifs:
   3/6/95 - creation
   4/6/95 - suite

**************************************************************************/


#ifndef _TLIST_H_
#define _TLIST_H_

#include "gMacros.h"	// for M_ASSERT, M_THROW
#include "gErrors.h"

//-------------------------------------------------------------------------
// "PV" acronym stands for "Pointer on Void"
// and is used to remind that hopefully only small structs or pointers
// should be used for the template herein


//******************************
template< class PV > class TList
//******************************
{
public:
					TList(si32 granularite = 8, PV empty = 0);
	virtual	~TList(void);

	// TBD : copy-constructor, operator=<list>, operator+=<list>

	inline	si32 size(void) { return mTaille; }

					rbool addItem(PV item);
					rbool removeItem(PV item);
					PV   	removeItemAt(si32 index);
					PV   	removeLastItem(void);
					void 	makeEmpty(void);

	inline	PV   	itemAt(si32 index);
	inline	PV   	operator[](si32 index) { return itemAt(index) ;}

private:
	si32 mGranularite;
	si32 mTaille;
	si32 mTailleReelle;
	PV   *mListe;
	PV		mEmpty;

}; // end of TList



//-------------------------------------------------------------------------
// template-based implementation
//-------------------------------------------------------------------------


//*******************************************************
template< class PV > TList< PV >::TList(si32 granularite)
//*******************************************************
{
	mGranularite = granularite;
	mListe = NULL;
	mTaille = 0;
	mTailleReelle = 0;

} // end of constructor for TList< PV >


//********************************************
template< class PV > TList< PV >::~TList(void)
//********************************************
{
	makeEmpty();

} // end of destructor for TList< PV >


//************************************************************
template< class PV > inline PV TList< PV >::itemAt(si32 index)
//************************************************************
/*
   Renvoie l'element "index" s'il existe ou une exception.
   Le premier element est a l'index 0.
*/
{
	if (!mListe || mTaille < 1) return mEmpty;
	if (index < 0 || index >= mTaille) M_THROW(kErrBadIndex);
	return mListe[index];
   
} // end of itemAt for TList


//****************************************************
template< class PV > void TList< PV >::makeEmpty(void)
//****************************************************
{
	if (mListe) delete [] mListe;
	mListe = NULL;
	mTaille = mTailleReelle = 0;

} // end of makeEmpty for TList< PV >


//******************************************************
template< class PV > rbool TList< PV >::addItem(PV elem)
//******************************************************
/*
   ajoute l'element "elem" a la fin de la liste.
   la liste grandit s'il le faut.
*/
{
	if (mTaille >= mTailleReelle)
	{
		si32 i;
		PV *nvxElem;

	  mTailleReelle += mGranularite;
	  nvxElem = new PV[mTailleReelle];
	  if (!nvxElem) return false;

	  for(i = 0; i < mTaille; i++) nvxElem[i] = mListe[i];
	  delete [] mListe;
	  mListe = nvxElem;
	}

	if (!mListe) return false;

	mListe[mTaille++] = elem;
	return true;

} // end of addItem for TList< PV >


//*******************************************************
template< class PV > PV TList< PV >::removeLastItem(void)
//*******************************************************
/*
   supprime le dernier element de la liste s'il existe.
   Renvoie cet element s'il existe ou mEmpty en cas de liste vide.
   Si la liste devient vide apres la suppression, le tableau est efface
   par un appel a makeEmpty.
*/
{
	PV p;

	if (!mListe || mTaille < 1) return mEmpty;

	p = mListe[--mTaille];
	if (!mTaille) makeEmpty();

	return p;

} // end of removeLastItem for TList< PV >


//*********************************************************
template< class PV > rbool TList< PV >::removeItem(PV elem)
//*********************************************************
/*
   supprime l'element "elem" s'il existe.
   bouche les trous en remontant toute la liste vers le bas (sic)
*/
{
	si32 i;
	PV *p2;

	if (!mListe || mTaille < 1) return false;

	/* cherche l'element : parcoure lineairement (!!) le tableau et
	  sort directement (avec i < mTaille) s'il le trouve.
	*/
	for(i = 0; i < mTaille; i++) if (mListe[i] == elem) break;

	/* pas trouve ? */
	if (i == mTaille) return false;

	/* supprime l'element en "packant" le haut du tableau d'une position
	  vers le bas.
	  On commence donc une position plus haut que celle de l'element
	  trouve et on descent chaque element vers le bas d'un cran.
	*/

	// Original code from 1995
	// for(i++; i < mTaille; i++) mListe[i-1] = mListe[i];

	// Code for 1998 : could use a memmove operation
	si32 nb = mTaille-i-1;
	p2 = &mListe[i];
	if (nb > 0) memmove(p2, p2+sizeof(PV), nb*sizeof(PV));

	/* la liste possede desormais un element en moins.
	  Libere la memoire si la liste devient vide.
	*/
	mTaille--;
	if (!mTaille) makeEmpty();

	return true;

} // end of removeItem for TList< PV >


//***********************************************************
template< class PV > PV TList< PV >::removeItemAt(si32 index)
//***********************************************************
/*
   Supprime l'element a l'index "index" s'il existe.
   Renvoie le pointeur de l'element s'il existe sinon exception.
   Bouche les trous en remontant toute la liste vers le bas (sic).
*/
{
	PV p;
	PV *p2;

	if (!mListe || mTaille < 1) return mEmpty;
	if (index < 0 || index >= mTaille) M_THROW(kErrBadIndex);

	p2 = &mListe[index];
	p = *p2;

	/* supprime l'element en "packant" le haut du tableau d'une position
	  vers le bas.
	  On commence donc une position plus haut que celle de l'element
	  trouve et on descent chaque element vers le bas d'un cran.
	*/

	// Original code from 1995
	// for(index++; index < mTaille; index++) mListe[index-1] = mListe[index];

	// Code for 1998 : could use a memmove operation
	si32 nb = mTaille-index-1;
	if (nb > 0) memmove(p2, p2+sizeof(PV), nb*sizeof(PV));

	/* la liste possede desormais un element en moins.
	  Libere la memoire si la liste devient vide.
	*/
	mTaille--;
	if (!mTaille) makeEmpty();

	return p;

} // end of removeItemAt for TList< PV >


#endif // of _TLIST_H_

//--------------------------------- eoh ----------------------------------
