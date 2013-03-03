/***********************************************************
	CFilledView.cpp
***********************************************************/

#ifndef _H_CFILLEDVIEW_
#define _H_CFILLEDVIEW_

#include "main.h"
#include "View.h"
#include "Window.h"

class CApplication;
extern CApplication *gApplication;

//--------------------------------------------------------

//******************************
class CFilledView : public BView
//******************************
{
public:
	CFilledView(BRect frame, char *title, long resize, long flags=B_WILL_DRAW);
	~CFilledView(void);

	bool init(void);

	inline virtual void Pulse(void)
	{
		if (Window()->LockWithTimeout(10e3) >= B_OK)
		{ Draw(srcRect); Window()->Unlock(); }
	}
	//virtual	void KeyDown(const char *bytes, int32 numBytes)
	//	{ gApplication->touches(bytes, numBytes, Window()->CurrentMessage()); }
	virtual void MouseDown(BPoint where) { MakeFocus(true); }

	void Draw(BRect upd);

	inline void getInfos(int32 &sx, int32 &sy, color_space &mode,
												uint8	* &buffer, int32 &bpr)
								{ sx = mapSx; sy = mapSy; mode = B_COLOR_8_BIT;
									buffer = mapPtr; bpr = mapBpr; }

#ifdef K_USE_ZOOM
	void setZoom(int32 sx, int32 sy);
#endif

private:
	BRect bounds;
	BRect srcRect;
	BRect blitRect;

	BBitmap *map;
	uchar 	*mapPtr;
	long		mapBpr, mapSize;
	long		mapSx, mapSy;
};


//--------------------------------------------------------
#endif // of _H_CFILLEDVIEW_

// eoh
