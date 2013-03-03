/*****************************************************************************

	Projet	: Portable Lib

	Fichier: vRServerSocket.cpp
	Partie	: Network

	Auteur	: RM
	Date		: 070398
	Format	: tabs==2

	typical usefull stuff :
		include <socket.h>
		include <netdb.h>
		netdb.h  : MAXHOSTNAMELEN
		socket.h : INADDR_ANY INADDR_BROADCAST INADDR_LOOPBACK B_UDP_MAX_SIZE (63 kB)

	Be warning : every call used "int" which is less than portable. Will use si32 herein.
	For Be (and Windows) assume socket_descriptor != file_descriptor.

	This class always use the AF_INET (Internet Family name).

	The class implementation is probably Be-specific and Windows-compliant but
	may not be Unix-compliant.

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "gMacros.h"
#include "RServerSocket.h"
#include "gErrors.h"

//------------------------------------------------------------


//************************************************************
RServerSocket::RServerSocket(void)
//************************************************************
{
	init();
} // end of default constructor for RServerSocket


//************************************************************
RServerSocket::~RServerSocket(void)
//************************************************************
{
	if (mSocket != -1) close();
} // end of destructor for RServerSocket


//************************************************************
void RServerSocket::init(void)
//************************************************************
{
	mSocket = -1;
} // end of init for RServerSocket


//------------------------------------------------------------


//****************************************************************************
void RServerSocket::listen(si32 port, struct sockaddr_in myadr, si32 count)
//****************************************************************************
{
	if (debug) printf("RServerSocket::listen -- port %d\n", port);

	myadr.sin_port = htons(port);

	try
	{
		// we shouldn't be doing such a thing (open an already opened socket)
		// as a side effect, the socket will be closed.
		if (mSocket != -1) throw RErrEx(kErrAlreadyConnected);

		// ouvre la socket cote 'unix'
		mSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (mSocket < 0) throw RErrEx(kErrSocket);
	
		// faire en sorte qu'un close sur une socket qui ecoute fasse effet
		// immediatement.
		int on=1;
		setsockopt(mSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&on,sizeof(on));

		if (bind(mSocket,(sockaddr*)&myadr,sizeof(myadr))) throw RErrEx(kErrSocket);

		::listen(mSocket, count);
	}
	catch(...)
	{
		close();
		throw;
	}

} // end of listen for RServerSocket


//************************************************************
RSocket * RServerSocket::accept(void)
//************************************************************
{
si32 connected;
struct sockaddr_in adr;

	if (debug) printf("RServerSocket::accept\n");
	if (mSocket == -1) return NULL;

	int lgadr=sizeof(adr);
	connected=::accept(mSocket,(sockaddr*)&adr,&lgadr);

	if (connected < 0 || mSocket == -1) return NULL;
	return new RSocket(connected);

} // end of accept for RServerSocket


//************************************************************
void RServerSocket::close(void)
//************************************************************
{
	if (debug) printf("RServerSocket::close\n");
	if (mSocket != -1)
	{
		closesocket(mSocket);
		mSocket = -1;
	}

} // end of close for RServerSocket




//------------------------------------------------------------


//************************************************************
void RServerSocket::printToStream(void)
//************************************************************
{
	printf("RServerSocket : desc %d\n", mSocket);

} // end of printToStream for RServerSocket

//------------------------------------------------------------

#endif // of RLIB_BEOS

// eof
