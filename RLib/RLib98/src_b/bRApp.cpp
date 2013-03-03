/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	bRApp.h
	Partie	: Portable App Wrapper

	Auteur	: RM
	Date		: 010698
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "gMacros.h"
#include "RApp.h"

#include <Entry.h>
#include <Path.h>

//---------------------------------------------------------------------------


//***************************************************************************
void RApp::RefsReceived(BMessage *message)
//***************************************************************************
{

	try
	{
		BEntry entry;
		BPath localpath;
		entry_ref ref;
		uint32 type;
		int32 count;
		int32 i;
		
		message->GetInfo("refs", &type, &count);
		if (type != B_REF_TYPE) return;
		
		for(i = count-1; i>=0; i--)
		{
			if (   message->FindRef("refs", i, &ref) >= B_NO_ERROR
					&& entry.SetTo(&ref) >= B_NO_ERROR
					&& entry.GetPath(&localpath) >= B_NO_ERROR)
			{
				RPath rpath(localpath.Path());
				openDocument(rpath, i==0);
			}
		}
	}
	M_CATCH("RApp::RefsReceived");

} // end of RefsReceived for RApp

//---------------------------------------------------------------------------

#endif // RLIB_BEOS

// eoc
