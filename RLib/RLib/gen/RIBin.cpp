/*****************************************************************

	Projet	: RLib

	Fichier	: RIBin.h
	Partie	: file access

	Auteur	: RM
	Date	: 180298
	Date	: 090999
	Date	: 180100 - Win32
	Format	: tabs==4

	An abstract base class for handling a stream of data
	with read/write/seek capabilities.

****************************************************************/

#include "RLib.h"
#include "RIBin.h"

//-------------------------------------------------------------


//**************************************************
rbool RIBin::Open(const RPath &path, EFileMode mode)
//**************************************************
// this serves only as a common utility for those implementation
// that don't need to care about RPaths.
{
	return Open(path.Path(), mode);
}


#define M_READ(name, type, le_be, swap) \
	type RIBin::Read ## name ## le_be (void) \
	{ type w = 0; Read((char *)&w, sizeof( type )); M_SWAP_ ## le_be ## _TO_HOST ## swap (w); return w; }

#define M_READCHECK(name, type, le_be, swap) \
	rbool RIBin::ReadCheck ## name ## le_be ( type &w) \
	{ if (!Read((char *)&w, sizeof( type ))) return false; \
	  M_SWAP_ ## le_be ## _TO_HOST ## swap (w); return true; }

#define M_WRITE(name, type, le_be, swap) \
	rbool RIBin::Write ## name ## le_be (type w) \
	{ M_SWAP_HOST_TO_ ## le_be ## swap (w); return Write((char *)&w, sizeof( type )); }

#define M_READ_READCHECK_WRITE(name, type, swap) \
	M_READ(		name, type, LE, swap) \
	M_READ(		name, type, BE, swap) \
	M_READCHECK(name, type, LE, swap) \
	M_READCHECK(name, type, BE, swap) \
	M_WRITE(	name, type, LE, swap) \
	M_WRITE(	name, type, BE, swap)

M_READ_READCHECK_WRITE(16, ui16, 16)
M_READ_READCHECK_WRITE(32, ui32, 32)
M_READ_READCHECK_WRITE(Fp, fp32, 32)


//********************
ui8 RIBin::Read8(void)
//********************
{
	ui8 w = 0;
	Read((char *)&w, 1);
	return w;
}

//*****************************
rbool RIBin::ReadCheck8(ui8 &b)
//*****************************
{
	return Read((char *)&b, 1);
}

//************************
rbool RIBin::Write8(ui8 b)
//************************
{
	return (Write((char *)&b, 1) == 1);
}


//-------------------------------------------------------------


//****************************************
void RIBin::PrintToStream(const sptr name)
//****************************************
{
	printf("RIBin %s\n", (name ? name : ""));
}


/****************************************************************

	$Log: RIBin.cpp,v $
	Revision 1.1  2000/04/01 21:50:20  Ralf
	Using new RIClass convention for interfaces
	
	Revision 1.1  2000/03/13 08:55:13  Ralf
	First revision
	
****************************************************************/

// eoc
