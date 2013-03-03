/*****************************************************************************

	Projet	: Portable Lib

	Fichier	:	RVertLayout.h
	Partie	: GUI kit

	Auteur	: RM
	Date		: 090798
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_RVERTLAYOUT_
#define _H_RVERTLAYOUT_

#include "gMachine.h"
#include "RLayoutInterface.h"
#include "gErrors.h"

//---------------------------------------------------------------------------

//*******************
enum EVertLayoutAlign
//*******************
{
	kVertLayoutAlignStack,
	kVertLayoutAlignEqual,
	kVertLayoutAlignTop,
	kVertLayoutAlignCenter,
	kVertLayoutAlignBottom,
};

//*****************************************
class RVertLayout : public RLayoutInterface
//*****************************************
{
public:
	RVertLayout(EVertLayoutAlign align = kVertLayoutAlignStack, rbool hAdjust = true);
	virtual ~RVertLayout(void);

	void adjustPane(RPane &parent);
	virtual void setBorder(si32 hBorder, si32 vBorder);

//----
protected:

	EVertLayoutAlign mAlign;
	rbool	mHAdjust;
	si32 mHBorder, mVBorder;

}; // end of class defs for RVertLayout


//---------------------------------------------------------------------------

#endif // of _H_RVERTLAYOUT_

// eoh
