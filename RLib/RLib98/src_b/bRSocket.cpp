/*****************************************************************************

	Projet	: Portable Lib

	Fichier	: bRSocket.cpp
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
#include "RSocket.h"
#include "gErrors.h"

//------------------------------------------------------------


//************************************************************
RSocket::RSocket(void)
//************************************************************
{
	init();
} // end of default constructor for RSocket


//************************************************************
RSocket::RSocket(si32 descriptor)
//************************************************************
{
	init();
	mSocket = descriptor;
} // end of existing-socket constructor for RSocket


//************************************************************
RSocket::~RSocket(void)
//************************************************************
{
	if (mSocket != -1) close();
} // end of destructor for RSocket


//************************************************************
void RSocket::init(void)
//************************************************************
{
	mHostName = "";
	mPort = -1;
	mSocket = -1;
} // end of init for RSocket


//------------------------------------------------------------


//************************************************************
void RSocket::connect(char *name, si32 port)
//************************************************************
{
struct hostent* server_info;
struct sockaddr_in server_addr;

	if (debug) printf("RSocket::connect -- name '%s' port %d\n", name, port);
	
	try
	{
		// we shouldn't be doing such a thing (open an already opened socket)
		// as a side effect, the socket will be closed.
		if (mSocket != -1) throw RErrEx(kErrAlreadyConnected);
	
		mHostName = name;
		mPort = port;

		if (debug) printf("  open socket\n");
		// ouvre la socket cote 'unix'
		mSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (mSocket < 0) throw RErrEx(kErrSocket);
	
		if (debug) printf("  get host by name\n");
		// HOOK RM d'apres lolo il faut mettre cela dans un mutex
		server_info = gethostbyname(name);
		if (!server_info) throw RErrEx(kErrHostNotFound);
	
		// construct the address
		bzero((char*) &server_addr, sizeof(struct sockaddr_in));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = port;
		server_addr.sin_addr.s_addr = *((u_long*)server_info->h_addr);
		if (debug) printf("  server_addr.sin_addr.s_addr : %08x\n",server_addr.sin_addr.s_addr);
	
		// connect to distant port
		si32 result = ::connect(mSocket, (sockaddr*) &server_addr, sizeof(struct sockaddr_in));
		if (debug) printf("  result connect = %d\n", result);
		if (result < 0)
		{
			if (debug) perror("RSocket::connect");
			throw RErrEx(kErrSocket);
		}
	
/*
		int size = sizeof(server_addr);
		if (getsockname(mSocket, (struct sockaddr*)&server_addr, &size) < 0)
			throw RErrEx(kErrSocket);
*/
	}
	catch(...)
	{
		close();
		throw;
	}

} // end of connect for RSocket


//************************************************************
void RSocket::close(void)
//************************************************************
{
	if (debug) printf("RSocket::close\n");
	if (mSocket != -1)
	{
		closesocket(mSocket);
		mSocket = -1;
	}

} // end of close for RSocket


//************************************************************
si32 RSocket::ip(void)
//************************************************************
{
	if (mSocket < 0) throw RErrEx(kErrSocket);

	struct sockaddr_in interface;
	int size = sizeof(interface);
	
	if (getsockname(mSocket, (struct sockaddr*)&interface, &size) < 0)
		throw RErrEx(kErrSocket);
	else
		return (interface.sin_addr.s_addr);

	return 0;
} // end of ip for RSocket


//------------------------------------------------------------



//************************************************************
si32 RSocket::read(void *buffer, si32 size)
//************************************************************
{
	if (debug) printf("RSocket::read -- size %d -- ptr %p\n",size,buffer);

	size = recv(mSocket, buffer, size, 0);
	return size;

} // end of read for RSocket


//************************************************************
si32 RSocket::write(void *buffer, si32 size)
//************************************************************
{
	if (debug) printf("RSocket::write\n");

	size = send(mSocket, buffer, size, 0);
	return size;

} // end of write for RSocket


//------------------------------------------------------------


//************************************************************
void RSocket::printToStream(void)
//************************************************************
{
	printf("CSocket : port %d -- desc %d -- hostname %s\n",
		mPort, mSocket, (char *)mHostName);

} // end of printToStream for RSocket

//------------------------------------------------------------

#endif // of RLIB_BEOS

// eof
