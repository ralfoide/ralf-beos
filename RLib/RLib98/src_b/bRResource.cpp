/*****************************************************************************

	Projet	: Portable lib

	Fichier	: bRResource.cpp
	Partie	: Core bitmap

	Auteur	: RM
	Date		: 020598
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "gMacros.h"
#include "RResource.h"
#include "gErrors.h"

#include <Application.h>
#include <Directory.h>
#include <Entry.h>
#include <Roster.h>

//------------------------------------------------------------


//************************************************************
RResource::RResource(void)
//************************************************************
{
	mTranslationKit = NULL;
	mBitmapIndex = 0;
	initTranslationKit();
} // end of default constructor for RResource


//************************************************************
RResource::~RResource(void)
//************************************************************
{
	makeEmpty();
	terminateTranslationKit();
} // end of destructor for RResource


//************************************************************
void RResource::makeEmpty(void)
//************************************************************
{
si32 count;

	for(count = mBitmapList.size(); count>0; count--)
	{
		SRsrcBitmapKey *item = mBitmapList.removeLastItem();
		if (item) delete item;
	}

} // end of makeEmpty for RResource


//------------------------------------------------------------


//***************************************************************************
void RResource::initTranslationKit(void)
//***************************************************************************
{
status_t err;
app_info info;
BEntry fichier,dummy;
BDirectory appDir, addonDir;

	mTranslationKit = new BTranslatorRoster;
	if (!mTranslationKit) M_THROW(kErrTranslationKit);

	err = mTranslationKit->AddTranslators(NULL); // load default translators
	printf("RResource::initTranslationKit [NULL] == %ld\n", err);

	be_app->GetAppInfo(&info);
	err = fichier.SetTo(&info.ref);
	if (!err) err = fichier.GetParent(&appDir);
	if (!err)
	{
		if (addonDir.SetTo(&appDir, "add-ons") < B_NO_ERROR) addonDir = appDir;
		BPath path(&addonDir, "Translators");
		mTranslationKit->AddTranslators(path.Path());  // load additional translators
		printf("RResource::initTranslationKit [%s] == %ld\n", path.Path(), err);
	}

} // end of initTranslationKit for RResource


//***************************************************************************
void RResource::terminateTranslationKit(void)
//***************************************************************************
{
	if (mTranslationKit) delete mTranslationKit;
	mTranslationKit = NULL;
} // end of terminateTranslationKit for RResource



//------------------------------------------------------------


//************************************************************
si32 RResource::findBitmapIndex(si32 key)
//************************************************************
{
SRsrcBitmapKey *item;
si32 count;
si32 k = key;

	count = mBitmapList.size();
	if (!count) return -1;
	count--;
	if (k >= count) k = count;
	
	while(1)
	{
		item = mBitmapList.itemAt(k);
		si32 k2 = item->mKey;
		if (k2 == key) return k;
		else if (k2 < key)
		{
			if (k <count) k++;
			else return -1;
		}
		else
		{
			if (!k) return -1;
			else k--;
		}
	}
	
	return -1;
	
} // end of findBitmapIndex for RResource


//************************************************************
si32 RResource::loadBitmap(RPath &path)
//************************************************************
{
	//if (debug) printf("RResource::loadBitmap %s\n", path.path());
	try
	{
		BFile file(path.path(), B_READ_ONLY); 
		BTranslatorRoster *roster = mTranslationKit;
		if (!roster) roster = BTranslatorRoster::Default(); 
		//if (debug) printf("  roster %p\n", roster);
		BBitmapStream stream; 
		BBitmap *result = NULL; 
		//if (debug) printf("  roster->Translate...\n");
		if (roster->Translate(&file, NULL, NULL, &stream, B_TRANSLATOR_BITMAP) < B_OK) 
			return -1; 
		stream.DetachBitmap(&result);
		//if (debug) printf("  translate result %p\n", result);
		
		if(result)
		{
			RBitmap *rmap = new RBitmap(*result, true);
			//if (debug) printf("  r_bitmap %p\n", rmap);
			if (rmap) return addBitmap(rmap);
			else return -1;
		}
		else return -1;
	}
	catch(...)
	{
		return -1;
	}
} // end of loadBitmap for RResource


//************************************************************
si32 RResource::addBitmap(RBitmap *bmap)
//************************************************************
{
SRsrcBitmapKey *item;

	//if (debug) printf("RResource::addBitmap %p\n", bmap);

	if (!bmap) M_THROW(kErrUnsupported);
	item = new SRsrcBitmapKey;
	M_ASSERT_PTR(item);
	item->mKey = mBitmapIndex++;
	item->mBitmap = bmap;
	//if (debug) printf("  item key %d, bitmap %p\n", item->mKey, bmap);
	
	if (!mBitmapList.addItem(item)) return -1; //M_THROW(kErr);
	//if (debug) printf("  RResource::addBitmap ok at key %d\n", item->mKey);
	return item->mKey;

} // end of addBitmap for RResource


//************************************************************
RBitmap * RResource::getBitmap(si32 key)
//************************************************************
{
SRsrcBitmapKey *item;
si32 i;

	i = findBitmapIndex(key);
	if (i < 0) return NULL;
	item = mBitmapList.itemAt(i);
	if (!item) return NULL;
	return item->mBitmap;

} // end of getBitmap for RResource


//************************************************************
void RResource::removeBitmap(si32 key)
//************************************************************
{
si32 i;

	i = findBitmapIndex(key);
	if (i >= 0) mBitmapList.removeItemAt(i);

} // end of removeBitmap for RResource


//------------------------------------------------------------


//************************************************************
void RResource::printToStream(void)
//************************************************************
{
SRsrcBitmapKey *item;
ui32 n = mBitmapList.size();

	printf("RResource : mBitmapList nb item %ld\n", n);
	for(ui32 i=0; i<n; i++)
	{
		item = mBitmapList.itemAt(i);
		printf("  item<%3ld> -- key %3ld -- bitmap %p\n", i, item->mKey, item->mBitmap);
	}

} // end of printToStream for RResource

//------------------------------------------------------------

#endif // of RLIB_BEOS

// eof
