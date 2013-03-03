/*****************************************************************************

	Projet	: Portable lib

	Fichier	: RSocket.h
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

#ifndef _H_BRSOCKET_
#define _H_BRSOCKET_

#include "RString.h"
#include "gErrors.h"


//------------------------------------------------------------


//************
class RSocket
//************
{
public:
	// create unconnected socket. Need to use init() then connect().
  RSocket(void);

	// create RSocket around an existing socket descriptor
  RSocket(si32 descriptor);

  // desctructing the socket will close() it if not already done
  virtual ~RSocket(void);

	// init() is here for the heritage legacy but actually does nothing
	virtual void init(void);

	// connect() actally creates the socket point
	virtual void connect(char *name, si32 port);

	// close() destroy the socket point
	virtual void close(void);

	// return local IP
	si32 ip(void);

	si32 read (void *buffer, si32 size);
	si32 write(void *buffer, si32 size);

	void printToStream(void);

protected:

	RString	mHostName;
	si32		mPort;
	si32		mSocket;		// "socket descriptor"
};

//------------------------------------------------------------

#endif // of _H_BRSOCKET_

#endif // of RLIB_BEOS

// eof
