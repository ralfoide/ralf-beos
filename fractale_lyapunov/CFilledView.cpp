/***********************************************************
	CFilledView.cpp
***********************************************************/

#include <stdio.h>
#include <string.h>
#include <Bitmap.h>
#include "CFilledView.h"
#include "CFilledWin.h"

//--------------------------------------------------------

#define DIVIDER 4

//--------------------------------------------------------


//***********************************************************
CFilledView::CFilledView(BRect frame, char *title, long resize, long flags)
				 :BView(frame, title, resize, flags)
//***********************************************************
{
	bounds = Bounds();
	srcRect = BRect(0,0,639,479);
	blitRect = srcRect;
	map = NULL;
	mapBpr = 0;
	mapPtr = NULL;
	mapSize = 0;
}


//***********************************************************
bool CFilledView::init(void)
//***********************************************************
{
	map = new BBitmap(blitRect, B_COLOR_8_BIT);
	if (!map) return false;

	mapBpr = map->BytesPerRow();
	mapPtr = (uchar *)map->Bits();
	mapSize = map->BitsLength();
	mapSx = blitRect.IntegerWidth()+1;
	mapSy = blitRect.IntegerHeight()+1;

	if (debug)
		printf("CFilledView::init / map %p, mapPtr %p, bpr %ld, sx-sy <%ldx%ld>, size %ld\n",
			map, mapPtr, mapBpr, mapSx, mapSy, mapSize);

	memset(mapPtr, 0, mapSize);
	return true;
}


//***********************************************************
CFilledView::~CFilledView(void)
//***********************************************************
{
	if (map) delete map;
}


//--------------------------------------------------------


//********************************************************
void CFilledView::Draw(BRect upd)
//********************************************************
{
	DrawBitmap(map, srcRect, blitRect);
}


#ifdef K_USE_ZOOM

//********************************************************
void CFilledView::setZoom(int32 sx, int32 sy)
//********************************************************
{
	blitRect.Set(0,0,sx-1,sy-1);
	ResizeTo(sx-1,sy-1);
}

#endif


// eoc
