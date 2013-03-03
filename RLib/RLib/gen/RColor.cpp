/*****************************************************************************

	Projet	: RLib

	Fichier	: RColor.h
	Partie	: Core bitmap

	Auteur	: RM
	Date	: 230899
	Format	: tabs==4

*****************************************************************************/

#include "RLib.h"
#include "RColor.h"

//------------------------------------------------------------

const RColor gBlack      (   0,   0,   0, 0xFF,    0);
const RColor gRed		 (0xFF,   0,   0, 0xFF,    1);
const RColor gYellow	 (0xFF,0xFF,   0, 0xFF,    2);
const RColor gGreen		 (   0,0xFF,   0, 0xFF,    3);
const RColor gCyan		 (   0,0xFF,0xFF, 0xFF,    4);
const RColor gBlue		 (   0,   0,0xFF, 0xFF,    5);
const RColor gMagenta	 (0xFF,   0,0xFF, 0xFF,    6);
const RColor gWhite		 (0xFF,0xFF,0xFF, 0xFF, 0xFF);
const RColor gOpaque	 (   0,   0,   0, 0xFF,    0);
const RColor gTransparent(   0,   0,   0,    0,    0);

//------------------------------------------------------------



//------------------------------------------------------------
//-------------------- RLIB MACOS ----------------------------
//------------------------------------------------------------

#if defined(RLIB_MACOS)


//*************************************************
RColor::RColor(const RGBColor &color, const ui8 _a)
//*************************************************
{
	a = _a;
	r = color.red   >> 8;
	g = color.green >> 8;
	b = color.blue  >> 8;
}


//**********************************************
RColor& RColor::operator=(const RGBColor &color)
//**********************************************
{
	r = color.red   >> 8;
	g = color.green >> 8;
	b = color.blue  >> 8;
	return *this;
}


//*******************************
RColor::operator RGBColor() const
//*******************************
{
	RGBColor color;
	color.red   = (r << 8) | ((r & 0xF) << 4) | (r & 0xF);
	color.green = (g << 8) | ((g & 0xF) << 4) | (g & 0xF);
	color.blue  = (b << 8) | ((b & 0xF) << 4) | (b & 0xF);
	return color;
}


//********************************************
void RColor::QdRgbColor(RGBColor &color) const
//********************************************
{
	color.red   = (r << 8) | ((r & 0xF) << 4) | (r & 0xF);
	color.green = (g << 8) | ((g & 0xF) << 4) | (g & 0xF);
	color.blue  = (b << 8) | ((b & 0xF) << 4) | (b & 0xF);
}


//------------------------------------------------------------
//-------------------- RLIB WIN32 ----------------------------
//------------------------------------------------------------


#elif defined(RLIB_WIN32)

// cf MSDN :
// COLORREF is ABGR with A=0
// Info keywords : COLORREF, PALETTERGB, PALETTEINDEX (macros), GetSysColor, etc.


//******************************************
RColor::RColor(COLORREF color, const ui8 _a)
//******************************************
{
	b = (color >> 16) & 0xFF;
	g = (color >>  8) & 0xFF;
	r = (color >>  0) & 0xFF;
	a = _a;
}


//*****************************
RColor::operator COLORREF(void)
//*****************************
{
	return (COLORREF)((b << 16) | (g << 8) | r);
}


//**************************************
void RColor::SetColorRef(COLORREF color)
//**************************************
{
	b = (color >> 16) & 0xFF;
	g = (color >>  8) & 0xFF;
	r = (color >>  0) & 0xFF;
	a = 0xFF;
}


//********************************
COLORREF RColor::GetColorRef(void)
//********************************
{
	return (COLORREF)((b << 16) | (g << 8) | r);
}


#endif	// RLIB_MACOS, RLIB_WIN32


//------------------------------------------------------------

/****************************************************************

	$Log: RColor.cpp,v $
	Revision 1.1  2000/03/13 08:55:13  Ralf
	First revision
	
****************************************************************/

// eof
