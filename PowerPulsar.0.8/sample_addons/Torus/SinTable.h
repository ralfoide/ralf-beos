/************************************************************
	Author	: X
	1997	: XD
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, X <x.com>

************************************************************/

// sin et cos table object
// 360-value table

#ifndef _SIN_TABLE_H_
#define _SIN_TABLE_H_
#include <math.h>
#define PI M_PI

class SinTable {

			float SinTab[360];
			float CosTab[360];
	
		public:
							SinTable();
inline	float	Sin(int a) { return SinTab[a]; }
inline	float	Cos(int a) { return CosTab[a]; }
	
};

#endif