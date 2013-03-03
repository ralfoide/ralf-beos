/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRThreadTranslator.cpp
	Partie	: Threads

	Auteur	: RM
	Format	: tabs==2

	Specific class for BeOS.
	This class inherits from a BLooper and knows how to send and receive
	RThread thread messages via the BLooper/BHandler messaging system.
	It provides a static public method for sending such a message and knows
	how to receive it and to *forward* it to the given target.

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "gMacros.h"
#include "RThreadTranslator.h"

#include <Message.h>

//---------------------------------------------------------------------------

//***************************************************************************
RThreadTranslator::RThreadTranslator(rbool immediateLaunch)
									:BLooper("RThreadTranslator")
//***************************************************************************
{
	if (debug) printf("RThreadTranslator::RThreadTranslator\n");
	mTarget = NULL;
	if (immediateLaunch) launch();
} // end of constructor for RThreadTranslator


//***************************************************************************
RThreadTranslator::RThreadTranslator(RThread *target, rbool immediateLaunch)
									:BLooper("RThreadTranslator")
//***************************************************************************
{
	if (debug) printf("RThreadTranslator::RThreadTranslator %p\n", target);
	mTarget = target;
	if (immediateLaunch) launch();
} // end of constructor for RThreadTranslator


//***************************************************************************
RThreadTranslator::~RThreadTranslator(void)
//***************************************************************************
{
	if (debug) printf("RThreadTranslator::~RThreadTranslator\n");
} // end of destructor for RThreadTranslator


//***************************************************************************
void RThreadTranslator::launch(void)
//***************************************************************************
{
	if (debug) printf("RThreadTranslator::launch\n");
	try
	{
		Run();
	}
	M_CATCH("RThreadTranslator launch");
} // end of launch for RThreadTranslator


//---------------------------------------------------------------------------


//***************************************************************************
void RThreadTranslator::MessageReceived(BMessage *msg)
//***************************************************************************
/*
	Ideally this should be derived by people lile comm_lib in order
	to filter comm_lib messenger replies. for example B_NO_REPLY.
	It's protocol-dependant, though
*/
{
	if (debug) printf("RThreadTranslator::MessageReceived ");
	// get rid of some specific message that are comm_lib dependant
	// (this should go into a commlib dereived instance later)
	try
	{
		if (msg->what == B_NO_REPLY)
		{
			if (debug)
			{
				printf("-- filtered B_NO_REPLY\n");
				msg->PrintToStream();
				return;
			}
		}
		else if (mTarget && msg)
		{
			ui32 lparam;
			vptr data = NULL;
			int32 temp1;
			const void *temp2;
			ssize_t numBytes;
			int32 channel;
			rbool hasChannel;
	
			hasChannel = (msg->FindInt32(K_RUNTRANS_CHANNEL, &channel) == B_NO_ERROR);
	
			if (msg->FindInt32(K_RUNTRANS_LPARAM, &temp1) != B_NO_ERROR)
			{
				if (debug)
				{
					printf("-- no CRun_lparam !!\n"
								"Currently don't throw(\"RThreadTranslator bad msg received\");\n");
					msg->PrintToStream();
					return;
				}
			}
	
			lparam = *((ui32*)(&temp1));
	
			// the 'data' ptr can be null, in which case no data is passed into the message
			if (msg->FindData(K_RUNTRANS_DATA, 'data', 0, &temp2, &numBytes) == B_NO_ERROR)
			{
				M_ASSERT_PTR(temp2);
				data = new char[numBytes];
				M_ASSERT_PTR(data);
				memcpy(data, temp2, numBytes);
			}
	
			RThread::postMessage(mTarget, msg->what, lparam, data, (hasChannel ? (ui32*)&channel : NULL));
		}
	}
	M_CATCH("RThreadTranslator");
} // end of MessageReceived for RThreadTranslator


//***************************************************************************
BMessage * RThreadTranslator::createMessage(ui32 channel,
																				 ui32 msgcode, ui32	lparam,
																				 vptr data, ui32 datasize)
//***************************************************************************
{
BMessage *msg = new BMessage(msgcode);
int32 temp1;

	if (!msg) return NULL;

	temp1 = *((int32 *)&lparam);

	msg->AddInt32(K_RUNTRANS_LPARAM, temp1);
	msg->AddInt32(K_RUNTRANS_CHANNEL, channel);
	if (data) msg->AddData(K_RUNTRANS_DATA, 'data', data, datasize);

	return msg;	

} // end of createMessage for RThreadTranslator




//---------------------------------------------------------------------------

#endif // of RLIB_BEOS

// eoc
