/***********************************************************
	CSdoundView.cpp
***********************************************************/

#ifndef _H_CSOUNDVIEW_
#define _H_CSOUNDVIEW_

#include "globals.h"
#include "CPulsarApp.h"

//--------------------------------------------------------

//*****************************
class CSoundView : public BView
//*****************************
{
public:
	CSoundView(BRect frame, char *title, long resize, long flags=B_WILL_DRAW, bool chan=false);
	virtual ~CSoundView(void);

	//virtual	void AttachedToWindow(void);
	//virtual	void DetachedFromWindow(void);

	virtual	void KeyDown(const char *bytes, int32 numBytes)
		{ gApplication->touches(bytes, numBytes); }
	virtual void MouseDown(BPoint where) { MakeFocus(true); }

	void Draw(BRect upd);
	void Pulse(void) { Draw(bounds); }

private:
	BRect bounds;
	bool channel;
	
	BBitmap *map;
	uchar 	*mapPtr;
	long		mapBpr, mapSize;
	long		mapSx, mapSy;

	uchar		colorFond, colorTrait;

};

//--------------------------------------------------------
#endif // of _H_CSOUNDVIEW_

// eoh
