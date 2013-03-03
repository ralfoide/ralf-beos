/*****************************************************************************

	Projet	: Portable lib

	Fichier	: RPoint.h
	Partie	: Core 2d Point

	Auteur	: RM
	Date		: 020598
	Format	: tabs==2

	RPoint represents a single x,y coordinate.
	RPoint coordinates are expressed in si32.

	Based on BeOS Point.h, Copyright 1993-98, Be Incorporated, All Rights Reserved

*****************************************************************************/

#include "gMachine.h"

#ifndef _H_RPOINT
#define _H_RPOINT

#ifdef RLIB_BEOS
	#include <Point.h>
#endif

class RRect;

//------------------------------------------------------------

//**************************************
class RPoint : virtual public RInterface
//**************************************
{
public:
	si32 x;
	si32 y;

	RPoint(void);
	RPoint(si32 X, si32 Y);
	RPoint(const RPoint& pt);
	
	RPoint	&operator=(const RPoint &from);
	void		set(si32 X, si32 Y);

	void		printToStream(void) const;
		
	RPoint	operator+ (const RPoint&) const;
	RPoint	operator- (const RPoint&) const;
	RPoint&	operator+=(const RPoint&);
	RPoint&	operator-=(const RPoint&);

	rbool		operator!=(const RPoint&) const;
	rbool		operator==(const RPoint&) const;

	// machine-dependent converters
	#ifdef RLIB_BEOS
		RPoint(BPoint p)	{ x = (si32)p.x; y = (si32)p.y; }
		operator BPoint() const { return BPoint(x,y); }
	#endif
};


//------------------------------------------------------------
// inline definitions


//*************************
inline RPoint::RPoint(void)
//*************************
{
	x = y = 0;
}

//***********************************
inline RPoint::RPoint(si32 X, si32 Y)
//***********************************
{
	x = X;
	y = Y;
}

//*************************************
inline RPoint::RPoint(const RPoint& pt)
//*************************************
{
	x = pt.x;
	y = pt.y;
}

//**************************************************
inline RPoint &RPoint::operator=(const RPoint& from)
//**************************************************
{
	x = from.x;
	y = from.y;
	return *this;
}

//*************************************
inline void RPoint::set(si32 X, si32 Y)
//*************************************
{
	x = X;
	y = Y;
}


//*******************************************************
inline RPoint RPoint::operator+(const RPoint& from) const
//*******************************************************
{
RPoint p(*this);
	p.x += from.x;
	p.y += from.y;
	return p;
}


//*******************************************************
inline RPoint RPoint::operator-(const RPoint& from) const
//*******************************************************
{
RPoint p(*this);
	p.x -= from.x;
	p.y -= from.y;
	return p;
}


//***************************************************
inline RPoint &RPoint::operator+=(const RPoint& from)
//***************************************************
{
	x += from.x;
	y += from.y;
	return *this;
}

//***************************************************
inline RPoint &RPoint::operator-=(const RPoint& from)
//***************************************************
{
	x -= from.x;
	y -= from.y;
	return *this;
}

//*******************************************************
inline rbool RPoint::operator!=(const RPoint& from) const
//*******************************************************
{
	return (x != from.x || y != from.y);
}


//*******************************************************
inline rbool RPoint::operator==(const RPoint& from) const
//*******************************************************
{
	return (x == from.x && y == from.y);
}

//------------------------------------------------------------

#endif // _H_RPOINT

//------------------------------------------------------------

//eoh
