/*****************************************************************************

	Projet	: Portable lib

	Fichier	: bRResource.h
	Partie	: App resource holder

	Auteur	: RM
	Date		: 210598
	Format	: tabs==2

	A resource holder contains lists of stuff, typically images and sounds.
	This is semi-generic.
	It is expected that only one instance of this stuff will exist.

	TBDL :
	- the holder could be a list of lists, with the ability to create/remove
	  sublists as needed, on the fly.
	- ability to share resource between several application/machines across the
	  network, with an inquire/update mechanism.

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_BRRESOURCE_
#define _H_BRRESOURCE_

#include "gErrors.h"
#include "RBitmap.h"
#include "RPath.h"
#include "TList.h"
// #include "RSound.h"

#include <TranslationKit.h>

//------------------------------------------------------------

//*******************
struct SRsrcBitmapKey
//*******************
{
	SRsrcBitmapKey(void)  { mBitmap = NULL; mKey = 0;    }
	~SRsrcBitmapKey(void) { if (mBitmap) delete mBitmap; }

	RBitmap	*mBitmap;
	si32 		 mKey;
};


//------------------------------------------------------------


//*****************************************
class RResource : virtual public RInterface
//*****************************************
{
public:

	RResource(void);
  virtual ~RResource(void);
  
	// -- utility routines --
	// TBDL : later extend using "do(action, TList * [bitmap/sound/etc])"

	si32		 loadBitmap(RPath &path);		// returns the key, <0 if invalid
	si32		 addBitmap(RBitmap *bmap);	// returns the key, <0 if invalid
	RBitmap *getBitmap(si32 key);
	void		 removeBitmap(si32 key);

	// -- debug aware stuff --
	void printToStream(void);

protected:

	void makeEmpty(void);

	si32	findBitmapIndex(si32 key);
	si32	mBitmapIndex;

	// -- static lists --

	TList<SRsrcBitmapKey *>	mBitmapList;

private:

	void initTranslationKit(void);
	void terminateTranslationKit(void);

	BTranslatorRoster *mTranslationKit;
};

//------------------------------------------------------------

#endif // of _H_BRRESOURCE_

#endif // of RLIB_BEOS

// eof
