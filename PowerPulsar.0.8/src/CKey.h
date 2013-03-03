/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CKey.h
	Partie	: Misc

	Auteur	: RM
	Date		: 180797
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CKEY_
#define _H_CKEY_

class CFilter;

//--------------------------------------------------------


//********
class CKey
//********
{
public:
	CKey(void);
	~CKey(void);

	CFIlter	*	filter(void)	{ return mFilter; }
	void			setFilter(CFilter *filter);

	BMessage*	param(void)	{ return mParam; }
	BMessage*	newParamModel(void);
	void			newParam(BMessage	*message);
	void			notifyParamChanged(int32 index);	// index -1 if anyone

private:
	CFilter		*	mFilter;			// NULL if not initialized
	int32				mMilliStart;
	int32				mMilliEnd;		// always >= to mMilliEnd 
	BMessage	*	mParam;				// NULL if no filter params
}; // end of class def CKey



//--------------------------------------------------------
#endif // of _H_CKEY_
// eoh
