/*****************************************************************************

	Projet	: Portable lib

	Fichier	: RPoint.cpp
	Partie	: Core 2d Point

	Auteur	: RM
	Date		: 020598
	Format	: tabs==2

	RPoint represents a single x,y coordinate.
	RPoint coordinates are expressed in si32.

	Based on BeOS Point.h, Copyright 1993-98, Be Incorporated, All Rights Reserved

*****************************************************************************/

#include "gMachine.h"
#include "RPoint.h"

//------------------------------------------------------------


//************************************
void RPoint::printToStream(void) const
//************************************
{
	DPRINTF("RPoint <x %d> <y %d>n", x,y);
}


//------------------------------------------------------------

//eoc
