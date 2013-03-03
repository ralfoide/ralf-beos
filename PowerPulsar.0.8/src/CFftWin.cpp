/*****************************************************************************

	Projet	: Pulsar

	Fichier	: CPulsarWin.cpp
	Partie	: Loader

	Auteur	: RM
	Date		: 140297
	Format	: tabs==2

*****************************************************************************/

#include "CFftWin.h"
#include "CFftView.h"
#include "CSoundView.h"
#include "CPulsarApp.h"

//---------------------------------------------------------------------------


//***********************************************************
CFondFftView::CFondFftView(BRect frame, char *title, long resize, long flags, bool chan)
				 			:BView(frame, title, resize, flags)
//***********************************************************
{
	mFond = gApplication->fondFft();
	SetViewColor(0,0,0);
	
}


//***********************************************************
void CFondFftView::Draw(BRect upd)
//***********************************************************
{
BRect rect;

	if (!mFond) return;

/*
#define M_PUT_RECT(x,y,sx,sy) rect.Set(x,y,sx,sy); DrawBitmap(mFond, rect, rect);

	M_PUT_RECT(0,0,						532,5			);
	M_PUT_RECT(10,100+5,			532-20,10	);
	M_PUT_RECT(10,200+15,			532-20,10	);
	M_PUT_RECT(10,300+25,			532-20,5	);
	M_PUT_RECT(0,5,						5,100			);
	M_PUT_RECT(256+5,5,				10,100		);
	M_PUT_RECT(512+15,5,			5,100			);
	M_PUT_RECT(0,100+5,				10,200+25	);
	M_PUT_RECT(512+15,100+5,	10,200+25	);

	Sync();
*/
	rect.Set(0,0,532,330);
	DrawBitmap(mFond, rect, rect);

} // end of Draw for CFondFftView


//---------------------------------------------------------------------------


//***************************************************************************
CFftWin::CFftWin(BRect frame,
											 const char *title, 
											 window_type type,
											 ulong flags,
											 ulong workspace)
						 :BWindow(frame, title, type, flags, workspace)
//***************************************************************************
{
	mView[0] = NULL;
	mView[1] = NULL;
	mView[2] = NULL;
	mView[3] = NULL;
	mFftView[0] = NULL;
	mFftView[1] = NULL;
	mSoundView[0] = NULL;
	mSoundView[1] = NULL;

	mFond = NULL;

} // end of constructor for CFftWin


//***************************************************************************
CFftWin::~CFftWin(void)
//***************************************************************************
{
	if (debug) printf("CFftWin::~CFftWin -- destructor\n");
} // end of destructor for CFftWin


//---------------------------------------------------------------------------


//***************************************************************************
bool CFftWin::init(void)
//***************************************************************************
{
BRect rect;
long i;

	if (debug) printf("CFftWin::init\n");

	// add fond view
	rect = Bounds();
	mFond = new CFondFftView(rect, "fond", B_FOLLOW_NONE);
	if (!mFond)
	{
		if (debug) printf("failed to add fond view\n");
		return false;
	}
	AddChild(mFond);

	// add fft views
	rect.Set(5,5, 5+255, 5+99);
	rect.PrintToStream();
	//if (debug) printf("rect : w/h = %f/%f\n", rect.Width(), rect.Height());
	for(i=0; i<2; i++)
	{
		mFftView[i]=new CFftView(rect, "fft", B_FOLLOW_NONE, B_WILL_DRAW, i==1);
		mView[i]=mFftView[i];
		if (mFftView[i]) mFond->AddChild(mFftView[i]);
		rect.OffsetBy(256+10,0);
	}

	// add sound views
	rect.Set(10,15+100, 10+512, 15+199);
	for(i=0; i<2; i++)
	{
		mSoundView[i]=new CSoundView(rect, "raw", B_FOLLOW_NONE, B_WILL_DRAW, i==1);
		mView[2+i]=mSoundView[i];
		if (mSoundView[i]) mFond->AddChild(mSoundView[i]);
		rect.OffsetBy(0,100+10);
	}

	if (mFftView[0]) mFftView[0]->MakeFocus();

	//Run();
	Minimize(true);
	Show();
	return true;
} // end of init for CFftWin


//---------------------------------------------------------------------------


//***************************************************************************
void CFftWin::MessageReceived(BMessage *message)
//***************************************************************************
{
	//if (debug) printf("CFftWin::MessageReceived\n");

	switch(message->what)
	{
		case K_MSG_STREAM:
			if (mView[0]) mView[0]->Pulse();
			if (mView[1]) mView[1]->Pulse();
			if (mView[2]) mView[2]->Pulse();
			if (mView[3]) mView[3]->Pulse();
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}

} // end of MessageReceived for CFftWin


//***************************************************************************
bool CFftWin::QuitRequested(void)
//***************************************************************************
{
	if (debug) printf("CFftWin::QuitRequested\n");

	mView[0] = NULL;
	mView[1] = NULL;
	mView[2] = NULL;
	mView[3] = NULL;

	return false;

} // end of QuitRequested for CFftWin



//---------------------------------------------------------------------------


//***************************************************************************
void CFftWin::forceRedraw(void)
//***************************************************************************
{
	if (mView[0]) mView[0]->Pulse();
	if (mView[1]) mView[1]->Pulse();
	if (mView[2]) mView[2]->Pulse();
	if (mView[3]) mView[3]->Pulse();
} // end of forceRedraw for CFftWin





//---------------------------------------------------------------------------

// eoc
