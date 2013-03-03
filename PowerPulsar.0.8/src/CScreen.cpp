/*****************************************************************************

	Projet	: Pulsar

	Fichier: CScreen.cpp
	Partie	: Loader

	Auteur	: RM
	Date	: 260697
	Format	: tabs==4

	How it works :
	- create a CScreen when needed,
	- call CScreen::init() (note for Chrish : all my classes work like this)
	- it just run its own way

	Notes sur le mode 16-bits :
	a) un mot = 0xFFFF --> un bit de Alpha, 5 de R, R de V, 5 de B.
	   Alpha = methode simple pour faire un melange (src+dest)/2
	b) masques des composantes :
		 A = 0x8000
		 R = 0x7C00
		 V = 0x03E0
		 B = 0x001F
	c) masque pour faire un pixel divise par 2 sur toutes ses composantes :
	   c'est possible en faisant dest=(src>>1)&masque car le bit qui part a
	   droite sur une composante ne gene pas la composante precedente, i.e.
	   les bits ne "bavent" pas d'une composante sur la suivante.
	   D'ou les masques apres decalage :
	   R = 0x3C00
	   V = 0x01E0
	   B = 0x000F
	   soit un total de : masque2 = 0x3DEF
	d) d'ou traitement d'un pixel :
		 si A=1 (int16 < 0) alors blending :
		 	dest = (srcA >> 1)&masque2 + (srcB>>1)&masque2
		 sinon, copie normale via une table:
		 	dest = mTable16to32[src]
	e) dans le cas de la recopie sur ecran l'alpha est ignore (pas de mixage a faire)
		 On remarquera que la table n'a besoin que de 32768 entrees mais qu'elle fait quand
		 meme 65536 entrees car A=0 ou A=1 donne le meme pixel et evite un masque AND 0x7FFF
	f) en 32 bits, le resultat dans la memoire video est en BGRA, cf. GraphicsDefs.h :
		enum color_space --> B_RGB_32_BIT = 8, -- Little-endian (8B 8G 8R 8A)

*****************************************************************************/

#include "CScreen.h"
#include "CPulsarApp.h"
#include "CFilter.h"

#include <StopWatch.h>

//---------------------------------------------------------------------------

long gEcrireIndex3[3] = {1,2,0};
#define K_OFFSCREEN_NEXT(x) (mOffscreenBuffering == kScreenBufferingDouble ? (!x) : \
							(mOffscreenBuffering == kScreenBufferingTripple ? gEcrireIndex3[(x)] : 0))
#define K_ONBOARD_NEXT(x) (mOnBoardBuffering == kScreenBufferingDouble ? (!x) : \
						  (mOnBoardBuffering == kScreenBufferingTripple ? gEcrireIndex3[(x)] : 0))

#if !defined(memcpy_c2nc)
	#define memcpy_c2nc memcpy
#endif

#define inherited BWindowScreen

//*************************************************
CScreen::CScreen(const char *title, ulong _space)
			  :BWindowScreen(title, _space, &error)
//*************************************************
{
	space = _space;
	if (_space == B_8_BIT_640x480) mode = kColor8Bit;
	else /* B_32_BIT_640x480 */ mode = kRgb32Bit;
	first=true;
	drawing_enable = false;
	mOffscreenPageNum = 0;
	mOnBoardPageNum = 0;
	row = 0;
	height = 0;
	base = NULL;
	draw_base = NULL;
	draw_lock = 0;
	draw_sem = B_ERROR;
	initCompleteSem = B_ERROR;
	mScreenBufSize1 = mScreenBufSize2 = 0;
	mScreenBuffer = NULL;
/*
	mMemory = kScreenMemoryOffscreen;
	mOnBoardBuffering = kScreenBufferingDouble;
	mOffscreenBuffering = kScreenBufferingDouble;
	mUmaxHack = false;
	mUseMatrox = false;
*/
	mMemory = gApplication->mPulsarPref.mMemory;
//	if (mode == kRgb32Bit) mMemory = kScreenMemoryOffscreen;
	mOnBoardBuffering = gApplication->mPulsarPref.mOnBoardBuffering;
	mOffscreenBuffering = gApplication->mPulsarPref.mOffscreenBuffering;
	mUmaxHack = gApplication->mPulsarPref.mUmaxHack;
	mUseMatrox = gApplication->mPulsarPref.mMatroxHack;

	mFakeView = new BView(BRect(0,0,639,479), "", 0, 0);
	AddChild(mFakeView);

} // end of constructor for CScreen


//*********************
CScreen::~CScreen(void)
//*********************
{
	if (debug) printf("CScreen::~CScreen -- destructor\n");
	// never call Quit() here
} // end of destructor for CScreen


//**********************
void CScreen::Quit(void)
//**********************
{
	if (debug) printf("CScreen::Quit\n");
	if (draw_sem >= B_NO_ERROR) delete_sem(draw_sem);
	if (initCompleteSem >= B_NO_ERROR) delete_sem(initCompleteSem);
	if (mScreenBuffer) delete mScreenBuffer;

	Disconnect();
	BWindowScreen::Quit();
} // end of Quit for CScreen


//---------------------------------------------------------------------------



//**********************
bool CScreen::init(void)
//**********************
{
	if (!mFakeView) return false;

	draw_lock = 0L;
	draw_sem = create_sem(0,"draw_sem");
	if (draw_sem < B_NO_ERROR) return false;

	initCompleteSem = create_sem(0,"init_sem");
	if (initCompleteSem < B_NO_ERROR) return false;

	Show();
	return true;
} // end of init for CScreen


//---------------------------------------------------------------------------


//*****************************
void CScreen::unlock_draw(void)
//*****************************
// Release the protection on the general io-registers
// (from S3 driver sample code, DR8.2, by Be Inc)
{
	int32	old;

	old = atomic_add (&draw_lock, -1);
	if (old > 1) release_sem(draw_sem);
}	


//***************************
void CScreen::lock_draw(void)
//***************************
// Protect the access to the memory or the graphic engine registers by
// locking a benaphore.
// (from S3 driver sample code, DR8.2, by Be Inc)
{
	int32	old;

	old = atomic_add (&draw_lock, 1);
	if (old >= 1) acquire_sem(draw_sem);	
}


//---------------------------------------------------------------------------


//****************************************
void CScreen::ScreenConnected(bool active)
//****************************************
{
	if (debug) printf("CScreen::ScreenConnected\n");

	lock_draw();
	try
	{
		if (!active)
		{
			if (debug) printf("CScreen::ScreenConnected -- NOT ACTIVE\n");
			// stop drawing.
			drawing_enable = false;
			gApplication->setFullScreenEnable(drawing_enable);
			unlock_draw();
			return;
		}

		if (debug) printf("CScreen::ScreenConnected -- ACTIVE\n");
		if (first)
		{
			if (debug) printf("CScreen::ScreenConnected -- FIRST TIME\n");
			first=false;

/*				DR8-like code that worked for AA:PR DR9
				if (	ProposeFrameBuffer((mode == B_COLOR_8_BIT ? 8 : 32), 640, &height) < B_NO_ERROR
				 || height < 960
				 || SetFrameBuffer(960, 640, 480) < B_NO_ERROR)
				throw(B_ERROR);
*/

			height=480*3;
			//if (	SetFrameBuffer(640, height) < B_NO_ERROR) throw(B_ERROR);
			status_t err = SetFrameBuffer(640, height);
			if (mUmaxHack && err < B_NO_ERROR)
			{
				mMemory = kScreenMemoryOffscreen;
				mOnBoardBuffering = kScreenBufferingSingle;
				mOffscreenBuffering = kScreenBufferingDouble;
				mUseMatrox = false;

				gApplication->mPulsarPref.mMemory = mMemory;
				gApplication->mPulsarPref.mOnBoardBuffering = mOnBoardBuffering;
				gApplication->mPulsarPref.mOffscreenBuffering = mOffscreenBuffering;
				gApplication->mPulsarPref.mMatroxHack = mUseMatrox;
			}

			if (debug)
			{
				printf("proposed height = %d\n", height);
				frame_buffer_info *fi = FrameBufferInfo();
				if (fi)
				{
					printf("FRAME BUFFER INFO\n"
							"bits_per_pixel\t%d\n"
							"bytes_per_row\t%d\n"
							"width\t\t%d\n"
							"height\t\t%d\n"
							"display_width\t%d\n"
							"display_height\t%d\n"
							"display_x\t%d\n"
							"display_y\t%d\n",
							fi->bits_per_pixel, fi->bytes_per_row, fi->width,
							fi->height,
							fi->display_width,
							fi->display_height ,
							fi->display_x,
							fi->display_y);
	
				}

				graphics_card_info *ci = CardInfo();
				if (ci)
				{
					printf("GRAPHIC CARD INFO\n"
							"bits_per_pixel\t%d\n"
							"bytes_per_row\t%d\n"
							"flags\t\t%08x\n"
							"width\t\t%d\n"
							"height\t\t%d\n"
							"rgb_order\t%d %d %d %d\n",
							ci->bits_per_pixel, ci->bytes_per_row,
							ci->width, ci->height,
							ci->flags,
							ci->width,
							ci->height,
							ci->rgba_order[0], ci->rgba_order[1], ci->rgba_order[2], ci->rgba_order[3]);
				}
			}
		} // end of first

		// Each time we get the screen again, we have to reinitialise the
		// double buffer management, redraw the frame_buffer and read all
		// the graphics card parameters.
		mOffscreenPageNum = 0;  				// current display : page 0
		mOnBoardPageNum = 0;
		MoveDisplayArea(0, 0); 					// set page 0 as visible
		row = CardInfo()->bytes_per_row;		// read graphics parameters
		base = (uchar*)CardInfo()->frame_buffer;

		// virtual frame buffer (in 8 or 32 bpp)
		if (!mScreenBuffer)
		{
			assert(row >= 640*(mode == kRgb32Bit ? 4 : 1));

			mScreenBufSize1 = row*480;
			mScreenBufSize2 = ((int32)mOffscreenBuffering)*mScreenBufSize1;
			mScreenBuffer = new uchar[mScreenBufSize2];
		}

		mIsOffscreen = (mMemory == kScreenMemoryOffscreen);

		// set base according to non-visible page : !page_num
		draw_base = base		+ row*480*K_ONBOARD_NEXT(mOnBoardPageNum);
		buffer 	 = mScreenBuffer+ mScreenBufSize1*K_OFFSCREEN_NEXT(mOffscreenPageNum);

		if (getenv("BENCHMARK"))
			benchmark_memory(draw_base, buffer);
		else if (debug) printf("Export BENCHMARK=1 for testing CScreen memory buffer copy\n");

		// get hooks...
		// done by the main program with getHooks().

		// update info for the application...
		gApplication->setFilterRef(640, 480, (mIsOffscreen ? buffer : draw_base),
													 row, mode, this);

		// allow the drawing in the frame buffer (when lock release).
		drawing_enable = true;
		gApplication->setFullScreenEnable(drawing_enable);
		release_sem(initCompleteSem);

		if(debug)
		{
			printf("CScreen Flags -----------\n\tumax %d\n\tbuffering mode : onboard %d, offscreen %d\n\tmemory mode %d\n",
					mUmaxHack, mOnBoardBuffering, mOffscreenBuffering, mMemory);
		}

		Activate(); // doesn't seem to be always the case... in fact just does nothing
		mFakeView->MakeFocus(true);
	}
	catch(...)
	{
		if (debug) printf("CScreen::ScreenConnected -- EXCEPTION THROW\n");
		gApplication->setFullScreenEnable(false);
		delete_sem(initCompleteSem);
		// the application will notice that the sem has been deleted and will
		// destroy the CScreen.
	}
	unlock_draw();

} // end of ScreenConnected for CScreen


//**************************************************************
void CScreen::getInfos(int32 &sx, int32 &sy, EColorSpace &_mode,
						uint8	* &_buffer, int32 &bpr,
						int32 &maxLine, int32 &pulsarLine)
//**************************************************************
{
	sx = 640;
	sy = 480;
	_mode = mode;
	_buffer = (mIsOffscreen ? buffer : draw_base);
	bpr = row;
	maxLine = height;
	pulsarLine = 2*480;
} // end of getInfos for CScreen


//**********************************
void CScreen::getHooks(CFilter &ref)
//**********************************
/*
	The hooks I want to grab :
typedef long (*line_8_proto)(long x1, long x2, long y1, long y2, uchar color,
			   bool useClip, short clipLeft, short clipTop, short clipRight, short clipBottom);
typedef long (*line_32_proto)(long x1, long x2, long y1, long y2, ulong color,
				bool useClip, short clipLeft, short clipTop, short clipRight, short clipBottom);
typedef long (*rect_8_proto)(long x1, long y1, long x2, long y2, uchar color);
typedef long (*rect_32_proto)(long x1, long y1, long x2, long y2, ulong color);
typedef long (*blit_proto)(long x1, long y1, long x2, long y2, long width, long height);
typedef long (*invert_rect_32_proto)(long x1, long y1, long x2, long y2);

at index :
// 0
	(graphics_card_hook)set_cursor_shape,
	(graphics_card_hook)move_cursor,
	(graphics_card_hook)show_cursor,
	(graphics_card_hook)line_8,
	(graphics_card_hook)line_32,
// 5
	(graphics_card_hook)rect_8,
	(graphics_card_hook)rect_32,
	(graphics_card_hook)blit,
	0L,
	0L,
// 10
	0L,
	(graphics_card_hook)invert_rect_32,

*/
{
	if (!mIsOffscreen)
	{
		ref.line_8			= (line_8_proto			)CardHookAt(3);
		ref.line_32			= (line_32_proto		)CardHookAt(4);
		ref.rect_8			= (rect_8_proto			)CardHookAt(5);
		ref.rect_32			= (rect_32_proto		)CardHookAt(6);
		ref.blit			= (blit_proto			)CardHookAt(7);
		ref.invert_rect_32	= (invert_rect_32_proto	)CardHookAt(11);
	}
	else
	{
		ref.line_8				= NULL;
		ref.line_32				= NULL;
		ref.rect_8				= NULL;
		ref.rect_32				= NULL;
		ref.blit					= NULL;
		ref.invert_rect_32	= NULL;
	}

	if (debug) printf("Hooks:\n\tline8\t%p\n\tline32\t%p\n\trect8\t%p\n\trect32\t%p\n\tblit\t%p\n\tinvert\t%p\n",
		ref.line_8,ref.line_32,ref.rect_8,ref.rect_32,ref.blit,ref.invert_rect_32);

}

//***********************************************
void CScreen::swap(uint8 * &screen, uint16 &line)
//***********************************************
{
	// buffer is the frame buffer filled by the caller.
	// if not NULL, must be copied into the real video frame buffer
	// which is currently pointed by draw_base
	if (mIsOffscreen)
	{
		memcpy(draw_base, screen, row*480);
		/*
		if (mode == B_COLOR_8_BIT)
		{
			// 8 bit mode : same format
			// memcpy_c2nc((mUmax ? base : draw_base), screen, row*480);
			memcpy_c2nc(draw_base, screen, row*480);
		}
		else if (mode == B_RGB_16_BIT)
		{
			memcpy_c2nc(draw_base, screen, row*480);
		}
		else
		{
			// mode == B_RGB_32_BIT --> buffer 16 bits, ecran 32 
			uint16 *src = ((uint16 *)screen)-1;
			double *base = ((double *)draw_base)-1;
			
			for(uint32 count=640*480; count>0; count-=2)
			{
				double temp;
				uint32 *t = (uint32 *)&temp;

				* t    = table[*(++src)];
				*(t+1) = table[*(++src)];
				*(++base) = temp;
			}
		}
		*/

		// set base according to the next non-visible page : page_num
		draw_base = base		+ row*480*mOnBoardPageNum;
		screen 	 = mScreenBuffer+ mScreenBufSize1*mOffscreenPageNum;
		// page num is the currently visible page, change it
		mOnBoardPageNum 	= K_ONBOARD_NEXT(mOnBoardPageNum);
		mOffscreenPageNum	= K_OFFSCREEN_NEXT(mOffscreenPageNum);
		// set display to new visible page
		line = 480*mOnBoardPageNum;
		BScreen().WaitForRetrace();
		MoveDisplayArea(0,line);
	}
	else
	{
		// no offscreen buffering is used here -- directly draw into the onbaord memory
		// next page visible
		mOnBoardPageNum = K_ONBOARD_NEXT(mOnBoardPageNum);
		// set display to new visible page
		BScreen().WaitForRetrace();
		MoveDisplayArea(0,480*mOnBoardPageNum);

		// set base according to the next non-visible page : page_num
		line = 480*K_ONBOARD_NEXT(mOnBoardPageNum);
		draw_base = base + row*line;
		screen 	 = draw_base;

		//if (mUmax) memcpy_c2nc(base, screen, row*480);
	}


} // end of swap for CScreen


//---------------------------------------------------------------------------


//*****************************************************************
void CScreen::DispatchMessage(BMessage *message, BHandler *handler)
//*****************************************************************
{
	if (!gApplication->handleMessageAddons(message))
		inherited::DispatchMessage(message, handler);
}

//---------------------------------------------------------------------------

//***************************
void CScreen::test_draw(void)
//***************************
{
uchar *d = (uchar*)CardInfo()->frame_buffer;
short row = CardInfo()->bytes_per_row; // read graphics parameters

	if (debug) printf("test draw screen %p, row %d\n", d, row);

	for(long j=0; j<256; j++)
	{
		memset(d, j, row/2);
		d+=row;
	}

} // end of test_draw for CScreen


void memcpy_double(uchar *dst, uchar *src, const uint32 len)
{
		#define mod sizeof(double)
		double *d = (double *)dst;
		double *s = (double *)src;
		uint32 n1 = len/mod;
		uint32 n2 = len & (mod-1);
		while(n1--) *(d++) = *(s++);
		if (n2)
		{
			uchar *d2 = (uchar *)d;
			uchar *s2 = (uchar *)s;
			while (n2--) *(d2++) = *(s2++);
		}
		#undef mod
}

void memcpy_int32(uchar *dst, uchar *src, const uint32 len)
{
		#define mod sizeof(int32)
		uint32 *d = (uint32 *)dst;
		uint32 *s = (uint32 *)src;
		uint32 n1 = len/mod;
		uint32 n2 = len & (mod-1);
		while(n1--) *(d++) = *(s++);
		if (n2)
		{
			uchar *d2 = (uchar *)d;
			uchar *s2 = (uchar *)s;
			while (n2--) *(d2++) = *(s2++);
		}
		#undef mod
}

class CStopWatch
{
public:
	CStopWatch(const char *t, int32 l) : text(t), len(l) { t1 = system_time(); }
	~CStopWatch(void)
	{
		bigtime_t t2 = system_time()-t1;
		printf("%s : %.2f ko, %ld us -- %.2f o/us -- %.2f ko/s\n",
			text, (float)len/1024.0, (int32)t2, (float)len/(float)t2, (float)len/1024.0*1e6/(float)t2);
		fflush(stdout);
	}
protected:
	bigtime_t t1;
	const char *text;
	int32 len;
};

void test_copy1(uchar *dst, uchar *src, int32 len, char *s1)
{
	if (s1) printf("%s\n", s1);
	{
		CStopWatch t2("  double", len);
		memcpy_double(dst, src, len);
	}
	{
		CStopWatch t1("  memcpy", len);
		memcpy(dst, src, len);
	}
	{
		CStopWatch t2("  int32 ", len);
		memcpy_int32(dst, src, len);
	}
}

//*********************************************************
void CScreen::benchmark_memory(uchar *video, uchar *memory)
//*********************************************************
{
	int32 block;

	// for 8 bpp the incoming buffer is certified to be able to hold the memory
	// but not for 32
	memory = new uchar[640*480*4*2];

	printf("benchmark -- video %p -- memory %p\n", video, memory);
	printf("alignement des adr : double -> %d, int32 -> %d\n", (uint32)video % sizeof(double), (uint32)video % sizeof(int32));
	fflush(stdout);

	printf("--- 8 BPP screen ---\n");
	block=640*480;
	test_copy1(memory, memory+block, block, "[mem->mem     ]");
	test_copy1(memory, video, block, 		"[memory->video]");
	test_copy1(video, video+block, block,	"[video->video ]");
	test_copy1(video, memory, block,		"[video->memory]");

	printf("--- 32 BPP screen ---\n");
	block=640*480*4;
	test_copy1(memory, memory+block, block, "[mem->mem     ]");
	test_copy1(memory, video, block, 		"[memory->video]");
	test_copy1(video, video+block, block,	"[video->video ]");
	test_copy1(video, memory, block,		"[video->memory]");

	printf("benchmark test done\n");
	fflush(stdout);
	delete memory;
}



//---------------------------------------------------------------------------

// eoc
