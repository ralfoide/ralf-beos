/***********************************************************
	CFftView.cpp
***********************************************************/

#ifndef _H_CFFTVIEW_
#define _H_CFFTVIEW_


#include "globals.h"
#include "CPulsarApp.h"
#include "CPulsarMsg.h"

//--------------------------------------------------------

//***************************
class CFftView : public BView
//***************************
{
public:
	CFftView(BRect frame, char *title, long resize, long flags=B_WILL_DRAW, bool chan=false);
	~CFftView(void);

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

	long *logIndex256;
	long *logAmpli;

	BBitmap *map;
	uchar 	*mapPtr;
	long		mapBpr, mapSize;
	long		mapSx, mapSy;

	uchar		colorFond, colorNoir;
};


//--------------------------------------------------------
#endif // of _H_CFFTVIEW_

// eoh
