/*****************************************************************************

	Projet	: Droids

	Fichier	:	bCThread.cpp
	Partie	: Encapsulation

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#include "machine.h"
#include "gCThread.h"

//---------------------------------------------------------------------------

const LONG CThread::mPriorityMap[4] =
{
	B_LOW_PRIORITY,
	B_NORMAL_PRIORITY,
	B_DISPLAY_PRIORITY,
	B_URGENT_PRIORITY
};
	

// humm plus de constante en DR9 PR ? (Ralf, 010897)
#ifndef B_MAX_PORT_COUNT
#define B_MAX_PORT_COUNT 128
#endif


//---------------------------------------------------------------------------


//***************************************************************************
CThread::CThread(void)
//***************************************************************************
{
	mThreadId = -1;	// valeur d'invalidite
	mPortId		= -1;

} // end of constructor for CThread


//***************************************************************************
CThread::~CThread(void)
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

} // end of destructor for CThread


//---------------------------------------------------------------------------


//***************************************************************************
BOOL CThread::init(void)
//***************************************************************************
/*
	NOP
*/
{
	return TRUE;
} // end of init for CThread


//***************************************************************************
BOOL CThread::launch(PFV func,ULONG pri)
//***************************************************************************
/*
*/
{
char nom[B_OS_NAME_LENGTH];

	// le thread est deja lance
	if (mThreadId != -1) return FALSE;

	if (1)
	{
		static long int index=0;
		++index;
		sprintf(nom, "bCThread_%ld", index);
		//printf("%s --> ", nom);
	}

	// creation thread
	mThreadId = spawn_thread((thread_entry)CThread::threadEntryPoint,
											nom,
											mPriorityMap[pri],
											(void *)func);

	//printf("thread : %d --/-- ", mThreadId);

	// erreur creation thread. Suicide !
	if (mThreadId < B_NO_ERROR)
	{
		mThreadId = -1;
		return FALSE;
	}

	// creation port
	mPortId = create_port(B_MAX_PORT_COUNT, nom);

	//printf("port : %d \n", mPortId);

	// erreur creation port. Suicide !
	if (mPortId < B_NO_ERROR)
	{
		mPortId = -1;
		return FALSE;
	}

	// reveille le thread qui est endormi a la creation
	resume_thread(mThreadId);
	
	return TRUE;

} // end of launch for CThread


//***************************************************************************
void CThread::sleep(void)
//***************************************************************************
/*
	Endort un thread.
*/
{
	if (mThreadId != -1) suspend_thread(mThreadId);
} // end of sleep for CThread


//***************************************************************************
void CThread::wake(void)
//***************************************************************************
/*
	Reveille un thread endormi.
*/
{
	if (mThreadId != -1) resume_thread(mThreadId);
} // end of wake for CThread


//***************************************************************************
void CThread::askToKill(void)
//***************************************************************************
/*
	Notifie le thread qu'il va mourrir.
	"You know what ?... you're dead."
	
	Dans la vie reele, endor simplement le thread qui sera bute par le destructeur.
	
	WARNING : sur le Be, si on endors le thread et qu'on fait un wait_for_thread
	ensuite, cela reveille le thread (gasp !)
*/
{
	//sleep();
} // end of askToKill for CThread



//***************************************************************************
void CThread::waitForEnd(void)
//***************************************************************************
/*
	Attente bloquante que le thread soit mort.
	Utilise apres un "askToKill".
*/
{
long exitCode;

	if (mThreadId != -1)
	{
		wait_for_thread(mThreadId, &exitCode);
	}
} // end of waitForEnd for CThread


//---------------------------------------------------------------------------


//***************************************************************************
void CThread::postMessage(CThread *dest,ULONG msg,ULONG lparam,APTR data)
//***************************************************************************
/*
	Rempli une structure SThreadMsg et l'envoi au port du thread de destination.
	Verifie auparavant que ce dernier existe bien.

	ATTENTION : cette fonction bloque si la queue de msg du destinataire est pleine.
	Actuellement, CThread::launch() cree un port avec 128 entrees max.
	On peut ecrire de facon non-bloquante avec un timeout (ne fait rien si queue pleine).
*/
{
SThreadMsg datamsg;
port_id id;

	if (!dest) return;		// pas de destinataire
	id = dest->mPortId;
	if (id == -1) return;	// pas de port alloue

	// construit message
	datamsg.lparam = lparam;
	datamsg.data   = data;

	// renvoie un code d'erreur ignore ici... Utiliser write_port_etc() pour timeout.
	write_port(id, (long)msg, &datamsg, sizeof(datamsg));

} // end of postMessage for CThread


//***************************************************************************
BOOL CThread::receiveMessage(ULONG &msg,ULONG &lparam,APTR &data)
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

	lparam = datamsg.lparam;
	data   = datamsg.data;	
	return TRUE;

} // end of receiveMessage for CThread


//---------------------------------------------------------------------------


//***************************************************************************
LONG CThread::threadEntryPoint(void *data)
//***************************************************************************
/*
	Point d'entree du thread.
	L'argument "data" est un pointeur de fonction (PFV func).
	Saute directement dans la fonction.
	Renvoie toujours 0 au parent.
*/
{
PFV func = (PFV)data;
	func();
	return 0;
} // end of threadEntryPoint for CThread


//---------------------------------------------------------------------------


//***************************************************************************
void CThread::kill(void)
//***************************************************************************
/*
	Cette fonction ne fait pas parti du jeu de fonctions "cross-platform".
*/
{
	if (mThreadId != -1) kill_thread(mThreadId);
} // end of kill for CThread


//---------------------------------------------------------------------------

// eoc
