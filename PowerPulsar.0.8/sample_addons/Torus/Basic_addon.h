/************************************************************
	Author	: X
	1997	: XD
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, X <x.com>

************************************************************/

/*****************************************************************************

	Projet	: Pulsar Sample FFT AddOn

	Fichier	: sample_addon.cpp
	Partie	: End

	Auteur	: RM
	Date	: 040597 -- version 0.2 (C funcs)
	Date	: 010797 -- version 0.3 (CFilter)
	Format	: tabs==4

*****************************************************************************/

#include "PP_Be.h"
#include "CFilter.h"
#include <math.h>
#include "Point3D.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" void * filterInit(uint32 index);

#define ADDON_NAME "Torus"

static const bool debug=false;

//--------------------------------------------------------------------

class SinTable;

//*************************************
class CSampleFftFilter : public CFilter
//*************************************
{
public:

	CSampleFftFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CSampleFftFilter(void) 	{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void);
	virtual void processFrame8(SFrameInfo &frame);
	virtual void processFrame32(SFrameInfo &frame);

private:

	void	pre_update(float *chanR);
	void	post_update(void);

	Point3D *donuts;
	int nb_point;
	int nb_lat;
	float *tetaMCos, *tetaMSin;
	float *tetamCos, *tetamSin;
	float Minor, Major;
	int16 rX,rY,rZ; // indice rotation
	int16 R;
	
	SinTable *sintab;

	float *FFT;

	// indice des anneaux rouges ...
	int y1,y2,y3;
	int Y1,Y2,Y3;
	
	int c1,c2,c3;
}; // end of class defs for CSampleFftFilter


