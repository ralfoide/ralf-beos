/*****************************************************************************

	Projet	: Portable lib

	Fichier	: RRect.h
	Partie	: Core Rect

	Auteur	: RM
	Date		: 020598
	Format	: tabs==2

	RRect represents a rectangular area.
	RRect coordinates are expressed in si32.
	RRect coordinates are inclusive : a fullscreen 640x480 is a rectangle
	ranging from 0..639 to 0..479 (left=top=0 and right=639, bottom=479).
	It's width() is 640 (639-0+1) and it's height() is 480 (479-0+1).
	The methods width() and height() correct this +1 offset, no need to do it in the
	code like with the BRects. A rectangle ranging from x:0..0 to y:0..0 is a 1-pixel
	width and height rectangle.

	Based on BeOS Rect.h, Copyright 1993-98, Be Incorporated, All Rights Reserved

*****************************************************************************/

#include "gMachine.h"

#ifndef _H_RRECT
#define _H_RRECT

#include "RPoint.h"

//------------------------------------------------------------

/// A 2D rectangle

//*********
class RRect
//*********
{
public:
///
	si32	left;
///
	si32	top;
///
	si32	right;
///
	si32	bottom;

///
	RRect(void);
///
	RRect(const RRect &);
///
	RRect(si32 l, si32 t, si32 r, si32 b);
///
	RRect(RPoint _leftTop, RPoint _rightBottom);

///
	RRect	&operator=(const RRect &from);
///
	void	set(si32 l, si32 t, si32 r, si32 b);

///
	void	printToStream(void) const;

	// RPoint selectors
///
	RPoint	leftTop    (void) const;
///
	RPoint	rightBottom(void) const;
///
	RPoint	leftBottom (void) const;
///
	RPoint	rightTop   (void) const;

	// transformation
///
	void	insetBy(si32 dx, si32 dy);
///
	void	offsetBy(si32 dx, si32 dy);
///
	void	offsetTo(si32 x, si32 y);

///
	void	insetBy(RPoint p)		{ insetBy (p.x, p.y); }
///
	void	offsetBy(RPoint p)	{ offsetBy(p.x, p.y); }
///
	void	offsetTo(RPoint p)	{ offsetTo(p.x, p.y); }

	// comparison
///
	rbool	operator==(RRect) const;
///
	rbool	operator!=(RRect) const;

	// intersection and union
///
	RRect	operator&(RRect) const;
///
	RRect	operator|(RRect) const;

	// utilities
///
	rbool		isValid(void) const;
///
	si32		width(void) const;
///
///
	si32		height(void) const;
///
	rbool		contains(RPoint) const;
///
	rbool		contains(RRect) const;
///
	rbool		intersects(RRect r) const;

	// machine-dependent converters
	#ifdef RLIB_BEOS
		RRect(BRect r) { left=r.left; right=r.right; top=r.top; bottom=r.bottom; }
		operator BRect() const { return BRect(left, top, right, bottom); }
	#elif defined(RLIB_WIN32)
		RRect(CRect r) { left=r.left; right=r.right; top=r.top; bottom=r.bottom; }
		operator CRect() const { return CRect(left, top, right, bottom); }
	#endif
};

//------------------------------------------------------------
// inline definitions 

//**********************************
inline RPoint RRect::leftTop() const
//**********************************
{
	return(*((RPoint*)&left));
}

//**************************************
inline RPoint RRect::rightBottom() const
//**************************************
{
	return(*((RPoint*)&right));
}

//*************************************
inline RPoint RRect::leftBottom() const
//*************************************
{
	return(RPoint(left, bottom));
}

//***********************************
inline RPoint RRect::rightTop() const
//***********************************
{
	return(RPoint(right, top));
}

//***********************
inline RRect::RRect(void)
//***********************
{
	top = left = 0;
	bottom = right = -1;
}

//*************************************************
inline RRect::RRect(si32 l, si32 t, si32 r, si32 b)
//*************************************************
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

//*********************************
inline RRect::RRect(const RRect &r)
//*********************************
{
	left = r.left;
	top = r.top;
	right = r.right;
	bottom = r.bottom;
}

//*******************************************************
inline RRect::RRect(RPoint _leftTop, RPoint _rightBottom)
//*******************************************************
{
	left = _leftTop.x;
	top = _leftTop.y;
	right = _rightBottom.x;
	bottom = _rightBottom.y;
}

//***********************************************
inline RRect &RRect::operator=(const RRect& from)
//***********************************************
{
	left = from.left;
	top = from.top;
	right = from.right;
	bottom = from.bottom;
	return *this;
}

//****************************************************
inline void RRect::set(si32 l, si32 t, si32 r, si32 b)
//****************************************************
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

//*********************************
inline rbool RRect::isValid() const
//*********************************
{
	if (left <= right && top <= bottom)
		return true;
	else
		return false;
}

//******************************
inline si32 RRect::width() const
//******************************
{
	return(right - left + 1);
}

//*******************************
inline si32 RRect::height() const
//*******************************
{
	return(bottom - top + 1);
}

//------------------------------------------------------------
// transformation

//******************************************
inline void RRect::insetBy(si32 dx, si32 dy)
//******************************************
{
	left += dx;
	top += dy;
	right -= dx;
	bottom -= dy;
}

//*******************************************
inline void RRect::offsetBy(si32 dx, si32 dy)
//*******************************************
{
	left += dx;
	top += dy;
	right += dx;
	bottom += dy;
}

//*****************************************
inline void RRect::offsetTo(si32 x, si32 y)
//*****************************************
{
	right += x-left;
	left = x;
	bottom += y-top;
	top = y;
}


/*-------------------------------------------------------------*/

#endif // _H_RRECT

// eoh

