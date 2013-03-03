/*****************************************************************************

	Projet	: Pegase

	Fichier	: RFlattenable.h
	Partie	: Portable lib

	Auteur	: RM
	Date	: 033000 (changed RFlattenable to RIFlat)
	Date	: 180298
	Format	: tabs==4

*****************************************************************************/

#ifndef _H_RFLATTENABLE_
#define _H_RFLATTENABLE_

#include "RFlatBlock.h"

//------------------------------------------------------------
// Exception to throw when the derived (non pure) unflatten rejects the
// given RFlatBlock

#define K_FLAT_EXCPT_REJECT		kErrCantUnflatten

//------------------------------------------------------------

/**	An interface to flat blocks.
	A flat block is used to encode a given structure in a piece of bytes
	that can be stored or transmitted and later unflattened.
	@see	RFlatBlock
	@see	TMessage
*/

//**********
class RIFlat
//**********
{
public:
			RIFlat(void)          	 { }
			RIFlat(RFlatBlock *data) { Unflatten(data); }
virtual		~RIFlat(void)			 { }

		// pure method that you must implement in derived classes 
		// flatten is mandatory, unflatten is not (lazy condition due to constructor 2).

		// flatten() create a new (dynamic allocation) RFlatBlock and fills it with the
		// info. The block will be destroyed later by the caller.
virtual	RFlatBlock*	Flatten(void) = 0;

		// unflatten receive a pointer onto an existing block and first check it's
		// type and revision, then if correct it gets back the field.
virtual void		Unflatten(RFlatBlock * /*data*/ ) { }

		// create a generic RFlatBlock and call the other virtual unflatten() method
		void		Unflatten(void *data) { RFlatBlock flat(data); Unflatten(&flat); }

protected:
		// this generic class has no members
};

//------------------------------------------------------------

#endif // of _H_RFLATTENABLE_

// eof
