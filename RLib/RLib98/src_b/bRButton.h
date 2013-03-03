/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRButton.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 120798
	Format	: tabs==2

	Leaf panes that reimplemented existing Be widgets should multiple inherit
	from the base class RPane and from their Be counterpart.

*****************************************************************************/

#ifndef _H_BRBUTTON_
#define _H_BRBUTTON_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RThread.h"
#include "RLayerPane.h"
#include "RValuePane.h"
#include <Button.h>

//---------------------------------------------------------------------------

//**************************************************************************
class RButton : public RLayerPane, public RValuePane, virtual public BButton
//**************************************************************************
{
public:
	RButton(RRect frame, sptr name, RPaneKey key, rbool isDefault=false, RThread *reply=NULL);
	RButton(sptr name, RPaneKey key, rbool isDefault=false, RThread *reply=NULL);

	virtual ~RButton(void);

	M_GLUE_PANE_TO_VIEW(RButton, RPane);

	// inherited from RPane, RLayerPane and RValuePane
	void attached(rbool _attached);

	// inherited from RLayerPane
	virtual rbool getIdealSize(RRect &minRect, RRect &maxRect);
	virtual void adjustSize(void);

	// -- BeOS specific callbacks --

	#ifdef RLIB_BEOS
		virtual	status_t	Invoke(BMessage *msg = NULL);
	#endif

protected:

	RThread *mReply; 

}; // end of class defs for RButton


//---------------------------------------------------------------------------

#endif // of RLIB_BEOS
#endif // of _H_BRBUTTON_

// eoh
