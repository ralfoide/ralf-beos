/************************************************************
	Author	: X
	1997	: XD
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, X <x.com>

************************************************************/

#include "SinTable.h"
#include <math.h>
#define PI M_PI

SinTable::SinTable() {
	for (int i=0;i<360;i++) {
		float a = (float)((float)i*PI)/180.0;
		SinTab[i] = sin(a);
		CosTab[i] = cos(a);
	}
}