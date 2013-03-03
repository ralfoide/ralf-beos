/*****************************************************************************

	Projet	: Pulsar Sample slider AddOn

	Fichier	: slider_addon.cpp
	Partie	: End

	Auteur	: RM
	Date	: 100797
	Format	: tabs==4

*****************************************************************************/

#include "PP_Be.h"
#include "CFilter.h"

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

extern "C" void * filterInit(uint32 index);

#define ADDON_NAME "Slider"

//--------------------------------------------------------------------

bool debug=false;

#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif

//--------------------------------------------------------------------


//**********************************
class CSliderFilter : public CFilter
//**********************************
{
public:

	CSliderFilter(void) : CFilter()	{ /* nothing */ }
	virtual ~CSliderFilter(void) 	{ /* nothing */ }

	virtual bool load(void);
	virtual bool prepare(void);
	virtual void processFrame8(SFrameInfo &frame);
	virtual void processFrame32(SFrameInfo &frame);
	virtual void terminate(void);
	virtual void unload();

private:
	thread_id		loader_id;
	sem_id			wait_lock;
	bool			must_quit;

	uchar			*mBits;
	uint32			mBitsLength; 
	BRect 			use_rect;
	EColorSpace		mMode;

	bool			mGrayDone;
	bool			mGrays[256];

	static int32 loader_entry(void *data);
	void loader8(void);
	void loader32(void);
	uint32 *makeBitmap32Bit(BBitmap *bitmap, int ratio);
	BBitmap *_makeBitmap8Bit(BBitmap *source, int ratio);

	BBitmap *GetBitmap(const char *path);

}; // end of class defs for CSliderFilter


//--------------------------------------------------------------------

//********************
int main(int, char **)
//********************
// Code to help a little the fellow user double-clicking the
// add-on icon directly.
{
	new BApplication("application/x-vnd.ralf-vanilla");
	BAlert *box = new BAlert(ADDON_NAME,
							 "Thanks for using " ADDON_NAME " !\n\n"
							 "Please move the " ADDON_NAME " executable into\n"
							 "the PowerPulsar add-on directory\n"
							 "and try again !",
							 "Great !");
	box->Go();
	delete be_app;
	return 0;
}

//--------------------------------------------------------------------


//*****************************
void * filterInit(uint32 index)
//*****************************
{
	// this add-on only declares ONE filter
	if (index > 0) return NULL;

	CSliderFilter *info = new CSliderFilter;
	if (!info) return NULL;		// memory error, give up

	// returns the instance -- the caller will check that this instance
	// is derived from CFilter but is _not_ a CFilter...
	return info;
} // end of filterInit


//****************************
bool CSliderFilter::load(void)
//****************************
{
	sFilter.name = "Img Slider";
	sFilter.author = "R'alf";
	sFilter.info = "Sample code";
	sFilter.majorVersion = 0;
	sFilter.minorVersion = 3;
	sFilter.position = kFilterPositionAny+0.2;
	sFilter.supportedMode = (EColorSpace)(kColor8Bit | kRgb32Bit);

	// create semaphores
	wait_lock = -1;
	must_quit = true;

	use_rect.Set(0,0,0,0);
	mBits = NULL;
	mBitsLength = 0;
	mMode = kColor8Bit;
	mGrayDone = false;

	return true;
}  // end of load for CSliderFilter





//******************************
void CSliderFilter::unload(void)
//******************************
{
	// wait_lock has already been deleted in terminate()
}


//**************************************************
BBitmap * CSliderFilter::GetBitmap(const char *path)
//**************************************************
{
	BBitmap *bitmap = NULL;

	try
	{
 		BTranslatorRoster *roster = BTranslatorRoster::Default(); 
 		BFile file(path, B_READ_ONLY); 
		BBitmapStream stream; 
		if (roster->Translate(&file, NULL, NULL, &stream, B_TRANSLATOR_BITMAP) < B_OK) 
			return NULL;
		stream.DetachBitmap(&bitmap);
	}
	catch(...)
	{
		return NULL;
	}
	return bitmap;
}


//*******************************
bool CSliderFilter::prepare(void)
//*******************************
{
	wait_lock = create_sem(0, "slider_wait_lock");
	if (wait_lock < B_NO_ERROR) return false;

	must_quit = false;
	mBitsLength = sPrepare.sxy;
	mBits = new uchar[mBitsLength];
	if (!mBits) return false;
	memset(mBits, 0, mBitsLength);

	mMode = sPrepare.mode;

	if (!mGrayDone)
	{
		mGrayDone = true;
		for(long i=0; i<256; i++)
		{
			rgb_color col = sPrepare.colorList[i];
			bool level = false;
			const uchar seuil = 40;
			if ((col.red < seuil && col.blue < seuil && col.green < seuil)
				&& (abs((long)col.red-(long)col.blue)<5 && abs((long)col.blue-(long)col.green)<5 
				&& abs((long)col.red-(long)col.green)<5))
				level = true;
			mGrays[i] = level;
		}
	}
	
	// launch the thread here
	loader_id = spawn_thread(loader_entry, "slider_loader", B_NORMAL_PRIORITY, (void *)this);
	if(loader_id >= B_NO_ERROR)
	{
		resume_thread(loader_id);
		return true;
	}
	return false;
}



//*********************************
void CSliderFilter::terminate(void)
//*********************************
{
	int32 val;
	sem_id sid = wait_lock;

	must_quit = true;					// indicate the thread it must quit asap
	wait_lock = -1;						// and prevent it from locking
	delete_sem(sid);
	wait_for_thread(loader_id, &val);	// the thread should thus terminate soon
	if (mBits) delete mBits;			// cleanup
	mBits = NULL;
}


//*******************************************
int32 CSliderFilter::loader_entry(void *data)
//*******************************************
{
	CSliderFilter *elthis = (CSliderFilter *)data;

	switch(elthis->mMode)
	{
		case kColor8Bit:	elthis->loader8();	break;
		case kRgb32Bit:		elthis->loader32();	break;
	}
	return 0;
}


//*******************************
void CSliderFilter::loader8(void)
//*******************************
{
	BDirectory imgdir(sLoad.dirImg.Path());
	BBitmap *b8 = NULL;
	BBitmap *bitmap = NULL;
	status_t err;

	while(!must_quit)
	{
		int32 count = imgdir.CountEntries();
		if (!count) must_quit = true;
		imgdir.Rewind();
		for(;!must_quit;)
		{
			BEntry entry;

			err = acquire_sem_etc(wait_lock, 1, B_TIMEOUT, (bigtime_t)500000);
			if (must_quit || err != B_TIMED_OUT) break;

			for(int kount=rand() % count; kount>=0; kount--)
			{
				err = imgdir.GetNextEntry(&entry);
				if (err < B_NO_ERROR) break;
			}
			if (err < B_NO_ERROR) break;
			
			BPath path;
			err = entry.GetPath(&path);
			if (err < B_NO_ERROR) continue;

			bitmap = GetBitmap(path.Path());
			if (!bitmap) continue;
			if (must_quit) break;

			BRect r;
			int32 sx,sy,rsx,rsy,xoffset,yoffset;
			r = bitmap->Bounds();
			sx = (int32)r.Width()+1;
			sy = (int32)r.Height()+1;
			int ratio = 1+max((sx/(640+40)),(sy/(480+40)));	// autorise 40 pixels de debordement

			sx /= ratio;	rsx = sx;
			sy /= ratio;	rsy = sy;

			b8 = _makeBitmap8Bit(bitmap, ratio);
			delete bitmap; bitmap = NULL;
			if (!b8) continue;
			if (must_quit) break;

			int32 bpr=b8->BytesPerRow();

			if (sx > 640) sx = 640;
			sx = sx & 0xFFF8;		// only keeps a 8-bytes aligned size
			if (sy > 480) sy = 480;
			xoffset = (rsx-sx)/2;	// take center of image
			yoffset = (rsy-sy)/2;
			uchar *bits;
			bits = (uchar *)b8->Bits();
			bits += xoffset + yoffset*bpr;
			
			uchar *dest = mBits;
			int32 destbpr = sPrepare.bpr;
			
			int32 px=0, py=0;
			if (sx < 640)		// modulo 0 doesn't work :-) ... :-/
				px = (rand() % (640-sx)) & 0xFFF8;	// align on 8 boundary for double-copy
			if (sy < 480)
				py = rand() % (480-sy);
			dest+=py*destbpr + px;

			//printf("bits %p (%d), dest %p (%d), sx %d (%d), sy %d (%d), px %d, py %d, xoff %d, yoff %d\n",
			//		bits, bpr, dest, destbpr, sx, rsx, sy, rsy, px, py, xoffset, yoffset);

			// old copy that works for non-aligned data
			for(long y=sy;y>0; y--)
			{
				memcpy(dest, bits, sx);
				bits+=bpr;
				dest+=destbpr;
			}

			use_rect.Set(px,py,px+sx,py+sy);

			delete b8; b8 = NULL;
		}

		delete b8; b8 = NULL;
		delete bitmap; bitmap = NULL;
	}
}


//********************************
void CSliderFilter::loader32(void)
//********************************
{
	BDirectory imgdir(sLoad.dirImg.Path());
	uint32 *b32 = NULL;
	BBitmap *bitmap = NULL;
	status_t err;

	while(!must_quit)
	{
		int32 count = imgdir.CountEntries();
		if (!count) must_quit = true;
		imgdir.Rewind();
		for(;!must_quit;)
		{
			BEntry entry;

			err = acquire_sem_etc(wait_lock, 1, B_TIMEOUT, (bigtime_t)2000000);
			if (must_quit || err != B_TIMED_OUT) break;

			for(int kount=rand() % count; kount>=0; kount--)
			{
				err = imgdir.GetNextEntry(&entry);
				if (err < B_NO_ERROR) break;
			}
			if (err < B_NO_ERROR) break;
			
			BPath path;
			err = entry.GetPath(&path);
			if (err < B_NO_ERROR) continue;
			
			bitmap = GetBitmap(path.Path());
			if (!bitmap) continue;
			if (must_quit) break;

			BRect r;
			int32 sx,sy,rsx,rsy,xoffset,yoffset;
			r = bitmap->Bounds();
			sx = (int32)r.Width()+1;
			sy = (int32)r.Height()+1;
			if (sx && sy)
			{
				int ratio = 1+max((sx/(640+40)),(sy/(480+40)));	// autorise 40 pixels de debordement
				sx /= ratio;	rsx = sx;
				sy /= ratio;	rsy = sy;

				b32 = makeBitmap32Bit(bitmap, ratio);
			}
			delete bitmap; bitmap = NULL;
			if (!b32) continue;
			if (must_quit) break;

			int32 bpr=rsx*4;

			if (sx > 640) sx = 640;
			sx = sx & 0xFFF8;		// only keeps a 4-bytes aligned size
			if (sy > 480) sy = 480;
			xoffset = (rsx-sx)/2;	// take center of image
			yoffset = (rsy-sy)/2;
			uchar *bits;
			bits = (uchar *)b32;
			bits += xoffset*2 + yoffset*bpr;

			uchar *dest = mBits;
			int32 destbpr = sPrepare.bpr;

			int32 px=0, py=0;
			if (sx < 640)		// modulo 0 doesn't work :-) ... :-/
				px = (rand() % (640-sx)) & 0xFFF8;	// align on 8 boundary for double-copy
			if (sy < 480)
				py = rand() % (480-sy);
			dest+=py*destbpr + px*4;

			//printf("bits %p (%d), dest %p (%d), sx %d (%d), sy %d (%d), px %d, py %d, xoff %d, yoff %d\n",
			//		bits, bpr, dest, destbpr, sx, rsx, sy, rsy, px, py, xoffset, yoffset);

			// old copy that works for non-aligned data
			for(long y=sy;y>0; y--)
			{
				memcpy(dest, bits, sx*4);
				bits+=bpr;
				dest+=destbpr;
			}

			use_rect.Set(px,py,px+sx,py+sy);

			delete b32; b32 = NULL;
			delete bitmap; bitmap = NULL;

			if (must_quit) break;
		}
		delete b32; b32 = NULL;
		delete bitmap; bitmap = NULL;
	}
}


//*****************************************************************
BBitmap *CSliderFilter::_makeBitmap8Bit(BBitmap *source, int ratio)
//*****************************************************************
{
	BRect r;
	BBitmap *dest;
	long sx,sy;

	if (!source) return NULL;

	r = source->Bounds();

	sx = (int32)r.Width()+1;
	sy = (int32)r.Height()+1;
	int sx2=sx/ratio;
	int sy2=sy/ratio;

	dest = new BBitmap(BRect(0, 0, sx2-1, sy2-1), B_COLOR_8_BIT);
	if (!dest) return NULL;

	uchar *sourceBits = (uchar *)source->Bits();
	long sourceBpr = source->BytesPerRow();
	long destBpr   = dest->BytesPerRow();
	color_space sourceCs = source->ColorSpace();

	if (sourceCs != B_RGB_32_BIT) return NULL;

	// transform B_RGB_32_BIT into B_COLOR_8_BIT
	// unfortunately, Be stores 32 bits BGRA in the bitmap bits while
	// the SetBits() expects 24 bits RGB when it receives the 32 bpp constant (!!!).
	// HOOK : this is unfortunately *very* system specific.
	// Note : I use SetBits() rather than using a Bitmap with a BView since
	// only SetBits() handles the dithering automagically.

	uchar *rvb= new uchar[sx*3];
	if (!rvb)
	{
		delete dest;
		return NULL;
	}

	long i,j,offset;
	long k =sx2*3;
	for(offset=0, j=0; j <sy; j+=ratio, sourceBits+=sourceBpr*ratio, offset += destBpr)
	{
		uchar *p=(rvb-1);
		uchar *s=sourceBits;
		for(i=0; i<sx; i+=ratio, s+=4*ratio)
		{
			// source is BBitmap 32, i.e. BGRA (gasp !) ----> :-(
			uchar r,g,b;
			r = s[2];
			g = s[1];
			b = s[0];
			if (ratio == 2)
			{
				r = (r+s[2+4]) >> 1;
				g = (g+s[1+4]) >> 1;
				b = (b+s[0+4]) >> 1;
			}
			*(++p) = r;
			*(++p) = g;
			*(++p) = b;	
		}
		dest->SetBits(rvb, k, offset, B_RGB_32_BIT);
	}

	delete rvb;
	return dest;
} // end of makeBitmap8Bit for CSliderFilter


//****************************************************************
uint32 *CSliderFilter::makeBitmap32Bit(BBitmap *source, int ratio)
//****************************************************************
/*
	ratio = unzomm index. 1=normal, 2=divide by 2, etc...
*/
{
	BRect r;
	uint32 *dest;
	long sx,sy;

	if (!source) return NULL;

	r = source->Bounds();
	sx = (int32)r.Width()+1;
	sy = (int32)r.Height()+1;
	int sx2=sx/ratio;
	int sy2=sy/ratio;
	dest = new uint32[sx*sy]; //[sx2*sy2];
	if (!dest) return NULL;

	uchar *sourceBits = (uchar *)source->Bits();
	long sourceBpr = source->BytesPerRow();
	color_space sourceCs = source->ColorSpace();

	uint32 *rvb= dest;
	uint32 *ptr = rvb;
	int bpr = sx2;

	long i,j;
	for(j=0; j <sy; j+=ratio, sourceBits+=sourceBpr*ratio, ptr+=bpr)
	{
		uchar *s=sourceBits;
		rvb = ptr;
		for(i=0; i<sx; i+=ratio, s+=4*ratio)
		{
			uchar r,g,b;
			r = s[2];
			g = s[1];
			b = s[0];
			if (ratio == 2)
			{
				r = (r+(s[2+4])) >> 1;
				g = (g+(s[1+4])) >> 1;
				b = (b+(s[0+4])) >> 1;
			}
			*(rvb++) = (b<<K_BLUE_OFFSET)|(g<<K_GREEN_OFFSET)|(r<<K_RED_OFFSET);
		}
	}

	return dest;
} // end of makeBitmap32Bit for CSliderFilter


// --



//**************************************************
void CSliderFilter::processFrame8(SFrameInfo &frame)
//**************************************************
{
	// the assumption here is that :
	// px is a multiple of 8,
	// sx is a multiple of 8,
	// base is double aligned
	// thus double copies can be used

	uchar *src;
	uchar *base;
	long px,py,sx,sy;

	src = mBits;
	if (!src) return;

	px=(int32)use_rect.left;
	py=(int32)use_rect.top;
	sx=(int32)use_rect.Width();
	sy=(int32)use_rect.Height();
	if (sx<=0) return;

	base = frame.screen;
	int32 bpr = sPrepare.bpr;
	base += px+py*bpr;
	src += px+py*bpr;

	// normal mode (square mode)
	double *s = (double *)(src-8);		// pour utiliser stfdu des le depart
	double *d = (double *)(base-8);		// pour utiliser lfdzu des le depart
	uint32 offset=(bpr-sx)/8;

	int32 sx2 = sx/8;
	for(; sy>0; sy--)
	{
		for(sx=sx2; sx>=4; sx-=4)
		{
			*(++d) = *(++s);
			*(++d) = *(++s);
			*(++d) = *(++s);
			*(++d) = *(++s);
		}
		if (sx > 0)
		{
			switch(sx)
			{
				case 3:	*(++d) = *(++s);
				case 2:	*(++d) = *(++s);
				case 1:	*(++d) = *(++s);
			}
		}
		d+=offset;
		s+=offset;
	}
}  // end of processFrame8


//***************************************************
void CSliderFilter::processFrame32(SFrameInfo &frame)
//***************************************************
{
	uchar *src;
	uchar *base;
	long px,py,sx,sy;

	src = mBits;
	if (!src) return;

	px=(int32)use_rect.left;
	py=(int32)use_rect.top;
	sx=(int32)use_rect.Width();
	sy=(int32)use_rect.Height();
	if (sx<=0) return;

	base = frame.screen;
	int32 bpr = sPrepare.bpr;
	base += px*4+py*bpr;
	src += px*4+py*bpr;

	sx *= 4;
	for(; sy>0; sy--)
	{
		memcpy(base, src, sx);
		base += bpr;
		src += bpr;
	}

}  // end of processFrame32


// eoc
