/*****************************************************************************

	Projet	: Pegase

	Fichier	: RFlattenable.h
	Partie	: Portable lib

	Auteur	: RM
	Date		: 180298
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RFLATTENABLE_
#define _H_RFLATTENABLE_

#include "RFlatBlock.h"

//------------------------------------------------------------
// Exception to throw when the derived (non pure) unflatten rejects the
// given RFlatBlock

#define K_FLAT_EXCPT_REJECT "Can't unflatten this data"

//------------------------------------------------------------


//****************
class RFlattenable
//****************
{
public:
  RFlattenable(void)          	 { }
  RFlattenable(RFlatBlock *data) { unflatten(data); }
  virtual ~RFlattenable(void)		 { }

	// pure method that you must implement in derived classes 
	// flatten is mandatory, unflatten is not (lazy condition due to constructor 2).

	// flatten() create a new (dynamic allocation) RFlatBlock and fills it with the
	// info. The block will be destroyed later by the caller.
	virtual RFlatBlock*	flatten(void) = 0;

	// unflatten receive a pointer onto an existing block and first check it's
	// type and revision, then if correct it gets back the field.
	virtual void				unflatten(RFlatBlock *data) { }

	// create a generic RFlatBlock and call the other virtual unflatten() method
	void unflatten(void *data) { RFlatBlock flat(data); unflatten(&flat); }

protected:
	// this generic class has no members
};

//------------------------------------------------------------

#endif // of _H_RFLATTENABLE_

// eof
