/*****************************************************************************

	Projet	: Portable lib

	Fichier	: RRect.cpp
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
#include "RRect.h"

//------------------------------------------------------------


//************************************
void	RRect::printToStream(void) const
//************************************
{
	DPRINTF("RRect %s<left %d> <right %d> <top %d> <bottom %d>\n",
		isValid() ? "" : "invalid ",
		left, right, top, bottom);
}


//------------------------------------------------------------
// comparison


//***********************************************
rbool RRect::operator==(RRect from) const
//***********************************************
{
	return (left == from.left && top == from.top
					&& right == from.right && bottom == from.bottom);
}


//***********************************************
rbool RRect::operator!=(RRect from) const
//***********************************************
{
	return (left != from.left || top != from.top
					|| right != from.right || bottom != from.bottom);
}

//------------------------------------------------------------
// intersection and union


//***********************************************
RRect RRect::operator&(RRect from) const
//***********************************************
{
RRect r(*this);

	if (from.left   > r.left  ) r.left   = from.left;
	if (from.top    > r.top   ) r.top    = from.top;
	if (from.right  < r.right ) r.right  = from.right;
	if (from.bottom < r.bottom) r.bottom = from.bottom;

	return r;
}


//***********************************************
RRect RRect::operator|(RRect from) const
//***********************************************
{
RRect r(*this);

	if (from.left   < r.left  ) r.left   = from.left;
	if (from.top    < r.top   ) r.top    = from.top;
	if (from.right  > r.right ) r.right  = from.right;
	if (from.bottom > r.bottom) r.bottom = from.bottom;

	return r;
}

//------------------------------------------------------------
// utilities

//******************************************
rbool RRect::contains(RPoint p) const
//******************************************
{
	const si32 x = p.x, y=p.y;
	return(x>=left && x<=right && y>=top && y<=bottom);
}

//*****************************************
rbool RRect::contains(RRect r) const
//*****************************************
{
	const si32 x1=r.left, y1=r.top, x2=r.right, y2=r.bottom;
	return(x1>=left && x1<=right && y1>=top && y1<=bottom
				&& x2>=left && x2<=right && y2>=top && y2<=bottom);
}

//*******************************************
rbool RRect::intersects(RRect r) const
//*******************************************
{
	const si32 x1=r.left, y1=r.top, x2=r.right, y2=r.bottom;
	const rbool b0=(x1>=left && x1<=right);
	const rbool b1=(x2>=left && x2<=right);
	const rbool b2=(y1>=top && y1<=bottom);
	const rbool b3=(y2>=top && y2<=bottom);
	return((b0 || b1) && (b2 || b3));
}

/*-------------------------------------------------------------*/

// eoc


