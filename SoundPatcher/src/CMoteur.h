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

#include "CSoundPlay.h"
#include "CSoundRecord.h"
#include "CAudioCodec.h"

#include <socket.h>

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

	void	initSoundPlay(void);
	void	initSoundRecord(void);
	void	closeSoundPlay(void);
	void	closeSoundRecord(void);

	// sound
	rbool	mUseSoundPlay;
	rbool	mUseSoundRecord;

	SSharedSoundBuffer	mSharedSoundPlayBuffer;
	SSharedSoundBuffer	mSharedSoundRecordBuffer;
	CSoundPlay					mSoundPlay;
	CSoundRecord				mSoundRecord;
	CAdpcmCodec					mAdpcmCodec;

	// this is more a hack for testing.
	// will move to a separate UDP socket class after

	int	mSocketSend;
	int	mSocketReceive;
	struct sockaddr_in mSendSocketAddr;
	struct sockaddr_in mReceiveSocketAddr;

	void	initSocketSend(void);
	void	initSocketReceive(void);
	void	closeSocketSend(void);
	void	closeSocketReceive(void);

	rbool	sendPacket(void *data, si32 len);
	si32	receivePacket(void *data, si32 len);

	si8		*mSendBuffer;
	si8		*mZlibSendBuffer;
	si32	mSendCount;
	void	sendRecordPackets(void);

	//-----------------------------------
	class CReceiveAndPlay : public RThread
	//-----------------------------------
	{
	public:
		CReceiveAndPlay(CMoteur *elvis) : RThread() { mMoteur = elvis; }
		virtual void run(void);
		CMoteur	*mMoteur;
	}; // end of class CReceiveAndPlay

	friend class CReceiveAndPlay;


}; // end of class defs for CMoteur


//---------------------------------------------------------------------------

#endif // of _H_GCMOTEUR_

// eoh
