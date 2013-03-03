/*****************************************************************************

	Projet	: Droids

	Fichier	:	bCThread.h
	Partie	: Encapsulation

	Auteur	: RM
	Date		: 120297
	Format	: tabs==2

	Sur Be :
	
		id = spawn_thread(...)
		resume_thread(id)	--> le reveille
		suspend_thread(id)	--> l'endort
		kill_thread(id) --> le tue
		snooze(temps_en_us)	--> endors le thread COURANT pendant x microsecondes
		send_data(id...) + has_data(id, ...) + receive_data(id,...) : gestion message
			deja inclue.

*****************************************************************************/

#ifndef _H_BCTHREAD_
#define _H_BCTHREAD_

#include "machine.h"


//---------------------------------------------------------------------------


//***********
class CThread
//***********
{
public:
	CThread(void);
	virtual ~CThread(void); // Appele quand les threads attendent TOUS la mort

	BOOL init(void);
	BOOL launch(PFV func,ULONG pri);
	void sleep(void);
	void wake(void);
	void askToKill(void);
	void waitForEnd(void);
	void postMessage(CThread *dest,ULONG msg,ULONG lparam,APTR data);
	BOOL receiveMessage(ULONG &msg,ULONG &lparam,APTR &data);

//----
protected:

	void kill(void);	// ne fait pas partie du jeu de fonctions cross-platform.

//----
	thread_id	mThreadId;	// id of thread
	port_id		mPortId;		// if of port (message queue)


//====
private:

	// point d'entree du thread. Recoit en argument un PFV func() qui
	// sera appele. Renvoie toujours 0 au parent a la sortie.
	static LONG threadEntryPoint(void *data);

//----
	// tableau de conversion entre nos CThread::priorite et celles de Be.
	static const LONG mPriorityMap[4];

//---- struct privee pour passer un message ----

	struct SThreadMsg
	{
		ULONG lparam;
		APTR data;
	};

}; // end of class defs for CThread


//---------------------------------------------------------------------------

#endif // of _H_BCTHREAD_

// eoh
