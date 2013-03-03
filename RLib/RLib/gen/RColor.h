/*****************************************************************************

	Projet	: RLib

	Fichier	: RColor.h
	Partie	: Core bitmap

	Auteur	: RM
	Date	: 230899
	Format	: tabs==4

	IMPORTANT ALPHA RULE : AND layer (0=see below, FF=see ourself)
	For one color : 0 means the color is transparent (see below),
	and 0xFF means the color is opaque (see itself).

*****************************************************************************/

#ifndef _H_RCOLOR_
#define _H_RCOLOR_

#include "RErrors.h"
#include "RMacros.h"

#ifdef RLIB_MACOS
	#include "Quickdraw.h"
#endif

//------------------------------------------------------------
// Native screen RGBA organisation (32 bits only)
// ARGB : A:31-24, R:23-16, G:15-8, B:7-0
// BGRA : B:31-24, G:23-16, R:15-8, A:7-0

#if defined(__INTEL__) || defined(RLIB_MACOS)
	#define K_RCOLOR_32_ARGB
	#undef  K_RCOLOR_32_BGRA
	#define K_RCOLOR_32_A_MASK 		0xFF000000
	#define K_RCOLOR_32_A					24
	#define K_RCOLOR_32_R					16
	#define K_RCOLOR_32_G					 8
	#define K_RCOLOR_32_B					 0
#else			// for BeOS PPC
	#undef  K_RCOLOR_32_ARGB
	#define K_RCOLOR_32_BGRA
	#define K_RCOLOR_32_A_MASK 		0x000000FF
	#define K_RCOLOR_32_A					 0
	#define K_RCOLOR_32_R					 8
	#define K_RCOLOR_32_G					16
	#define K_RCOLOR_32_B					24
#endif

//------------------------------------------------------------

/// A color definition, using the RGBA or indexed color model

//***********
struct RColor
//***********
{
	RColor(ui8 _r=0, ui8 _g=0, ui8 _b=0, ui8 _a=0xFF, ui8 _i=0) {r=_r; g=_g; b=_b; a=_a; i=_i; }

	ui8 r;
	ui8 g;
	ui8 b;
	ui8	a;		// 0xFF is opaque, 0 is transparent
	ui8	i;		// for 256-indexed, mono or gray

	// ----

	inline ui32 GetColor32(void);
	inline void SetColor32(ui32 c, ui8 _i=0);

	#if defined(RLIB_MACOS)

					RColor(const RGBColor &color, const ui8 _a = 0xFF);
		RColor&		operator=(const RGBColor &color);
		operator	RGBColor() const;
		void		QdRgbColor(RGBColor &color) const;

	#elif defined(RLIB_WIN32)

					RColor(COLORREF color, const ui8 _a = 0xFF);
					operator COLORREF(void);

		void		SetColorRef(COLORREF color);
		COLORREF	GetColorRef(void);

	#endif

};

extern const RColor gBlack;
extern const RColor gRed;
extern const RColor gYellow;
extern const RColor gGreen;
extern const RColor gCyan;
extern const RColor gBlue;
extern const RColor gMagenta;
extern const RColor gWhite;
extern const RColor gOpaque;
extern const RColor gTransparent;

//------------------------------------------------------------

// HOOK TBDL : move this to a separate file once implementation is fine
// HOOK : RRefCount methods should be attach() & detach(), not acquire/release.


//*************
class RColorMap
//*************
{
public:
	RColorMap(ui32 nbColor = 256) : mNbColor(nbColor), mColor(NULL)
		{
			mColor = new RColor[mNbColor];
			M_ASSERT_PTR(mColor);
		}

	~RColorMap(void) { if (mColor) delete [] mColor; }

	// services

	RColor& operator[](ui32 index)	{ return ItemAt(index); }
	RColor& ItemAt    (ui32 index)	{ if (index>=mNbColor || !mColor) M_THROW(kErrBadIndex); return mColor[index]; }
	ui32	Size	  (void)		{ return mNbColor; }

protected:

	ui32	mNbColor;
	RColor *mColor;

}; // end of class def for RColorMap


//------------------------------------------------------------
// inline implementations

//**********************************
inline ui32 RColor::GetColor32(void)
//**********************************
{
	return	  (a << K_RCOLOR_32_A)
			| (r << K_RCOLOR_32_R)
			| (g << K_RCOLOR_32_G)
			| (b << K_RCOLOR_32_B);
}


//********************************************
inline void RColor::SetColor32(ui32 c, ui8 _i)
//********************************************
{
	i = _i;
	a = (c >> K_RCOLOR_32_A) & 0xFF;
	r = (c >> K_RCOLOR_32_R) & 0xFF;
	g = (c >> K_RCOLOR_32_G) & 0xFF;
	b = (c >> K_RCOLOR_32_B) & 0xFF;
}

//------------------------------------------------------------


#endif // of _H_RCOLOR_

/****************************************************************

	$Log: RColor.h,v $
	Revision 1.1  2000/03/13 08:55:13  Ralf
	First revision
	
****************************************************************/


// eof

