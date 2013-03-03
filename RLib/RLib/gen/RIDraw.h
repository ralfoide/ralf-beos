/*****************************************************************************

	Projet	: RLib

	Fichier	: RDrawInterface.h
	Partie	: Core bitmap/window utils

	Auteur	: RM
	Date		:	230899
	Format	: tabs==2

	Basic drawing tools,
	Used by the RBitmap and the RBlitWindow.
	May not be implemented always on every platform.

*****************************************************************************/

#ifndef _H_RDRAWINTERFACE_
#define _H_RDRAWINTERFACE_

#include "RRect.h"
#include "RColor.h"

//------------------------------------------------------------

class RBitmap;

//------------------------------------------------------------

//******************
class RDrawInterface
//******************
{
public:
virtual			 ~RDrawInterface() = 0																	{ }

virtual	void erase(RColor color = gBlack)														{ }
virtual	void copyBits(RRect source, RRect dest)											{ }
virtual	void exchangeBits(RRect source, RRect dest)									{ }
virtual	void setPixel(si32 x1, si32 y1, RColor color)								{ }
virtual	void line(si32 x1, si32 y1, si32 x2, si32 y2, RColor color)	{ }
virtual	void fill(si32 x1, si32 y1, si32 x2, si32 y2, RColor color)	{ }
virtual	void putSprite(si32 x1, si32 y1, RBitmap &sprite)						{ }
virtual	void getSprite(si32 x1, si32 y1, RBitmap &sprite)						{ }

virtual	void line(RPoint p1, RPoint p2, RColor color) { line(p1.x, p1.y, p2.x, p2.y, color); }
virtual	void fill(RPoint p1, RPoint p2, RColor color) { fill(p1.x, p1.y, p2.x, p2.y, color); }
virtual	void fill(RRect r, RColor color) 							{ if (r.isValid()) fill(r.left, r.top, r.right, r.bottom, color); }
virtual	void putSprite(RPoint p, RBitmap &sprite)			{ putSprite(p.x, p.y, sprite); }
virtual	void getSprite(RPoint p, RBitmap &sprite)			{ getSprite(p.x, p.y, sprite); }


	// -- extra effects --
	//
	// [need to move to outside class]
	//
	// Ratio is a number between 0..1, bornes incluses
	// Effects generally assume that both bitmaps have same characteristics
	// Effects using two sources work following this formula : this = source1 op source2.
	// This way the effect can be applied many times without need of duplicating source1
	// every time. For some effects, 'this' and source1 could be equal but not all of them.
	//
	// There is need for "state" flags : 
	// - take or discard source alpha channel,
	// - operation mode (copy, blend) for copying into 'this'
	// - 
	
	/*
	// more utilities...
	dest = void	transition(RBitmap &source1, RBitmap &source2, fp32 ratio);
	dest = void	superpose (RBitmap &source1, RBitmap &source2, fp32 ratio);
	dest = void	zoom			(RBitmap &source, RRect &src);

	// just for the fun...
	void	starfield(ui32 sequence);
	void	tunnel(ui32 sequence);
	void	fire(ui32 sequence);
	void	plasma(ui32 sequence);
	void	degrade(RColor color1, RColor color2, rbool circular);
	*/
};

//------------------------------------------------------------

#endif // of _H_RDRAWINTERFACE_

// eof
