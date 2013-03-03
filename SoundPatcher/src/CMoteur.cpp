/*****************************************************************************

	Projet	: VideoShooter

	Fichier	:	CMoteur.cpp
	Partie	: Moteur

	Auteur	: RM
	Date		: 220798
	Format	: tabs==2

*****************************************************************************/

#include <zlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <socket.h>
#include <netdb.h>

#include "CMoteur.h"
#include "externs.h"
#include "gMacros.h"
#include "gErrors.h"

#include "RPaneDialog.h"

//---------------------------------------------------------------------------

#define K_PACKET_MAX_SIZE	8192		// B_UDP_MAX_SIZE is 63 kB
#define K_PACKET_HEAD			256
#define K_PACKET_TAIL			256

//---------------------------------------------------------------------------

// HOOK TBDL HACK HACK !

extern char *gFilename;

const fp64 K_FRAME_RATE			= 0.5e6;

#define K_PORT	31415
#define K_PACKET_SIG		'apcm'


#define DEBUG 1

#if DEBUG==1
	#define DPRINTF(s) printf s
#else
	#define DPRINTF(s)
#endif

#define DPRINTF2(s) printf s
//#define DPRINTF2(s)

bool global_hack = false;


struct sStat
{
	fp64	recv_time;
	ui32	recv_packets;
	ui32	recv_samples;
	fp64	send_time;
	ui32	send_packets;
	ui32	send_samples;

	sStat(void) { recv_time = (fp64)system_time(); recv_packets = 0; recv_samples = 0;
								send_time = (fp64)system_time(); send_packets = 0; send_samples = 0; }
	void 	update(ui32 dp, ui32 ds, fp64 &t, ui32 &p, ui32 &s, const char *st)
	{
		fp64 t2 = (fp64)system_time();
		fp64 dt = 5e6;	// in useconds
		p += dp;
		s += ds;
		if (t2 > t+dt)
		{
			dt = t2-t;
			printf("%s : packets %ld [%5.2f p/s] -- samples %6ld [%7.2f s/s] -- %5ld sp/packs -- %4.2f kB/s\n",
						st,
						p, (fp64)p*1e6/dt,
						s, (fp64)s*1e6/dt,
						(p!=0) ? s/p : 0,
						(fp64)s/2.*1e6/dt/1024.		// 1 sample == .5 bytes dans un packet
						);
			// reinit
			t = t2;
			p = 0;
			s = 0;
		}
	}
	void	update_recv(ui32 dp, ui32 ds)	{ update(dp, ds, recv_time, recv_packets, recv_samples, "Receive"); }
	void	update_send(ui32 dp, ui32 ds)	{ update(dp, ds, send_time, send_packets, send_samples, "Send   "); }
};

sStat gStat;


//***************************************************************************
CMoteur::CMoteur(void)
				:RThread(),
				 mSoundPlay(mSharedSoundPlayBuffer),
				 mSoundRecord(mSharedSoundRecordBuffer)
//***************************************************************************
{
	mUseSoundPlay = true;
	mUseSoundRecord = true;
	mSocketSend = -1;
	mSocketReceive = -1;
	mSendBuffer = NULL;
	mZlibSendBuffer = NULL;
	mSendCount = 0;

} // end of constructor for CMoteur


//***************************************************************************
CMoteur::~CMoteur(void)
//***************************************************************************
{
} // end of destructor for CMoteur



//---------------------------------------------------------------------------



//***************************************************************************
void CMoteur::run(void)
//***************************************************************************
{
	ui32 msg;
	ui32 msgParam;
	vptr msgData;
	rbool loop = true;
	fp64 temps, deltatemps, tempsInitial;
	RSem frameRateSem;
	bool debug_toggle = true;

	DPRINTF(("CMoteur::run\n"));

	CReceiveAndPlay	mReceiver(this);

	frameRateSem.acquire();	// rend le semaphore bloquant

	bool allow_sound = (getenv("NO_SOUND")==NULL);
	bool send_only = (getenv("SEND_ONLY")!=NULL);
	bool recv_only = (getenv("RECEIVE_ONLY")!=NULL);
	if (!send_only && !recv_only)
	{
		send_only=true;
		recv_only=true;
	}

	DPRINTF(("CURRENT MODE -- send_only %d -- recv_only %d\n", send_only, recv_only));

	try
	{
		// this is a hack to have a window to quit
		RPaneDialog *dialog;
		if(1)
		{
			dialog = new RPaneDialog(RRect(50,50,50+200,50+150),
									"SoundPatcher Run",
									"Quit", "Toggle");
									// ok, cancel, retry
			M_ASSERT_PTR(dialog);
			dialog->go(*this, 'TEST', false);
		}

		mSendBuffer = (si8 *)malloc(K_PACKET_MAX_SIZE+K_PACKET_HEAD+K_PACKET_TAIL);
		mZlibSendBuffer = (si8 *)malloc(K_PACKET_MAX_SIZE+K_PACKET_HEAD+K_PACKET_TAIL);
		M_ASSERT_PTR(mSendBuffer);
		M_ASSERT_PTR(mZlibSendBuffer);

		DPRINTF(("init sound & sockets\n"));

		if (recv_only && allow_sound)	initSoundPlay();
		if (send_only)								initSocketSend();
		if (send_only && allow_sound)	initSoundRecord();
		if (recv_only)								initSocketReceive();
		if (recv_only)								mReceiver.launch();

		if (!mUseSoundPlay || !mUseSoundRecord)
		{
			loop = false;
			DPRINTF(("Lack of sound (play %d, record %d\n", mUseSoundPlay, mUseSoundRecord));
		}

		tempsInitial = systemTime();

		while(loop)
		{
			temps = systemTime();

			//lecture message
			if (receiveMessage(msg, msgParam, msgData))
			{
				// process message
				switch(msg)
				{
					case K_OK_BUTTON_PANEKEY:
						DPRINTF(("Message K_OK_BUTTON_PANEKEY ! (Quit)\n"));
						if(dialog)
						{
							dialog->deleteWindow();
							dialog = NULL;
							gApplication->processMessage(K_MSG_RTHREAD_EXIT);
						}
						break;
					case K_CANCEL_BUTTON_PANEKEY:
						DPRINTF(("Message K_CANCEL_BUTTON_PANEKEY ! (Toggle)\n"));
						debug_toggle = !debug_toggle;
						global_hack = !global_hack;
						break;
					case K_MSG_RTHREAD_EXIT:
						DPRINTF(("CMoteur::msg K_MSG_RTHREAD_EXIT\n"));
						loop = false;
						break;
				}
			}

			if (debug_toggle)
				if (send_only)
					sendRecordPackets();

			// synchronize to frame rate
			deltatemps = temps+K_FRAME_RATE-systemTime();
			//DPRINTF(("deltatemps %f\n", deltatemps));
			if (deltatemps > 0) frameRateSem.acquireTimeout(deltatemps);
		}

		if(dialog) dialog->deleteWindow();
	}
	M_CATCH("Moteur Play");

	DPRINTF(("ask receiver to kill\n"));
	if (recv_only) mReceiver.askToKill();

	DPRINTF(("close sockets and sound\n"));
	if (recv_only)								closeSocketReceive();
	if (send_only && allow_sound)	closeSoundRecord();
	if (send_only)								closeSocketSend();
	if (recv_only && allow_sound)	closeSoundPlay();

	if (mSendBuffer) free(mSendBuffer);
	if (mZlibSendBuffer) free(mZlibSendBuffer);

	DPRINTF(("wait for receiver to end\n"));
	mReceiver.waitForEnd();

	// ask application to kill itself (i.e. to quit cleanly)
	gApplication->processMessage(K_MSG_RTHREAD_EXIT);

} // end of run for CMoteur

//---------------------------------------------------------------------------



//***************************************************************************
void CMoteur::initSoundPlay(void)
//***************************************************************************
{
	DPRINTF(("CMoteur::initSoundPlay\nuse sound %d\n", mUseSoundPlay));
	mUseSoundPlay = mSoundPlay.soundDeviceOpen();
	DPRINTF(("mSoundPlay.soundDeviceOpen -- use sound %d\n", mUseSoundPlay));
	if (!mUseSoundPlay) DPRINTF(("CMoteur Play : no sound device open\n"));

	if (mUseSoundPlay)
	{
		// start sound right now
		mSoundPlay.start();
	}

} // end of initSoundPlay for CMoteur


//***************************************************************************
void CMoteur::closeSoundPlay(void)
//***************************************************************************
{
	DPRINTF(("CMoteur::closeSoundPlay\n"));
	if (mUseSoundPlay) mSoundPlay.stop();

	mUseSoundPlay = true;

} // end of closeSoundPlay for CMoteur


//***************************************************************************
void CMoteur::initSoundRecord(void)
//***************************************************************************
{
	DPRINTF(("CMoteur::initSoundRecord\n"));

	mUseSoundRecord = mSoundRecord.soundDeviceOpen();
	if (!mUseSoundRecord) DPRINTF(("CMoteur initSoundRecord : no sound device open\n"));

	if (mUseSoundRecord)
	{
		// start sound right now
		mSoundRecord.start();
	}

} // end of initSound for CMoteur


//***************************************************************************
void CMoteur::closeSoundRecord(void)
//***************************************************************************
{
	DPRINTF(("CMoteur::closeSoundRecord\n"));
	if (mUseSoundRecord) mSoundRecord.stop();

	mUseSoundRecord = true;

} // end of closeSoundRecord for CMoteur

//---------------------------------------------------------------------------


//***************************************************************************
void CMoteur::initSocketSend(void)
//***************************************************************************
{
	DPRINTF(("CMoteur::initSocketSend\n"));
	// Reminder : http://www.ecst.csuchico.edu/~beej/guide/net/

	// ouvre la socket cote 'unix'
	mSocketSend = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mSocketSend < 0) throw RErrEx(kErrSocket);

	struct hostent* server_info;
	//ina.sin_addr.s_addr = inet_addr(gFilename);
	// HOOK RM d'apres lolo il faut mettre cela dans un mutex
	server_info = gethostbyname(gFilename);
	if (!server_info) throw RErrEx(kErrHostNotFound);
	// construct the address
	memset((char*) &mSendSocketAddr, 0, sizeof(struct sockaddr_in));
	mSendSocketAddr.sin_family = AF_INET;
	mSendSocketAddr.sin_port = htons(K_PORT);
	mSendSocketAddr.sin_addr.s_addr = *((u_long*)server_info->h_addr);
	DPRINTF(("  mSendSocketAddr.sin_addr.s_addr : %08x\n",mSendSocketAddr.sin_addr.s_addr));


} // end of initSocketSend for CMoteur


//***************************************************************************
void CMoteur::closeSocketSend(void)
//***************************************************************************
{
	DPRINTF(("CMoteur::closeSocketSend\n"));
	closesocket(mSocketSend);
} // end of closeSocketSend for CMoteur


//***************************************************************************
void CMoteur::initSocketReceive(void)
//***************************************************************************
{
	DPRINTF(("CMoteur::initSocketReceive\n"));
	// Reminder : http://www.ecst.csuchico.edu/~beej/guide/net/

	// ouvre la socket cote 'unix'
	mSocketReceive = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mSocketReceive < 0) throw RErrEx(kErrSocket);

	//struct hostent* server_info;
	//ina.sin_addr.s_addr = inet_addr(gFilename);
	// HOOK RM d'apres lolo il faut mettre cela dans un mutex
	//server_info = gethostbyname(gFilename);
	//if (!server_info) throw RErrEx(kErrHostNotFound);
	// construct the address
	//bzero((char*) &mReceiveSocketAddr, sizeof(struct sockaddr_in));
	mReceiveSocketAddr.sin_family = AF_INET;
	mReceiveSocketAddr.sin_port = htons(K_PORT);
	mReceiveSocketAddr.sin_addr.s_addr = INADDR_ANY; //*((u_long*)server_info->h_addr);
	DPRINTF(("  mReceiveSocketAddr.sin_addr.s_addr : %08x\n",mReceiveSocketAddr.sin_addr.s_addr));

	bind(mSocketReceive, (sockaddr *) &mReceiveSocketAddr, sizeof( sockaddr_in ) );

} // end of initSocketReceive for CMoteur


//***************************************************************************
void CMoteur::closeSocketReceive(void)
//***************************************************************************
{
	DPRINTF(("CMoteur::closeSocketReceive\n"));
	closesocket(mSocketReceive);
} // end of closeSocketReceive for CMoteur


//***************************************************************************
rbool CMoteur::sendPacket(void *data, si32 len)
//***************************************************************************
{
	if (mSocketSend<0 || !data || !len) return false;
	si32 result;
	result = sendto(mSocketSend, data, len, 0, 
               		(struct sockaddr *)&mSendSocketAddr,
               		sizeof(struct sockaddr));
	DPRINTF2(("CMoteur::sendPacket data %p -- len %ld --> result %ld\n", data, len, result));
	return result == len;
	
} // end of sendPacket for CMoteur


//***************************************************************************
si32 CMoteur::receivePacket(void *data, si32 len)
//***************************************************************************
{
	DPRINTF2(("CMoteur::receivePacket -- mSocketReceive %d -- data %p -- len %ld\n", mSocketReceive, data, len));
	if (mSocketReceive<0 || !data || !len) return -1;
	si32 result;
	int fromlen = sizeof(struct sockaddr);
	result = recvfrom(mSocketReceive, data, len, 0,
               		(struct sockaddr *)&mReceiveSocketAddr,
               		&fromlen);
	DPRINTF2(("\nReceived from addrs %08x -- result %ld\n", mReceiveSocketAddr.sin_addr.s_addr, result));
	return result;
} // end of closeSocketReceive for CMoteur


//***************************************************************************
void CMoteur::CReceiveAndPlay::run(void)
//***************************************************************************
{
	ui32 msg;
	ui32 msgParam;
	vptr msgData;
	bool loop=true;
	si8 *packet = (si8 *)malloc(K_PACKET_MAX_SIZE);
	ui32 *sig = (ui32*)packet;
	si32 *count = (si32 *)(sig+1);
	si8	 *data, *data_init = (si8 *)(sig+2);
	si32 last_count = -1;
	ui32 totalsample = 0;
	ui32 totalpackets = 0;


	try
	{
		DPRINTF(("CMoteur::CReceiveAndPlay::run starting\n"));
		M_ASSERT_PTR(packet);
		while(loop)
		{
			//lecture message
			if (receiveMessage(msg, msgParam, msgData))
			{
				DPRINTF2(("CMoteur::CReceiveAndPlay::run -> msg %08lx\n", msg));
				// process message
				switch(msg)
				{
					case K_MSG_RTHREAD_EXIT:
						DPRINTF(("CMoteur::CReceiveAndPlay::run::msg K_MSG_RTHREAD_EXIT\n"));
						loop = false;
						break;
				}
			}
			// lecture packet network
			si32 taille = K_PACKET_MAX_SIZE;
			taille = mMoteur->receivePacket(packet, taille);
			*sig = ntohl(*sig);
			*count = ntohl(*count);
			data = data_init;
			totalpackets++;
	
			DPRINTF2(("RECEIVED packet --> sig %08lx (%4.4s) -- taille %ld -- count %ld\n", *sig, (char *)sig, taille, *count));
			if (taille >= 8
					&& *sig == K_PACKET_SIG
					&& *count > last_count
					&& mMoteur->mUseSoundPlay)
			{
				last_count = *count;
	
				while(1)
				{
					ui32 nbs = ntohl(*((ui32 *)data));
					DPRINTF2(("Parse block size %ld\n", nbs));
					if (!nbs) break;
	
					data += sizeof(ui32);
					ui32 nbSample2 = nbs/2;
	
					SSoundBuffer *buf = new SSoundBuffer(nbs);
					M_ASSERT_PTR(buf && buf->mBuffer);
		
					// and decompress it
					mMoteur->mAdpcmCodec.decode((ui8 *)data, buf->mBuffer, nbSample2);
					data += nbSample2;
	
					totalsample += nbs;
	
#if 1
	static si32 a=0;
	if ((++a)&31==31)
	{
		printf("DECODE [%08ld]  ", a);
		for(si32 b=0; b<6; b++) printf("%4.4x ", (ui16)buf->mBuffer[b]);
		printf("\n");
	}
#endif

					// insert buffer : add data to the sound list
printf("mMoteur->mSharedSoundPlayBuffer.mSem.acquire\n");
					mMoteur->mSharedSoundPlayBuffer.mSem.acquire();
printf("mMoteur->mSharedSoundPlayBuffer.mList.addLast\n");
					mMoteur->mSharedSoundPlayBuffer.mList.addLast(buf);
printf("mMoteur->mSharedSoundPlayBuffer.mSem.release\n");
					mMoteur->mSharedSoundPlayBuffer.mSem.release();
				}
			}
	
			gStat.update_recv(totalpackets, totalsample);
			totalpackets = 0;
			totalsample = 0;
		}
		DPRINTF(("CMoteur::CReceiveAndPlay::run stoping\n"));

		if (packet) free(packet);
	}
	M_CATCH("CMoteur::CReceiveAndPlay::run");

}


//***************************************************************************
void CMoteur::sendRecordPackets(void)
//***************************************************************************
{
	// first we fake it
	if (!mSendBuffer || !mZlibSendBuffer) return;
	ui32 *sig = (ui32*)(mSendBuffer /*+K_PACKET_HEAD*/);
	si32 *count = (si32 *)(sig+1);
	ui8	 *data = (ui8 *)(sig+2);
	si32 predictsize = 0;
	ui32 nbBuf;
	ui32 totalbuf = 0;
	ui32 totalsample = 0;
	ui32 totalpackets = 0;
	*sig = htonl(K_PACKET_SIG);

	mSharedSoundRecordBuffer.mSem.acquire();
	nbBuf = mSharedSoundRecordBuffer.mList.size();
	mSharedSoundRecordBuffer.mSem.release();

	// process buffers
	for(; nbBuf > 0; nbBuf--)
	{
		SSoundBuffer *buf;

		mSharedSoundRecordBuffer.mSem.acquire();
		buf = mSharedSoundRecordBuffer.mList.removeFirst();
		mSharedSoundRecordBuffer.mSem.release();

		if (!buf)
		{
			// there are less buffers than expected, write a null-size
			// tag in the file and give up.
			break;
		}

		// write number of _samples_ (not bytes !!)
		// when we will store the ADPCM data, we know the compression has
		// a factor of 1/4 in bytes-size, so there will be samples/2 bytes
		// in the compressed buffer
		
		totalbuf++;
		ui32 nbSample = buf->mNbSample;
		ui32 nbSample2 = nbSample/2;
		//ui32 nbSample2 = nbSample*2;

		DPRINTF2(("Recorded buf %ld over %ld -- block size %ld\n", totalbuf, nbBuf, nbSample));

#if 0
static si32 a=0;
	if ((++a)&31==31)
	{
		printf("ENCODE [%08ld]  ", a);
		for(si32 b=0; b<6; b++) printf("%4.4x ", (ui16)buf->mBuffer[b]);
		printf("\n");
	}
#endif

		//  and compress it
		*((ui32 *)data) = htonl(nbSample);
		data += sizeof(ui32);
		mAdpcmCodec.encode(buf->mBuffer, data, nbSample);
		//memcpy(data, buf->mBuffer, nbSample2);
		data += nbSample2;
		totalsample += nbSample;

		// adjust count, predic how much buffers would fit
		if (!predictsize)
		{
			ui32 nb = (K_PACKET_MAX_SIZE-8)/(nbSample2+4);
			if (nb < nbBuf)
			{
				DPRINTF(("sending only %ld buffers out of %ld\n", nb ,nbBuf));
				nbBuf = nb;
			}
		}

		// free data and buffer
		delete buf;
	}

	if (totalbuf)
	{
		// terminate the data packet
		*((uint32 *)data) = htonl(0);
		data += sizeof(uint32);

		// update the counter
		mSendCount++;
		*count = htonl(mSendCount);
	
		uint32 size = (uint32)data - (uint32)mSendBuffer;
	
		DPRINTF2(("sending that damn packet with size %ld\n", size));
		sendPacket(mSendBuffer, size);

/*
		if(size)	// test zlib
		{
				uLongf packsize = (uLongf)K_PACKET_MAX_SIZE;
				int result = compress((Bytef *)mZlibSendBuffer, &packsize, (const Bytef *)mSendBuffer, (uLong) size);
				if (result == Z_OK)
					printf("zlib --> %8ld -> %8ld --- ratio %0.2f\n", size, packsize, (float)packsize/(float)size);
				else
					printf("zlib error %d\n", result);
		}
*/

		totalpackets++;
	}

	gStat.update_send(totalpackets, totalsample);
}

//---------------------------------------------------------------------------

// eoc
