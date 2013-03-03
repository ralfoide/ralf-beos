/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CFilter.cpp
	Partie	: Add-ons for filters

	Auteur	: RM
	Format	: tabs==4

*****************************************************************************/

#include "PP_Be.h"

//#include "globals.h"

#include <OS.h>
#include <image.h>
#include <GameKit.h>
#include <Path.h>

#if K_USE_DATATYPE
#include <DataIO.h>
#include <SupportDefs.h>
#include <DataFormats.h>
#include "Datatypes.h"
#include "GetBitmap.h"
#include "BitmapStream.h"
#endif

#define _PP_CF_INC
#include "CFilter.h"

#define debug 0

//---------------------------------------------------------------------------
// Export all function defined here in _APP_ so that it can be linked against
// by add-ons

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif
	
#if K_USE_DATATYPE
	#include "GetBitmap.cpp"
	#include "BitmapStream.cpp"
#endif

//---------------------------------------------------------------------------

// extern _IMPORT BTranslatorRoster *gTranslationRoster;

//---------------------------------------------------------------------------

#define M_SWAP(type, x,y) { type w=x; x=y; y=w; }



//****************************************************************************************
_CFILTER_IMPEXP void CFilter::blitRect8(long x1, long y1, long x2, long y2, uchar couleur)
//****************************************************************************************
/*
	If you think this is not optimal, please rewrite it first, then send me the
	source so I include it and then go and critize as much as you like. :)

	Note : width and height are aligned on external boundaries, i.e. x from 0 to 1
	means two pixels (from column 0 to and including column 1).
*/
{
	if (rect_8 && sPrepare.isGameKit)
	{
		int32 a=sFrame->startingLine;
		rect_8(x1,a+y1,x2,a+y2,couleur);
	}
	else
	{
		uchar *dest = sFrame->screen;
		uint32 bpr = sPrepare.bpr;
		int32 msx  = sPrepare.sx - 1;
		int32 msy  = sPrepare.sy - 1;
		
		// can I trust the user ? no -> swap & clip !
		if (y2 < y1) M_SWAP(long, y1, y2);
		if (x2 < x1) M_SWAP(long, x1, x2);

		// Zzzz...
		if (x1<0) x1=0;
		if (x2<0) x2=0;
		if (y1<0) y1=0;
		if (y2<0) y2=0;
		if (x1>msx) x1=msx;
		if (x2>msx) x2=msx;
		if (y1>msy) y1=msy;
		if (y2>msy) y2=msy;

		// let's optimize this (boring?) stuff later, the deadline is coming soon.
		// here's the brute-force method
		dest+=y1*bpr+x1;
		long dx = x2-x1+1;
		y2 -= y1;
		for(; y2>=0; y2--, dest+=bpr) memset(dest, couleur, dx);
	} // end of else if direct rect_8...
} // end of blitRect8 for CFilter


//*****************************************************************************************
_CFILTER_IMPEXP void CFilter::blitRect32(long x1, long y1, long x2, long y2, ulong couleur)
//*****************************************************************************************
/*
	See comments in blitRect8
*/
{
	if (rect_32 && sPrepare.isGameKit)
	{
		int32 a=sFrame->startingLine;
		rect_32(x1,a+y1,x2,a+y2,couleur);
	}
	else
	{
		ulong *dest = (ulong *)sFrame->screen;
		uint32 bpr = sPrepare.bpr / sizeof(ulong);
		int32 msx  = sPrepare.sx - 1;
		int32 msy  = sPrepare.sy - 1;
		
		// can I trust the user ? no -> swap & clip !
		if (y2 < y1) M_SWAP(long, y1, y2);
		if (x2 < x1) M_SWAP(long, x1, x2);

		// Zzzz...
		if (x1<0) x1=0;
		if (x2<0) x2=0;
		if (y1<0) y1=0;
		if (y2<0) y2=0;
		if (x1>msx) x1=msx;
		if (x2>msx) x2=msx;
		if (y1>msy) y1=msy;
		if (y2>msy) y2=msy;

		// here's the brute-force method
		dest += y1*bpr + x1;
		long dx = x2-x1;
		y2 -= y1;		// dy
		for(; y2>=0; y2--, dest += bpr)
			for(long x=0; x<=dx; x++)
				dest[x] = couleur;
	} // end of else if direct rect_32...
} // end of blitRect32 for CFilter


//---------------------------------------------------------------------------
#pragma mark -


//****************************************************************************************
_CFILTER_IMPEXP void CFilter::blitLine8(long x1, long y1, long x2, long y2, uchar couleur)
//****************************************************************************************
// draws a line in the current buffer using bresenham simple pass algorithm.
// (if you have the double algorithm at hand, I take it, I'm just _that_ lazy
// to write it down...)
// do not call this if screen, sx, sy & bpr are NOT correct.
// clips the line agains the screen size.
{
	if (line_8 && sPrepare.isGameKit)
	{
		int32 a=sFrame->startingLine;
		line_8(x1,x2,a+y1,a+y2,couleur,true,0,a,639,a+479);
	}
	else
	{
	
	register uchar *dest = sFrame->screen;
	register uchar *p;
	register long bpr = sPrepare.bpr;
	register long msx = sPrepare.sx;
	register long msy = sPrepare.sy;
	
	//-----------------------------------------
	// this code is extracted from ST Magazine and was originally asm 68k.
	register int dx, dy, t1, t2, t3, x, y, sy;
	sy = 1;

	if (y1 == y2)
	{
		//--- horizontal line ---

		if (x1 > x2) M_SWAP(long, x1, x2);
		
		// clipping
		if (x2 >= 0 && x1 < msx && y1 >= 0 && y1 < msy) 
		{
			if (x1 < 0) x1 = 0;
			if (x2 >= msx) x2 = msx-1;

			for (x = x1, p=dest+x+bpr*y1; x <= x2; x++) *(p++) = couleur;
		}
	}
	else if ( (dx = x2 - x1) == 0)
	{
		//--- vertical line ---

		if (y1 > y2) M_SWAP(long, y1, y2);
		
		// clipping
		if (y2 >= 0 && y1 < msy && x1 >= 0 && x1 < msx)
		{
			if (y1 < 0) y1 = 0;
			if (y2 >= msy) y2 = msy-1;
			
			for (y = y1, p=dest+x1+bpr*y; y <= y2; y++, p+=bpr) *p = couleur;
		}
	}
	else
	{
		if (dx >= 0)
		{
			x = x1;
			y = y1;
			dy = y2 - y1;
		}
		else
		{
			x = x2;
			y = y2;
			dx = -dx;
			dy = y1 - y2;
		}

		if (dy <= 0)
		{
			dy = -dy;
			sy = -1;
		}

		if (dx >= dy)
		{
			t3 = dy << 1;
			t1 = t3 - dx;
			t2 = t1 - dx;

			p = dest+x+bpr*y;
			for (; dx >= 0; x++, dx--, p++)
			{
				if (y >= 0 && y < msy && x >= 0 && x < msx) *p = couleur;

				if (t1 < 0)
					t1 += t3;
				else
				{
					t1 += t2;
					y += sy;
					p += bpr*sy;
				}
			}
		}
		else
		{
			t3 = dx << 1;
			t1 = t3 - dy;
			t2 = t1 - dy;

			p = dest+x+bpr*y;
			for (; dy >= 0; y+=sy, dy--, p+=bpr*sy)
			{
				if (y >= 0 && y < msy && x >= 0 && x < msx) *p = couleur;

				if (t1 < 0)
					t1 += t3;
				else
				{
					t1 += t2;
					x++;
					p++;
				}
			}
		}
	}

	} // end of if direct line_8
	
} // end of blitLine8 for CFilter



//*****************************************************************************************
_CFILTER_IMPEXP void CFilter::blitLine32(long x1, long y1, long x2, long y2, ulong couleur)
//*****************************************************************************************
// draws a line in the current buffer using bresenham simple pass algorithm.
// (if you have the double algorithm at hand, I take it, I'm just _that_ lazy
// to write it down...)
// do not call this if screen, sx, sy & bpr are NOT correct.
// clips the line agains the screen size.
{
	if (line_32 && sPrepare.isGameKit)
	{
		int32 a=sFrame->startingLine;
		line_32(x1,x2,a+y1,a+y2,couleur,true,0,a,639,a+479);
	}
	else
	{

	register ulong *dest = (ulong *)sFrame->screen;
	register ulong *p;
	register long bpr = sPrepare.bpr/sizeof(ulong);
	register long msx = sPrepare.sx;
	register long msy = sPrepare.sy;

	//-----------------------------------------
	// this code is extracted from ST Magazine and was originally asm 68k.
	register int dx, dy, t1, t2, t3, x, y, sy;
	sy = 1;

	if (y1 == y2)
	{
		//--- horizontal line ---

		if (x1 > x2) M_SWAP(long, x1, x2);
		
		// clipping
		if (x2 >= 0 && x1 < msx && y1 >= 0 && y1 < msy) 
		{
			if (x1 < 0) x1 = 0;
			if (x2 >= msx) x2 = msx-1;

			for (x = x1, p=dest+x+bpr*y1; x <= x2; x++) *(p++) = couleur;
		}
	}
	else if ( (dx = x2 - x1) == 0)
	{
		//--- vertical line ---

		if (y1 > y2) M_SWAP(long, y1, y2);
		
		// clipping
		if (y2 >= 0 && y1 < msy && x1 >= 0 && x1 < msx)
		{
			if (y1 < 0) y1 = 0;
			if (y2 >= msy) y2 = msy-1;
			
			for (y = y1, p=dest+x1+bpr*y; y <= y2; y++, p+=bpr) *p = couleur;
		}
	}
	else
	{
		if (dx >= 0)
		{
			x = x1;
			y = y1;
			dy = y2 - y1;
		}
		else
		{
			x = x2;
			y = y2;
			dx = -dx;
			dy = y1 - y2;
		}

		if (dy <= 0)
		{
			dy = -dy;
			sy = -1;
		}

		if (dx >= dy)
		{
			t3 = dy << 1;
			t1 = t3 - dx;
			t2 = t1 - dx;

			p = dest+x+bpr*y;
			for (; dx >= 0; x++, dx--, p++)
			{
				if (y >= 0 && y < msy && x >= 0 && x < msx) *p = couleur;

				if (t1 < 0)
					t1 += t3;
				else
				{
					t1 += t2;
					y += sy;
					p += bpr*sy;
				}
			}
		}
		else
		{
			t3 = dx << 1;
			t1 = t3 - dy;
			t2 = t1 - dy;

			p = dest+x+bpr*y;
			for (; dy >= 0; y+=sy, dy--, p+=bpr*sy)
			{
				if (y >= 0 && y < msy && x >= 0 && x < msx) *p = couleur;

				if (t1 < 0)
					t1 += t3;
				else
				{
					t1 += t2;
					x++;
					p++;
				}
			}
		}
	}

	} // end of if direct line_32
	
} // end of blitLine32 for CFilter


//---------------------------------------------------------------------------
#pragma mark -


//****************************************************************
_CFILTER_IMPEXP bool CFilter::findDir(BDirectory *dir, char *name)
//****************************************************************
// find a directory by name under the Application directory
// returns false if directory can't be found
{
	if (!dir) return false;
	if (!name || strlen(name) < 1) return false;
	BDirectory app(sLoad.dirApplication.Path());
	return (dir->SetTo(&app, name) >= B_NO_ERROR);
} // end of findDir for CFilter


//*******************************************************
_CFILTER_IMPEXP BBitmap * CFilter::loadBitmap(char *name)
//*******************************************************
// use datatypes to open the 'name' image in the img directory
// returns NULL if error.
{
	// first try a local file
	BFile file(name, B_READ_ONLY);
	if (file.InitCheck() < B_NO_ERROR)
	{
		// if (debug) printf("CFilter::loadBitmap -> no local %s\n", name);
		// try a name relative to the img directory
		BPath path(sLoad.dirImg.Path(),name);
		if (file.SetTo(path.Path(), B_READ_ONLY) < B_NO_ERROR)
		{
			// if (debug) printf("CFilter::loadBitmap -> no path %s\n", path.Path());
			return NULL;
		}
	}

	// if (debug) printf("CFilter::loadBitmap -> trying to load %s\n", name);

#if K_USE_DATATYPE

	BitmapStream output;
	uint32 ext=0;
	uint32 len=strlen(path.Path());
	if(len > 4) strncpy((char *)&ext,&path.Path()[len-4],4);
	if (   (ext=='.png' && !DATATranslate(file, NULL, NULL, output, DATA_BITMAP, 'PNGg'))
		  || (ext=='.tga' && !DATATranslate(file, NULL, NULL, output, DATA_BITMAP, 'TGA '))
		  || (ext=='.jpg' && !DATATranslate(file, NULL, NULL, output, DATA_BITMAP, 'JPEG')))
	{
		BBitmap *ret = NULL;
		if (output.DetachBitmap(ret))
			return NULL;
		return ret;
	}
	else return GetBitmap(path.Path());

#else

	BTranslatorRoster *roster = NULL ; //gTranslationRoster; -- HACK HERE HOOK RM 21031999
	if (!roster) roster = BTranslatorRoster::Default(); 
	BBitmapStream stream; 
	BBitmap *result = NULL; 
	status_t err = roster->Translate(&file, NULL, NULL, &stream, B_TRANSLATOR_BITMAP);
	if (err < B_OK) 
	{
		// if (debug) printf("CFilter::loadBitmap - translate err 0x%08lx - %s\n", err, strerror(err));
		return NULL; 
	}
	stream.DetachBitmap(&result); 
	return result; 

#endif

} // end of loadBitmap for CFilter


//*************************************************************************
_CFILTER_IMPEXP BBitmap *CFilter::makeIconFromHeader(int width, int height,
													color_space cspace,
													int bytesperpixel,
													unsigned char *bits)
//*************************************************************************
// cf Be News Letter 77/78, William Adams DevEnhancer@ be.com
{
	BBitmap *bitmap = new BBitmap(BRect(0,0,width-1, height-1), cspace);

	if (!bitmap) return NULL;
	int bpr = bytesperpixel*width;
	int destbpr = bitmap->BytesPerRow();
	int offset=0, destof7=0;
	int j;
	for(j=0; j<height; j++, offset+=bpr, destof7+=destbpr)
		bitmap->SetBits(bits+destof7, bpr, offset, cspace);
	return bitmap;
} // end of blitLine for CFilter


//***************************************************************
_CFILTER_IMPEXP BBitmap *CFilter::makeBitmap8Bit(BBitmap *source)
//***************************************************************
{
	BRect r;
	BBitmap *dest;
	long sx,sy;

	if (!source) return NULL;

	r = source->Bounds();
	dest = new BBitmap(r, B_COLOR_8_BIT);
	if (!dest) return NULL;

	sx = (long)r.Width()+1;
	sy = (long)r.Height()+1;

	uchar *sourceBits = (uchar *)source->Bits();
	long sourceBpr = source->BytesPerRow();
	color_space sourceCs = source->ColorSpace();

	if (sourceCs == B_COLOR_8_BIT)
	{
		dest->SetBits(sourceBits, sourceBpr, 0, sourceCs);
		return dest;
	}

	// transform B_RGB_32_BIT into B_COLOR_8_BIT
	// unfortunately, Be stores 32 bits BGRA in the bitmap bits while
	// the SetBits() expects 24 bits RGB when it receives the 32 bpp constant (!!!).
	// HOOK : this is unfortunately *very* system specific.
	// Note : I use SetBits() rather than using a Bitmap with a BView since
	// only SetBits() handles the dithering automagically.

	uchar *rvb= new uchar[sx*sy*3];
	if (!rvb)
	{
		delete dest;
		return NULL;
	}

	long i,j;
	uchar *p=rvb;
	for(j=0; j <sy; j++, sourceBits+=sourceBpr)
	{
		uchar *s=sourceBits;
		for(i=0; i<sx; i++, s+=4)
		{
			// source is BBitmap 32, i.e. BGRA (gasp !) ----> :-(
			*(p++) = s[2];
			*(p++) = s[1];
			*(p++) = s[0];	
		}
	}

	dest->SetBits(rvb, sx*sy*3, 0, B_RGB_32_BIT);
	delete rvb;
	return dest;
} // end of makeBitmap8Bit for CFilter

//---------------------------------------------------------------------------
// suppress the pragma export on effect (obviously uneeded here since this is
// the end of the file)

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

//---------------------------------------------------------------------------

// eocpp

