/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CTrack.h
	Partie	: Misc

	Auteur	: RM
	Date		: 180797
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CTRACK_
#define _H_CTRACK_

class CKey;
class CInputMethod;

//--------------------------------------------------------

//**********
class CTrack
//**********
{
public:
	CTrack(void);
	~CTrack(void);

	CInputMethod*	method(void)	{ return mMethod; }
	bool					setMethod(CInputMethod *method);
	
	int32					milliLength(void)	{ return mMilliLength; }

	// key list management
	CKey				*	keyAt(int32 index);
	bool					addKey(CKey *key);
	bool					removeKey(CKey *key);

	void					removeAllKey(void);

private:
	CInputMethod	*mMethod;
	BMessage			*mMethodParam;

	BList					mKeyList;			// list of ptr on CKey
	int32					mMilliLength;	// time in millisecond or -1=infinite
	BLocker				mLock;

}; // end of class def CTrack

//--------------------------------------------------------
#endif // of _H_CTRACK_
// eoh
