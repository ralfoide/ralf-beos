/*****************************************************************************

	Projet	: Pulsar

	Fichier: CMatroxScreen.cpp
	Partie	: Loader

	Auteur	: RM
	Date		: 281097
	Format	: tabs==2


*****************************************************************************/

#include "CMatroxScreen.h"
#include "CPulsarApp.h"

#include <PCI.h>

// Matrox's PCI vendor code
#define MATROX_VENDOR	0x102b
// the original Millennium
#define MGA_2064W		0x0519
// the Mystique (both original and -220 versions)
#define MGA_1064S		0x051a
// the Millennium-II
#define MGA_2164W		0x051b

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export on
#endif

//---------------------------------------------------------------------------

long gMatroxEcrireIndex3[3] = {1,2,0};
#define K_OFFSCREEN_NEXT(x) (mOffscreenBuffering == kScreenBufferingDouble ? (!x) : \
									(mOffscreenBuffering == kScreenBufferingTripple ? gMatroxEcrireIndex3[(x)] : 0))
#define K_ONBOARD_NEXT(x) (mOnBoardBuffering == kScreenBufferingDouble ? (!x) : \
									(mOnBoardBuffering == kScreenBufferingTripple ? gMatroxEcrireIndex3[(x)] : 0))


void test_pci(void);
void test_pci(void)
{
	int i = 0;
	pci_info pcii;

	throw "HOOK RM 161198 -- R4b4 ** PCI interface changed ?";
/* -- HOOK RM 161198 -- R4b4
	while (get_nth_pci_info(i++, &pcii) == B_NO_ERROR)
	{
		int vid=pcii.vendor_id;
		int did=pcii.device_id;
		printf("slot %d --> Vendor %5d [0x%04x] -- Device %5d [0x%04x]\n"
		       "            bus %d, device %d, function %d, revision %d\n"
		       "            class_api %d, class_sub %d, class_base %d\n",
					i-1,vid,vid,did,did,
					pcii.bus, pcii.device, pcii.function, pcii.revision, 
					pcii.class_api, pcii.class_sub, pcii.class_base);
	}
*/

	/*
		Typical answers :
		
		IMS128:
					slot 2 --> Vendor  4320 [0x10e0] -- Device 37160 [0x9128]
            bus 0, device 14, function 0, revision 1
            class_api 0, class_sub 128, class_base 3
            [class display sub class other display]

		S3:
					slot 4 --> Vendor 21299 [0x5333] -- Device 34833 [0x8811]
            bus 1, device 1, function 0, revision 84
            class_api 0, class_sub 0, class_base 3
            [class display sub class vga display]
	
		Matrox Millenium:
					slot 4 --> Vendor  4139 [0x102b] -- Device  1305 [0x0519]
            bus 1, device 1, function 0, revision 1
            class_api 0, class_sub 0, class_base 3
	*/
}


//***************************************************************************
CMatroxScreen::CMatroxScreen(const char *title, ulong _space)
			  :BWindow(BRect(20,20,20+64,20+64),
								title, 
								B_DOCUMENT_WINDOW,
								B_NOT_RESIZABLE|B_NOT_CLOSABLE|B_NOT_ZOOMABLE|B_NOT_MINIMIZABLE)
#define inherited BWindow
//***************************************************************************
{
	space = _space;
	if (_space == B_8_BIT_640x480) mode = kColor8Bit;
	else mode = kRgb32Bit; // for _space B_32_BIT_640x480
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
	mMemory = kScreenMemoryOffscreen;
	mOnBoardBuffering = kScreenBufferingDouble;
	mOffscreenBuffering = kScreenBufferingDouble;
	mUmaxHack = false;
	mUseMatrox = false;

	mFakeView = new BView(Bounds(), "", 0, 0);
	AddChild(mFakeView);

	// private driver stuff
	char *driverAddOnPath = getenv("MATROX_ADDON");
	if (debug) printf("Please 'export MATROX_ADDON=\"/boot/beos.../matrox\"' first\n");
	if (!driverAddOnPath) driverAddOnPath = "/boot/beos/system/add-ons/app_server/matrox09_16bpp";
	printf("USING MATROX DRIVER PATH : %s\n", driverAddOnPath);

	long result;
	status_t status;

	iid = load_add_on(driverAddOnPath);
	if (iid < B_NO_ERROR)
	{
		printf("Couldn't load matrox driver add-on! %d\n", iid);
		throw(B_ERROR);
	}

	status = get_image_symbol(iid, "control_graphics_card__FUlPv", B_SYMBOL_TYPE_TEXT, (void **)&cgc);
	if (status < B_NO_ERROR)
	{
		printf("Couldn't get symbol from add-on! %d\n", status);
		return;
	}

	long i=0;
	throw "HOOK RM 161198 -- R4b4 ** PCI interface changed ?";
/* -- HOOK RM 161198 -- R4b4
	while (get_nth_pci_info(i++, &pcii) == B_NO_ERROR)
*/
	while(0)
	{
		if ((pcii.vendor_id == MATROX_VENDOR) && (pcii.device_id == MGA_2064W))
		{
			printf("Found matrox card at pci slot %d\n", i-1);
			// configure the card
			gcs.vendor_id = pcii.vendor_id;
			gcs.device_id = pcii.device_id;
			gcs.screen_base = (void *)pcii.u.h0.base_registers[0];

			result = (cgc)(B_OPEN_GRAPHICS_CARD, &gcs);
			printf("result of control_graphics_card(B_OPEN_GRAPHICS_CARD): %d\n", result);

			gcc.space = B_8_BIT_640x480; // B_16_BIT_1024x768;
			gcc.refresh_rate = 60;
			gcc.h_position = gcc.v_position = gcc.h_size = gcc.v_size = 50;

			result = (cgc)(B_CONFIG_GRAPHICS_CARD, &gcc);
			printf("result of control_graphics_card(B_CONFIG_GRAPHICS_CARD): %d\n", result);

			result = (cgc)(B_GET_GRAPHICS_CARD_INFO, &gci);
			printf("result of control_graphics_card(B_GET_GRAPHICS_CARD_INFO): %d\n", result);
			printf("frame buffer: 0x%08x\n", gci.frame_buffer);

			result = (cgc)(B_GET_GRAPHICS_CARD_HOOKS, &hooks);
			printf("result of control_graphics_card(B_GET_GRAPHICS_CARD_HOOKS): %d\n", result);

			fbi.bits_per_pixel = 8;
			fbi.bytes_per_row = 640;
			fbi.width = 640;
			fbi.height = 480;
			fbi.display_width = 640;
			fbi.display_height = 480;
			fbi.display_x = 0;
			fbi.display_y = 0;

			if(1)
			{
				BScreen screen;
				indexed_color ic;
				// set up the same palette than the BeOS one
				
				int32 i;
				for(i=0; i<256; i++)
				{
					ic.index = i;
					ic.color = screen.ColorMap()->color_list[i];
					result = (cgc)(B_SET_INDEXED_COLOR, &ic);
				}
			}

			//dr = (draw_rect_1555_bit)hooks[41];
			//(dr)(0,0,1023,767, 0x7c00);
			//dl = (draw_line_1555_bit)hooks[40];
			//ir = (invert_rect)hooks[11];
			//(ir)(100, 50, 500, 500);

			mUseMatrox = true;
			break;
		}
	}

	if (!mUseMatrox)
	{
		printf("Matrox card not found on the PCI bus\nDump of PCI bus follows :\n\n");
		test_pci();
		throw(B_ERROR);
	}

} // end of constructor for CMatroxScreen


//***************************************************************************
CMatroxScreen::~CMatroxScreen(void)
//***************************************************************************
{
	if (debug) printf("CMatroxScreen::~CMatroxScreen -- destructor\n");
	if (iid >= B_NO_ERROR) unload_add_on(iid);
	iid = B_ERROR;
	// never call Quit() here
} // end of destructor for CMatroxScreen


//***************************************************************************
bool CMatroxScreen::QuitRequested(void)
//***************************************************************************
{
	if (debug) printf("CMatroxScreen::QuitRequested\n");
	return false;
	
} // end of Quit for CMatroxScreen


//***************************************************************************
void CMatroxScreen::Quit(void)
//***************************************************************************
{
	if (debug) printf("CMatroxScreen::Quit\n");
	if (draw_sem >= B_NO_ERROR) delete_sem(draw_sem);
	if (initCompleteSem >= B_NO_ERROR) delete_sem(initCompleteSem);
	if (mScreenBuffer) delete mScreenBuffer;

	inherited::Quit();
} // end of Quit for CMatroxScreen


//---------------------------------------------------------------------------



//***************************************************************************
bool CMatroxScreen::init(void)
//***************************************************************************
{
	if (!mFakeView) return false;

	draw_lock = 0L;
	draw_sem = create_sem(0,"draw_sem");
	if (draw_sem < B_NO_ERROR) return false;

	initCompleteSem = create_sem(0,"init_sem");
	if (initCompleteSem < B_NO_ERROR) return false;

	Show();
	return true;
} // end of init for CMatroxScreen


//---------------------------------------------------------------------------


//***************************************************************************
void CMatroxScreen::unlock_draw(void)
//***************************************************************************
// Release the protection on the general io-registers
// (from S3 driver sample code, DR8.2, by Be Inc)
{
	int32	old;

	old = atomic_add (&draw_lock, -1);
	if (old > 1) release_sem(draw_sem);
}	


//***************************************************************************
void CMatroxScreen::lock_draw(void)
//***************************************************************************
// Protect the access to the memory or the graphic engine registers by
// locking a benaphore.
// (from S3 driver sample code, DR8.2, by Be Inc)
{
	int32	old;

	old = atomic_add (&draw_lock, 1);
	if (old >= 1) acquire_sem(draw_sem);	
}


//---------------------------------------------------------------------------


//***************************************************************************
void CMatroxScreen::ScreenConnected(bool active)
//***************************************************************************
{
	if (debug) printf("CMatroxScreen::ScreenConnected\n");

	Lock();
	lock_draw();
	try
	{
		if (!active)
		{
			if (debug) printf("CMatroxScreen::ScreenConnected -- NOT ACTIVE\n");
			// stop drawing.
			drawing_enable = false;
			gApplication->setFullScreenEnable(drawing_enable);
			unlock_draw();
			Unlock();
			return;
		}

		if (debug) printf("CMatroxScreen::ScreenConnected -- ACTIVE\n");
		if (first)
		{
			if (debug) printf("CMatroxScreen::ScreenConnected -- FIRST TIME\n");
			first=false;

/*				DR8-like code that worked for AA:PR DR9
				if (	ProposeFrameBuffer((mode == B_COLOR_8_BIT ? 8 : 32), 640, &height) < B_NO_ERROR
				 || height < 960
				 || SetFrameBuffer(960, 640, 480) < B_NO_ERROR)
				throw(B_ERROR);
*/

			height=480*3;
			//if (	SetFrameBuffer(640, height) < B_NO_ERROR) throw(B_ERROR);

			if (debug)
			{
				printf("proposed height = %d\n", height);
				frame_buffer_info   *fi = FrameBufferInfo();
				if (fi)
				{
					if (debug) printf(" bits_per_pixel %d\n"
							"bytes_per_row  %d\n"
							"width %d\n"
							"height %d\n"
							"display_width %d\n"
							"display_height %d\n"
							"display_x %d\n"
							"display_y %d\n",
							fi->bits_per_pixel, fi->bytes_per_row, fi->width,
							fi->height,
							fi->display_width,
							fi->display_height ,
							fi->display_x,
							fi->display_y);
	
				}
			}
		} // end of first

		// Each time we get the screen again, we have to reinitialise the
		// double buffer management, redraw the frame_buffer and read all
		// the graphics card parameters.
		mOffscreenPageNum = 0;  					// current display : page 0
		mOnBoardPageNum = 0;
		MoveDisplayArea(0, 0); 						// set page 0 as visible
		row = CardInfo()->bytes_per_row;		// read graphics parameters
		base = (uchar*)CardInfo()->frame_buffer;

		// virtual frame buffer (in 8 or 16 bpp)
		mScreenBufSize1 = row*480*(mode == kColor8Bit ? 1 : 4);
		mScreenBufSize2 = ((int32)mOffscreenBuffering)*mScreenBufSize1;
		mScreenBuffer = new uchar[mScreenBufSize2];

		mIsOffscreen = (mMemory == kScreenMemoryOffscreen);

		// set base according to non-visible page : !page_num
		draw_base = base					+ row*480*K_ONBOARD_NEXT(mOnBoardPageNum);
		buffer 	 = mScreenBuffer	+ mScreenBufSize1*K_OFFSCREEN_NEXT(mOffscreenPageNum);

		// get hooks...
		// done by the main program with getHooks().

		// update info for the application...
		gApplication->setFilterRef(640, 480, (mIsOffscreen ? buffer : draw_base),
													 row, kColor8Bit, (BWindowScreen *)this);

		// allow the drawing in the frame buffer (when lock release).
		drawing_enable = true;
		gApplication->setFullScreenEnable(drawing_enable);
		release_sem(initCompleteSem);

		if(debug)
		{
			printf("CMatroxScreen Flags -----------\n\tumax %d\n\tbuffering mode : onboard %d, offscreen %d\n\tmemory mode %d\n",
						mUmaxHack, mOnBoardBuffering, mOffscreenBuffering, mMemory);
		}

		Activate(); // doesn't seem to be always the case... in fact just does nothing
		mFakeView->MakeFocus(true);
	}
	catch(...)
	{
		if (debug) printf("CMatroxScreen::ScreenConnected -- EXCEPTION THROW\n");
		gApplication->setFullScreenEnable(false);
		delete_sem(initCompleteSem);
		// the application will notice that the sem has been deleted and will
		// destroy the CMatroxScreen.
	}
	unlock_draw();
	Unlock();

} // end of ScreenConnected for CMatroxScreen


//***************************************************************************
void CMatroxScreen::getInfos(int32 &sx, int32 &sy, EColorSpace &_mode,
												uint8	* &_buffer, int32 &bpr,
												int32 &maxLine, int32 &pulsarLine)
//***************************************************************************
{
	sx = 640;
	sy = 480;
	_mode = mode;
	_buffer = (mIsOffscreen ? buffer : draw_base);
	bpr = row;
	maxLine = height;
	pulsarLine = 2*480;
} // end of getInfos for CMatroxScreen


//***************************************************************************
void CMatroxScreen::getHooks(CFilter &ref)
//***************************************************************************
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
		ref.line_8				= (line_8_proto				)CardHookAt(3);
		ref.line_32				= (line_32_proto				)CardHookAt(4);
		ref.rect_8				= (rect_8_proto				)CardHookAt(5);
		ref.rect_32				= (rect_32_proto				)CardHookAt(6);
		ref.blit					= (blit_proto					)CardHookAt(7);
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

//***************************************************************************
void CMatroxScreen::swap(uint8 * &screen, uint16 &line)
//***************************************************************************
{
	// buffer is the frame buffer filled by the caller.
	// if not NULL, must be copied into the real video frame buffer
	// which is currently pointed by draw_base
	if (mIsOffscreen)
	{
		memcpy(draw_base, screen, row*480);

/*
		if (mode == kColor8Bit)
		{
			// 8 bit mode : same format
			// memcpy_c2nc((mUmax ? base : draw_base), screen, row*480);
//			memcpy_c2nc(draw_base, screen, row*480);
#ifdef __POWERPC__
			memcpy_c2nc(draw_base, screen, row*480);
#elif __INTEL__
			memcpy(draw_base, screen, row*480);
#endif
		}
		else
		{
			// mode == kRgb32Bit
			uint32 *src = ((uint32 *)screen)-1;
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
		draw_base = base					+ row*480*mOnBoardPageNum;
		screen 	 = mScreenBuffer	+ mScreenBufSize1*mOffscreenPageNum;
		// page num is the currently visible page, change it
		mOnBoardPageNum 		= K_ONBOARD_NEXT(mOnBoardPageNum);
		mOffscreenPageNum	= K_OFFSCREEN_NEXT(mOffscreenPageNum);
		// set display to new visible page
		line = 480*mOnBoardPageNum;
		MoveDisplayArea(0,line);
	}
	else
	{
		// no offscreen buffering is used here -- directly draw into the onbaord memory
		// next page visible
		mOnBoardPageNum 		= K_ONBOARD_NEXT(mOnBoardPageNum);
		// set display to new visible page
		MoveDisplayArea(0,480*mOnBoardPageNum);

		// set base according to the next non-visible page : page_num
		line = 480*K_ONBOARD_NEXT(mOnBoardPageNum);
		draw_base = base + row*line;
		screen 	 = draw_base;

		//if (mUmax) memcpy_c2nc(base, screen, row*480);
	}


} // end of swap for CMatroxScreen


//---------------------------------------------------------------------------

//***************************************************************************
void CMatroxScreen::test_draw(void)
//***************************************************************************
{
uchar *d = (uchar*)CardInfo()->frame_buffer;
short row = CardInfo()->bytes_per_row; // read graphics parameters

	if (debug) printf("test draw screen %p, row %d\n", d, row);

	for(long j=0; j<256; j++)
	{
		memset(d, j, row/2);
		d+=row;
	}

} // end of test_draw for CMatroxScreen


//***************************************************************************
void CMatroxScreen::DispatchMessage(BMessage *message, BHandler *handler)
//***************************************************************************
{
	//if (debug) printf("Screen ### DispatchMessage\nmsg=");
	//message->PrintToStream();
	if (message->what == B_KEY_DOWN)
	{
		//gApplication->PostMessage(K_MSG_STOP_STREAM);
		char byte;
		message->FindInt8("byte", (int8 *)&byte);
		gApplication->touches(&byte, 1, message);
	}
	else
		inherited::DispatchMessage(message, handler);
}

//---------------------------------------------------------------------------


//***************************************************************************
void CMatroxScreen::MoveDisplayArea(short x, short y)
//***************************************************************************
{
	long result;
	
	fbi.display_x = x;
	fbi.display_y = y;
	
	result = (cgc)(B_MOVE_DISPLAY_AREA, &fbi);
}

#ifdef K_USE_PRAGMA_EXPORT
	#pragma export reset
#endif

//---------------------------------------------------------------------------

// eoc
