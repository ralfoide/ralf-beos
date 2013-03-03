/*****************************************************************************

	Projet	: Portable lib

	Fichier: RServerSocket.h
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

#ifdef RLIB_BEOS

#ifndef _H_BRSERVERSOCKET_
#define _H_BRSERVERSOCKET_

#include "RString.h"
#include "RSocket.h"
#include "gErrors.h"


//------------------------------------------------------------


//******************
class RServerSocket
//******************
{
public:
	// create unconnected socket. Need to use init() then connect().
  RServerSocket(void);

  // desctructing the socket will close() it if not already done
  virtual ~RServerSocket(void);

	// init() is here for the heritage legacy but actually does nothing
	virtual void init(void);

	// listen() throws kErrWouldBlock or kErrSocket or nothing if listening is OK
	virtual void listen(si32 port, struct sockaddr_in myadr /* ? */, si32 count);

	// once listen is OK, patiently wait for connections
	virtual RSocket *accept(void);

	// close() destroy the socket point
	virtual void close(void);

	void printToStream(void);

protected:

	si32		mSocket;		// "socket descriptor"
};

//------------------------------------------------------------

#endif // of _H_BRSERVERSOCKET_

#endif // of RLIB_BEOS

// eof
