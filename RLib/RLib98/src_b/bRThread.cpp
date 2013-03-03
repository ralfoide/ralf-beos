/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRThread.cpp
	Partie	: Thread

	Auteur	: RM
	Date		: 120297 -- 050398
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "gMacros.h"
#include "gErrors.h"
#include "RThread.h"

//---------------------------------------------------------------------------

//*************************************
const int32 RThread::mPriorityMap[4] =
//*************************************
{
	B_LOW_PRIORITY,
	B_NORMAL_PRIORITY,
	B_DISPLAY_PRIORITY,
	B_URGENT_PRIORITY
};
	


//---------------------------------------------------------------------------


//***************************************************************************
RThread::RThread(void)
//***************************************************************************
{
	mThreadId = -1;	// valeur d'invalidite
	mPortId		= -1;

} // end of constructor for RThread


//***************************************************************************
RThread::~RThread(void)
//***************************************************************************
/*
	Theoriquement : Appele quand les threads attendent TOUS la mort
*/
{
	// le thread devrait avoir ete tue. Si ce n'est pas le cas, on le bute
	// froidement.
	if (mThreadId != -1) kill_thread(mThreadId);
	if (mPortId   != -1) delete_port(mPortId);
	mThreadId = -1;
	mPortId   = -1;

} // end of destructor for RThread


//---------------------------------------------------------------------------


//***************************************************************************
void RThread::launch(ui32 pri)
//***************************************************************************
/*
*/
{
	// le thread est deja lance
	if (mThreadId != -1) M_THROW(kErrThreadAlreadyLaunched);

	static long int index=0;
	sprintf(nom, "bRThread_%ld", index++);

	// creation thread
	runningSem.acquire();
	mThreadId = spawn_thread((thread_entry)RThread::threadEntryPoint,
											nom,
											mPriorityMap[pri],
											(void *)this);

	// erreur creation thread. Suicide !
	if (mThreadId < B_NO_ERROR)
	{
		status_t err = mThreadId;
		mThreadId = -1;
		throw RErrEx(err);
	}

	// creation port
	mPortId = create_port(K_MAX_PORT_COUNT, nom);

	// erreur creation port. Suicide !
	if (mPortId < B_NO_ERROR)
	{
		status_t err = mPortId;
		mPortId = -1;
		throw RErrEx(err);
	}

	// reveille le thread qui est endormi a la creation
	resume_thread(mThreadId);
	runningSem.release();

} // end of launch for RThread


//***************************************************************************
void RThread::sleep(void)
//***************************************************************************
/*
	Endort un thread.
*/
{
	if (mThreadId != -1) suspend_thread(mThreadId);
} // end of sleep for RThread


//***************************************************************************
void RThread::wake(void)
//***************************************************************************
/*
	Reveille un thread endormi.
*/
{
	if (mThreadId != -1) resume_thread(mThreadId);
} // end of wake for RThread


//***************************************************************************
void RThread::askToKill(void)
//***************************************************************************
/*
	Notifie le thread qu'il va mourrir.
	"You know what ?... you're dead."
*/
{
	//fprintf(stderr,"RThread::askToKill -- thread %s\n", nom);
	postMessage(this,K_MSG_RTHREAD_EXIT,0L,NULL);
} // end of askToKill for RThread


//***************************************************************************
void RThread::okToKill(void)
//***************************************************************************
{
	//fprintf(stderr,"RThread::okToKill -- thread %s\n", nom);
	runningSem.release();
	return;
}


//***************************************************************************
void RThread::waitForEnd(void)
//***************************************************************************
/*
	Attente bloquante que le thread soit mort.
	Utilise apres un "askToKill".
*/
{
	if (mThreadId != -1)
	{
		//fprintf(stderr,"RThread::waitForEnd -- thread %s\n", nom);
		runningSem.acquire();
		runningSem.release();
	}
} // end of waitForEnd for RThread


//---------------------------------------------------------------------------


//***************************************************************************
void RThread::postMessage(RThread *dest,ui32 msg,ui32 lparam,vptr data, ui32 *channel)
//***************************************************************************
/*
	Rempli une structure SThreadMsg et l'envoi au port du thread de destination.
	Verifie auparavant que ce dernier existe bien.

	ATTENTION : cette fonction bloque si la queue de msg du destinataire est pleine.
	Actuellement, RThread::launch() cree un port avec 128 entrees max.
	On peut ecrire de facon non-bloquante avec un timeout (ne fait rien si queue pleine).
*/
{
	SThreadMsg datamsg;
	port_id id;

	if (!dest) return;		// pas de destinataire
	id = dest->mPortId;
	if (id == -1) return;	// pas de port alloue

	//fprintf(stderr, "RThread::postMessage -- port %d -- msg %d -- L = %d -- D = %p\n",
	//	id, msg, lparam, data);

	// construit message
	datamsg.channel = (channel ? *channel : 0);
	datamsg.lparam = lparam;
	datamsg.data   = data;

	// renvoie un code d'erreur ignore ici... Utiliser write_port_etc() pour timeout.
	write_port(id, (long)msg, &datamsg, sizeof(datamsg));

} // end of postMessage for RThread


//***************************************************************************
rbool RThread::receiveMessage(ui32 &msg,ui32 &lparam,vptr &data, ui32 *channel)
//***************************************************************************
/*
	Examine le port de message du thread.
	Renvoie FALSE si aucun message n'est en attente, TRUE si il y a un msg.
	Si renvoie TRUE, rempli msg, lparam et data avec les donnees du message.

	Utilise le port de message de facon non-bloquante avec port_buffer_size_etc.
*/
{
	long taille;
	SThreadMsg datamsg;

	if (mPortId == -1) return FALSE;	// pas de port alloue

	// cherche si message present et si oui de quelle taille. Non bloquant.
	taille = port_buffer_size_etc(mPortId, B_TIMEOUT, (bigtime_t)0);

	// renvoie B_WOULD_BLOCK ou B_BAD_PORT_ID ou B_TIMED_OUT. Laisse tomber si erreur.
	if (taille < B_NO_ERROR) return FALSE;

	// lit message de facon bloquante : un msg existe ici !
	taille = read_port(mPortId, (long *)&msg, &datamsg, sizeof(datamsg));
	if (taille != sizeof(datamsg)) return FALSE;

	if (channel) *channel = datamsg.channel;
	lparam = datamsg.lparam;
	data   = datamsg.data;	

	//fprintf(stderr, "RThread::receiveMessage -- port %d -- msg %d -- L = %d -- D = %p\n",
	//	mPortId, msg, lparam, data);

	return TRUE;

} // end of receiveMessage for RThread


//***************************************************************************
void RThread::waitForMessage(ui32 &msg,ui32 &lparam,vptr &data, ui32 *channel)
//***************************************************************************
/*
	Examine le port de message du thread.
	Renvoie FALSE si aucun message n'est en attente, TRUE si il y a un msg.
	Si renvoie TRUE, rempli msg, lparam et data avec les donnees du message.

	Utilise le port de message de facon bloquante avec port_buffer_size.
*/
{
	long taille;
	SThreadMsg datamsg;

	if (mPortId == -1) return;	// pas de port alloue

	// cherche si message present et si oui de quelle taille. Bloquant.
	taille = port_buffer_size(mPortId);

	// renvoie B_WOULD_BLOCK ou B_BAD_PORT_ID ou B_TIMED_OUT. Laisse tomber si erreur.
	if (taille < B_NO_ERROR) return;

	// lit message de facon bloquante : un msg existe ici !
	taille = read_port(mPortId, (long *)&msg, &datamsg, sizeof(datamsg));
	if (taille != sizeof(datamsg)) return;

	if (channel) *channel = datamsg.channel;
	lparam = datamsg.lparam;
	data   = datamsg.data;	

} // end of waitForMessage for RThread


//---------------------------------------------------------------------------


//***************************************************************************
int32 RThread::threadEntryPoint(void *data)
//***************************************************************************
/*
	Point d'entree du thread.
	L'argument "data" est un pointeur de fonction (PFV func).
	Saute directement dans la fonction.
	Renvoie toujours 0 au parent.
*/
{
	RThread *elvis = (RThread *)data;
	elvis = M_CAST_AS(elvis, RThread);
	if (elvis)
	{
		elvis->runningSem.acquire();
		elvis->readyToRun();
		elvis->okToKill();
	}
	else M_THROW(kErrClassMismatch);
	return 0;
} // end of threadEntryPoint for RThread


//***************************************************************************
void RThread::readyToRun(void)
//***************************************************************************
{
	try
	{
		run();
	}
	M_CATCH("RThread run");
} // end of readyToRun for RThread




//---------------------------------------------------------------------------

#endif // of RLIB_BEOS

// eoc
