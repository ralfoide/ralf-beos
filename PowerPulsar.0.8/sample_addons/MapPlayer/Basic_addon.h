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

	Projet	: Pulsar Sample FFT AddOn

	Fichier	: sample_addon.cpp
	Partie	: End

	Auteur	: RM
	Date	: 040597 -- version 0.2 (C funcs)
	Date	: 010797 -- version 0.3 (CFilter)
	Format	: tabs==2

*****************************************************************************/

#include "PP_Be.h"
#include "CFilter.h"
#include <math.h>
#include "Point3D.h"
#include <TranslationKit.h>

#define ADDON_NAME "MapPlayer"

//--------------------------------------------------------------------

//*******************************
class CMapPlayer : public CFilter
//*******************************
{
public:

	CMapPlayer(void) : CFilter()	{ /* nothing */ }
	virtual ~CMapPlayer(void) 		{ /* nothing */ }

	virtual bool load(void);
	virtual void unload(void);
	virtual bool prepare(void);
	virtual void processFrame8(SFrameInfo &frame);

	BBitmap *GetBitmapFile(BEntry *entry);
private:

	void DrawTriangleFlat(int, int, int, int, int, int, int);
	inline void swap(int &a,int &b)
	{
		int tmp;
		tmp=a;
		a=b;
		b=tmp;
	}
	// fin process perso.

	// workshop
	long counter;
	bool Go;
	BList BitMapList;
	static long LoadImages(void*);
	bool StopLoad;
	bool LoadStoped;
	thread_id thead_load;
	bool ReadyToGo;
}; // end of class defs for CMapPlayer


