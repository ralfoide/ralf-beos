/****************
*	main.h	*
****************/

#ifndef _H_MAIN
#define _H_MAIN

#include <Application.h>
#include "gCThread.h"

class CFilledWin;
extern class CApplication *gApplication;
extern bool debug;

//------------------------------------------------

#define K_APP_NAME "filled-win-sample"

//------------------------------------------------

class CApplication : public BApplication
{
public:
							CApplication();
virtual				~CApplication() {}

				bool	init(void);
				void	calcul_mandel(void);
				void	calcul_bifurc(void);
				void	calcul_lyapunov(int32 y1, int32 y2);

static	void	launch_calcul(void);

virtual	bool	QuitRequested(void);
virtual	void	DispatchMessage(BMessage *message, BHandler *handler);

				#define NBT 2
				CThread			mCalculThread[NBT];
				CFilledWin *mFilledWin;
				int32				mSx;
				int32				mSy;
				color_space	mMode;
				uint8	*			mBuffer;
				int32				mBpr;
				bool				mCalculMustQuit;
				char				mFormula;
};

//------------------------------------------------


#endif // _H_MAIN
/* eof */