/*****************************************************************************

	Projet	: Portable lib

	Fichier	: bRBitmap.cpp
	Partie	: Core bitmap

	Auteur	: RM
	Date		: 020598
	Format	: tabs==2

*****************************************************************************/

#include "gMachine.h"

#ifdef RLIB_BEOS

#include "gMacros.h"
#include "RBitmap.h"
#include "gErrors.h"

#include <assert.h>

//------------------------------------------------------------
// BPR multiple (in bytes)
// This value must be a power of 2
// the byte size of the BPR will be rounded to this value, in order to
// have lines starting at least at a pair address in 8 bpp mode.

#define K_RBITMAP_BPR_MULTIPLE	4

//------------------------------------------------------------

const RColor gBlack 			= {   0,   0,   0,    0,    0};
const RColor gWhite				= {0xFF,0xFF,0xFF,    0, 0xFF};
const RColor gOpaque			= {   0,   0,   0,    0,    0};
const RColor gTransparent	= {   0,   0,   0, 0xFF,    0};

//------------------------------------------------------------


//************************************************************
RBitmap::RBitmap(void)
//************************************************************
{
	mPixelMode = kColorUnsuported;
	mExtraFeatures = false;
	mClipEnable = false;
	mBitmap = NULL;
	mData = NULL;
	mMask = NULL;
	mSx = mSy = mSxy = 0;
	mHasMask = false;

} // end of default constructor for RBitmap


//************************************************************
RBitmap::RBitmap(ui32 sx, ui32 sy, ui32 pixelMode, rbool extraFeatures)
//************************************************************
{
	mSx = sx;
	mSy = sy;
	mSxy = 0;

	mPixelMode = pixelMode;
	mExtraFeatures = extraFeatures;
	mClipEnable = false;
	mBitmap = NULL;
	mData = NULL;
	mMask = NULL;
	mHasMask = false;

	init();

} // end of settings constructor for RBitmap


//************************************************************
RBitmap::RBitmap(const BBitmap& bmap, rbool useAlpha)
//************************************************************
{
	// currently only accept 32 bpp images
	// TBDL accept 8 bpp images plus a hint flag to make them 8 or 24 or 32 bpp.
	if (bmap.ColorSpace() == B_RGB_32_BIT)
	{
		ui32 sx = bmap.Bounds().IntegerWidth()+1;
		ui32 sy = bmap.Bounds().IntegerHeight()+1;
		
		// setup this image
		reset(sx, sy, (useAlpha ? kPixel24 : kPixel24Alpha),false);
		
		ui8  *src    	= (ui8 *)bmap.Bits();
		//ui32 rcBpr		= bmap.BytesPerRow();
		ui8  *dest   	= data();
		ui8  *msk		 	= mask();
		//ui32 destBpr	= dataBpr();
		//ui32 mskBpr   = maskBpr();
		
		for(; sy>0; sy--)
		{
			ui32 x = sx;
			ui8 *s=src;
			ui8 *d=dest;
			for(; x>0; x--) { *(s++) = *(d++); *(s++) = *(d++); *(s++) = *(d++); s++; }
			if (msk)
			{
				s=src+3;
				d=msk;
				for(x=sx; x>0; x--) { *(d++) = *s; s+=4; }
			}
		}
		
	}
	else
	{
		M_THROW(kErrUnsupported);
	}

} // end of BBitmap constructor for RBitmap


//************************************************************
RBitmap::~RBitmap(void)
//************************************************************
{
	makeEmpty();
} // end of destructor for RBitmap


//************************************************************
void RBitmap::init(void)
//************************************************************
{
	mBounds.right = mSx-1;
	mBounds.bottom = mSy-1;

	// reject invalid colormode/flavor combination plus TBD ones
	switch(mPixelMode)
	{
		// these modes are OK right now
		case kPixel256:
		case kPixel16:
		case kPixel16_1555:
		case kPixel16Alpha:
		case kPixel24:
		case kPixel24Alpha:
		case kPixel32Alpha:
			break;
		// features TBD later
		default:
			throw(kErrUnsupported);
			break;
	}

	// precompute a pixel size in byte
	switch(colorMode())
	{
		case kColorMono:			mPixelSize = 1;	break;	// packed 8 pixels per byte
		case kColorGray:
		case kColor256:				mPixelSize = 1;	break;
		case kColorRgb15:
		case kColorRgb16:			mPixelSize = 2;	break;
		case kColorRgb24:			mPixelSize = 3;	break;
		case kColorRgb32:			mPixelSize = 4;	break;
	}
	

	// make the bpr a multiple of K_RBITMAP_BPR_MULTIPLE
	mBpr = mSx * mPixelSize;
	mBpr = (mBpr+K_RBITMAP_BPR_MULTIPLE-1) & ~(K_RBITMAP_BPR_MULTIPLE-1);
	mMaskBpr = (mSx+K_RBITMAP_BPR_MULTIPLE-1) & ~(K_RBITMAP_BPR_MULTIPLE-1);

	// allocated size of the data (the mask is always 8 bit)
	mSxy = mBpr*mSy;
	mMaskSxy = mMaskBpr*mSy;

	// allocate the data
	
	mData = new ui8[mSxy];
	M_ASSERT_PTR(mData);
	memset(mData, 0, mSxy);
	
	// allocate a separate mask but only if needed

	if (flavor() == kRgbAlpha)
	{
		mHasMask = true;
		mMask = new ui8[mMaskSxy];
		M_ASSERT_PTR(mMask);
		memset(mMask, 0, mMaskSxy);
	}
	else if (flavor() == kRgb1555)
		mHasMask = true;

} // end of init for RBitmap



//************************************************************
void RBitmap::copyFrom(RBitmap *from)
//************************************************************
{
	assert(0);
} // end of copyFrom for RBitmap


//************************************************************
void RBitmap::makeEmpty(void)
//************************************************************
{
	if (mBitmap)	delete mBitmap;	mBitmap = NULL;
	if (mData)		delete mData;		mData = NULL;
	if (mMask)		delete mMask;		mMask = NULL;
} // end of makeEmpty for RBitmap



//************************************************************
void RBitmap::reset(ui32 sx, ui32 sy, ui32 pixelMode, rbool extraFeatures)
//************************************************************
{
	makeEmpty();

	mBounds.left = 0;
	mBounds.top = 0;
	mSx = sx;
	mBounds.right = sx-1;
	mSy = sy;
	mBounds.bottom = sy-1;
	mSxy = 0;

	mPixelMode = pixelMode;
	mExtraFeatures = extraFeatures;
	mClipEnable = false;

	init();

} // end of reset for RBitmap


//------------------------------------------------------------


//************************************************************
void RBitmap::erase(RColor color)
//************************************************************
{
ui8 *p8;
ui16 *p16, s16;
ui32 *p32, s32;
si32 n;

	if (!mData) return;
	
	if (mMask) memset(mMask, color.a, mMaskSxy);

	switch(mPixelSize)
	{
		case 1:
			memset(mData, color.i, mMaskSxy);
			break;

		case 2:
			p16 = (ui16 *)mData;
			if (flavor() & kRgb565)
				s16 = ((color.r >> 3) << 11) | ((color.g >> 2) << 5) | (color.b);
			else
			{
				s16 = ((color.r >> 3) << 10) | ((color.g >> 3) << 5) | (color.b);
				if (flavor() & kRgb1555) s16 |= ((color.a & 1) << 15);
			}
			for(n = mSxy/sizeof(ui16); n>0; n--) *(p16++) = s16;
			break;

		case 3:
			{
				p8 = mData;
				ui8 r,g,b;
				r = color.r;
				g = color.g;
				b = color.b;
				ui32 bpr=mBpr;
				si32 np = mSx;
				for (si32 y=mSy; y>0; y--, p8+=bpr)
				{
					ui8 *p = p8;
					for(n = np; n>0; n--) { *(p++) = r; *(p++) = g; *(p++) = b; }
				}
			}
			break;
	
		case 4:
			p32 = (ui32 *)mData;
			s32 = (color.r << 24) | (color.g << 16) | (color.b << 8) | (color.a);
			for(n = mSxy/sizeof(ui32); n>0; n--) *(p32++) = s32;
			break;

		default:
			// outch !
			return;
	}

} // end of erase for RBitmap


//************************************************************
void RBitmap::copyBits(RRect source, RRect dest)
//************************************************************
{
} // end of copyBits for RBitmap


//************************************************************
void RBitmap::exchangeBits(RRect source, RRect dest)
//************************************************************
{
} // end of exchangeBits for RBitmap


//************************************************************
void RBitmap::setPixel(si32 x1, si32 y1, RColor color)
//************************************************************
{
ui8 *p8;
ui16 *p16, s16;
ui32 *p32, s32;

	if (!mData) return;

	if (mMask)
	{
		mMask[mMaskBpr*y1 + x1] = color.a;
	}

	switch(mPixelSize)
	{
		case 1:
			mData[mBpr*y1 + x1] = color.i;
			break;

		case 2:
			p16 = (ui16 *)mData;
			if (flavor() == kRgb565)
				s16 = ((color.r >> 3) << 11) | ((color.g >> 2) << 5) | (color.b);
			else
			{
				s16 = ((color.r >> 3) << 10) | ((color.g >> 3) << 5) | (color.b);
				if (flavor() & kRgb1555) s16 |= ((color.a & 1) << 15);
			}
			p16[mBpr*y1+2*x1] = s16;
			break;

		case 3:
			p8 = mData+mBpr*y1+3*x1;
			*(p8++) = color.r;
			*(p8++) = color.g;
			*(p8++) = color.b;
			break;
	
		case 4:
			p32 = (ui32 *)mData;
			s32 = (color.r << 24) | (color.g << 16) | (color.b << 8) | (color.a);
			p32[mBpr*y1+4*x1] = s32;
			break;

		default:
			// outch !
			return;
	}

} // end of setPixel for RBitmap


//************************************************************
void RBitmap::line(si32 x1, si32 y1, si32 x2, si32 y2, RColor color)
//************************************************************
{

} // end of line for RBitmap


//************************************************************
void RBitmap::fill(si32 x1, si32 y1, si32 x2, si32 y2, RColor color)
//************************************************************
{

} // end of fill for RBitmap


//************************************************************
void RBitmap::putSprite(si32 x1, si32 y1, RBitmap &sprite)
//************************************************************
{

} // end of putSprite for RBitmap


//************************************************************
void RBitmap::getSprite(si32 x1, si32 y1, RBitmap &sprite)
//************************************************************
{

} // end of getSprite for RBitmap



//------------------------------------------------------------


//********************************************************************
void RBitmap::add(RBitmap &source, RRect srcRect, RRect destRect)
//********************************************************************
{

	//if (source.colorMode() != colorMode()) M_THROW(kErrUnsupported);
	if (source.mPixelSize != mPixelSize) M_THROW(kErrUnsupported);
	if (!mData || !source.mData) return;
	
	//if (mMask) memset(mMask, color.a, mMaskSxy);

	if(1)
	{
		ui8 *ps8 = source.mData;
		ui8 *pd8 = mData;
		ui32 sbpr = source.mBpr;
		ui32 dbpr = mBpr;
		ui32 dsx = destRect.width();
		ui32 dsy = destRect.height();
		ui32 ssx = srcRect.width();
		ui32 ssy = srcRect.height();

		if (ssx == dsx && ssy == dsy)
		{
			// pure copy
			memcpy(mData, source.mData, mSxy);
		}
		else
		{
			// if (ssx != dsx ) ... if (ssy != dsy) --> zoom factor
			ps8 += srcRect.left * mPixelSize + srcRect.top * sbpr;
			pd8 += destRect.left * mPixelSize + destRect.top * dbpr;
	
			ssx *= mPixelSize;
			for(; ssy>0; ssy--)
			{
				memcpy(pd8, ps8, ssx);
				pd8 += dbpr;
				ps8 += sbpr;
			}
		}
	}

} // end of add for RBitmap



//************************************************************
void RBitmap::fastCopy(RBitmap &source)
//************************************************************
{
	// only copy a bitmap into this bitmap if it has the same
	// size and colormode/flavor.

} // end of fastCopy for RBitmap


//------------------------------------------------------------


//************************************************************
BBitmap * RBitmap::bBitmap(void)
//************************************************************
{
rbool is8bpp = (colorMode() == kColor256);

	// return NULL if the bitmap can't be created (BBitmap can't be 16 for example)
	if (colorMode() != kColor256 && colorMode() != kColorRgb24 && colorMode() != kColorRgb32) return NULL;

	// allocate bitmap if doesn't exists
	if (!mBitmap)
	{
		mBitmap = new BBitmap(mBounds, (is8bpp ? B_CMAP8 : B_RGB32));
		if (!mBitmap) return NULL;
	}

	// update it if needed
	
	//if (mUpdate...)
	{
		ui8 *bits;
		ui32 bpr  = mBitmap->BytesPerRow();
		ui32 bpr2 = mBpr;
		si32 sx   = mSx;
		si32 sy   = mSy;
		
		if (is8bpp)
		{
			if (bpr == bpr2) memcpy(mBitmap->Bits(), mData, mSxy);
			else
			{
				ui32 offset = 0;
				bits = mData;
				for(; sy>0; sy--)
				{
					mBitmap->SetBits(bits, sx, offset, B_CMAP8);
					bits += bpr2;
					offset += bpr;
				}
			}
		}
		else if (colorMode() == kColorRgb24)
		{
			ui32 offset = 0;
			bits = mData;
			sx *= 3;
			for(; sy>0; sy--)
			{
				mBitmap->SetBits(bits, sx, offset, B_RGB32 /*B_RGB_32_BIT*/ );
				bits += bpr2;
				offset += bpr;
			}
		}
		else
		{
			bits = (ui8 *)mBitmap->Bits();
			M_THROW(kErr);
		}
	}

	return mBitmap;

} // end of bBitmap for RBitmap


//------------------------------------------------------------


//************************************************************
void RBitmap::printToStream(void)
//************************************************************
{
	printf("RBitmap <%3ldx%3ld> -- bpr %ld -- pixmod %04lx, size %ld -- mask %s %ld -- extra %ld\n",
				mSx, mSy, mBpr, mPixelMode, mPixelSize, mHasMask ? "yes" : "no", mMaskBpr, mExtraFeatures);
} // end of printToStream for RBitmap

//------------------------------------------------------------

#endif // of RLIB_BEOS

// eof
