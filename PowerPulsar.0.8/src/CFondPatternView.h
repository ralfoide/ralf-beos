/*****************************************************************************

	Projet	: Pulsar

	Fichier	:	CInterfaceWin.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CFONDPATTERNVIEW_
#define _H_CFONDPATTERNVIEW_

#include "globals.h"
#include <Path.h>

//---------------------------------------------------------------------------

//***********************************
class CFondPatternView : public BView
//***********************************
{
public:
	CFondPatternView(BRect frame,
										char *title,
										long resize,
										const char *bitmapname,
										long flags=B_WILL_DRAW);
	virtual ~CFondPatternView(void) { /*if (mImage8) delete mImage8;
														 if (mImage32) delete mImage32; */ }

	virtual	void MouseDown(BPoint where);
	virtual	void MouseMoved(BPoint where, uint32 code, const BMessage *a_message);

	void Draw(BRect upd);
private:

	BRect bounds;
	BBitmap *mImage32;
	BBitmap *mImage8;
	uint32	 mSx, mSy;
};


//***********************************
class CFondView : public BView
//***********************************
{
public:
	CFondView(BRect frame,
								char *title,
								long resize,
								const char *bitmapname,
								long flags=B_WILL_DRAW);
	virtual ~CFondView(void) { }

	void Draw(BRect upd) { };
private:

};




//---------------------------------------------------------------------------

#endif // of _H_CFONDPATTERNVIEW_

// eoh

