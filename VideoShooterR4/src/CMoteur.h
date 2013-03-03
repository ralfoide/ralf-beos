/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CMoteur.h
	Partie	: Moteur

	Auteur	: RM
	Date		: 220798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_GCMOTEUR_
#define _H_GCMOTEUR_

#include "gMachine.h"
#include "RThread.h"
#include "RPaneDialog.h"
#include "RPath.h"

#include "CMoteurPlay.h"

//---------------------------------------------------------------------------
// messages

#define K_MSG_PLAY_DOCUMENT		'pDoc'		// vparam is &RPath
#define K_MSG_PLAY_DEFAULT		'plDf'
#define K_MSG_RECORD_DEFAULT	'rdDf'

//---------------------------------------------------------------------------
// forward defs

class CMoteurPlay;
class CMoteurRecord;

//---------------------------------------------------------------------------


//****************************
class CMoteur : public RThread
//****************************
{
public:
	CMoteur(void);
	virtual ~CMoteur(void);

	void run(void);

//----
protected:

	void closeDocument(RInterface *doc_win);
	void closeAllDocuments(void);
	void openPlayDocument(RPath *path);
	void openRecordDocument(RPath *path);

	TList<CMoteurDocument *>	mDocList;
	

}; // end of class defs for CMoteur


//---------------------------------------------------------------------------

#endif // of _H_GCMOTEUR_

// eoh
