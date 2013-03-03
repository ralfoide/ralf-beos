/****************
*	main.cpp	*
****************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "main.h"
#include "CFilledWin.h"
#include "CFilledView.h"
#include "gCSem.h"

#include <Alert.h>

bool debug;
CApplication *gApplication = NULL;

//------------------------------------------------

//***************************
CApplication::CApplication(void)
						 :BApplication("application/x-vnd.ralf.filled-win-sample")
//***************************
{
	mFilledWin = NULL;
	mSx = mSy = 0;
	mBuffer = NULL;
	mBpr = 0;
	mCalculMustQuit = false;
	mFormula = 'm';
}

CSem gSem;

//***************************
bool CApplication::init(void)
//***************************
{
	// create a new window with a bitmap
	mFilledWin = new CFilledWin(BRect(10,10,10+639,10+479), "Filled Sample",
													B_TITLED_WINDOW, B_NOT_RESIZABLE);

	if (!mFilledWin) return false;

	// then don't forget to init my class -- all my classes need to be initialized
	// after their construction
	mFilledWin->init();

	// this created a window containing a view that can blit its own bitmap.
	// now get info about the bitmap created :
	mFilledWin->getInfos(mSx, mSy, mMode, mBuffer, mBpr);

	// run the thread
	gSem.init();
	for(int32 i=0; i<NBT; i++)
	{
		mCalculThread[i].init();
		mCalculThread[i].launch(launch_calcul, THREAD_PRI_IDLE);
	}

	return true;
} // end of init for CApplication



//************************************
bool CApplication::QuitRequested(void)
//************************************
{
	// tell calcul threads to quit
	mCalculMustQuit = true;
	for(int32 i=0; i<NBT; i++)
	{
		mCalculThread[i].askToKill();
		// wait until they all quit
		mCalculThread[i].waitForEnd();
	}

	// salvagely close the window
	if (mFilledWin && mFilledWin->Lock()) mFilledWin->Quit();
	mFilledWin = NULL;

	// and tell that the app can quit
	return BApplication::QuitRequested();
}


//**********************************************************************
void CApplication::DispatchMessage(BMessage *message, BHandler *handler)
//**********************************************************************
{
	/*
	// this is an arbitrary test
	// when a message is received, change something in the bitmap
	// and redraw it

	if (mFilledWin && mBuffer)
	{
		uint8 color=rand() & 0xFF;	// get a random color

		// the size of the buffer is mBpr*mSy, it can be bigger than mSx*mSy
		memset(mBuffer, color, mBpr*mSy);

		// force the redraw of the window -- this call is SYNCHRONOUS
		// this calls CFilledWin::forceRedraw() which in turn directly
		// calls CFilledView::Pulse() which calls CFilledView::Draw()
		mFilledWin->forceRedraw();
		
		// this is an asynchronous redraw
		// this sends a message to the window which execute the same calls
		// than above but the drawing is done in the BWindow thread, not this thread.
		// Of course this adds an overhead for the messaging.
		mFilledWin->PostMessage(K_MSG_STREAM);
	}
	*/

	// call the superclass handler to process the message
	BApplication::DispatchMessage(message, handler);

} // end of DispatchMessage for CApplication


//------------------------------------------------


//************************************
void CApplication::launch_calcul(void)
//************************************
{
	if (gApplication->mFormula == 'l')
		gApplication->calcul_lyapunov(0, 1);
	else if (gApplication->mFormula == 'b')
		gApplication->calcul_bifurc();
	else
		gApplication->calcul_mandel();
}


//************************************
void CApplication::calcul_mandel(void)
//************************************
{
	assert(mBuffer && mSx && mSy);
	assert(mMode == B_COLOR_8_BIT);

	int32 sx = mSx;
	int32 sy = mSy;
	int32 bpr = mBpr;

	int32 x,y;
	uint8 *line_buf = mBuffer;

	// first, do a mandelbrot to check the code

	const float amin = 2;
	const float amax =(1.5 + amin)/(float)sx;
	const float bmin = 1.5;
	const float bmax =(1.5 + bmin)/(float)sy;

	for(y=0; y<sy && !mCalculMustQuit; y++, line_buf += bpr)
	{
		uint8 *pixel = line_buf;
		for(x=0; x<sx && !mCalculMustQuit; x++, pixel++)
		{
			const float a=amin-x*amax;
			const float b=bmin-y*bmax;
			float xx=0, yy=0;
			int32 k=0;

			for(; xx*xx+yy*yy<4.0 && k<254; k++)
			{
				float xp = xx*xx-yy*yy-a;
				yy = 2*xx*yy-b;
				xx = xp;
			}

			*pixel = k;
		}
		mFilledWin->PostMessage(K_MSG_STREAM);
	}
}


//**************************************
void CApplication::calcul_lyapunov(int32 y1_, int32 y2_)
//**************************************
{
	assert(mBuffer && mSx && mSy);
	assert(mMode == B_COLOR_8_BIT);

	int32 sx = mSx;
	int32 sy = mSy;
	int32 bpr = mBpr;

gSem.acquire();
	static int32 nb_thread = 0;
	assert(nb_thread<2);
	int32 y1 = (sy/2)*nb_thread;
	int32 y2 = y1+(sy/2);
	nb_thread++;
printf("thread %ld\n", nb_thread);
gSem.release();

	int32 x,y;
	uint8 *line_buf = mBuffer + bpr * y1;

	// do a lyapunov
	const float amin = 0;
	const float amax =(4-amin)/(float)sx;
	const float bmin = 0;
	const float bmax =(4-bmin)/(float)sy;

	static float min_exp = 0;
	static float max_exp = 0;

	for(y=y1; y<y2 && !mCalculMustQuit; y++, line_buf += bpr)
	{
		uint8 *pixel = line_buf;
		for(x=0; x<sx && !mCalculMustQuit; x++, pixel++)
		{
			const float a=amin+x*amax;
			const float b=bmin+y*bmax;
			const float seed = 0.66;
			float lexp = 0;
			float p = seed;

			const int32 n1 = 256;
			const int32 n2 = 256;
			int32 i;

			for(i=0; i<n1; i++)
			{
				float r = (i&1 ? b : a);
				p = r * p * (1.-p);
			}

			for(i=0; i<n2; i++)
			{
				float r = (i&1 ? b : a);
				p = r * p * (1.-p);
				lexp += log(fabs(r-2*r*p));
			}
			lexp /= (float)n2;

			uint8 color= 0 ;	// 0=black, 31=white, 1->30=gray
			//if (lexp > 0) color = 0;	// assign black to lexp>0 (default)
			if (fabs(lexp)<1e-6)				// assign white to lexp=0
				color = 31;
			else if (lexp < 0)					// assign gray: -inf=black up to 0=white
				#define coef 8
				color = (uint8)(lexp < - coef ? 0 : 31+lexp*30./coef);
				//color = (uint8)(lexp);

//printf("color = % 3d, exp = %f\n", color, lexp);

			if (lexp < min_exp && isfinite(lexp)) min_exp = lexp;
			if (lexp > max_exp) max_exp = lexp;

			*pixel = color;
		}
		mFilledWin->PostMessage(K_MSG_STREAM);
	}

printf("exp : min = %f, max = %f\n", min_exp, max_exp);
}


//**************************************
void CApplication::calcul_bifurc(void)
//**************************************
{
	assert(mBuffer && mSx && mSy);
	assert(mMode == B_COLOR_8_BIT);

	int32 sx = mSx;
	int32 sy = mSy;
	int32 bpr = mBpr;

	int32 x;
	uint8 *line_buf = mBuffer;

	// do a bifurcation fractal
	const float amin = 0.;
	const float amax = 4./(float)sx;

	for(x=0; x<sx && !mCalculMustQuit; x++, line_buf++)
	{
		const float r=amin+x*amax;
		const float seed = 0.66;
		const int32 n1 = 256;
		const int32 n2 = 100;
		int32 i;

		float p = seed;
		for(i=0; i<n1; i++) p = r * p * (1.-p);

		for(i=0; i<n2; i++)
		{
			p = r * p * (1.-p);
			int32 y= (int32)(p*sy);
			if (y >= 0 && y < sy) line_buf[bpr*y] = 31;
		}

		mFilledWin->PostMessage(K_MSG_STREAM);
	}
}



//------------------------------------------------


//*****************************
int main(int argc,char *argv[])
//*****************************
{
	CApplication gApp;

	debug = (argc>2);

	gApplication = &gApp; // hairy and ballsy but works... 

	if (argc < 2)
		printf("Usage: %s [m | l]\n- m = mandelbrot (default)\n- l = lyapunov- b = bifurcation\n\n", argv[0]);
	else
		gApplication->mFormula = argv[1][0];

	try
	{
		if (gApp.init()) gApp.Run();
		else throw("Error during initialization...\n");
	}
	catch(status_t err)
	{
		BAlert *box;
		char s2[256];
		sprintf(s2,	K_APP_NAME " Error\n"
							"Exception catched : %ld (%08lxh)\n"
							"(can be a BeOS error number)", (int32)err, (int32)err);
		box = new BAlert("Abcd - Error box", s2, "    OK    ", NULL, NULL,
									B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
		if (box) box->Go();
	}
	catch(char *s)
	{
		BAlert *box;
		char s2[1024];
		if (strlen(s) > 800) s[800] = '\0';	// ugly but safe... :-/
		sprintf(s2,	K_APP_NAME " Error\n"
							"Exception catched : %s\n", s);
		box = new BAlert(K_APP_NAME " Error", s2, "    OK    ", NULL, NULL,
									B_WIDTH_FROM_WIDEST, B_STOP_ALERT);
		if (box) box->Go();
	}
	catch(...)
	{
		BAlert *box = new BAlert(K_APP_NAME " Error",
													K_APP_NAME " Error\nUnhandled Exception thrown,"
													"\ncatched in main()",
													"   OK   ");
		if(box) box->Go();
	}

	return 0;
}

/* eof */