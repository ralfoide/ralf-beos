/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RLayoutPane.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 100798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RLAYOUTPANE_
#define _H_RLAYOUTPANE_

#include "gMachine.h"
#include "RLayerPane.h"
#include "RLayoutInterface.h"
#include "gErrors.h"

//---------------------------------------------------------------------------


//*************************************
class RLayoutPane : public RLayerPane
//*************************************
{
public:
	RLayoutPane(void);
	RLayoutPane(RLayoutInterface *method);
	virtual ~RLayoutPane(void);

	// virtual from RPaneInterface/RPane
	virtual void attached(rbool _attached);

	// virtual from RLayerPane
	virtual rbool getIdealSize(RRect &minRect, RRect &maxRect);
	virtual void adjustSize(void);

	void				 			setLayout(RLayoutInterface *method);
	RLayoutInterface *getLayout(void)	{ return mLayoutMethod; }

//----
protected:

	RLayoutInterface *mLayoutMethod;
	rbool							mAdjustNeeded;

}; // end of class defs for RLayoutPane


//---------------------------------------------------------------------------

#endif // of _H_RLAYOUTPANE_

// eoh
