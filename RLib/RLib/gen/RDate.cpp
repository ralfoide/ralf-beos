/*****************************************************************************

	Projet	: Portable lib

	Fichier	: RDate.cpp
	Partie	: Types

	Auteur	: RM
	Date	: 180298
	Format	: tabs==4

*****************************************************************************/

#include "RLib.h"
#include "RDate.h"

//------------------------------------------------------------
// public services

//***************************
time_t RDate::Get(void) const
//***************************
{
	time_t t = time(NULL);
	tm ptm;
	ptm = *localtime(&t);

	sscanf(mDate+ 0, "%02d", &ptm.tm_mday);
	sscanf(mDate+ 2, "%02d", &ptm.tm_mon);
	sscanf(mDate+ 4, "%04d", &ptm.tm_year);
	sscanf(mDate+ 8, "%02d", &ptm.tm_hour);
	sscanf(mDate+10, "%02d", &ptm.tm_min);
	sscanf(mDate+12, "%02d", &ptm.tm_sec);

	//DPRINTF(("debug heure : D=%d M=%d Y=%d -- H=%d M=%d S=%d\n", 
	//	ptm.tm_mday, ptm.tm_mon, ptm.tm_year, ptm.tm_hour, ptm.tm_min, ptm.tm_sec));

	return mktime(&ptm);
}


//------------------------------------------------------------


// eof
