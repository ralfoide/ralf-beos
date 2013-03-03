/*****************************************************************************

	Projet	: Pulsar

	Fichier	:	CPreference.h
	Partie	: Prefs

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CPREFERENCE_
#define _H_CPREFERENCE_

#include <String.h>
#include <Message.h>


//---------------------------------------------------------------------------


//***************
class CPreference
//***************
{
public:

	// dirname & prefname should not contain any '/' chars !
	
	CPreference(const char *dirname, const char *prefname = NULL);
	~CPreference(void);

	bool	load(void);
	bool	save(void);

	bool	getPref(const char *name, BMessage &msg);
	bool	setPref(const char *name, BMessage &msg);

private:

	BString		mDirName;
	BString		mPrefName;
	BMessage	mPref;
};

//---------------------------------------------------------------------------

#endif // of _H_CPREFERENCE_

// eoh

