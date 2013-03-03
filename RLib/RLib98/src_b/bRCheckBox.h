/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRCheckBox.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 070499
	Format	: tabs==2

	Leaf panes that reimplemented existing Be widgets should multiple inherit
	from the base class RPane and from their Be counterpart.

*****************************************************************************/

#ifndef _H_BRCHECKBOX_
#define _H_BRCHECKBOX_

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "RThread.h"
#include "RLayerPane.h"
#include "RValuePane.h"
#include <CheckBox.h>

//---------------------------------------------------------------------------

//******************************************************************************
class RCheckBox : public RLayerPane, public RValuePane, virtual public BCheckBox
//******************************************************************************
{
public:
	RCheckBox(RRect frame, sptr name, RPaneKey key, RThread *reply=NULL);
	RCheckBox(sptr name, RPaneKey key, RThread *reply=NULL);

	virtual ~RCheckBox(void);

	M_GLUE_PANE_TO_VIEW(RCheckBox, RPane);

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

}; // end of class defs for RCheckBox


//---------------------------------------------------------------------------

#endif // of RLIB_BEOS
#endif // of _H_BRCHECKBOX_

// eoh
