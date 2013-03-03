/*****************************************************************************

	Projet	: Pulsar Sample FFT AddOn

	Fichier	: sample_addon.cpp
	Partie	: End

	Auteur	: LP
	Date	: 040597 -- version 0.2 (C funcs)
	Date	: 010797 -- version 0.3 (CFilter)
	Format	: tabs==2

*****************************************************************************/

/************************************************************
	Author	: Laurent Pontier
	1997	: LP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Laurent Pontier

************************************************************/


#include "PP_Be.h"
#include "CFilter.h"
#include <math.h>

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" void * filterInit(uint32 index);

#define ADDON_NAME "BeatGrid3D"

#include "SinTable.h"
#include "Point3D.h"

static const bool debug=false;

#define DOTNUMBER	16
//--------------------------------------------------------------------

//*************************************
class CSampleFftFilter : public CFilter
//*************************************
{
public:

	CSampleFftFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CSampleFftFilter(void) 	{ /* nothing */ }

	virtual bool load(void);
	virtual void unload(void);
	virtual bool prepare(void);
	virtual void processFrame8(SFrameInfo &frame);
	virtual void processFrame32(SFrameInfo &frame);

private:

	// process perso de trace de zone.
	inline void Line8(long x1,long y1,long x2,long y2,int color)
	{	
			for(int n=0;n<Epaisseur;n++) blitLine8(x1,y1+n,x2,y2+n,color);
	}

	inline void Line32(long x1,long y1,long x2,long y2,ulong color)
	{	
			for(int n=0;n<Epaisseur;n++) blitLine32(x1,y1+n,x2,y2+n,color);
	}

	void DrawTriangleFlat(int, int, int, int, int, int, int);

	void swap(int &a,int &b)
	{
		int tmp;
		tmp=a;
		a=b;
		b=tmp;
	}
	// fin process perso.

	bool mUseGk8;
	bool mUseGk32;
	
	// configurqtion.
	int Epaisseur;
	bool Croix;
	int TailleCroix;
	int NombreImageRotation;
	float AmplitudeVagues;
	float AmplitudeOscillation;
	float VitesseOscillation;
	
	// workshop
	Point3D *p[DOTNUMBER][DOTNUMBER];
	Point3D *res[DOTNUMBER][DOTNUMBER];
	long counter;
}; // end of class defs for CSampleFftFilter


