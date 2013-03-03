/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RLayoutInterface.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 090798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RLAYOUTINTERFACE_
#define _H_RLAYOUTINTERFACE_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RLayerPane.h"
#include "gErrors.h"

//---------------------------------------------------------------------------

#define K_LAYOUT_DEFAULT_BORDER	5

//---------------------------------------------------------------------------


//************************************************
class RLayoutInterface : virtual public RInterface
//************************************************
{
public:

	virtual void setBorder(si32 hBorder, si32 vBorder) = 0;
	virtual void adjustPane(RPane &parent) = 0;

}; // end of class defs for RLayoutInterface


//---------------------------------------------------------------------------

#endif // of RLIB_BEOS
#endif // of _H_RLAYOUTINTERFACE_

// eoh
