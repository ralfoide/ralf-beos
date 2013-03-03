/************************************************************
	Author	: Laurent Pontier
	1997	: LP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Laurent Pontier

************************************************************/

/*****************************************************************************

	Projet	: Pulsar Fog AddOn

	Fichier	: TechnoFog_addon.cpp
	Partie	: End

	Auteur	: Lol
	Date	: 040597 -- version 0.2 (C funcs)

*****************************************************************************/

#include "PP_Be.h"
#include "Basic_addon.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" void *filterInit(uint32 index);

static const bool debug=true;

//---------------------------------------------------------

/*
BBitmap *GetBitmapFile(BEntry *entry);

BBitmap *GetBitmapFile(BEntry *entry)
{
	BPath path;
	if (!entry) return NULL;
	entry->GetPath(&path);
	return GetBitmap(path.Path());
}
*/


//***********************************************
BBitmap *CMapPlayer::GetBitmapFile(BEntry *entry)
//***********************************************
{
	BPath path;
	if (!entry) return NULL;
	entry->GetPath(&path);
	if (debug) printf("CMapPlayer::GetBitmapFile -- path '%s'\n", path.Path());
	if (entry->InitCheck() >= B_NO_ERROR && entry->Exists())
		return loadBitmap((char *)path.Path());
	if (debug) printf("CMapPlayer::GetBitmapFile -- failed to load entry\n");
	return NULL;
}

//--------------------------------------------------------------------

//********************
int main(int, char **)
//********************
// Code to help a little the fellow user double-clicking the
// add-on icon directly.
{
	new BApplication("application/x-vnd.ralf-vanilla");
	BAlert *box = new BAlert(ADDON_NAME,
							 "Thanks for using " ADDON_NAME " !\n\n"
							 "Please move the " ADDON_NAME " executable into\n"
							 "the PowerPulsar add-on directory\n"
							 "and try again !",
							 "Great !");
	box->Go();
	delete be_app;
	return 0;
}

//--------------------------------------------------------------------


//*****************************
void * filterInit(uint32 index)
//*****************************
{
	// this add-on only declares ONE filter
	if (index > 0) return NULL;

	CMapPlayer *info = new CMapPlayer;
	if (!info) return NULL;		// memory error, give up

	// returns the instance -- the caller will check that this instance
	// is derived from CFilter but is _not_ a CFilter...
	return (void *)info;
} // end of filterInit


//*************************
bool CMapPlayer::load(void)
//*************************
{
	sFilter.name = "MapPlayer";
	sFilter.author = "Laurent Pontier AKA BeLou";
	sFilter.info = "...";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 1;
	sFilter.supportedMode = kColor8Bit;
	sFilter.eraseScreen = true;
	sFilter.icon = loadBitmap("./add-ons.icons/fft.tga");

	counter=0;
	Go=FALSE;
	ReadyToGo=FALSE;
	StopLoad=FALSE;
	LoadStoped=FALSE;
	thread_id thead_load=spawn_thread(LoadImages,"LoadImages",B_LOW_PRIORITY,this);
	resume_thread(thead_load);

	return true;
}  // end of load for CMapPlayer


//****************************
bool CMapPlayer::prepare(void)
//****************************
{
	counter=0;
	Go=FALSE;

	return TRUE;
} // end of prepare for CMapPlayer


//*************************************
long CMapPlayer::LoadImages(void *data)
//*************************************
{
	CMapPlayer *obj=(CMapPlayer*)data;
	BPath path(obj->sLoad.dirAddon.Path(), "image_map");
	BDirectory map_dir(path.Path());
	BBitmap *raw = NULL;
	BBitmap *fin = NULL;

	if (debug) printf("CMapPlayer::LoadImages\n");

	// **

	for(int32 i=1; i<=6 && !obj->StopLoad; i++)
	{
		BEntry e;
		status_t err;
		char s[12];
		sprintf(s, "image%ld.jpg", i);
		if (debug) printf("CMapPlayer::LoadImages -> Loading %s...\n", s);
		err = map_dir.FindEntry(s,&e);
		if (err != B_NO_ERROR)
		{
			if (debug) printf("CMapPlayer::LoadImages -> Error find entry: 0x%08lx - %s\n", err, strerror(err));
			goto bail_out;
		}

		raw = obj->GetBitmapFile(&e);
		if (!raw)
		{
			if (debug) printf("CMapPlayer::LoadImages -> Error Get Bimap File\n");
			goto bail_out;
		}

		fin = obj->makeBitmap8Bit(raw);
		if (!fin)
		{
			if (debug) printf("CMapPlayer::LoadImages -> Error makeBitmap8Bit\n");
			goto bail_out;
		}

		delete raw;						raw = NULL;
		obj->BitMapList.AddItem(fin);	fin = NULL;
	}

bail_ok:

	if (debug) printf("CMapPlayer::LoadImages -> OK\n");
	if (!obj->StopLoad) obj->ReadyToGo=TRUE;

bail_out:

	delete raw;
	delete fin;
	obj->LoadStoped=TRUE;
	return B_NO_ERROR;
}


//***************************
void CMapPlayer::unload(void)
//***************************
{
	StopLoad=TRUE;
	while(LoadStoped==FALSE) snooze(100000);
	// free memory...
	int32 max=BitMapList.CountItems();
	for(int32 n=0;n<max;n++) delete (BBitmap*)(BitMapList.ItemAt(n));
	BitMapList.MakeEmpty();
}


//***********************************************
void CMapPlayer::processFrame8(SFrameInfo &frame)
//***********************************************
{
	float *chanL;
	float *chanR;
	int i,j;
	long nframe;
	int y1;
	int h = sPrepare.sy;
	int w = sPrepare.sx;
	int w2 = w/2;
	int h2 = h/2;

	chanL	= frame.fftHigh;
	chanR	= frame.fftBass;
	nframe	= frame.frame;

	// on efface l'ecran si rien a afficher.
	if (!ReadyToGo)
	{
		blitRect8(0,0,w,h,0);
		return;
	}

	// pointeur video.
	uchar *dest = sFrame->screen;

	if (chanR[10]*(1/50000.0) >35.0) Go=TRUE;

	BBitmap *b=(BBitmap*)BitMapList.ItemAt(counter);
	if (b!=NULL) memcpy(dest,b->Bits(),640*480);
	if (counter==5)
	{
		counter=0;
		Go=FALSE;
	}
	else if (Go) counter++;

}  // end of processFrame8 for CMapPlayer



#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

// eoc

