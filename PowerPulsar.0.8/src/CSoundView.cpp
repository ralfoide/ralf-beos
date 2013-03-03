/***********************************************************
	CSdoundView.cpp
***********************************************************/

#include "CSoundView.h"
#include "CPulsarMsg.h"

//--------------------------------------------------------


//********************************************************
CSoundView::CSoundView(BRect frame, char *title, long resize, long flags, bool chan)
					 :BView(frame, title, resize, flags)
//********************************************************
{
	if (debug) printf("CSoundView::CSoundView -- name %s\n", title);

	channel = chan;
	bounds = Bounds();

	map = new BBitmap(bounds, B_COLOR_8_BIT);
	mapSx = (long)bounds.Width();
	mapSy = (long)bounds.Height();
	if (map)
	{
		mapBpr = map->BytesPerRow();
		mapPtr = (uchar *)map->Bits();
		mapSize = map->BitsLength();
		if (!mapPtr)
		{
			delete map;
			map = NULL;
		}
	}

	//if (channel)	colorFond = index_for_color(152, 203, 203);
	//else 					colorFond = index_for_color(102, 152, 152);
	//colorTrait = index_for_color(240, 0, 0);
	colorFond = 0;
	if (channel)	colorTrait = 250;
	else					colorTrait = 52;
}


//********************************************************
CSoundView::~CSoundView(void)
//********************************************************
{
	if (map) delete map;
}


//--------------------------------------------------------


//********************************************************
void CSoundView::Draw(BRect upd)
//********************************************************
{
int i;
//signed short int *s;
int32 *s;
uchar *base;
//int b = upd.left, e = upd.right;
int b = 0, e = N;
long bpr = mapBpr;
uchar bleu=colorFond, rouge=colorTrait;

	if (!map) return; //Draw_Old(upd);

	base = mapPtr;
	memset(base, bleu, mapSize);

	//if (e>N) e=N;
	int h2 = (int)bounds.Height();
	int h = h2/2;
	int y1, y2;

	//s = (channel ? streamLeft : streamRight);
	s = (channel ? gSourceHigh : gSourceBass);

	for(i = b, base+=b; i<e; i++, base++)
	{
		y1 = h+h*s[i]/32768;
		y2 = h+h*s[i+1]/32768;
		if (y1 > y2) { int a=y1; y1=y2; y2=a; }
		if (y1 < 0) y1 = 0;
		if (y2 > h2) y2 = h2-1;

		uchar *p = base+y1*bpr;
		int y;

		for(y=y1; y<=y2; y++, p+=bpr) *p = rouge;
	}

	Window()->Lock();
	DrawBitmap(map, upd, upd);
	Window()->Unlock();
}


//--------------------------------------------------------
// eoc
