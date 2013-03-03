/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRThread.h
	Partie	: Thread

	Auteur	: RM
	Date		: 120297 -- 050398
	Format	: tabs==2

	Sur Be :
	
		id = spawn_thread(...)
		resume_thread(id)	--> le reveille
		suspend_thread(id)	--> l'endort
		kill_thread(id) --> le tue
		snooze(temps_en_us)	--> endors le thread COURANT pendant x microsecondes
		send_data(id...) + has_data(id, ...) + receive_data(id,...) : gestion message
			deja inclue.

	Usage :
		1- deriver le RThread et implementer run() afin de faire des receiveMessage();
		2- le lancer comme suit :
		try {
			RThread *toto = new RThread
			toto->launch(K_RUNABLE_PRI_NORMAL);
		} catch(...) { probleme creation thread }
	
		3- pour l'arreter :
		toto->askToKill();
		... (ceci envoie un message K_MSG_RTHREAD_EXIT au moteur qui doit sortir de sa boucle)
		if (toto->waitForEnd()) delete toto; toto = NULL

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_BRRUNABLE_
#define _H_BRRUNABLE_

#include "gMachine.h"
#include "RSem.h"

#include <Looper.h>	// for B_LOOPER_PORT_DEFAULT_CAPACITY

//---------------------------------------------------------------------------
// Priority that are used to launch a thread

#define K_RUNABLE_PRI_IDLE				0
#define K_RUNABLE_PRI_NORMAL			1
#define K_RUNABLE_PRI_HIGH				2
#define K_RUNABLE_PRI_REALTIME		3

//---------------------------------------------------------------------------

#define K_MAX_PORT_COUNT B_LOOPER_PORT_DEFAULT_CAPACITY
#define K_MSG_RTHREAD_EXIT	'RuEx'


//****************************************
class RThread : virtual public RInterface
//****************************************
{
public:
	RThread(void);
	virtual ~RThread(void); // Appele quand les threads attendent TOUS la mort

	virtual void run(void) = 0;	// make this an abstract method

	void launch(ui32 pri=K_RUNABLE_PRI_NORMAL);
	void sleep(void);
	void wake(void);
	void askToKill(void);
	void okToKill(void);
	void waitForEnd(void);
	static void postMessage(RThread *dest, ui32 msg, ui32 lparam=0, vptr data=NULL, ui32 *channel=NULL);
	void postMessage(ui32 msg, ui32 lparam=0, vptr data=0, ui32 *channel=NULL) { postMessage(this, msg, lparam, data, channel); }
	rbool receiveMessage(ui32 &msg, ui32 &lparam, vptr &data, ui32 *channel=NULL);	// non blocking
	void waitForMessage(ui32 &msg, ui32 &lParam, vptr &data, ui32 *channel=NULL);		// blocking

	// utility routine
	// simplified posting message method
	// same as second form of postMessage() -- self... is now obsolete -- HOOK RM 280299
	void selfPostMessage(ui32 msg, ui32 lparam=0, vptr data=0, ui32 *channel=NULL) { postMessage(this, msg, lparam, data, channel); }

	// return system wide time in microseconds (to compute frame rates for example)
	fp64	systemTime(void)	{ return system_time(); }

//----
protected:

//----
	thread_id	mThreadId;	// id of thread
	port_id		mPortId;		// if of port (message queue)
	RSem			runningSem;


//====
private:

	// point d'entree du thread. Recoit en argument un PFV func() qui
	// sera appele. Renvoie toujours 0 au parent a la sortie.
	static int32 threadEntryPoint(void *data);
	void readyToRun(void);

//----
	// tableau de conversion entre nos RThread::priorite et celles de Be.
	static const int32 mPriorityMap[4];

//---- struct privee pour passer un message ----

	struct SThreadMsg
	{
		ui32  channel;
		ui32	lparam;
		vptr		data;
	};

	char nom[B_OS_NAME_LENGTH];

}; // end of class defs for RThread


//---------------------------------------------------------------------------

#endif // of _H_BRRUNABLE_
#endif // of RLIB_BEOS

// eoh
