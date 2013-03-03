/************************************************************
	Author	: Laurent Pontier
	1997	: LP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Laurent Pontier

************************************************************/

// sin et cos table object
// 360-value table

#ifndef _SIN_TABLE_H_
#define _SIN_TABLE_H_

#include <math.h>

#define PI M_PI

class SinTable {

			float SinTab[360*100];
			float CosTab[360*100];
	
		public:
							SinTable();
inline	float	Sin(double a) { return SinTab[(int)((a*180.0)/PI)]; }
inline	float	Cos(double a) { return CosTab[(int)((a*180.0)/PI)]; }
};

#endif