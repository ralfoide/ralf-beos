/***********************************************************
	CFftView.cpp
***********************************************************/

#include "CFftView.h"

//--------------------------------------------------------



//***********************************************************
CFftView::CFftView(BRect frame, char *title, long resize, long flags, bool chan)
				 :BView(frame, title, resize, flags)
//***********************************************************
{
long i;

	if (debug) printf("CFftView::CFftView -- name %s\n", title);

	channel = chan;
	bounds = Bounds();

	logIndex256 = new long[N/2];
	logAmpli = new long[100];
	if (!logIndex256 || !logAmpli) throw(kAllocErr);

	for(logIndex256[0]=0, i=1; i<N/2; i++)
		logIndex256[i] = (long)(256.0*log(1.0+(i/10.0))/log(1.0+25.6));

	for(i=0;i<100; i++)
		logAmpli[i]=(long)(100.0*log(1.0+i/5.0)/log(1.0+100.0/5.0));
		//logAmpli[i]=100.0*log(1.0+i/10.0)/log(1.0+100.0/10.0);


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
	//colorRouge = index_for_color(240, 0, 0);
	//colorNoir = index_for_color(0, 0, 0);

	colorFond = 0;
	if (channel)	colorNoir = 250;
	else			colorNoir = 52;

}


//***********************************************************
CFftView::~CFftView(void)
//***********************************************************
{
	if (logIndex256) delete logIndex256;
	if (logAmpli) delete logAmpli;
	if (map) delete map;
}


//--------------------------------------------------------


//--------------------------------------------------------



//********************************************************
void CFftView::Draw(BRect upd)
//********************************************************
{
int i;
uchar *base;
//int b = upd.left, e = upd.right;
int b = 0, e = N;
long bpr = mapBpr;
uchar bleu=colorFond, noir=colorNoir; //, rouge=colorRouge;

	if (!map) return;

	base = mapPtr;
	memset(base, bleu, mapSize);

#if 0 // streamer == streamer_B.
	Vector *vec;
	Cmpx *complex;
	vec = (channel ? L : R);
	complex = vec->v;
#else
	float *result;
	result = (channel ? gFftHigh : gFftBass);
#endif

	int h = (int)bounds.Height();
	int y1, y2;

	for(i = b, base+=b; i<e; i++, base++)
	{
		//y1 = h-(!complex[i])/16384*2;
		//y2 = h-(!complex[i+1])/16384*2;
		y1 = (int)(h-(result[i])/16384*2);
		y2 = (int)(h-(result[i+1])/16384*2);
		if (y1<0) y1=0;
		if (y2<0) y2=0;
		//if (y1>0 && y1<=100) y1=logAmpli[y1];
		//if (y2>0 && y2<=100) y2=logAmpli[y2];
		if (y1 > y2) { int a=y1; y1=y2; y2=a; }
		if (y1 > 99) y1=99;
		if (y2 > 99) y2=99;

		uchar *p = base+y1*bpr;
		int y;

		for(y=y1; y<=y2; y++, p+=bpr) *p = noir;
	}

	Window()->Lock();
	DrawBitmap(map, upd, upd);
	Window()->Unlock();
}


// © 1995, 1996 B. T.
