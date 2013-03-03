/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRThreadTranslator.h
	Partie	: Threads

	Auteur	: RM
	Date		: 120297 -- 050398
	Format	: tabs==2

	Sur Be :
	

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_BRTHREADTRANSLATOR_
#define _H_BRTHREADTRANSLATOR_

#include "gMachine.h"
#include "RThread.h"
#include <Looper.h>

//---------------------------------------------------------------------------

#define K_RUNTRANS_LPARAM		"RRunLpar"
#define K_RUNTRANS_DATA			"RRunData"
#define K_RUNTRANS_CHANNEL	"RRunChan"

//---------------------------------------------------------------------------


//*****************************************************************
class RThreadTranslator : public BLooper, virtual public RInterface
//*****************************************************************
{
public:
	RThreadTranslator(rbool immediateLaunch = true);
	RThreadTranslator(RThread *target, rbool immediateLaunch = true);
	virtual ~RThreadTranslator(void);
	void setTarget(RThread *target)	{ mTarget = M_CAST_AS(target, RThread); }

	void launch(void);
	virtual	void MessageReceived(BMessage *msg);

	static BMessage *createMessage(ui32 channel, ui32 msg, ui32	lparam, vptr data, ui32 datasize);

//----
protected:

	RThread *mTarget;


}; // end of class defs for RThreadTranslator


//---------------------------------------------------------------------------

#endif // of _H_BRTHREADTRANSLATOR_

#endif // of RLIB_BEOS

// eoh
