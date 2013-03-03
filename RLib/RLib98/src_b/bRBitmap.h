/*****************************************************************************

	Projet	: Portable lib

	Fichier	: bRBitmap.h
	Partie	: Core bitmap

	Auteur	: RM
	Date		: 020598
	Format	: tabs==2

	TBDL : 
	- implement extra effects/features
	- implement "linked bitmaps" (i.e.a fake small bitmap that in reality points to
	  a parent, bigger bitmap. the fake bitmap points to the father data, and has the
	  same bpr than the parent. Hot feature : destroying the father should return an
	  error or do nothing [use a ref counter], etc.)

*****************************************************************************/

#ifdef RLIB_BEOS

#ifndef _H_BRBITMAP_
#define _H_BRBITMAP_

#include "gErrors.h"
#include "RRect.h"
#include <Bitmap.h>

//------------------------------------------------------------
// colormode for bitmaps
// note that not every mode is intended to be supported

// first enumerate the basic color modes
#define K_COLOR_MODE_MASK		0x00FF00
enum
{
	kColorUnsuported	= 0,
	kColorMono				= 0x0100,		// 1-bit per pixel (unsupported yet)
	kColorGray				= 0x0400,		// 8-bit per pixel (unsupported yet)
	kColor256					= 0x0800,		// 8-bit indexed per pixel (needs indexed)
	kColorRgb15				= 0x1500,		// rgb 555
	kColorRgb16				= 0x1600,		// rgb 565 or 1555
	kColorRgb24				= 0x2400,		// rgb 888
	kColorRgb32				= 0x3200		// 24 bit + 8 alpha
};

// second enumerator some flavors for the above modes
// this is a 8-bitfield, not all combinations are meaningfull
#define K_FLAVOR_MASK		0x0000FF
enum
{
	// default value
	kRgbVanilla = 0,

	// color mode details
	kRgb555			= 1,
	kRgb565			= 2,
	kRgb1555		= 4,	// mixed alpha using only one bit (0=hidden, 1=visible)
	kRgbRgba		= 8,
	kRgbAbgr		= 16, //unsupported yet

	// maintain an 8-bit alpha AND layer (0=see below, FF=see ourself)
	kRgbAlpha		= 32
};

// third enumerator : the valid combination of pixel mode currently supported
enum
{
	kPixel256			= kColor256   + kRgbVanilla,						// color-map needed
	kPixel16			= kColorRgb16 + kRgbVanilla,
	kPixel16_1555	= kColorRgb16 + kRgb1555,
	kPixel16Alpha	= kColorRgb16 + kRgbAlpha,
	kPixel24			= kColorRgb24 + kRgbRgba + kRgbVanilla,
	kPixel24Alpha	= kColorRgb24 + kRgbRgba + kRgbAlpha,		// 3-byte pixel + separate alpha channel
	kPixel32Alpha = kColorRgb32 + kRgbRgba + kRgbAlpha,		// doesn't exist without alpha
	// more to come
};

//------------------------------------------------------------
// color definition

struct RColor
{
	ui8 	r;
	ui8 	g;
	ui8 	b;
	ui8		a;
	ui8		i;		// for 256-indexed, mono or gray
};

extern const RColor gBlack;
extern const RColor gWhite;
extern const RColor gOpaque;
extern const RColor gTransparent;

//------------------------------------------------------------


//***************************************
class RBitmap : virtual public RInterface
//***************************************
{
public:

	RBitmap(void);
	RBitmap(ui32 sx, ui32 sy, ui32 pixelmode, rbool extraFeatures = false);
	RBitmap(const BBitmap& bmap, rbool useAlpha = true);		// BeOS only
	virtual ~RBitmap(void);

	// copy-constructor and copy-operator
	RBitmap(const RBitmap& b) 			{ copyFrom((RBitmap *)&b); }
	RBitmap& operator=(RBitmap& b)	{ copyFrom(&b); return (*this); }

	// reset the whole bitmap to suit the new parameters
	void reset(ui32 sx, ui32 sy, ui32 pixelmode, rbool extraFeatures);

	// convert() creates a new bitmap that the new attributes
	// mail fail with an exception if the conversion is not supported
	RBitmap *convert(ui32 sx, ui32 sy, ui32 colormode, ui32 flavor, rbool resize);

	// get params
	RRect bounds       (void)				{ return mBounds; 			 }
	ui32	pixelMode    (void)				{ return mPixelMode; 		 }
	ui32	extraFeatures(void)				{ return mExtraFeatures; }
	rbool	clipping		 (void)				{ return mClipEnable; 	 }
	ui8*	data				 (void)				{ return mData;					 }
	ui8*	mask				 (void)				{ return mMask;					 }

	ui32	dataSx			 (void)				{ return mSx; 				}
	ui32	dataSy			 (void)				{ return mSy; 				}
	ui32	dataSxy			 (void)				{ return mSxy; 				}
	ui32	dataBpr			 (void)				{ return mBpr; 				}
	ui32	maskBpr			 (void)				{ return mMaskBpr; 		}
	ui32	maskSxy			 (void)				{ return mMaskSxy; 		}
	ui32	pixelSize		 (void)				{ return mPixelSize;	}


	// set params
	void setClipping(rbool enable)	{ mClipEnable = enable; }

	// -- extra features, may not be implemented always --

	void erase(RColor color = gBlack);

	void copyBits(RRect source, RRect dest);
	void exchangeBits(RRect source, RRect dest);

	void setPixel(si32 x1, si32 y1, RColor color);
	void line(si32 x1, si32 y1, si32 x2, si32 y2, RColor color);
	void fill(si32 x1, si32 y1, si32 x2, si32 y2, RColor color);
	void putSprite(si32 x1, si32 y1, RBitmap &sprite);
	void getSprite(si32 x1, si32 y1, RBitmap &sprite);

	void line(RPoint p1, RPoint p2, RColor color) { line(p1.x, p1.y, p2.x, p2.y, color); }
	void fill(RPoint p1, RPoint p2, RColor color) { fill(p1.x, p1.y, p2.x, p2.y, color); }
	void fill(RRect r, RColor color) 							{ if (r.isValid()) fill(r.left, r.top, r.right, r.bottom, color); }
	void putSprite(RPoint p, RBitmap &sprite)			{ putSprite(p.x, p.y, sprite); }
	void getSprite(RPoint p, RBitmap &sprite)			{ getSprite(p.x, p.y, sprite); }

	RBitmap *extractSprite(RRect r);

	// -- more extra stuff --

	void	fastCopy(RBitmap &source);		// copy a same-sized bitmap only
	void 	add(RBitmap &source, RRect srcRect, RRect destRect);

	// -- extra effects --
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
	
	/* // some utilities...
	void	transition(RBitmap &source1, RBitmap &source2, fp32 ratio);
	void	superpose (RBitmap &source1, RBitmap &source2, fp32 ratio);
	void	zoom			(RBitmap &source, RRect &src);
	*/

	/* // just for the fun...
	void	starfield(ui32 sequence);
	void	tunnel(ui32 sequence);
	void	fire(ui32 sequence);
	void	plasma(ui32 sequence);
	void	degrade(RColor color1, RColor color2, rbool circular);
	*/

	// -- machine-dependent converters --
	#ifdef RLIB_BEOS
		BBitmap *bBitmap(void);
	#endif

	// -- debug aware stuff --
	void printToStream(void);

protected:

	// init() is where the real bitmap is allocated (needed only for the default constructor)
	void init(void);
	// calling makeEmpty() is not necessary if you destruct the bitmap
	void makeEmpty(void);

	void copyFrom(RBitmap *from);		// make empty && re-assign a any-sized bitmap
	ui32 colorMode(void)	{ return mPixelMode & K_COLOR_MODE_MASK; }
	ui32 flavor(void)			{ return mPixelMode & K_FLAVOR_MASK; 		 }

	// members
	
	RRect	mBounds;
	rbool	mClipEnable;
	ui32	mPixelMode;
	ui32	mExtraFeatures;
	
	ui32	mSx, mSy, mSxy;
	ui32	mMaskBpr, mMaskSxy;
	ui32	mBpr;
	ui32	mPixelSize;
	
	rbool	mHasMask;

	// private data pointer and mask pointer
	// the mask may be null when it is stored with the pixels in the data (ie Rgb24+8)
	ui8	*mData;
	ui8	*mMask;
	
	// BeOS stuff

	#ifdef RLIB_BEOS
		BBitmap *mBitmap;
	#endif

};

//------------------------------------------------------------

#endif // of _H_BRBITMAP_

#endif // of RLIB_BEOS

// eof
