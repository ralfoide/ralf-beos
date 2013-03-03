/************************************************************
	Author	: Laurent Pontier
	1997	: LP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Laurent Pontier

************************************************************/

#include "SinTable.h"

SinTable::SinTable() {
	for (int i=0;i<360*100;i++) {
		float a = (float)(((float)i/100.0)*PI)/180.0;
		SinTab[i] = sin(a);
		CosTab[i] = cos(a);
	}
}