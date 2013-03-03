/***********************************************************
	Projet	: Pulsar

	Fichier	: CFilledView.cpp
	Partie	: Visu

	Auteur	: RM
	Date	: xxxx97
	Format	: tabs==4

***********************************************************/

#include "globals.h"
#include "CFilledView.h"
#include "CFilledWin.h"

#include "fastmath.h"
#include <math.h>

//--------------------------------------------------------

#define DIVIDER 4

//--------------------------------------------------------


//*************************************************************************
CFilledView::CFilledView(BRect frame, char *title, long resize, long flags)
			:BView(frame, title, resize, flags)
//*************************************************************************
{
	bounds = Bounds();
	srcRect = BRect(0,0,639,479);
	blitRect = srcRect;
	map = NULL;
	mapBpr = 0;
	mapPtr = NULL;
	mapSize = 0;
}


//**************************
bool CFilledView::init(void)
//**************************
{
	map = NULL;
	return setColorSpace(kColor8Bit, false);
}


//*****************************
CFilledView::~CFilledView(void)
//*****************************
{
	if (map) delete map;
}


//--------------------------------------------------------


//***************************
void CFilledView::Pulse(void)
//***************************
{
	if (Window()->LockWithTimeout(10e3) >= B_OK)
	{
		Draw(srcRect);
		Window()->Unlock();
	}
}


//--------------------------------------------------------


//****************************************************************
bool CFilledView::setColorSpace(EColorSpace mode, bool force_lock)
//****************************************************************
{
	if (map && colorSpace == mode) return true;

	if (force_lock && !LockLooper())
	{
		if (debug) printf("**** CFilledView::setColorSpace ---> LockLooper FAILED !!! ****\n");
		return false;
	}

	if (map)
	{
		delete map;
		map = NULL;
		mapPtr = NULL;
	}

	colorSpace = mode;
	map = new BBitmap(blitRect, (colorSpace == kColor8Bit ? B_CMAP8 : B_RGB32));
	if (!map) return false;

	mapBpr = map->BytesPerRow();
	mapPtr = (uchar *)map->Bits();
	mapSize = map->BitsLength();
	mapSx = blitRect.IntegerWidth()+1;
	mapSy = blitRect.IntegerHeight()+1;

	if (debug)
		printf("CFilledView::init / map %p, mapPtr %p, bpr %d, sx-sy <%dx%d>, size %d\n",
			map, mapPtr, mapBpr, mapSx, mapSy, mapSize);

	memset(mapPtr, 0, mapSize);

	if (force_lock) UnlockLooper();
	return true;
}


//*******************************
void CFilledView::Draw(BRect upd)
//*******************************
{
	// if (debug) printf("CFilledView::DRAW / map %p, mapPtr %p, bpr %d, sx-sy <%dx%d>, size %d\n", map, mapPtr, mapBpr, mapSx, mapSy, mapSize);

	if (blitRect.Width() == 640.0)
		DrawBitmap(map, BPoint(0,0));
	else
		DrawBitmap(map, srcRect, blitRect);
	//DrawBitmapAsync(map, srcRect, blitRect);
	//Flush();
}


//*******************************************
void CFilledView::setZoom(int32 sx, int32 sy)
//*******************************************
{
	blitRect.Set(0,0,sx-1,sy-1);
	ResizeTo(sx-1,sy-1);
}



// eoc
